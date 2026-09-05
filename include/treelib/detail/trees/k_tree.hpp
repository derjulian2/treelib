
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
        struct _K_Node_Base
        {
            using _M_node_t      = _NodeT;
            using _M_node_ptr_t  = _M_node_t*;
            using _M_cnode_ptr_t = const _M_node_t*;
            using _M_hook_t = std::size_t;

            std::array<_M_node_ptr_t, K> _M_children_array;

        protected:

            friend _M_node_t;

            /***************************************************
             * constructor (1).
             * default-constructible,
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             ***************************************************/
            _K_Node_Base()
                : _M_children_array({nullptr})
            { }

        public:
            
            constexpr void
            _M_hook_at(_M_hook_t at, _M_node_ptr_t node)
            {
                if (at >= K)
                    throw modification_error("hook-index out-of-range");
            #ifdef TREELIB_K_NODE_NO_SHIFT
                #ifdef TREELIB_NO_EXCEPTIONS
                    assert(this->_M_children_array[at] != nullptr);
                #else
                    if (this->_M_children_array[at] != nullptr)
                    { throw modification_error("cannot insert at occupied hook"); }
                #endif
            #else
                if (this->_M_children_array[at] != nullptr) {
                    node->_M_children_array[at]->_M_hook_at(at, this->_M_children_array[at]);
                }
            #endif
                this->_M_children_array[at] = node;
            }

            constexpr _M_node_ptr_t
            _M_unhook_at(_M_hook_t at)
                noexcept
            {
                if (at >= K)
                    throw modification_error("hook-index out-of-range");
                _M_node_ptr_t res = this->_M_children_array[at];
                this->_M_children_array[at] = nullptr;
                return res;
            }

            constexpr void
            _M_unhook_if(_M_node_ptr_t node)
                noexcept
            {
                for (_M_cnode_ptr_t& p : this->_M_children_array)
                    if (p == node)
                        { p = nullptr; break; }
            }

            constexpr auto
            _M_children()
                noexcept
            {
                return this->_M_children_array
                       | std::views::filter
                       ([](_M_node_ptr_t p) 
                       { return p != nullptr; });
            }

            constexpr auto
            _M_children()
                const noexcept
            {
                return this->_M_children_array
                       | std::views::filter
                       ([](_M_cnode_ptr_t p) 
                        { return p != nullptr; });
            }

            template <typename Fn>
                requires std::invocable<Fn, _M_hook_t, _M_node_ptr_t, _M_cnode_ptr_t>
            constexpr void
            _M_mimic(_M_cnode_ptr_t src, Fn&& insert_fn)
            {
                for (_M_hook_t at = 0; at < K; ++at)
                {
                    _M_cnode_ptr_t cur = src->_M_children_array[at];
                    if (cur != nullptr)
                    {
                        insert_fn(at, this, cur); // assume that .hook_at will be called
                        assert(this->_M_children_array[at] != nullptr);
                        this->_M_children_array[at]->_M_mimic(cur, std::forward<Fn>(insert_fn));
                    }
                }
            }
        };


        template <std::size_t K>
        struct _K_Node
            : public _K_Node_Base<_K_Node<K>, K>
        { 
            _K_Node() = default;
        };

        template <std::size_t K>
        struct _Parent_K_Node
            : public _Parent_Node_Base<_K_Node_Base<_Parent_K_Node<K>, K>>
        { 
            _Parent_K_Node() = default;
        };

    }

    
    template <typename T, 
              std::size_t K,
              typename Allocator = std::allocator<T>>
    class outward_k_tree
        : public detail::_Outward_Tree_Base<detail::_K_Node<K>, Allocator>
    { 
    protected:
        using _M_base_t = detail::_Outward_Tree_Base<detail::_K_Node<K>, Allocator>;

    public:
        
        using _M_base_t::_M_base_t;
    };


    template <typename T, 
              std::size_t K,
              typename Allocator = std::allocator<T>>
    class k_tree
        : public detail::_Tree_Base<detail::_Parent_K_Node<K>, Allocator>
    {
    protected:
        using _M_base_t = detail::_Tree_Base<detail::_Parent_K_Node<K>, Allocator>;

    public:
        
        using _M_base_t::_M_base_t;
    };

}

#endif