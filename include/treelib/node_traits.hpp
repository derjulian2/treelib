#ifndef TRL_NODE_TRAITS_HPP
#define TRL_NODE_TRAITS_HPP

/**
 * @file    node_traits.hpp
 * @date    15.09.2025
 * @author  Julian Benzel
 * 
 * @brief
 * unified interface to retrieve information about a node's placement in a tree from an iterator.
 *
 * @details
 * compile-options:
 * - #define TRL_NODE_TRAITS_NOEXCEPT
 *   disables exception-safety for invalid operations on a tree.
 * - #define NDEBUG (defined in release-builds)
 *   disables debug-asserts for invalid operations on a tree.
 *
 * @note 
 * when using `std::reverse_iterator` on `trl::iterator`s, make sure to define
 * the corresponding TRL_<SOME_TREE>_STL_REVERSE_ITER macro.
 */

#include <iterator>
#include <cstddef>

#ifndef TRL_NODE_TRAITS_NOEXCEPT
    #define TRL_NOEXCEPT
#else
    #define TRL_NOEXCEPT noexcept
#endif

namespace trl
{

    namespace detail__ 
    {
        /**
         * @brief
         * accessor-trait for node_traits to be std::reverse-iterator-adaptor compatible.
         * when using std::reverse_iterator on trl::iterators, make sure to define
         * the corresponding TRL_<SOME_TREE>_STL_REVERSE_ITER macro.
         */
        template <typename IterTp__>
        struct iter_traits__
        {
            static typename IterTp__::base_ptr_T_ get_node_ptr_M_(const IterTp__& iter__)
            { return iter__.node_ptr_M_(); }

            static void set_node_ptr_M_(IterTp__& iter__, typename IterTp__::pointer p__)
            { iter__->ptr_M_ = p__; }

            static IterTp__ construct_from_ptr_M_(typename IterTp__::base_ptr_T_ ptr__)
            { return IterTp__(ptr__); }
        };

        template <typename IterTp__>
        struct iter_traits__<std::reverse_iterator<IterTp__>>
        {
            /*
             * this additional increment is necessary to get the correct node from the reverse-iterator,
             * because of how std::reverse_iterator is implemented. it actually holds an iterator from
             * before (or after?) the node that is actually being referenced because of something with
             * 'there is no guarantee of a valid memory-address before the start of an array'. 
             * so the STL-designers probably had a valid reason to do so,
             * however in this context, it will massively slow down reverse-iteration because every dereference,
             * the iteration-algorithm is called again.
             *
             * maybe just ditch std::reverse_iterator entirely as it just clutters everything.
             *
             * so i can either break standard-compliance with std::reverse_iterator, or take the performance hit
             * (although just using `for (iter_t i = --tree.end(); i != tree.end(); --i)` achieves the same result without
             *  any performance-tradeoffs.)
             */
            static typename IterTp__::base_ptr_T_ get_node_ptr_M_(const std::reverse_iterator<IterTp__>& iter__)
            {
                std::reverse_iterator<IterTp__> riter__ = iter__;
                ++riter__;
                return riter__.base().node_ptr_M_(); 
            }

            static void set_node_ptr_M_(IterTp__& iter__, typename IterTp__::pointer p__)
            { iter__.base()->ptr_M_ = p__; }

            static std::reverse_iterator<IterTp__> construct_from_ptr_M_(typename IterTp__::base_ptr_T_ ptr__)
            { return std::reverse_iterator<IterTp__>(IterTp__(ptr__)); }
        };

    }
    
