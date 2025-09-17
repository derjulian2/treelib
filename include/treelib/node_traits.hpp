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
 */

#include <iterator>
#include <cstddef>

namespace trl
{

    namespace detail__ 
    {
        /**
         * @brief
         * accessor-trait for node_traits to be iterator-adaptor-compatible.
         */
        template <typename IterTp__>
        struct iter_util__
        {
            static typename IterTp__::base_ptr_T_ get_node_ptr_M_(const IterTp__& iter__)
            { return iter__.node_ptr_M_(); }

            static IterTp__ construct_from_ptr_M_(typename IterTp__::base_ptr_T_ ptr__)
            { return IterTp__(ptr__); }
        };

        template <typename IterTp__>
        struct iter_util__<std::reverse_iterator<IterTp__>>
        {
            static typename IterTp__::base_ptr_T_ get_node_ptr_M_(const std::reverse_iterator<IterTp__>& iter__)
            { return iter__.base().node_ptr_M_(); }

            static std::reverse_iterator<IterTp__> construct_from_ptr_M_(typename IterTp__::base_ptr_T_ ptr__)
            { return std::reverse_iterator<IterTp__>(IterTp__(ptr__)); }
        };

    }
    
    /**
     * @brief
     * provides (optionally exception-safe) information about a node's placement in a tree.
     */
    struct node_traits
    {

        template <typename IteratorType>
        static IteratorType
        parent(IteratorType iter) 
        {
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (ptr__->is_root_M_()) { throw std::logic_error("root-node cannot have a parent-node"); }
        #else
            assert(!ptr__->is_root_M_()); 
        #endif
            return detail__::iter_util__<IteratorType>::construct_from_ptr_M_(ptr__->parent_M_);
        }

        template <typename IteratorType>
        static IteratorType
        next(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_next_M_()) { throw std::logic_error("node does not have a next node"); }
        #else
            assert(ptr__->has_next_M_()); 
        #endif
            return detail__::iter_util__<IteratorType>::construct_from_ptr_M_(ptr__->next_M_);
        }

        template <typename IteratorType>
        static IteratorType
        previous(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_prev_M_()) { throw std::logic_error("node does not have a previous node"); }
        #else
            assert(ptr__->has_prev_M_()); 
        #endif
            return detail__::iter_util__<IteratorType>::construct_from_ptr_M_(ptr__->prev_M_);
        }

        template <typename IteratorType>
        static IteratorType
        first_child(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_children_M_()) { throw std::logic_error("node does not have any child-nodes"); }
        #else
            assert(ptr__->has_children_M_()); 
        #endif
            return detail__::iter_util__<IteratorType>::construct_from_ptr_M_(ptr__->first_child_M_);
        }

        template <typename IteratorType>
        static IteratorType
        last_child(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_children_M_()) { throw std::logic_error("node does not have any child-nodes"); }
        #else
            assert(ptr__->has_children_M_()); 
        #endif
            return detail__::iter_util__<IteratorType>::construct_from_ptr_M_(ptr__->last_child_M_);
        }

        template <typename IteratorType>
        static std::size_t 
        depth(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->depth_M_(); 
        }

        template <typename IteratorType>
        static std::size_t 
        child_count(IteratorType iter) 
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->child_count_M_; 
        }

        template <typename IteratorType>
        static bool 
        is_root(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->is_root_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_first_child(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->is_first_child_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_last_child(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->is_last_child_M_();
        }

        template <typename IteratorType>
        static bool 
        has_next(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->has_next_M_();
        }

        template <typename IteratorType>
        static bool 
        has_previous(IteratorType iter) 
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->has_prev_M_();
        }

        template <typename IteratorType>
        static bool 
        has_children(IteratorType iter) 
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
            return ptr__->has_children_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_only_child(IteratorType iter)
        { 
            auto ptr__ = detail__::iter_util__<IteratorType>::get_node_ptr_M_(iter);
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (ptr__->is_root_M_()) { throw std::logic_error("root-node cannot be an only-child"); }
        #else
            assert(!ptr__->is_root_M_());
        #endif
            return ptr__->is_only_child_M_(); 
        }

    };

}

#endif