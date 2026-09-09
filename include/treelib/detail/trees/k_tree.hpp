
#ifndef TREELIB_K_TREE_HPP
#define TREELIB_K_TREE_HPP

/***************************************************
 * @file   treelib/detail/trees/k_tree.hpp
 * @author Julian Benzel
 * @date   04.09.2026
 *
 * @brief   type-generic k-ary-tree.
 * @details compile-options:
 *          1.) #define TREELIB_NO_EXCEPTIONS
 *              disables exception-safety. asserts are
 *              still used in debug-mode.
 *          
 *          2.) #define TREELIB_K_NODE_NO_SHIFT
 *              disables shifting-behaviour of k-tree-nodes:
 *              if a node should hook another to an 
 *              already occupied spot, that node will
 *              still be inserted, and the 'replaced
 *              one' will be shifted down to the same
 *              hook-spot of the newly inserted node.
 *
 *              disabling this will throw 
 *              tl::modification_error instead.
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/tree.hpp>
#include <treelib/detail/base/forest.hpp>

#include <memory>
#include <array>
#include <cassert>
#include <ranges>

namespace tl
{
    namespace detail 
    {
        template <typename _NodeT, std::size_t K>
        struct _k_node_base
        {
            using _m_node_t      = _NodeT;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;
            using _m_hook_t = std::size_t;

            std::array<_m_node_ptr_t, K> _m_children_array;

        protected:

            friend _m_node_t;

            /***************************************************
             * constructor (1).
             * default-constructible,
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             ***************************************************/
            constexpr
            _k_node_base()
                : _m_children_array({nullptr})
            { }

        public:
            
            constexpr void
            _m_hook_at(_m_hook_t at, _m_node_ptr_t node)
            {
                if (at >= K)
                    throw modification_error("hook-index out-of-range");
            #ifdef TREELIB_K_NODE_NO_SHIFT
                #ifdef TREELIB_NO_EXCEPTIONS
                    assert(this->_m_children_array[at] != nullptr);
                #else
                    if (this->_m_children_array[at] != nullptr)
                    { throw modification_error("cannot insert at occupied hook"); }
                #endif
            #else
                if (this->_m_children_array[at] != nullptr) {
                    node->_m_children_array[at]->_m_hook_at(at, this->_m_children_array[at]);
                }
            #endif
                this->_m_children_array[at] = node;
            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t at)
                noexcept
            {
                if (at >= K)
                    throw modification_error("hook-index out-of-range");
                _m_node_ptr_t res = this->_m_children_array[at];
                this->_m_children_array[at] = nullptr;
                return res;
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t node)
                noexcept
            {
                for (_m_cnode_ptr_t& p : this->_m_children_array)
                    if (p == node)
                        { p = nullptr; break; }
            }

            constexpr auto
            _m_children()
                noexcept
            {
                return this->_m_children_array
                       | std::views::filter
                       ([](_m_node_ptr_t p) 
                       { return p != nullptr; });
            }

            constexpr auto
            _m_children()
                const noexcept
            {
                return this->_m_children_array
                       | std::views::filter
                       ([](_m_cnode_ptr_t p) 
                        { return p != nullptr; });
            }

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_node_ptr_t, _m_cnode_ptr_t>
            constexpr void
            _m_mimic(_m_cnode_ptr_t src, Fn&& insert_fn)
            {
                for (_m_hook_t at = 0; at < K; ++at)
                {
                    _m_cnode_ptr_t cur = src->_m_children_array[at];
                    if (cur != nullptr)
                    {
                        insert_fn(at, this, cur); // assume that .hook_at will be called
                        assert(this->_m_children_array[at] != nullptr);
                        this->_m_children_array[at]->_m_mimic(cur, std::forward<Fn>(insert_fn));
                    }
                }
            }
        };


        template <std::size_t K>
        struct _k_node
            : public _k_node_base<_k_node<K>, K>
        { 
            constexpr
            _k_node() = default;
        };

        template <std::size_t K>
        struct _bidirectional_k_node
            : public _bidirectional_node_base<_k_node_base<_bidirectional_k_node<K>, K>>
        { 
            using _m_base_t = _bidirectional_node_base<_k_node_base<_bidirectional_k_node<K>, K>>;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;

            constexpr
            _bidirectional_k_node() = default;

            constexpr bool
            _m_is_last()
                const noexcept
            {
                assert(!this->_m_is_root());
                return this->_m_get_parent()->_m_children_array.back() == this;
            }

            constexpr bool
            _m_is_first()
                const noexcept
            {
                assert(!this->_m_is_root());
                return this->_m_get_parent()->_m_children_array.front() == this;
            }

            constexpr _m_node_ptr_t
            _m_next_sibling()
                noexcept
            {
                if (this->_m_is_root() || this->_m_is_last())
                    return nullptr;
                // calculate offset of this node to get to it's position
                // in the child_array of the parent-node
                std::ptrdiff_t _off = this - this->_m_get_parent()->_m_child_array.cdata();
                return this->_m_get_parent()->_m_child_array[_off + 1];
            }

            constexpr _m_cnode_ptr_t
            _m_next_sibling()
                const noexcept
            {
                if (this->_m_is_root() || this->_m_is_last())
                    return nullptr;
                // calculate offset of this node to get to it's position
                // in the child_array of the parent-node
                std::ptrdiff_t _off = this - this->_m_get_parent()->_m_child_array.cdata();
                return this->_m_get_parent()->_m_child_array[_off + 1];
            }

            constexpr _m_node_ptr_t
            _m_prev_sibling()
                noexcept
            {
                if (this->_m_is_root() || this->_m_is_first())
                    return nullptr;
                // calculate offset of this node to get to it's position
                // in the child_array of the parent-node
                std::ptrdiff_t _off = this - this->_m_get_parent()->_m_child_array.cdata();
                return this->_m_get_parent()->_m_child_array[_off - 1];
            }

            constexpr _m_cnode_ptr_t
            _m_prev_sibling()
                const noexcept
            {
                if (this->_m_is_root() || this->_m_is_first())
                    return nullptr;
                // calculate offset of this node to get to it's position
                // in the child_array of the parent-node
                std::ptrdiff_t _off = this - this->_m_get_parent()->_m_child_array.cdata();
                return this->_m_get_parent()->_m_child_array[_off - 1];
            }
        };

    }

    
    template <typename T, 
              std::size_t K,
              typename Allocator = std::allocator<T>>
    using outward_k_tree 
        = detail::_root_outward_tree<detail::_k_node<K>, Allocator>;


    template <typename T, 
              std::size_t K,
              typename Allocator = std::allocator<T>>
    using k_tree 
        = detail::_root_outward_tree<detail::_bidirectional_k_node<K>, Allocator>;

}

#endif