    /**
     * @brief
     * iterator-adaptor to iterate through the child-nodes of a specific tree-node, essentially
     * defining `[first_child, parent)` as a subrange in the tree.
     *
     * this template is to be used with `trl::node_traits::lbegin()`/`lend()` 
     * as in leaf-begin and leaf-end that define the bounds of the range.
     */
    template <typename IteratorType>
    struct leaf_iterator
        : public IteratorType
    {
        using base_type = IteratorType;
        using iterator_category = base_type::iterator_category;
        using value_type = base_type::value_type;
        using difference_type = base_type::difference_type;
        using pointer = base_type::pointer;
        using reference = base_type::reference;

        base_type& 
        base()
        { return static_cast<base_type&>(*this); }

        const base_type& 
        base() const
        { return static_cast<const base_type&>(*this); }

        leaf_iterator& 
        operator++()
        { this->ptr_M_ = this->ptr_M_->is_last_child_M_() ? this->ptr_M_->parent_M_ : this->ptr_M_->next_M_; return *this; }

        leaf_iterator 
        operator++(int)
        { leaf_iterator old{*this}; ++(*this); return old; }

        leaf_iterator& 
        operator--()
        { this->ptr_M_ = this->ptr_M_->is_first_child_M_() ? this->ptr_M_->parent_M_ : this->ptr_M_->prev_M_; return *this; }

        leaf_iterator 
        operator--(int)
        { leaf_iterator old{*this}; --(*this); return old; }

    };

    /**
     * @brief
     * provides (optionally exception-safe) information about a node's placement in a tree.
     */
    struct node_traits
    {

        template <typename IteratorType>
        static IteratorType
        parent(IteratorType iter) TRL_NOEXCEPT
        {
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (ptr__->is_root_M_()) { throw std::logic_error("root-node cannot have a parent-node"); }
        #else
            assert(!ptr__->is_root_M_()); 
        #endif
            return detail__::iter_traits__<IteratorType>::construct_from_ptr_M_(ptr__->parent_M_);
        }

        template <typename IteratorType>
        static IteratorType
        next(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_next_M_()) { throw std::logic_error("node does not have a next node"); }
        #else
            assert(ptr__->has_next_M_()); 
        #endif
            return detail__::iter_traits__<IteratorType>::construct_from_ptr_M_(ptr__->next_M_);
        }

        template <typename IteratorType>
        static IteratorType
        previous(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_prev_M_()) { throw std::logic_error("node does not have a previous node"); }
        #else
            assert(ptr__->has_prev_M_()); 
        #endif
            return detail__::iter_traits__<IteratorType>::construct_from_ptr_M_(ptr__->prev_M_);
        }

        template <typename IteratorType>
        static IteratorType
        first_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_children_M_()) { throw std::logic_error("node does not have any child-nodes"); }
        #else
            assert(ptr__->has_children_M_()); 
        #endif
            return detail__::iter_traits__<IteratorType>::construct_from_ptr_M_(ptr__->first_child_M_);
        }

        template <typename IteratorType>
        static IteratorType
        last_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_children_M_()) { throw std::logic_error("node does not have any child-nodes"); }
        #else
            assert(ptr__->has_children_M_()); 
        #endif
            return detail__::iter_traits__<IteratorType>::construct_from_ptr_M_(ptr__->last_child_M_);
        }

        template <typename IteratorType>
        static std::size_t 
        depth(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->depth_M_(); 
        }

        template <typename IteratorType>
        static std::size_t 
        child_count(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->child_count_M_; 
        }

        template <typename IteratorType>
        static bool 
        is_root(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->is_root_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_first_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->is_first_child_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_last_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->is_last_child_M_();
        }

        template <typename IteratorType>
        static bool 
        has_next(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->has_next_M_();
        }

        template <typename IteratorType>
        static bool 
        has_previous(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->has_prev_M_();
        }

        template <typename IteratorType>
        static bool 
        has_children(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->has_children_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_only_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = detail__::iter_traits__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (ptr__->is_root_M_()) { throw std::logic_error("root-node cannot be an only-child"); }
        #else
            assert(!ptr__->is_root_M_());
        #endif
            return ptr__->is_only_child_M_(); 
        }

        template <typename IteratorType>
        static leaf_iterator<IteratorType>
        lbegin(IteratorType iter) TRL_NOEXCEPT
        { return leaf_iterator<IteratorType>(first_child(iter)); }

        template <typename IteratorType>
        static leaf_iterator<IteratorType>
        lend(IteratorType iter) TRL_NOEXCEPT
        { return leaf_iterator<IteratorType>(iter); }

    };

}

#endif