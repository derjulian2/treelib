
#ifndef TREELIB_ROSE_TREE_HPP
#define TREELIB_ROSE_TREE_HPP

/***************************************************
 * @file   treelib/detail/trees/rose_tree.hpp
 * @author Julian Benzel
 * @date   25.09.2026
 *
 * @brief  type-generic trees without any constraints
 *         on the number of children per node.
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/bits/initializer_tree.hpp>

#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/tree.hpp>

#include <vector>
#include <variant>
#include <cstdint>
#include <algorithm>

namespace tl
{
    /***************************************************
     * @brief options for inserting a rose-tree-node
     *        vertically (as first/last child).
     ***************************************************/
    enum struct vrose
        : std::uint8_t
    {
        first,
        last
    };

    /***************************************************
     * @brief options for inserting a rose-tree-node
     *        horizontally (as next/previous sibling).
     ***************************************************/
    enum struct hrose
        : std::uint8_t
    {
        next,
        prev
    };

    namespace _detail
    {
        /***************************************************
         * @brief visitor-helper class for easy std::visit.
         ***************************************************/
        template <typename... Fns>
        struct _visitor : public Fns...
        { using Fns::operator()...; };

        /*************************************************************
         * @brief   CRTP-base-class for rosetree-nodes that hold
         *          pointers to their child-nodes in 
         *          an instance of std::vector.
         *
         * @details this node-type will be a 'dynamic-node'
         *          as it requires additional dynamic
         *          memory to function. 
         *          @see tl::_detail::_dynamic_tree_node for more.
         *************************************************************/
        template <typename NodeT,
                  typename VecAllocT>
        struct _vecrose_node_base
        {   
            using _m_node_t      = NodeT;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;

            using _m_alloc_t = VecAllocT;
            using _m_alloc_traits_t = std::allocator_traits<_m_alloc_t>;
            using _m_childvec_alloc_t
                = typename _m_alloc_traits_t::template rebind_alloc<_m_node_ptr_t>;
            using _m_childvec_alloc_traits_t
                = std::allocator_traits<_m_childvec_alloc_t>;

            // either an index, or first/last
            using _m_hook_t = std::variant<std::size_t, vrose>;

            std::vector<_m_node_ptr_t, _m_childvec_alloc_t> _m_child_vec;

            _vecrose_node_base(const _m_alloc_t& _alloc)
                : _m_child_vec(_alloc)
            { }

            constexpr _m_node_ptr_t
            _m_node_ptr()
                noexcept
            { return static_cast<_m_node_ptr_t>(this); }

            constexpr _m_cnode_ptr_t
            _m_node_ptr()
                const noexcept
            { return static_cast<_m_cnode_ptr_t>(this); }

            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
            {
                std::visit(_visitor
                {
                    [&](const std::size_t& _idx) 
                        -> void
                    { this->_m_child_vec.insert(this->_m_child_vec.cbegin() + _idx, _node); },
                    [&](const vrose& _v)
                        -> void 
                    { 
                        switch (_v)
                        {
                        case (vrose::first):
                            this->_m_child_vec.insert(this->_m_child_vec.cbegin(), _node);
                            break;
                        case (vrose::last):
                            this->_m_child_vec.push_back(_node);
                            break;
                        }
                    }
                }, _at);
            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t _at)
                noexcept
            {
                return 
                std::visit(_visitor
                {
                    [&](const std::size_t& _idx) 
                        -> _m_node_ptr_t
                    { 
                        _m_node_ptr_t _res = this->_m_child_vec[_idx];
                        this->_m_child_vec.erase(this->_m_child_vec.cbegin() + _idx);
                        return _res; 
                    },
                    [&](const vrose& _v)
                        -> _m_node_ptr_t 
                    { 
                        _m_node_ptr_t _res;
                        switch (_v)
                        {
                        case (vrose::first):
                            _res = this->_m_child_vec.front();
                            this->_m_child_vec.erase(this->_m_child_vec.cbegin());
                            break;
                        case (vrose::last):
                            _res = this->_m_child_vec.back();
                            this->_m_child_vec.pop_back();
                            break;
                        }
                        return _res;
                    }
                }, _at);
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t _node)
                noexcept
            {
                std::remove(this->_m_child_vec.begin(), this->_m_child_vec.end(), _node);
            }

            constexpr std::vector<_m_node_ptr_t>&
            _m_children()
                noexcept
            { return _m_child_vec; }

            constexpr const std::vector<_m_node_ptr_t>&
            _m_children()
                const noexcept
            { return _m_child_vec; }

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_node_ptr_t, _m_cnode_ptr_t>
            constexpr void
            _m_mimic(_m_cnode_ptr_t _src, Fn&& _insert_fn)
            {
                for (_m_cnode_ptr_t _child
                     : _src->_m_children())
                {
                    _insert_fn(vrose::last, this->_m_node_ptr(), _child);
                    this->_m_child_vec.back()->_m_mimic(_child, std::forward<Fn>(_insert_fn));
                }
            }

            template <typename InitT, typename FnT>
                requires _initializer_node_interface<InitT>
                         && std::invocable<FnT, _m_hook_t, _m_node_ptr_t, const InitT&>
            constexpr void 
            _m_mimic_initializer(const InitT& _init, FnT&& _insert_fn)
            {
                for (const InitT& _child
                     : _init._m_children())
                {
                    _insert_fn(vrose::last, this->_m_node_ptr(), _child);
                    this->_m_child_vec.back()->_m_mimic_initializer(_child, std::forward<FnT>(_insert_fn));
                }
            }
        };

        /*************************************************************
         * @brief   CRTP-base-class for rosetree-nodes that hold
         *          pointers to their first/last child-nodes and
         *          next/previous sibling-nodes, resembling 
         *          a doubly-linked-list with vertical pointers.
         *************************************************************/
        template <typename NodeT>
        struct _listrose_node_base
        {   
            using _m_node_t      = NodeT;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;

            // either first/last or next/prev
            using _m_hook_t = std::variant<vrose, hrose>;
        
            _m_node_ptr_t _m_next;
            _m_node_ptr_t _m_prev;

            _m_node_ptr_t _m_first;
            _m_node_ptr_t _m_last;

            _listrose_node_base()
                : _m_next(nullptr)
                , _m_prev(nullptr)
                , _m_first(nullptr)
                , _m_last(nullptr)
            { }
            
            constexpr bool
            _m_is_leaf()
                const noexcept
            { return this->_m_first == nullptr; }

            constexpr void
            _m_hook_at(_m_hook_t at, _m_node_ptr_t node)
            {

            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t at)
                noexcept
            {
                return nullptr;
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t node)
                noexcept
            {

            }

            constexpr auto
            _m_children()
                noexcept
            {
                return std::vector<_m_node_ptr_t>{};
            }

            constexpr auto
            _m_children()
                const noexcept
            {
                return std::vector<_m_node_ptr_t>{};
            }

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_node_ptr_t, _m_cnode_ptr_t>
            constexpr void
            _m_mimic(_m_cnode_ptr_t _src, Fn&& _insert_fn)
            {

            }
        };

        /**********************************************
         * @brief forward-declarations for the
         *        actual node-types and
         *        aliases to abbreviate the
         *        CRTP/mixin-base-classes and make
         *        them a bit more readable.
         **********************************************/

        template <typename VecAllocT>
        struct _outward_vecrose_node;

        template <typename VecAllocT>
        struct _vecrose_node;

        template <typename VecAllocT>
        using _outward_vecrose_base
            = _vecrose_node_base<_outward_vecrose_node<VecAllocT>, VecAllocT>;

        template <typename VecAllocT>
        using _vecrose_base
            = _bidirectional_node<_vecrose_node_base<_vecrose_node<VecAllocT>, VecAllocT>>;

        template <typename VecAllocT>
        struct _outward_vecrose_node
            : public _outward_vecrose_base<VecAllocT>
        { 
            using _m_base_t = _outward_vecrose_base<VecAllocT>;
            using _m_base_t::_m_base_t;
        };

        template <typename VecAllocT>
        struct _vecrose_node
            : public _vecrose_base<VecAllocT>
        { 
            using _m_base_t = _vecrose_base<VecAllocT>;
            using _m_base_t::_m_base_t;
        };

        struct _outward_listrose_node
            : public _listrose_node_base<_outward_listrose_node>
        { };

        struct _listrose_node
            : public _bidirectional_node<_listrose_node_base<_listrose_node>>
        { 
            using _m_base_t = _bidirectional_node<_listrose_node_base<_listrose_node>>;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;

            constexpr bool
            _m_is_last()
                const noexcept
            {
                return false;
            }

            constexpr bool
            _m_is_first()
                const noexcept
            {
                return false;
            }

            constexpr _m_node_ptr_t
            _m_next_sibling()
                noexcept
            {
                return nullptr;
            }

            constexpr _m_cnode_ptr_t
            _m_next_sibling()
                const noexcept
            {
                return nullptr;
            }

            constexpr _m_node_ptr_t
            _m_prev_sibling()
                noexcept
            {
                return nullptr;
            }

            constexpr _m_cnode_ptr_t
            _m_prev_sibling()
                const noexcept
            {
                return nullptr;
            }
        };
    }

    template <typename T, 
              typename Allocator = std::allocator<T>>
    using outward_vecrose_tree
        = _detail::_header_outward_tree<_detail::_outward_vecrose_node<Allocator>, Allocator>;

    /******************************************************************
     * @brief   type-generic rose-tree container with
     *          constant-time insertion (neglecting optional
     *          depth/height updates) and linear-time erasure.
     *          allows node-insertion between any two child-nodes
     *          and at the front/back of the child-node-chain.
     *
     *          differs from @see tl::outward_vecrose_tree in that the nodes
     *          have back-pointers to their parent-node, which allows
     *          for some additional capabilities.
     *
     * @tparam  T         the element-type.
     * @tparam  Allocator the allocator-type. 
     *                    the default is std::allocator<T>.
     *
     * @details implemented as a collection of nodes each
     *          holding a vector of node-pointers to their
     *          child-nodes. the tree originates from a single
     *          valueless header-node, meaning the root-node is
     *          not a valid point in the [begin, end)-range.
     *          this is a design-choice to allow for an arbitrary 
     *          amount of nodes in the first 'layer' of the tree.
     *          
     *          each node takes ownership of it's child-nodes, 
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
     *          - queued; @see vecrose_tree::queued_iterator<...>
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
     *          - traversing; @see vecrose_tree::traversing_iterator<...>
     *            these types of iterators are the 'slim'-iterators
     *            as they only hold a pointer to the current-node
     *            (and, depending on the chosen traversal-strategy,
     *             some additional state-variables).
     ******************************************************************/
    template <typename T, 
              typename Allocator = std::allocator<T>>
    using vecrose_tree
        = _detail::_header_tree<_detail::_vecrose_node<Allocator>, Allocator>;

    template <typename T, 
              typename Allocator = std::allocator<T>>
    using outward_listrose_tree
        = _detail::_header_outward_tree<_detail::_outward_listrose_node, Allocator>;

    /******************************************************************
     * @brief   type-generic rose-tree container with
     *          constant-time insertion (neglecting optional
     *          depth/height updates) and linear-time erasure.
     *          allows node-insertion at the front/back
     *          of a child-node-chain and as next/previous sibling. 
     *
     *          differs from @see tl::outward_listrose_tree in that the
     *          nodes have back-pointers to their parent-node, which allows
     *          for some additional capabilities.
     *
     * @tparam  T         the element-type.
     * @tparam  Allocator the allocator-type. 
     *                    the default is std::allocator<T>.
     *
     * @details implemented as a collection of nodes each
     *          holding first/last and next/prev node-pointers to
     *          their child- and sibling-nodes. 
     *
     *          the tree originates from a single valueless
     *          header-node, meaning the root-node is
     *          not a valid point in the [begin, end)-range.
     *          this is a design-choice to allow for an arbitrary 
     *          amount of nodes in the first 'layer' of the tree, 
     *          as well as to prevent node-leaks when inserting
     *          a next/prev-sibling into that first layer. 
     *          
     *          each node takes ownership of it's child-nodes, 
     *          which means removing one node also means removing
     *          each child-node (hence linear-time-erasure).
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
     *          - queued; @see listrose_tree::queued_iterator<...>
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
     *          - traversing; @see listrose_tree::traversing_iterator<...>
     *            these types of iterators are the 'slim'-iterators
     *            as they only hold a pointer to the current-node
     *            (and, depending on the chosen traversal-strategy,
     *             some additional state-variables).
     ******************************************************************/
    template <typename T, 
              typename Allocator = std::allocator<T>>
    using listrose_tree
        = _detail::_header_tree<_detail::_listrose_node, Allocator>;



    template <typename T,
              typename Allocator = std::allocator<T>>
    using outward_rose_tree = outward_vecrose_tree<T, Allocator>;

    template <typename T,
              typename Allocator = std::allocator<T>>
    using rose_tree = vecrose_tree<T, Allocator>;
}


#endif