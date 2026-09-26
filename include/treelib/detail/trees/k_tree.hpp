
#ifndef TREELIB_K_TREE_HPP
#define TREELIB_K_TREE_HPP

/***************************************************
 * @file   treelib/detail/trees/k_tree.hpp
 * @author Julian Benzel
 * @date   25.09.2026
 *
 * @brief   type-generic k-ary-tree.
 * @details compile-options:
 *          - #define _treelib_k_tree_no_shift
 *            throws tl::modification_error if
 *            insertion is requested at an already
 *            occupied spot instead of shifting 
 *            the nodes downwards.
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/bits/initializer_tree.hpp>

#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/tree.hpp>

#include <memory>
#include <array>
#include <cassert>
#include <ranges>

namespace tl
{
    namespace _detail 
    {
        /***************************************************
         * @brief CRTP-base-class for tree-nodes with
         *        at most K children.
         ***************************************************/
        template <typename NodeT, std::size_t K>
        struct _k_node_base
        {
            using _m_node_t      = NodeT;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;
            using _m_hook_t = std::size_t;

            std::array<_m_node_ptr_t, K> _m_children_array;

            friend _m_node_t;

            static constexpr std::size_t _s_arity = K;

            constexpr _m_node_ptr_t
            _m_node_ptr()
                noexcept
            { return static_cast<_m_node_ptr_t>(this); }

            constexpr _m_cnode_ptr_t
            _m_node_ptr()
                const noexcept
            { return static_cast<_m_cnode_ptr_t>(this); }

            constexpr
            _k_node_base()
                : _m_children_array({nullptr})
            { }

            constexpr bool
            _m_is_leaf()
                const noexcept
            { 
                return std::all_of(this->_m_children_array.cbegin(),
                                   this->_m_children_array.cend(),
                                   [](_m_cnode_ptr_t _p) { return _p == nullptr; });
            }

            constexpr void
            _m_hook_at(_m_hook_t at, _m_node_ptr_t node)
            {
                if (at >= K)
                    throw modification_error("hook-index out-of-range");
            #ifdef _treelib_k_tree_no_shift
                #ifdef _treelib_no_exceptions
                    assert(this->_m_children_array[at] == nullptr);
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
                for (_m_node_ptr_t& p : this->_m_children_array)
                    if (p == node)
                        { p = nullptr; break; }
            }

            constexpr decltype(auto)
            _m_children()
                const noexcept
            {
                return this->_m_children_array
                       | std::views::filter([](_m_cnode_ptr_t p) 
                                            { return p != nullptr; });
            }

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_node_ptr_t, _m_cnode_ptr_t>
            constexpr void
            _m_mimic(_m_cnode_ptr_t _src, Fn&& _insert_fn)
            {
                for (_m_hook_t _at = 0; _at < K; ++_at)
                {
                    _m_cnode_ptr_t cur = _src->_m_children_array[_at];
                    if (cur != nullptr)
                    {
                        _insert_fn(_at, this->_m_node_ptr(), cur); // assume that .hook_at will be called
                        assert(this->_m_children_array[_at] != nullptr);
                        this->_m_children_array[_at]->_m_mimic(cur, std::forward<Fn>(_insert_fn));
                    }
                }
            }

            template <typename InitT, typename FnT>
                requires _initializer_node_interface<InitT>
                         && std::invocable<FnT, _m_hook_t, _m_node_ptr_t, const InitT&>
            constexpr void 
            _m_mimic_initializer(const InitT& _init, FnT&& _insert_fn)
            {
                auto _it = _init._m_children().begin();
                for (_m_hook_t _at = 0; 
                     _at < K && _it != _init._m_children().end(); 
                     ++_it, ++_at)
                {
                    _insert_fn(_at, this->_m_node_ptr(), *_it);
                    assert(this->_m_children_array[_at] != nullptr);
                    this->_m_children_array[_at]->_m_mimic_initializer(*_it, std::forward<FnT>(_insert_fn));
                }
            }
        };

        /**********************************************
         * @brief forward-declarations for the
         *        actual node-types and
         *        aliases to abbreviate the
         *        CRTP/mixin-base-classes and make
         *        them a bit more readable.
         **********************************************/

        template <std::size_t>
        struct _outward_k_node;

        template <std::size_t K>
        struct _k_node;

        template <std::size_t K>
        using _outward_k_node_base
    #ifdef _treelib_store_depth
            = _depth_node<_k_node_base<_k_node<K>, K>>;
    #else
            = _k_node_base<_outward_k_node<K>, K>;
    #endif

        template <std::size_t K>
        using _bidirectional_k_node_base
    #ifdef _treelib_store_depth
            = _depth_node<_bidirectional_node<_k_node_base<_bidirectional_k_node<K>, K>>>
    #else
            = _bidirectional_node<_k_node_base<_k_node<K>, K>>;
    #endif

        /**********************************************
         * @brief outward-k-tree and k-tree node-types.
         **********************************************/

        template <std::size_t K>
        struct _outward_k_node
            : public _outward_k_node_base<K>
        { 
            using _outward_k_node_base<K>::_outward_k_node_base; 

            friend _node_traits<_outward_k_node>;
        };

        template <std::size_t K>
        struct _k_node
            : public _bidirectional_k_node_base<K>
        {
            using _m_base_t = _bidirectional_k_node_base<K>;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;

            using _m_base_t::_m_base_t;

            friend _node_traits<_k_node>;

            constexpr bool
            _m_is_root()
                const noexcept
            { return this->_m_parent() == nullptr; }

            constexpr bool
            _m_is_last()
                const noexcept
            { 
                assert(!this->_m_is_root()); 
                return this->_m_parent()->_m_children_array.back() == this; 
            }

            constexpr bool
            _m_is_first()
                const noexcept
            { 
                assert(!this->_m_is_root());
                return this->_m_parent()->_m_children_array.front() == this; 
            }

            constexpr _m_node_ptr_t
            _m_next_sibling()
                const noexcept
            {
                if (this->_m_is_root() || this->_m_is_last())
                    return nullptr;
                return *(std::find(this->_m_parent()->_m_children_array.begin(),
                                   this->_m_parent()->_m_children_array.end(),
                                   this) + 1);
            }

            constexpr _m_node_ptr_t
            _m_prev_sibling()
                const noexcept
            {
                if (this->_m_is_root() || this->_m_is_first())
                    return nullptr;
                return *(std::find(this->_m_parent()->_m_children_array.begin(),
                                   this->_m_parent()->_m_children_array.end(),
                                   this) - 1);
            }
        };

    }

    /******************************************************************
     * @brief   type-generic k-ary-tree container with
     *          constant-time insertion (neglecting optional
     *          depth/height updates) and linear-time erasure.
     *
     *          differs from @see tl::k_tree in that the nodes
     *          have no back-pointers to their parent-node.
     *
     * @tparam  T         the element-type.
     * @tparam  K         the maximum number of child-nodes.
     * @tparam  Allocator the allocator-type. 
     *                    the default is std::allocator<T>.
     *
     * @details implemented as a collection of nodes each
     *          with K node-pointers to their child-nodes
     *          stored in an array. the tree originates
     *          from a single value-holding root-node. each
     *          node takes ownership of it's child-nodes, 
     *          which means removing one node also means removing
     *          each child-node (hence linear-time-erasure).
     *
     *          like 'std::list', trees may be spliced to
     *          perform insertion of already existing 
     *          subtrees in constant-time (again, neglecting
     *          optional bookkeeping-variables).
     *
     *          there are several options iterating/traversing
     *          the tree. iterator-types are divided into:
     *          1.) 'traversal-strategy'
     *          a tree is a non-linear data-structure and can
     *          be traversed in different ways (a good resource
     *          can be found at https://en.wikipedia.org/wiki/Tree_traversal).
     *          the strategies that can be chosen from are:
     *          - depth-first-pre-order [optionally reversed]
     *            which visits the node first and then
     *            all of it's children.
     *          - depth-first-in-order [optionally reversed]
     *            which visits the first half of a node's children
     *            first, then the node itself, and then the
     *            second half of the children.
     *          - depth-first-post-order [optionally reversed]
     *            which visits all of a node's children before
     *            visiting the node itself.
     *          - breadth-first/level-order [optionally reversed]
     *            which visits all nodes of the tree
     *            level-by-level. breadth-first is only supported
     *            for queued-iterators.
     *
     *          2.) 'traversal-approach'
     *          which for outward-trees is only:
     *          - queued; @see outward_k_tree::queued_iterator<...>
     *            these types of iterators form a queue of nodes
     *            while iterating over the tree, which is just
     *            an instance of 'std::list<NodeType>' which gets
     *            gradually expanded as the iteration goes on.
     *              
     *            the queue will only get formed once and can then
     *            be cheaply traversed forwards and backwards,
     *            however queued-iterators have a bigger memory-
     *            footprint and dynamic-allocation requirements.
     *          traversing iterators are not supported for
     *          outward-trees as there is not always a pointer-path
     *          to follow to the target-node.
     ******************************************************************/
    template <typename T, 
              std::size_t K,
              typename Allocator = std::allocator<T>>
    using outward_k_tree 
        = _detail::_root_outward_tree<_detail::_outward_k_node<K>, Allocator>;

    /******************************************************************
     * @brief   type-generic k-ary-tree container with
     *          constant-time insertion (neglecting optional
     *          depth/height updates) and linear-time erasure.
     *
     *          differs from @see tl::outward_k_tree in that the nodes
     *          have back-pointers to their parent-node, which allows
     *          for some additional capabilities.
     *
     * @tparam  T         the element-type.
     * @tparam  K         the maximum number of child-nodes.
     * @tparam  Allocator the allocator-type. 
     *                    the default is std::allocator<T>.
     *
     * @details implemented as a collection of nodes each
     *          with K node-pointers to their child-nodes
     *          stored in an array. the tree originates
     *          from a single value-holding root-node. each
     *          node takes ownership of it's child-nodes, 
     *          which means removing one node also means removing
     *          each child-node (hence linear-time-erasure).
     *
     *          like 'std::list', trees may be spliced to
     *          perform insertion of already existing 
     *          subtrees in constant-time (again, neglecting
     *          optional bookkeeping-variables).
     *
     *          there are several options iterating/traversing
     *          the tree. iterator-types are divided into:
     *          1.) 'traversal-strategy'
     *          a tree is a non-linear data-structure and can
     *          be traversed in different ways (a good resource
     *          can be found at https://en.wikipedia.org/wiki/Tree_traversal).
     *          the strategies that can be chosen from are:
     *          - depth-first-pre-order [optionally reversed]
     *            which visits the node first and then
     *            all of it's children.
     *          - depth-first-in-order [optionally reversed]
     *            which visits the first half of a node's children
     *            first, then the node itself, and then the
     *            second half of the children.
     *          - depth-first-post-order [optionally reversed]
     *            which visits all of a node's children before
     *            visiting the node itself.
     *          - breadth-first/level-order [optionally reversed]
     *            which visits all nodes of the tree
     *            level-by-level. breadth-first is only supported
     *            for queued-iterators.
     *
     *          2.) 'traversal-approach'
     *          which is one of:
     *          - queued; @see k_tree::queued_iterator<...>
     *            these types of iterators form a queue of nodes
     *            while iterating over the tree, which is just
     *            an instance of 'std::list<NodeType>' which gets
     *            gradually expanded as the iteration goes on.
     *              
     *            the queue will only get formed once and can then
     *            be cheaply traversed forwards and backwards,
     *            however queued-iterators have a bigger memory-
     *            footprint and dynamic-allocation requirements.
     *
     *          - traversing; @see k_tree::traversing_iterator<...>
     *            these types of iterators are the 'slim'-iterators
     *            as they only hold a pointer to the current-node
     *            (and, depending on the chosen traversal-strategy,
     *             some additional state-variables).
     ******************************************************************/
    template <typename T, 
              std::size_t K,
              typename Allocator = std::allocator<T>>
    using k_tree 
        = _detail::_root_tree<_detail::_k_node<K>, Allocator>;
}

#endif