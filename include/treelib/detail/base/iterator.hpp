
#ifndef TREELIB_BASE_ITERATOR_HPP
#define TREELIB_BASE_ITERATOR_HPP

/***************************************************
 * @file   treelib/detail/base/iterator.hpp
 * @author Julian Benzel
 * @date   25.09.2026
 *
 * @brief  classes to enable various methods of
 *         tree-traversal between instances of
 *         a node-type.
 *
 * @details implements the following tree-traversal
 *          algorithms:
 *          - depth-first-pre-order
 *          - depth-first-post-order
 *          - depth-first-in-order
 *          - breadth-first/level-order
 *          all algorithms are supported via a
 *          queued-iterator, that is an iterator
 *          that either builds a full traversal
 *          queue upon construction (greedy) or
 *          incrementally builds the queue during
 *          iteration (lazy).
 *          alternatively, there are also 
 *          traversing iterators, which only hold
 *          a pointer to a current node (and some
 *          state-variables).
 ***************************************************/

#include <treelib/detail/bits/except.hpp>

#include <treelib/detail/base/node.hpp>

#include <memory>
#include <iterator>
#include <list>
#include <vector>
#include <queue>
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
        // shortcut/alias for depth_first_pre_order
        depth_first,

        breadth_first_in_order,
        breadth_first_reverse_order,
        // shortcut/alias for breadth_first_in_order
        breadth_first
    };

    namespace _detail
    {  
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
            using _m_node_ptr_t    = _m_node_t*;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            struct _traversing
            {
                _m_node_ptr_t _m_node;

                constexpr
                _traversing(_m_node_ptr_t _node = nullptr)
                    : _m_node(_node)
                { }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { return this->_m_node; }

                static constexpr _m_node_ptr_t
                _s_advance_forward(_m_node_ptr_t _node)
                {
                    _m_node_ptr_t _sibling = nullptr;
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

                static constexpr _m_node_ptr_t
                _s_advance_reverse(_m_node_ptr_t _node)
                {
                    _m_node_ptr_t _sibling = nullptr;
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

                constexpr void
                _m_advance_forward()
                { this->_m_node = _s_advance_forward(this->_m_current()); }

                constexpr void
                _m_advance_reverse()
                { this->_m_node = _s_advance_reverse(this->_m_current()); }

                constexpr void
                _m_advance()
                {
                    if constexpr (Reversed)
                        this->_m_advance_reverse();
                    else
                        this->_m_advance_forward();
                }
            };

            template <typename QueueAllocT>
            struct _queued_greedy
            {
                using _m_thunk_t = _m_node_ptr_t;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::vector<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;
                using _m_offset_t 
                    = _m_queue_t::size_type;

                _m_queue_t  _m_queue;
                _m_offset_t _m_offset;

                constexpr 
                _queued_greedy(_m_node_ptr_t _node,
                               const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue(_alloc)
                    , _m_offset(0)
                { this->_m_expand(_node); }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return std::next(this->_m_queue.begin(), this->_m_offset); }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : *this->_m_current_iter(); 
                }

                constexpr void
                _m_expand_forward(_m_node_ptr_t _node)
                {
                    this->_m_queue.emplace_back(_node);
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(_node))
                        this->_m_expand_forward(_child);
                }

                constexpr void
                _m_expand_reverse(_m_node_ptr_t _node)
                {
                    this->_m_queue.emplace_back(_node);
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(_node)
                        | std::views::reverse)
                        this->_m_expand_reverse(_child);
                }

                constexpr void
                _m_expand(_m_node_ptr_t _node)
                {
                    if constexpr (Reversed)
                        this->_m_expand_reverse(_node);
                    else
                        this->_m_expand_forward(_node);
                }

                constexpr void
                _m_advance()
                { ++this->_m_offset; }
            };

            template <typename QueueAllocT>
            struct _queued_lazy
            {
                using _m_thunk_t = _m_node_ptr_t;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::vector<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;
                using _m_offset_t 
                    = _m_queue_t::size_type;

                _m_queue_t  _m_queue;
                _m_offset_t _m_offset;

                constexpr
                _queued_lazy(_m_node_ptr_t _node,
                             const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue({_node}, _alloc)
                    , _m_offset(0)
                { }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return std::next(this->_m_queue.begin(), this->_m_offset); }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : *this->_m_current_iter(); 
                }

                constexpr void
                _m_advance_forward()
                {
                    _m_queue_iter_t _cur = this->_m_current_iter();
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(*_cur))
                        this->_m_queue.emplace(std::next(_cur), _child);
                }

                constexpr void
                _m_advance_reverse()
                {
                    _m_queue_iter_t _cur = this->_m_current_iter();
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(*_cur)
                        | std::views::reverse)
                        this->_m_queue.emplace(std::next(_cur), _child);
                }

                constexpr void
                _m_advance()
                {
                    if constexpr (Reversed)
                        this->_m_advance_reverse();
                    else
                        this->_m_advance_forward();
                    ++this->_m_offset;
                }
            };

            template <typename QueueAllocT>
            using _m_greedy_queued_base_t = _queued_greedy<QueueAllocT>;
            template <typename QueueAllocT>
            using _m_lazy_queued_base_t   = _queued_lazy<QueueAllocT>;
            using _m_trav_base_t   = _traversing;
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
            using _m_node_ptr_t    = _m_node_t*;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            struct _traversing
            {
                using _m_flag_t = bool;

                /*****************************************
                 * idea based on kpeeter's post-order
                 * iterator from:
                 * https://github.com/kpeeters/tree.hh
                 *****************************************/

                _m_node_ptr_t _m_node;
                _m_flag_t     _m_skip_children;

                constexpr
                _traversing(_m_node_ptr_t _node = nullptr)
                    : _m_node(_node)
                    , _m_skip_children(false)
                { }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { return this->_m_node; }

                static constexpr _m_node_ptr_t
                _s_advance_forward(_m_node_ptr_t _node, _traversing& _iter)
                {
                    _m_node_ptr_t _sibling = nullptr;
                    if (!(_m_node_traits_t::_s_is_leaf(_node) || _iter._m_skip_children))
                        return _m_node_traits_t::_s_seek_leftmost(_node);
                    if (_m_node_traits_t::_s_is_root(_node))
                        return nullptr;
                    if (_m_node_traits_t::_s_is_last_child(_node))
                    {
                        _iter._m_skip_children = true;
                        return _m_node_traits_t::_s_parent(_node);
                    }
                    _iter._m_skip_children = false;
                    _sibling = _m_node_traits_t::_s_next_sibling(_node);
                    if (_m_node_traits_t::_s_is_leaf(_sibling))
                        return _sibling;
                    else
                        return _s_advance_forward(_sibling, _iter);
                }

                static constexpr _m_node_ptr_t
                _s_advance_reverse(_m_node_ptr_t _node, _traversing& _iter)
                {
                    _m_node_ptr_t _sibling = nullptr;
                    if (!(_m_node_traits_t::_s_is_leaf(_node) || _iter._m_skip_children))
                        return _m_node_traits_t::_s_seek_rightmost(_node);
                    if (_m_node_traits_t::_s_is_root(_node))
                        return nullptr;
                    if (_m_node_traits_t::_s_is_last_child(_node))
                    {
                        _iter._m_skip_children = true;
                        return _m_node_traits_t::_s_parent(_node);
                    }
                    _iter._m_skip_children = false;
                    _sibling = _m_node_traits_t::_s_prev_sibling(_node);
                    if (_m_node_traits_t::_s_is_leaf(_sibling))
                        return _sibling;
                    else
                        return _s_advance_forward(_sibling, _iter);
                }

                constexpr void
                _m_advance_forward()
                { this->_m_node = _s_advance_forward(this->_m_current(), *this); }

                constexpr void
                _m_advance_reverse()
                { this->_m_node = _s_advance_reverse(this->_m_current(), *this); }

                constexpr void
                _m_advance()
                {
                    if constexpr (Reversed)
                        this->_m_advance_reverse();
                    else
                        this->_m_advance_forward();
                }
            };

            template <typename QueueAllocT>
            struct _queued_greedy
            {
                using _m_thunk_t = _m_node_ptr_t;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::vector<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;
                using _m_offset_t 
                    = _m_queue_t::size_type;

                _m_queue_t  _m_queue;
                _m_offset_t _m_offset;

                constexpr 
                _queued_greedy(_m_node_ptr_t _node,
                               const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue(_alloc)
                    , _m_offset(0)
                { this->_m_expand(_node); }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return std::next(this->_m_queue.begin(), this->_m_offset); }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : *this->_m_current_iter(); 
                }

                constexpr void
                _m_expand_forward(_m_node_ptr_t _node)
                {
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(_node))
                        this->_m_expand_forward(_child);
                    this->_m_queue.emplace_back(_node);
                }

                constexpr void
                _m_expand_reverse(_m_node_ptr_t _node)
                {
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(_node)
                        | std::views::reverse)
                        this->_m_expand_reverse(_child);
                    this->_m_queue.emplace_back(_node);
                }

                constexpr void
                _m_expand(_m_node_ptr_t _node)
                {
                    if constexpr (Reversed)
                        this->_m_expand_reverse(_node);
                    else
                        this->_m_expand_forward(_node);
                }

                constexpr void
                _m_advance()
                { ++this->_m_offset; }
            };

            template <typename QueueAllocT>
            struct _queued_lazy
            {
                using _m_thunk_t = _m_node_ptr_t;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::list<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;
                using _m_flag_t = bool;

                _m_queue_t      _m_queue;
                _m_queue_iter_t _m_queue_iter;
                _m_flag_t       _m_skip_children;

                constexpr
                _queued_lazy(_m_node_ptr_t _node,
                             const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue({_node}, _alloc)
                    , _m_queue_iter(this->_m_queue.begin())
                    , _m_skip_children(false)
                { }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return this->_m_queue_iter; }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : *this->_m_current_iter(); 
                }

                constexpr void
                _m_insert_forward()
                {
                    _m_queue_t _tmp;
                    _m_queue_iter_t _it;
                    do
                    {
                        auto _children = _m_node_traits_t::_s_children(this->_m_current());
                        _tmp = std::list(std::ranges::begin(_children), 
                                         std::ranges::end(_children),
                                         this->_m_queue.get_allocator());
                        _it = _tmp.begin();
                        this->_m_queue.splice(this->_m_queue_iter, _tmp);
                        this->_m_queue_iter = _it;
                    }
                    while (_m_node_traits_t::_s_is_leaf(this->_m_current()));
                }

                constexpr void
                _m_insert_reverse()
                {
                    _m_queue_t _tmp;
                    _m_queue_iter_t _it;
                    do
                    {
                        auto _children = _m_node_traits_t::_s_children(this->_m_current()) 
                                         | std::views::reverse;
                        _tmp = std::list(std::ranges::begin(_children), 
                                         std::ranges::end(_children),
                                         this->_m_queue.get_allocator());
                        _it = _tmp.begin();
                        this->_m_queue.splice(this->_m_queue_iter, _tmp);
                        this->_m_queue_iter = _it;
                    }
                    while (_m_node_traits_t::_s_is_leaf(this->_m_current()));
                }

                constexpr void
                _m_advance_forward()
                {
                    if (this->_m_skip_children)
                    {
                        ++this->_m_queue_iter;
                        if (_m_node_traits_t::_s_is_leaf(this->_m_current()))
                            this->_m_insert_forward();
                        this->_m_skip_children = false;
                    }
                    else 
                    {
                        if (_m_node_traits_t::_s_is_leaf(this->_m_current()))
                        {
                            if (_m_node_traits_t::_s_is_last_child_of(
                                    *std::next(this->_m_current_iter()), 
                                    this->_m_current()))
                                this->_m_skip_children = true;
                            ++this->_m_queue_iter;
                        }
                        else 
                            this->_m_insert_forward();
                    }
                }

                constexpr void
                _m_advance_reverse()
                {
                    if (this->_m_skip_children)
                    {
                        ++this->_m_queue_iter;
                        if (_m_node_traits_t::_s_is_leaf(this->_m_current()))
                            this->_m_insert_reverse();
                        this->_m_skip_children = false;
                    }
                    else 
                    {
                        if (_m_node_traits_t::_s_is_leaf(this->_m_current()))
                        {
                            if (_m_node_traits_t::_s_is_first_child_of(
                                    *std::next(this->_m_current_iter()), 
                                    this->_m_current()))
                                this->_m_skip_children = true;
                            ++this->_m_queue_iter;
                        }
                        else 
                            this->_m_insert_reverse();
                    }
                }

                constexpr void
                _m_advance()
                {
                    if constexpr (Reversed)
                        this->_m_advance_reverse();
                    else
                        this->_m_advance_forward();
                }
            };

            template <typename QueueAllocT>
            using _m_greedy_queued_base_t = _queued_greedy<QueueAllocT>;
            template <typename QueueAllocT>
            using _m_lazy_queued_base_t   = _queued_lazy<QueueAllocT>;
            using _m_trav_base_t   = _traversing;
        };

        /***************************************************
         * @brief traversal-implementation-type 
         *        for depth-first-in-order.
         *      
         *        depth-first-in-order traverses half of the
         *        current-node's children first, then the
         *        current-node and then the remaining half
         *        afterwards.
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _depth_first_in_order
        {
            using _m_node_t        = NodeT;
            using _m_node_ptr_t    = _m_node_t*;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            struct _traversing
            {
                enum struct _tag 
                    : std::uint8_t
                { _none, _left, _right, _both };
                using _m_tag_t = _tag;

                _m_node_ptr_t _m_node;
                _m_tag_t      _m_tag;

                constexpr
                _traversing(_m_node_ptr_t _node = nullptr)
                    : _m_node(_node)
                { }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { return this->_m_node; }

                static constexpr _m_node_ptr_t
                _s_advance_forward(_m_node_ptr_t _node, _traversing& _iter)
                {
                    if (_m_node_traits_t::_s_is_leaf(_node)
                        || _iter._m_tag == _m_tag_t::_both)
                    {
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                        _m_node_ptr_t _parent = _m_node_traits_t::_s_parent(_node);
                        if (_node == _m_node_traits_t::_s_last_left_child(_parent))
                        {
                            _iter._m_tag = _m_tag_t::_left;
                            return _parent;
                        }
                        else if (_m_node_traits_t::_s_is_last_child(_node))
                        {
                            _iter._m_tag = _m_tag_t::_both;
                            return _s_advance_forward(_parent, _iter);
                        }
                        else 
                            return _m_node_traits_t::_s_next_sibling(_node);
                    }
                    else if (_iter._m_tag == _m_tag_t::_left)
                    {
                        if (!_m_node_traits_t::_s_is_right_leaf(_node))
                        {
                            _iter._m_tag = _m_tag_t::_none;
                            return _s_advance_forward(_m_node_traits_t::_s_first_right_child(_node), _iter);
                        }
                        else 
                        {
                            _iter._m_tag = _m_tag_t::_both;
                            return _s_advance_forward(_node, _iter);
                        }
                    }
                    else 
                    {
                        _iter._m_tag = _m_tag_t::_left;
                        return _m_node_traits_t::_s_seek_leftmost(_node);
                    }
                }

                static constexpr _m_node_ptr_t
                _s_advance_reverse(_m_node_ptr_t _node, _traversing& _iter)
                {
                    if (_m_node_traits_t::_s_is_leaf(_node)
                        || _iter._m_tag == _m_tag_t::_both)
                    {
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                        _m_node_ptr_t _parent = _m_node_traits_t::_s_parent(_node);
                        if (_node == _m_node_traits_t::_s_last_right_child(_parent))
                        {
                            _iter._m_tag = _m_tag_t::_right;
                            return _parent;
                        }
                        else if (_m_node_traits_t::_s_is_first_child(_node))
                        {
                            _iter._m_tag = _m_tag_t::_both;
                            return _s_advance_reverse(_parent, _iter);
                        }
                        else 
                            return _m_node_traits_t::_s_prev_sibling(_node);
                    }
                    else if (_iter._m_tag == _m_tag_t::_right)
                    {
                        if (!_m_node_traits_t::_s_is_left_leaf(_node))
                        {
                            _iter._m_tag = _m_tag_t::_none;
                            return _s_advance_reverse(_m_node_traits_t::_s_first_left_child(_node), _iter);
                        }
                        else 
                        {
                            _iter._m_tag = _m_tag_t::_both;
                            return _s_advance_reverse(_node, _iter);
                        }
                    }
                    else 
                    {
                        _iter._m_tag = _m_tag_t::_right;
                        return _m_node_traits_t::_s_seek_rightmost(_node);
                    }
                }

                constexpr void
                _m_advance_forward()
                { this->_m_node = _s_advance_forward(this->_m_current(), *this); }

                constexpr void
                _m_advance_reverse()
                { this->_m_node = _s_advance_reverse(this->_m_current(), *this); }

                constexpr void
                _m_advance()
                {
                    if constexpr (Reversed)
                        this->_m_advance_reverse();
                    else
                        this->_m_advance_forward();
                }
            };

            template <typename QueueAllocT>
            struct _queued_greedy
            {
                using _m_thunk_t = _m_node_ptr_t;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::vector<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;
                using _m_offset_t 
                    = _m_queue_t::size_type;

                _m_queue_t  _m_queue;
                _m_offset_t _m_offset;

                constexpr 
                _queued_greedy(_m_node_ptr_t _node,
                               const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue(_alloc)
                    , _m_offset(0)
                { this->_m_expand(_node); }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return std::next(this->_m_queue.begin(), this->_m_offset); }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : *this->_m_current_iter(); 
                }

                constexpr void
                _m_expand_forward(_m_node_ptr_t _node)
                {
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_left_children(_node))
                        this->_m_expand_forward(_child);
                    this->_m_queue.emplace_back(_node);
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_right_children(_node))
                        this->_m_expand_forward(_child);
                }

                constexpr void
                _m_expand_reverse(_m_node_ptr_t _node)
                {
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_right_children(_node))
                        this->_m_expand_reverse(_child);
                    this->_m_queue.emplace_back(_node);
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_left_children(_node))
                        this->_m_expand_reverse(_child);
                }

                constexpr void
                _m_expand(_m_node_ptr_t _node)
                {
                    if constexpr (Reversed)
                        this->_m_expand_reverse(_node);
                    else
                        this->_m_expand_forward(_node);
                }

                constexpr void
                _m_advance()
                { ++this->_m_offset; }
            };

            template <typename QueueAllocT>
            struct _queued_lazy
            {
                struct _thunk
                {
                    using _m_flag_t = bool;

                    _m_node_ptr_t _m_node;
                    _m_flag_t     _m_visited;

                    constexpr
                    _thunk(_m_node_ptr_t _node)
                        noexcept
                        : _m_node(_node)
                        , _m_visited(false)
                    { }
                };

                using _m_thunk_t = _thunk;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::list<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;

                _m_queue_t      _m_queue;
                _m_queue_iter_t _m_queue_iter;

                constexpr
                _queued_lazy(_m_node_ptr_t _node,
                             const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue({_node}, _alloc)
                    , _m_queue_iter(this->_m_queue.begin())
                { }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return this->_m_queue_iter; }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : this->_m_current_iter()->_m_node; 
                }

                constexpr void
                _m_insert_forward()
                {
                    _m_queue_t _left_tmp;
                    _m_queue_t _right_tmp;
                    _m_queue_iter_t _it;
                    do
                    {
                        auto _left_children  
                            = _m_node_traits_t::_s_left_children(this->_m_current()); 
                        auto _right_children 
                            = _m_node_traits_t::_s_right_children(this->_m_current());
                        
                        _left_tmp = std::list(std::ranges::begin(_left_children), 
                                              std::ranges::end(_left_children),
                                              this->_m_queue.get_allocator());
                        _right_tmp = std::list(std::ranges::begin(_left_children), 
                                               std::ranges::end(_left_children),
                                               this->_m_queue.get_allocator());
                        _it = _left_tmp.begin();

                        this->_m_queue.splice(this->_m_queue_iter, _left_tmp);
                        this->_m_queue.splice(std::next(this->_m_queue_iter), _right_tmp);
                        this->_m_queue_iter = _it;
                    }
                    while (_m_node_traits_t::_s_is_leaf(this->_m_current()));
                }

                constexpr void
                _m_insert_reverse()
                {
                    _m_queue_t _left_tmp;
                    _m_queue_t _right_tmp;
                    _m_queue_iter_t _it;
                    do
                    {
                        auto _left_children  
                            = _m_node_traits_t::_s_left_children(this->_m_current()); 
                        auto _right_children 
                            = _m_node_traits_t::_s_right_children(this->_m_current());
                        
                        _left_tmp = std::list(std::ranges::begin(_left_children), 
                                              std::ranges::end(_left_children),
                                              this->_m_queue.get_allocator());
                        _right_tmp = std::list(std::ranges::begin(_left_children), 
                                               std::ranges::end(_left_children),
                                               this->_m_queue.get_allocator());
                        _it = _right_tmp.begin();

                        this->_m_queue.splice(this->_m_queue_iter, _right_tmp);
                        this->_m_queue.splice(std::next(this->_m_queue_iter), _left_tmp);
                        this->_m_queue_iter = _it;
                    }
                    while (_m_node_traits_t::_s_is_leaf(this->_m_current()));
                }

                constexpr void
                _m_advance_forward()
                {
                    _m_queue_iter_t _cur = this->_m_current_iter();
                    if (_cur->_m_visited)
                        ++this->_m_queue_iter;
                    else
                    {
                        _cur->_m_visited = true;
                        if (_m_node_traits_t::_s_is_leaf(_cur->_m_node))
                            ++this->_m_queue_iter;
                        else
                            this->_m_insert_forward();
                    }
                }

                constexpr void
                _m_advance_reverse()
                {
                    _m_queue_iter_t _cur = this->_m_current_iter();
                    if (_cur->_m_visited)
                        ++this->_m_queue_iter;
                    else
                    {
                        _cur->_m_visited = true;
                        if (_m_node_traits_t::_s_is_leaf(_cur->_m_node))
                            ++this->_m_queue_iter;
                        else
                            this->_m_insert_reverse();
                    }
                }

                constexpr void
                _m_advance()
                {
                    if constexpr (Reversed)
                        this->_m_advance_reverse();
                    else
                        this->_m_advance_forward();
                }
            };

            template <typename QueueAllocT>
            using _m_greedy_queued_base_t = _queued_greedy<QueueAllocT>;
            template <typename QueueAllocT>
            using _m_lazy_queued_base_t   = _queued_lazy<QueueAllocT>;
            using _m_trav_base_t   = _traversing;
        };

        /***************************************************
         * @brief traversal-implementation-type 
         *        for breadth-first/level-order.
         *      
         *        depth-first-in-order traverses half of
         *        the current-node's children first, then 
         *        visits current-node itself and finishes
         *        with the other half of the children.
         ***************************************************/
        template <bool Reversed,
                typename NodeT>
        struct _breadth_first
        {
            using _m_node_t        = NodeT;
            using _m_node_ptr_t    = _m_node_t*;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            struct _traversing
            {
                using _m_depth_t = typename _m_node_traits_t::_m_depth_t;

                _m_node_ptr_t _m_node;

                constexpr
                _traversing(_m_node_ptr_t _node = nullptr)
                    : _m_node(_node)
                { }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { return this->_m_node; }

                static constexpr _m_node_ptr_t
                _s_find_same_level_forward(_m_node_ptr_t _node)
                {
                    if (_m_node_traits_t::_s_is_root(_node))
                        return nullptr;
                    _m_depth_t _steps {0};
                    while (_m_node_traits_t::_s_is_last_child(_node))
                    {
                        _node = _m_node_traits_t::_s_parent(_node);
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                        ++_steps;
                    }
                    return _s_seek_level_forward(
                                _m_node_traits_t::_s_next_sibling(_node), 
                                _steps);
                }

                static constexpr _m_node_ptr_t
                _s_find_same_level_reverse(_m_node_ptr_t _node)
                {
                    if (_m_node_traits_t::_s_is_root(_node))
                        return nullptr;
                    _m_depth_t _steps {0};
                    while (_m_node_traits_t::_s_is_first_child(_node))
                    {
                        _node = _m_node_traits_t::_s_parent(_node);
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                        ++_steps;
                    }
                    return _s_seek_level_reverse(
                                _m_node_traits_t::_s_prev_sibling(_node), 
                                _steps);
                }

                static constexpr _m_node_ptr_t
                _s_seek_level_forward(_m_node_ptr_t _node, _m_depth_t _depth)
                {
                    if (_depth == 0)
                        return _node;
                    else
                    {
                        _m_node_ptr_t _res;
                        for (_m_node_ptr_t _child
                            : _m_node_traits_t::_s_children(_node))
                        {
                            if ((_res = _s_seek_level_forward(_child, _depth - 1)))
                                return _res;
                        }
                        return nullptr;
                    }
                }

                static constexpr _m_node_ptr_t
                _s_seek_level_reverse(_m_node_ptr_t _node, _m_depth_t _depth)
                {
                    if (_depth == 0)
                        return _node;
                    else
                    {
                        _m_node_ptr_t _res;
                        for (_m_node_ptr_t _child
                            : _m_node_traits_t::_s_children(_node)
                            | std::views::reverse)
                        {
                            if ((_res = _s_seek_level_reverse(_child, _depth - 1)))
                                return _res;
                        }
                        return nullptr;
                    }
                }

                static constexpr _m_node_ptr_t
                _s_advance_forward(_m_node_ptr_t _node)
                {
                    _m_node_ptr_t _tmp;
                    _m_depth_t _depth;
                    if (_m_node_traits_t::_s_is_root(_node))
                        return _m_node_traits_t::_s_first_child(_node);
                    if (_m_node_traits_t::_s_is_last_child(_node))
                    {
                        _depth = _m_node_traits_t::_s_depth(_node);
                        if ((_tmp = _s_find_same_level_forward(_node)))
                            return _tmp;
                        else
                            return _s_seek_at_level_forward(
                                    _m_node_traits_t::_s_seek_root(_node), 
                                    _depth + 1); 
                    }
                    else
                        return _m_node_traits_t::_s_next_sibling(_node);
                }

                static constexpr _m_node_ptr_t
                _s_advance_reverse(_m_node_ptr_t _node)
                {
                    _m_node_ptr_t _tmp;
                    _m_depth_t _depth;
                    if (_m_node_traits_t::_s_is_root(_node))
                        return _m_node_traits_t::_s_last_child(_node);
                    if (_m_node_traits_t::_s_is_first_child(_node))
                    {
                        _depth = _m_node_traits_t::_s_depth(_node);
                        if ((_tmp = _s_find_same_level_reverse(_node)))
                            return _tmp;
                        else
                            return _s_seek_at_level_reverse(
                                    _m_node_traits_t::_s_seek_root(_node), 
                                    _depth + 1); 
                    }
                    else
                        return _m_node_traits_t::_s_prev_sibling(_node);
                }

                constexpr void
                _m_advance_forward()
                { this->_m_node = _s_advance_forward(this->_m_current(), *this); }

                constexpr void
                _m_advance_reverse()
                { this->_m_node = _s_advance_reverse(this->_m_current(), *this); }

                constexpr void
                _m_advance()
                {
                    if constexpr (Reversed)
                        this->_m_advance_reverse();
                    else
                        this->_m_advance_forward();
                }
            };

            template <typename QueueAllocT>
            struct _queued_greedy
            {
                using _m_thunk_t = _m_node_ptr_t;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::vector<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;
                using _m_offset_t 
                    = _m_queue_t::size_type;

                _m_queue_t  _m_queue;
                _m_offset_t _m_offset;

                constexpr 
                _queued_greedy(_m_node_ptr_t _node,
                               const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue(_alloc)
                    , _m_offset(0)
                { this->_m_expand(_node); }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return std::next(this->_m_queue.begin(), this->_m_offset); }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : *this->_m_current_iter(); 
                }

                constexpr void
                _m_expand_forward(_m_node_ptr_t _node)
                {
                    std::queue<_m_queue_t> _queue({_node}, this->_m_queue.get_allocator());
                    while (!_queue.empty())
                    {
                        _m_node_ptr_t _cur = _queue.front();
                        this->_m_queue.emplace_back(_cur); 
                        _queue.pop();
                        for (_m_node_ptr_t _child
                             : _m_node_traits_t::_s_children(_cur))
                            _queue.push(_child);
                    }
                }

                constexpr void
                _m_expand_reverse(_m_node_ptr_t _node)
                {
                    std::queue<_m_queue_t> _queue({_node}, this->_m_queue.get_allocator());
                    while (!_queue.empty())
                    {
                        _m_node_ptr_t _cur = _queue.front();
                        this->_m_queue.emplace_back(_cur); 
                        _queue.pop();
                        for (_m_node_ptr_t _child
                             : _m_node_traits_t::_s_children(_cur)
                             | std::views::reverse)
                            _queue.push(_child);
                    }
                }

                constexpr void
                _m_expand(_m_node_ptr_t _node)
                {
                    if constexpr (Reversed)
                        this->_m_expand_reverse(_node);
                    else
                        this->_m_expand_forward(_node);
                }

                constexpr void
                _m_advance()
                { ++this->_m_offset; }
            };

            template <typename QueueAllocT>
            struct _queued_lazy
            {
                using _m_thunk_t = _m_node_ptr_t;
                using _m_alloc_t = QueueAllocT;
                using _m_queue_alloc_t 
                    = std::allocator_traits<QueueAllocT>::template rebind_alloc<_m_thunk_t>;
                using _m_queue_t 
                    = std::vector<_m_node_ptr_t, QueueAllocT>;
                using _m_queue_iter_t
                    = _m_queue_t::iterator;
                using _m_offset_t
                    = _m_queue_t::size_type;

                _m_queue_t  _m_queue;
                _m_offset_t _m_offset;

                constexpr
                _queued_lazy(_m_node_ptr_t _node,
                             const _m_alloc_t& _alloc = _m_alloc_t())
                    : _m_queue({_node}, _alloc)
                    , _m_offset(0)
                { }

                constexpr _m_queue_iter_t
                _m_current_iter()
                    const noexcept
                { return std::next(this->_m_queue.begin(), this->_m_offset); }

                constexpr _m_node_ptr_t
                _m_current()
                    const noexcept
                { 
                    return this->_m_current_iter() == this->_m_queue.end() 
                            ? nullptr 
                            : *this->_m_current_iter(); 
                }

                constexpr void
                _m_expand_forward()
                {
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(this->_m_current()))
                        this->_m_queue.emplace_back(_child);
                }

                constexpr void
                _m_expand_reverse()
                {
                    for (_m_node_ptr_t _child
                        : _m_node_traits_t::_s_children(this->_m_current())
                        | std::views::reverse)
                        this->_m_queue.emplace_back(_child);
                }

                constexpr void
                _m_expand()
                {
                    if constexpr (Reversed)
                        this->_m_expand_reverse();
                    else
                        this->_m_expand_forward();
                    ++this->_m_offset;
                }
            };

            template <typename QueueAllocT>
            using _m_greedy_queued_base_t = _queued_greedy<QueueAllocT>;
            template <typename QueueAllocT>
            using _m_lazy_queued_base_t   = _queued_lazy<QueueAllocT>;
            using _m_trav_base_t   = _traversing;
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

        template <typename IterT>
        struct _iter_traits;

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
            _m_current()
                const noexcept
            { return this->_m_iter()->_m_current(); }

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
            { return a._m_current() == b._m_current(); }

            /***************************************************
             * @brief value accessors.
             ***************************************************/

            [[nodiscard]]
            constexpr reference 
            operator*()
                const _treelib_noexcept
            {
            #ifdef _treelib_no_exceptions
                assert(this->_m_current() != nullptr)
            #else
                if (this->_m_current() == nullptr)
                    throw std::out_of_range("cannot dereference end-iterator");
                return static_cast<_m_vnode_ptr_t>(this->_m_current())->_m_value();
            #endif
            }

            [[nodiscard]]
            constexpr pointer 
            operator->()
                const _treelib_noexcept
            { return std::addressof(this->operator*()); }

            /***************************************************
             * @brief post-increment operator.
             ***************************************************/

            constexpr _m_iter_t
            operator++(int)
            {
                _m_iter_t _tmp = *this->_m_iter();
                ++(*this->_m_iter());
                return _tmp;
            }
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

            /********************************************************
             * @brief constructor (1).
             *        always constructible from mutable iterator,
             *        maybe from const-iterator.
             *        
             * @details note that the derived-iterator-type is also
             *          a template-variable, because iterators of
             *          the same node-type of any kind (queued/
             *          traversing/leaf/sibling) should all be
             *          convertible to each other, if the constness
             *          allows it.
             ********************************************************/
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
                  traversal Trav,
                  typename NodeT,
                  typename ValueT,
                  typename IterT>
        struct _traversing_iterator_base
            : public _iter_base<IsConst, ValueT, NodeT, IterT>
            , protected _to_traversal_t<Trav, NodeT>::_traversing
        {
        protected:

            using _m_trav_t      = _to_traversal_t<Trav, NodeT>;
            using _m_base_t      = _iter_base<IsConst, ValueT, NodeT, IterT>;
            using _m_trav_base_t = _m_trav_t::_queued;
            using typename _m_base_t::_m_iter_t;

            using _m_trav_base_t::_m_trav_base_t;

        public:  

            constexpr
            _traversing_iterator_base()
                : _m_trav_base_t()
            { }

            constexpr _m_iter_t& 
            operator++()
            { 
                this->_m_advance();
                return *this->_m_iter();
            }

            using _m_base_t::operator++;
        };

        /***************************************************
         * @brief CRTP-base for queued iteration. 
         ***************************************************/
        template <bool IsConst,
                  traversal Trav,
                  typename ValueT,
                  typename NodeT,
                  typename QueueAllocT,
                  typename IterT>
        class _queued_iterator_base
            : public _iter_base<IsConst, ValueT, NodeT, IterT>
            , protected _to_traversal_t<Trav, NodeT>::_queued
        {
        protected:
    
            using _m_trav_t      = _to_traversal_t<Trav, NodeT>;
            using _m_base_t      = _iter_base<IsConst, ValueT, NodeT, IterT>;
            using _m_trav_base_t = _m_trav_t::_queued;
            using typename _m_base_t::_m_iter_t;

            friend _m_base_t;

            using _m_trav_base_t::_m_trav_base_t;

        public:
 
            using allocator_type = _m_trav_base_t::_m_alloc_t;

            constexpr
            _queued_iterator_base(const allocator_type& alloc = allocator_type())
                : _m_trav_base_t(alloc)
            { }

            constexpr _m_iter_t& 
            operator++()
            { 
                this->_m_advance();
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
                  traversal Trav,
                  typename ValueT,
                  typename NodeT,
                  typename QueueAllocT>
        class _queued_iterator;

        template <bool IsConst,
                  traversal Trav,
                  typename ValueT,
                  typename NodeT>
        class _traversing_iterator;

        template <bool IsConst,
                  traversal Trav,
                  typename ValueT,
                  typename NodeT,
                  typename QueueAllocT>
        using _queued_iter_base
            = _convertible_iter<IsConst, ValueT, NodeT,
                _queued_iterator_base<IsConst, Trav, ValueT, NodeT, QueueAllocT,
                  _queued_iterator<IsConst, Trav, ValueT, NodeT, QueueAllocT>>>;

        template <bool IsConst,
                  traversal Trav,
                  typename ValueT,
                  typename NodeT>
        using _traversing_iter_base
            = _convertible_iter<IsConst, ValueT, NodeT,
                _traversing_iterator_base<IsConst, Trav, ValueT, NodeT,
                  _traversing_iterator<IsConst, Trav, ValueT, NodeT>>>;

        /***************************************************
         * @brief iterator that traverses a tree iteratively, 
         *        (no breadth-first). 
         ***************************************************/
        template <bool IsConst,
                  traversal Trav,
                  typename ValueT,
                  typename NodeT>
        struct _traversing_iterator
            : public _traversing_iter_base<IsConst, Trav, ValueT, NodeT>
        {
        protected:

            using _m_base_t      = _traversing_iter_base<IsConst, Trav, ValueT, NodeT>;
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
                  traversal Trav,
                  typename ValueT,
                  typename NodeT,
                  typename QueueAllocT>
        class _queued_iterator
            : public _queued_iter_base<IsConst, Trav, ValueT, NodeT, QueueAllocT>
        {
        protected:

            using _m_base_t      = _queued_iter_base<IsConst, Trav, ValueT, NodeT, QueueAllocT>;
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
            { return _iter._m_current(); }

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
              traversal Trav,
              typename ValueT,
              typename NodeT,
              typename QueueAllocT = std::allocator<ValueT>>
    using greedy_queued_iterator
        = _detail::_queued_iterator<IsConst, Trav, ValueT, NodeT, QueueAllocT>;

    template <bool IsConst, 
              traversal Trav,
              typename ValueT,
              typename NodeT,
              typename QueueAllocT = std::allocator<ValueT>>
    using lazy_queued_iterator
        = _detail::_queued_iterator<IsConst, Trav, ValueT, NodeT, QueueAllocT>;

    template <bool IsConst, 
              traversal Trav,
              typename ValueT,
              typename NodeT,
              typename QueueAllocT = std::allocator<ValueT>>
    using queued_iterator
        = lazy_queued_iterator<IsConst, Trav, ValueT, NodeT, QueueAllocT>;

    template <bool IsConst,
              traversal Trav,
              typename ValueT,
              typename NodeT>
    using traversing_iterator
        = _detail::_traversing_iterator<IsConst, Trav, ValueT, NodeT>;
}

#endif