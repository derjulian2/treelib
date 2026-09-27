
#ifndef TREELIB_BASE_ITERATOR_HPP
#define TREELIB_BASE_ITERATOR_HPP

/***************************************************
 * @file   treelib/detail/base/iterator.hpp
 * @author Julian Benzel
 * @date   25.09.2026
 *
 * @brief  classes to enable various methods of
 *         tree-traversal (depth-first/breadth-first)
 *         between instances of a node-type.
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/base/node.hpp>

#include <memory>
#include <iterator>
#include <list>
#include <algorithm>
#include <cassert>

namespace tl
{
    /***************************************************
     * @brief values for traversal-strategy selection.
     ***************************************************/
    enum struct traversal
    {
        depth_first_pre_order,
        depth_first_in_order,
        depth_first_post_order,
        depth_first_reverse_pre_order,
        depth_first_reverse_in_order,
        depth_first_reverse_post_order,
        // alias for depth_first_pre_order
        depth_first,


        breadth_first_in_order,
        breadth_first_reverse_order,
        // alias for breadth_first_in_order
        breadth_first
    };

    namespace _detail
    {  
        template <typename IterT>
        struct _iter_traits;

        template <bool Reversed, typename NodeT>
        struct _depth_first_post_order;

        /********************************************************
         * @brief type that is stored within the queue
         *        of queued-iterators. contains a node-ptr
         *        and two boolean flags:
         *        1.) _m_expanded, indicating if this node
         *            was already expanded or not.
         *        2.) _m_requires_skip_expand, indicating
         *            if the traversal requires early expansion
         *            of the next unexpanded node after this
         *            node for correctness.
         *            this is basically just to enable
         *            somewhat 'lazy' expansion for
         *            post/in-order.
         ********************************************************/
        template <typename NodeT>
        struct _queue_thunk
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;
            using _m_flag_t        = bool;

            _m_node_ptr_t _m_node;
            _m_flag_t     _m_expanded;
            _m_flag_t     _m_requires_skip_expand;

            constexpr explicit
            _queue_thunk(_m_node_ptr_t _node = nullptr,
                         _m_flag_t _mark_skip_expand = false)
                : _m_node(_node)
                , _m_expanded(false)
                , _m_requires_skip_expand(_mark_skip_expand)
            { }

            constexpr void
            _m_set()
                noexcept
            { this->_m_expanded = true; }

            constexpr void
            _m_mark_skip_expand()
                noexcept
            { this->_m_requires_skip_expand = true; }
        };

        /***************************************************
         * @brief additional state-variables for pre/post
         *        order traversing-iterators.
         *        idea by kpeeter's post-order-iterator at
         *        https://github.com/kpeeters/tree.hh
         ***************************************************/
        struct _traversing_iter_pre_post_order_state
        { 
            using _m_flag_t = bool;

            _m_flag_t _m_skip_children;

            constexpr
            _traversing_iter_pre_post_order_state(_m_flag_t _value = false)
                : _m_skip_children(_value)
            { }
        };

        /***************************************************
         * @brief traversal-implementation-type 
         *        for depth-first-pre-order.
         *      
         *        depth-first-pre-order traverses the
         *        current-node first and then moves on
         *        to the child-nodes afterwards.  
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _depth_first_pre_order
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * queue-expansion for queued-iterators.
             ***************************************************/

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            template <typename QueueAllocT>
            using _m_queue_t = std::list<_m_thunk_t, QueueAllocT>;
            template <typename QueueAllocT>
            using _m_iter_t  = typename _m_queue_t<QueueAllocT>::iterator;

            // inserts child-nodes after '_first' and returns
            // iterator to first-child 
            template <typename QueueAllocT>
            static constexpr _m_iter_t<QueueAllocT>
            _s_expand_queue(_m_iter_t<QueueAllocT> _first, 
                            _m_queue_t<QueueAllocT>& _queue)
            {
                _first->_m_set();
                _m_iter_t<QueueAllocT> _next = std::next(_first);
                // 'drag' the iterator one down after each 
                // insert, to insert sequentially after _first
                if constexpr (Reversed)
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node)
                         | std::views::reverse)
                         _queue.emplace(_next, _child);
                else
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node))
                         _queue.emplace(_next, _child);
                // return first-child or next
                return std::next(_first);
            }

            /***************************************************
             * next/prev algorithms for traversing-iterators.
             ***************************************************/

            using _m_iter_state_t = _traversing_iter_pre_post_order_state;

            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_next(IterT& _iter)
                requires _bidirectional_tree_node<_m_node_t>
            {
                _m_node_ptr_t _node = _iter_traits<IterT>::_s_to_node(_iter);
                _m_node_ptr_t _sibling = nullptr;
                if (_node == nullptr)
                    return nullptr;
                if constexpr (Reversed)
                {
                    if (!_m_node_traits_t::_s_is_leaf(_node))
                        return _m_node_traits_t::_s_last_child(_node);
                    while (!(_sibling = _m_node_traits_t::_s_prev_sibling(_node)))   
                    {
                        _node = _m_node_traits_t::_s_parent(_node);
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                    }
                    return _sibling;
                }
                else
                {
                    if (!_m_node_traits_t::_s_is_leaf(_node))
                        return _m_node_traits_t::_s_first_child(_node);
                    while (!(_sibling = _m_node_traits_t::_s_next_sibling(_node)))   
                    {
                        _node = _m_node_traits_t::_s_parent(_node);
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                    }
                    return _sibling;
                }
            }

            // prev for pre-order is the
            // reversed next from post-order
            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_prev(IterT& _iter)
                requires _bidirectional_tree_node<_m_node_t>
            { return _depth_first_post_order<!Reversed, _m_node_t>::_s_next(_iter); }

            /***************************************************
             * find traversal-begin from root/header-node.
             ***************************************************/

            // root-node is the pre-order begin-node
            template <typename IterT>
            static constexpr void
            _s_root_begin(IterT& _iter)
            { }

            // skip to first-child of header-node
            template <typename IterT>
            static constexpr void
            _s_header_begin(IterT& _iter)
            { ++_iter; }
        };

        /***************************************************
         * @brief traversal-implementation-type 
         *        for depth-first-post-order.
         *      
         *        depth-first-post-order traverses the
         *        current-node's children first and visits
         *        the node only after all children have
         *        been visited.
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _depth_first_post_order
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * queue-expansion for queued-iterators.
             ***************************************************/

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            template <typename QueueAllocT>
            using _m_queue_t = std::list<_m_thunk_t, QueueAllocT>;
            template <typename QueueAllocT>
            using _m_iter_t  = typename _m_queue_t<QueueAllocT>::iterator;

            template <typename QueueAllocT>
            static constexpr _m_iter_t<QueueAllocT>
            _s_expand_queue(_m_iter_t<QueueAllocT> _first, 
                            _m_queue_t<QueueAllocT>& _queue)
            {
                _first->_m_set();
                if (_m_node_traits_t::_s_is_leaf(_first->_m_node))
                    return std::next(_first);

                // capture iterator to first-child for recursive search
                bool _first_child = true;
            #define _treelib_insert_capture_first(_varname, _where) \
                { if (_first_child) \
                { _varname = _queue.emplace(_where, _child); _first_child = false; } \
                else \
                { _queue.emplace(_where, _child); } }
                
                _m_iter_t<QueueAllocT> _res;
                if constexpr (Reversed)
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node)
                         | std::views::reverse)
                        _treelib_insert_capture_first(_res, _first)
                else
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node))
                        _treelib_insert_capture_first(_res, _first)
                // mark first/last-child for skip-expansion
                std::prev(_first)->_m_mark_skip_expand();
            #undef _treelib_capture_first
                // keep expanding until first-child is a leaf
                if (!_m_node_traits_t::_s_is_leaf(_res->_m_node))
                    return _s_expand_queue(_res, _queue);
                return _res;
            }

            /***************************************************
             * next/prev algorithms traversing-iterators.
             ***************************************************/

            using _m_iter_state_t = _traversing_iter_pre_post_order_state;

            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_next(IterT& _iter)
                requires _bidirectional_tree_node<_m_node_t>
            {
                using _iter_traits_t = _iter_traits<IterT>;
                _m_iter_state_t& _state = _iter_traits_t::_s_trav_state(_iter);
                _m_node_ptr_t _node = _iter_traits_t::_s_to_node(_iter);
                _m_node_ptr_t _next;
                if (_node == nullptr)
                    return nullptr;
                if constexpr (Reversed)
                {
                    if (!_m_node_traits_t::_s_is_leaf(_node) 
                        && !_state._m_skip_children)
                        return _m_node_traits_t::_s_seek_rightmost(_node);
                    if ((_next = _m_node_traits_t::_s_prev_sibling(_node)))
                    {
                        _state._m_skip_children = false; 
                        if (!_m_node_traits_t::_s_is_leaf(_next))
                            return _m_node_traits_t::_s_seek_rightmost(_next);
                        return _next;
                    }
                    _state._m_skip_children = true;
                    if (_m_node_traits_t::_s_is_root(_node))
                        return nullptr;
                    return _m_node_traits_t::_s_parent(_node);
                }
                else
                {
                    if (!_m_node_traits_t::_s_is_leaf(_node) 
                        && !_state._m_skip_children)
                        return _m_node_traits_t::_s_seek_leftmost(_node);
                    if ((_next = _m_node_traits_t::_s_next_sibling(_node)))
                    {
                        _state._m_skip_children = false; 
                        if (!_m_node_traits_t::_s_is_leaf(_next))
                            return _m_node_traits_t::_s_seek_leftmost(_next);
                        return _next;
                    }
                    _state._m_skip_children = true;
                    if (_m_node_traits_t::_s_is_root(_node))
                        return nullptr;
                    return _m_node_traits_t::_s_parent(_node);
                }   
            }

            // prev for post-order is the
            // reversed next from pre-order
            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_prev(IterT& _iter)
                requires _bidirectional_tree_node<_m_node_t>
            { return _depth_first_pre_order<!Reversed, _m_node_t>::_s_next(_iter); }

            /***************************************************
             * find traversal-begin from root/header-node.
             ***************************************************/

            // skip to outermost
            template <typename IterT>
            static constexpr void
            _s_root_begin(IterT& _iter)
            { ++_iter; }

            // skip to outermost
            template <typename IterT>
            static constexpr void
            _s_header_begin(IterT& _iter)
            { ++_iter; }
        };

        /***************************************************
         * @brief traversal-implementation-type 
         *        for depth-first-in-order.
         *      
         *        depth-first-in-order traverses half of
         *        the current-node's children first, then 
         *        visits current-node itself and finishes
         *        with the other half of the children.
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _depth_first_in_order
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * queue-expansion for queued-iterators.
             ***************************************************/

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            template <typename QueueAllocT>
            using _m_queue_t = std::list<_m_thunk_t, QueueAllocT>;
            template <typename QueueAllocT>
            using _m_iter_t  = typename _m_queue_t<QueueAllocT>::iterator;

            template <typename QueueAllocT>
            static constexpr _m_iter_t<QueueAllocT>
            _s_expand_queue(_m_iter_t<QueueAllocT> _first, 
                            _m_queue_t<QueueAllocT>& _queue)
            {
                _first->_m_set();
                if (_m_node_traits_t::_s_is_leaf(_first->_m_node))
                    return std::next(_first);

                auto _children = _m_node_traits_t::_s_children(_first->_m_node);
                auto _half = std::ranges::begin(_children);
                std::advance(_half, _m_node_traits_t::_s_child_count(_first->_m_node) / 2);

                bool _first_child = true;
            #define _treelib_insert_capture_first2(_varname, _where) \
                { if (_first_child) \
                { _varname = _queue.emplace(_where, *_it); _first_child = false; } \
                else \
                { _queue.emplace(_where, *_it); } }

                _m_iter_t<QueueAllocT> _next = std::next(_first);
                _m_iter_t<QueueAllocT> _res, _last = _queue.end();
                if constexpr (Reversed)
                {

                }
                else
                {
                    // insert first half before the node
                    for (auto _it = std::ranges::begin(_children);
                         _it != _half;
                         ++_it)
                        _treelib_insert_capture_first2(_res, _first)
                    // insert second half after node
                    for (auto _it = _half;
                         _it != std::ranges::end(_children);
                         ++_it)
                        _last = _queue.emplace(_next, *_it);
                }
                // mark first/last for skip-expansion
                if (_last != _queue.end())
                    _last->_m_mark_skip_expand();
            #undef _treelib_insert_capture_first2
                // keep expanding until first-child is a leaf
                if (!_m_node_traits_t::_s_is_leaf(_res->_m_node))
                    return _s_expand_queue(_res, _queue);
                return _res;
            }

            /***************************************************
             * next/prev algorithms traversing-iterators.
             ***************************************************/

            using _m_iter_state_t = _traversing_iter_pre_post_order_state;

            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_next(IterT& _iter)
                requires _bidirectional_tree_node<_m_node_t>
            {
                using _iter_traits_t = _iter_traits<IterT>;
                _m_iter_state_t& _state = _iter_traits_t::_s_trav_state(_iter);
                _m_node_ptr_t _node = _iter_traits_t::_s_to_node(_iter);
                _m_node_ptr_t _next;
                if (_node == nullptr)
                    return nullptr;
                if constexpr (Reversed)
                {

                }
                else
                {

                }   
            }

            // prev for in-order is the
            // reversed next 
            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_prev(IterT& _iter)
                requires _bidirectional_tree_node<_m_node_t>
            { return _depth_first_in_order<!Reversed, _m_node_t>::_s_next(_iter); }

            /***************************************************
             * find traversal-begin from root/header-node.
             ***************************************************/

            // skip to outermost
            template <typename IterT>
            static constexpr void
            _s_root_begin(IterT& _iter)
            { ++_iter; }

            // skip to outermost
            template <typename IterT>
            static constexpr void
            _s_header_begin(IterT& _iter)
            { ++_iter; }
        };

        /***************************************************
         * @brief traversal-implementation-type 
         *        for breadth-first/level-order.
         *      
         *        depth-first-in-order traverses half of
         *        the current-node's children first, then 
         *        visits current-node itself and finishes
         *        with the other half of the children.
         *
         * @note  traversing-iterators do not support
         *        breadth-first.
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _breadth_first
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * queue-expansion for queued-iterators.
             ***************************************************/

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            template <typename QueueAllocT>
            using _m_queue_t = std::list<_m_thunk_t, QueueAllocT>;
            template <typename QueueAllocT>
            using _m_iter_t  = typename _m_queue_t<QueueAllocT>::iterator;

            // inserts all children at the back of the queue to
            // visit all nodes of the current level before them
            template <typename QueueAllocT>
            static constexpr _m_iter_t<QueueAllocT>
            _s_expand_queue(_m_iter_t<QueueAllocT> _first, 
                            _m_queue_t<QueueAllocT>& _queue)
            {
                _first->_m_set();
                if constexpr (Reversed)
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node)
                         | std::views::reverse)
                        _queue.emplace_back(_child);
                else
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node))
                        _queue.emplace_back(_child);
                return std::next(_first);
            }

            /***************************************************
             * find traversal-begin from root/header-node.
             ***************************************************/

            // root-node is the level-order begin-node
            template <typename IterT>
            static constexpr void
            _s_root_begin(IterT& _iter)
            { }

            // skip to first-child of header-node
            template <typename IterT>
            static constexpr void
            _s_header_begin(IterT& _iter)
            { ++_iter; }
        };

        /***************************************************
         * @brief type-trait to convert interface-enum
         *        to traversal-implementation-type.
         ***************************************************/

        template <traversal Trav, typename NodeT>
        struct _to_traversal;

        template <typename NodeT>
        struct _to_traversal<traversal::depth_first_pre_order, NodeT>
        { using _m_trav_t = _detail::_depth_first_pre_order<false, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::depth_first_reverse_pre_order, NodeT>
        { using _m_trav_t = _detail::_depth_first_pre_order<true, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::depth_first_in_order, NodeT>
        { using _m_trav_t = _detail::_depth_first_in_order<false, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::depth_first_reverse_in_order, NodeT>
        { using _m_trav_t = _detail::_depth_first_in_order<true, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::depth_first_post_order, NodeT>
        { using _m_trav_t = _detail::_depth_first_post_order<false, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::depth_first_reverse_post_order, NodeT>
        { using _m_trav_t = _detail::_depth_first_post_order<true, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::depth_first, NodeT>
        { using _m_trav_t = _detail::_depth_first_pre_order<false, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::breadth_first_in_order, NodeT>
        { using _m_trav_t = _detail::_breadth_first<false, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::breadth_first_reverse_order, NodeT>
        { using _m_trav_t = _detail::_breadth_first<true, NodeT>; };

        template <typename NodeT>
        struct _to_traversal<traversal::breadth_first, NodeT>
        { using _m_trav_t = _detail::_breadth_first<false, NodeT>; };

        // _t-abbreviation
        template <traversal Trav, typename NodeT>
        using _to_traversal_t = _to_traversal<Trav, NodeT>::_m_trav_t;

        /***************************************************
         * @brief CRTP-base for common functionality
         *        of tree-iterators.
         * 
         *        supplies iterator-member-types,
         *        post-increment/decrement, dereferencing
         *        and equality-comparison.
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename NodeT,
                  typename IterT>
        class _iter_base
        {
        protected:

            template <typename T>
            using _m_maybe_const_t = std::conditional_t<IsConst, const T, T>;

            using _m_iter_t       = IterT;
            using _m_value_t      = ValueT;
            using _m_node_t       = NodeT;
            using _m_node_ptr_t   = _m_node_t*;
            using _m_cnode_ptr_t  = const _m_node_t*;
            using _m_value_node_t = _value_node<_m_node_t, _m_value_t>;
            using _m_vnode_ptr_t  = _m_value_node_t*;

            template <bool OtherIsConst, typename OtherIterT>
            using _m_other_iter_t 
                = _iter_base<OtherIsConst, _m_value_t, _m_node_t, OtherIterT>;

            template <bool, typename, typename, typename>
            friend class _iter_base;

            /***************************************************
             * @brief since this is a CRTP-base,
             *        we can assume that casting itself to
             *        to a _m_iter_t* is valid. 
             ***************************************************/

            constexpr _m_iter_t*
            _m_iter()
                noexcept
            { return static_cast<_m_iter_t*>(this); }

            constexpr const _m_iter_t*
            _m_iter()
                const noexcept
            { return static_cast<const _m_iter_t*>(this); }

            /***************************************************
             * @brief accessor to derived-class's current node.
             ***************************************************/

            constexpr _m_node_ptr_t
            _m_cur()
                const noexcept
            { return this->_m_iter()->_m_cur(); }

        public:

            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = _m_value_t;
            using pointer           = _m_maybe_const_t<value_type>*;
            using const_pointer     = const value_type*;
            using reference         = _m_maybe_const_t<value_type>&;
            using const_reference   = const value_type&;

            /***************************************************
             * @brief compare two iterators based on their
             *        current node.
             *
             * @details note that the derived-iterator-type is also
             *          a template-variable because iterators of
             *          the same node-type of any kind (queued/
             *          traversing/leaf/sibling) should all be
             *          comparable to each other.
             ***************************************************/
            template <bool OtherIsConst, typename OtherIterT>
            [[nodiscard]]
            friend constexpr bool
            operator==(const _iter_base& a, 
                       const _m_other_iter_t<OtherIsConst, OtherIterT>& b)
                noexcept
            { return a._m_cur() == b._m_cur(); }

            /***************************************************
             * @brief value accessors.
             ***************************************************/

            [[nodiscard]]
            constexpr reference 
            operator*()
                const _treelib_noexcept
            {
            #ifdef _treelib_no_exceptions
                assert(this->_m_cur() != nullptr)
            #else
                if (this->_m_cur() == nullptr)
                    throw std::out_of_range("cannot dereference end-iterator");
                return static_cast<_m_vnode_ptr_t>(this->_m_cur())->_m_value();
            #endif
            }

            [[nodiscard]]
            constexpr pointer 
            operator->()
                const _treelib_noexcept
            { return std::addressof(this->operator*()); }

            /***************************************************
             * @brief post-increment operators.
             ***************************************************/

            constexpr _m_iter_t
            operator++(int)
            {
                _m_iter_t _tmp = *this->_m_iter();
                ++(*this->_m_iter());
                return _tmp;
            }

            // constexpr _m_iter_t
            // operator--(int)
            // { 
            //     _m_iter_t _tmp = *this->_m_iter();
            //     --(*this->_m_iter());
            //     return _tmp;
            // }
        };

        /***************************************************
         * @brief CRTP-mixin defining a constructor
         *        for conversions between any iterator-type
         *        of correct node/value-type and constness.
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename NodeT,
                  typename IterT>
        class _convertible_iter
            : public IterT
        {
        protected:

            using _m_base_t = IterT;
            using typename _m_base_t::_m_iter_t;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_value_t;

            template <bool OtherIsConst, typename OtherIterT>
            using _m_other_iter_t 
                = _convertible_iter<OtherIsConst, _m_value_t, _m_node_t, OtherIterT>;

            template <bool, typename, typename, typename>
            friend class _convertible_iter;

        public:

            using _m_base_t::_m_base_t;

            /***************************************************
             * @brief constructor (1).
             *        always constructible from mutable iterator.
             *        
             * @details note that the derived-iterator-type is also
             *          a template-variable, because iterators of
             *          the same node-type of any kind (queued/
             *          traversing/leaf/sibling) should all be
             *          convertible to each other, if the constness
             *          allows it.
             ***************************************************/
            template <bool OtherIsConst, typename OtherIterT>
                // either this is const, or both are mutable
                requires (IsConst || !OtherIsConst)
            _convertible_iter(const _m_other_iter_t<OtherIsConst, OtherIterT>& other)
                noexcept(std::is_nothrow_constructible_v<_m_base_t, _m_node_ptr_t>)
                : _m_base_t(other._m_cur())
            { }
        };

        /***************************************************
         * @brief interface for retrieving meta-information
         *        about a node in a tree (e.g. depth, 
         *        child-count, siblings).
         ***************************************************/
        template <typename NodeT>
        struct _node_info
        {
            
        };

        /***************************************************
         * @brief iterator that only visits nodes without
         *        any further child-nodes.
         ***************************************************/
        template <typename ValueT,
                  typename NodeT>
        struct _leaf_iterator
        {

        };

        /***************************************************
         * @brief iterator that visits every node that
         *        is subordinate to a specified other node.
         ***************************************************/
        template <typename ValueT,
                  typename NodeT>
        struct _child_iterator
        {

        };

        /********************************************************
         * @brief CRTP-base for traversing-iteration.
         *        
         * @note  some traversal-methods require more
         *        state than others (some booleans or pointers
         *        to certain points), this class inherits
         *        these state-types based on the traversal-type.
         ********************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
                  typename IterT>
        struct _traversing_iterator_base
            : public _iter_base<IsConst, ValueT, typename TraversalT::_m_node_t, IterT>
            , protected TraversalT::_m_iter_state_t
        {
            using _m_trav_t       = TraversalT;
            using _m_trav_state_t = typename _m_trav_t::_m_iter_state_t;
            using _m_base_t = _iter_base<IsConst, ValueT, typename TraversalT::_m_node_t, IterT>;
            using typename _m_base_t::_m_iter_t;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;

            friend _m_base_t;
            friend _iter_traits<_m_iter_t>;

            _m_node_ptr_t _m_node;

            /***************************************************
             * find traversal-begin from root/header-node.
             ***************************************************/

            // no further modifications necessary
            static constexpr _m_iter_t
            _s_root_begin(_m_node_ptr_t _node)
            {
                _m_iter_t _iter(_node);
                _m_trav_t::_s_root_begin(_iter);
                return _iter;
            }

            // no further modifications necessary
            static constexpr _m_iter_t
            _s_header_begin(_m_node_ptr_t _node)
            { 
                _m_iter_t _iter(_node);
                _m_trav_t::_s_header_begin(_iter);
                return _iter;
            }

            constexpr _m_node_ptr_t
            _m_cur()
                const noexcept
            { return this->_m_node; }

            constexpr explicit
            _traversing_iterator_base(_m_node_ptr_t _node)
                : _m_node(_node)
            { }

        public:

            using traversal_type = _m_trav_t;  

            constexpr
            _traversing_iterator_base()
                : _m_node(nullptr)
            { }

            constexpr _m_iter_t& 
            operator++()
            { 
                this->_m_node = _m_trav_t::_s_next(*this->_m_iter());
                return *this->_m_iter();
            }

            using _m_base_t::operator++;
        };

        /***************************************************
         * @brief CRTP-base for queued iteration. 
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
                  typename QueueAllocT,
                  typename IterT>
        class _queued_iterator_base
            : public _iter_base<IsConst, ValueT, typename TraversalT::_m_node_t, IterT>
        {
        protected:
    
            using _m_trav_t = TraversalT;
            using _m_base_t = _iter_base<IsConst, ValueT, typename TraversalT::_m_node_t, IterT>;
            using typename _m_base_t::_m_iter_t;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            using _m_alloc_t = QueueAllocT;
            using _m_queue_alloc_t 
                = std::allocator_traits<_m_alloc_t>::template rebind_alloc<_m_thunk_t>;
            using _m_queue_t /* this type is a cutie */ 
                = std::list<_m_thunk_t, _m_queue_alloc_t>;
            using _m_queue_iter_t = typename _m_queue_t::iterator;

            friend _m_base_t;

            _m_queue_t      _m_queue;
            _m_queue_iter_t _m_queue_cur;

            /***************************************************
             * find traversal-begin from root/header-node.
             ***************************************************/

            // no further modifications necessary
            static constexpr _m_iter_t
            _s_root_begin(_m_node_ptr_t _node, const _m_alloc_t& _alloc)
            { 
                _m_iter_t _iter(_node, _alloc);
                _m_trav_t::_s_root_begin(_iter);
                return _iter; 
            }

            // additionally erase header from queue
            static constexpr _m_iter_t
            _s_header_begin(_m_node_ptr_t _node, const _m_alloc_t& _alloc)
            { 
                _m_iter_t _iter(_node, _alloc);
                _m_trav_t::_s_header_begin(_iter);
                std::erase_if(_iter._m_queue, [&](const _m_thunk_t& _t) { return _t._m_node == _node; });
                return _iter;
            }

            constexpr _m_node_ptr_t 
            _m_cur()
                const noexcept
            { 
                return this->_m_queue_cur == this->_m_queue.end()
                       ? nullptr
                       : this->_m_queue_cur->_m_node;
            }

            constexpr void
            _m_skip_expand_if()
            {
                if (this->_m_queue_cur->_m_requires_skip_expand)
                {
                    auto _unexpanded = [](const _m_thunk_t& _t)
                                       { return !_t._m_expanded; };
                    _m_queue_iter_t _next 
                        = std::find_if(std::next(this->_m_queue_cur), this->_m_queue.end(), _unexpanded);
                    if (_next != this->_m_queue.end())
                        _m_trav_t::_s_expand_queue(_next, this->_m_queue);
                }
            }

            constexpr void
            _m_enqueue()
            {
                this->_m_skip_expand_if();
                this->_m_queue_cur
                    = _m_trav_t::_s_expand_queue(this->_m_queue_cur, this->_m_queue);
            }

            constexpr bool
            _m_should_enqueue()
                const noexcept
            { return !this->_m_queue_cur->_m_expanded; }

            constexpr explicit
            _queued_iterator_base(_m_node_ptr_t _node,
                                  const _m_alloc_t& _alloc = _m_alloc_t())
                : _m_queue({_m_thunk_t(_node)}, _alloc)
                , _m_queue_cur(_m_queue.begin())
            { }

        public:

            using traversal_type = _m_trav_t;  
            using allocator_type = _m_alloc_t;

            constexpr
            _queued_iterator_base(const allocator_type& alloc = allocator_type())
                : _m_queue(alloc)
                , _m_queue_cur(this->_m_queue.end())
            { }

            constexpr _m_iter_t& 
            operator++()
            { 
                if (this->_m_should_enqueue())
                    this->_m_enqueue();
                else 
                    ++this->_m_queue_cur;
                return *this->_m_iter();
            }

            using _m_base_t::operator++;
        };

        /**********************************************
         * @brief forward-declarations for the
         *        actual node-types and
         *        aliases to abbreviate the
         *        CRTP/mixin-base-classes and make
         *        them a bit more readable.
         **********************************************/

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
                  typename QueueAllocT>
        class _queued_iterator;

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT>
        class _traversing_iterator;

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
                  typename QueueAllocT>
        using _queued_iter_base
            = _convertible_iter<IsConst, ValueT, typename TraversalT::_m_node_t,
                _queued_iterator_base<IsConst, ValueT, TraversalT, QueueAllocT,
                  _queued_iterator<IsConst, ValueT, TraversalT, QueueAllocT>>>;

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT>
        using _traversing_iter_base
            = _convertible_iter<IsConst, ValueT, typename TraversalT::_m_node_t,
                _traversing_iterator_base<IsConst, ValueT, TraversalT,
                  _traversing_iterator<IsConst, ValueT, TraversalT>>>;

        /***************************************************
         * @brief iterator that traverses a tree iteratively, 
         *        (no breadth-first). 
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT>
        struct _traversing_iterator
            : public _traversing_iter_base<IsConst, ValueT, TraversalT>
        {
        protected:

            using _m_base_t      = _traversing_iter_base<IsConst, ValueT, TraversalT>;
            using _m_iter_base_t = typename _m_base_t::_m_base_t;
            using typename _m_base_t::_m_iter_t;

            friend _iter_traits<_m_iter_t>;
            friend _m_iter_base_t;

        public:

            using _m_base_t::_m_base_t;
        };

        /***************************************************
         * @brief iterator that traverses a tree by
         *        progressively building up a queue of nodes.
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
                  typename QueueAllocT>
        class _queued_iterator
            : public _queued_iter_base<IsConst, ValueT, TraversalT, QueueAllocT>
        {
        protected:

            using _m_base_t      = _queued_iter_base<IsConst, ValueT, TraversalT, QueueAllocT>;
            using _m_iter_base_t = typename _m_base_t::_m_base_t;
            using typename _m_base_t::_m_iter_t;

            friend _iter_traits<_m_iter_t>;
            friend _m_iter_base_t;

        public:

            using _m_base_t::_m_base_t;
        };

        /***************************************************
         * @brief uniform interface for tree-iterators.
         ***************************************************/
        template <typename IterT>
        struct _iter_traits
        {
            using _m_iter_t     = IterT;
            using _m_node_t     = typename _m_iter_t::_m_node_t;
            using _m_node_ptr_t = typename _m_iter_t::_m_node_ptr_t;

            using _m_value_t = typename IterT::value_type;
            using _m_ref_t   = typename IterT::reference;
            using _m_ptr_t   = typename IterT::pointer;

            static constexpr bool
            _s_constness = std::is_const_v<std::remove_reference_t<_m_ref_t>>;

            template <typename... ArgsTs>
            static constexpr _m_iter_t
            _s_root_begin(_m_node_ptr_t _node, ArgsTs&&... _args)
                _treelib_noexcept_if(_m_iter_t::_s_root_begin(_node, std::forward<ArgsTs>(_args)...))
            { return _m_iter_t::_s_root_begin(_node, std::forward<ArgsTs>(_args)...); }

            template <typename... ArgsTs>
            static constexpr _m_iter_t
            _s_header_begin(_m_node_ptr_t _node, ArgsTs&&... _args)
                _treelib_noexcept_if(_m_iter_t::_s_header_begin(_node, std::forward<ArgsTs>(_args)...))
            { return _m_iter_t::_s_header_begin(_node, std::forward<ArgsTs>(_args)...); }

            template <typename... ArgsTs>
            static constexpr _m_iter_t
            _s_to_iter(ArgsTs&&... _args)
                noexcept(std::is_nothrow_constructible_v<_m_iter_t, _m_node_ptr_t, ArgsTs...>)
            { return IterT(_args...); }

            static constexpr _m_node_ptr_t 
            _s_to_node(const _m_iter_t& _iter)
                noexcept
            { return _iter._m_cur(); }

            static constexpr auto&
            _s_trav_state(_m_iter_t& _iter)
                requires _treelib_has_member_type(_m_iter_t, _m_trav_state_t)
            {
                using _state_t = typename _m_iter_t::_m_trav_state_t;
                return static_cast<_state_t&>(_iter); 
            }
        };
    }

    /***************************************************
     * @brief iterator-type aliases.
     *
     * @note  the ugly template-parameters
     *        should get deduced when constructing these
     *        types from an existing iterator.
     *
     *        these aliases exist only for the convenience
     *        of not having to prefix the tree-type
     *        when wanting to specify an iterator.
     ***************************************************/

    template <bool IsConst, 
              typename ValueT, 
              typename TraversalT,
              typename QueueAllocT = std::allocator<ValueT>>
    using queued_iterator
        = _detail::_queued_iterator<IsConst, ValueT, TraversalT, QueueAllocT>;

    template <bool IsConst, 
              typename ValueT, 
              typename NodeT,
              typename QueueAllocT = std::allocator<ValueT>>
    using queued_depth_first_iterator
        = queued_iterator<IsConst, ValueT, _detail::_depth_first_pre_order<false, NodeT>, QueueAllocT>;

    template <bool IsConst, 
              typename ValueT, 
              typename NodeT,
              typename QueueAllocT = std::allocator<ValueT>>
    using queued_depth_first_post_order_iterator
        = queued_iterator<IsConst, ValueT, _detail::_depth_first_post_order<false, NodeT>, QueueAllocT>;

    template <bool IsConst, 
              typename ValueT, 
              typename NodeT,
              typename QueueAllocT>
    using queued_breadth_first_iterator
        = queued_iterator<IsConst, ValueT, _detail::_breadth_first<false, NodeT>, QueueAllocT>;
}

#endif