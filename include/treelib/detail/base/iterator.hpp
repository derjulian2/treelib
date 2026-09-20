
#ifndef TREELIB_BASE_ITERATOR_HPP
#define TREELIB_BASE_ITERATOR_HPP

/***************************************************
 * @file   treelib/detail/base/iterator.hpp
 * @author Julian Benzel
 * @date   14.09.2026
 *
 * @brief  classes to enable various methods of
 *         tree-traversal (depth-first/breadth-first)
 *         between instances of a node-type.
 *
 * @todo   - other iterator-types
 *         - depth-first in-order/post-order traversals
 *         - tree-meta info struct 'node_info'
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/traits.hpp>

#include <iterator>
#include <list>
#include <algorithm>
#include <cassert>

namespace tl
{
    /***************************************************
     * @brief traversal-selection interface.
     ***************************************************/

    enum struct traversal
        // somehow convert a value here to a traversal-type
    {
        depth_first_pre_order,
        depth_first_in_order,
        depth_first_post_order,
        depth_first_reverse_pre_order,
        depth_first_reverse_in_order,
        depth_first_reverse_post_order,
        depth_first,

        breadth_first_in_order,
        breadth_first_reverse_order,
        breadth_first
    };

    namespace _detail
    {  
        /***************************************************
         * @brief type that is stored within the queue
         *        of queued iterators. contains a node-ptr
         *        and a boolean flag, indicating if this
         *        node was already expanded or not.
         ***************************************************/
        template <typename NodeT>
        struct _queue_thunk
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;
            using _m_flag_t        = bool;

            _m_node_ptr_t _m_node;
            _m_flag_t     _m_expanded;

            constexpr
            _queue_thunk()
                : _m_node(nullptr)
                , _m_expanded(false)
            { }

            constexpr explicit
            _queue_thunk(_m_node_ptr_t _node)
                : _m_node(_node)
                , _m_expanded(false)
            { }

            constexpr void
            _m_set()
                noexcept
            { this->_m_expanded = true; }
        };


        /***************************************************
         * @brief traversal-type for depth-first-pre-order.
         *        
         * @details depth-first-pre-order traverses the
         *          current-node first and then moves on
         *          to the child-nodes afterwards.  
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _depth_first_pre_order
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            using _m_queue_t = std::list<_m_thunk_t>;
            using _m_iter_t  = typename _m_queue_t::iterator;

            struct _traversing_iter_state
            { };
            
            using _m_iter_state_t = _traversing_iter_state;

            template <typename IterT>
            static constexpr IterT
            _s_root_begin(_m_node_ptr_t _node)
                noexcept(std::is_nothrow_copy_constructible_v<IterT>)
            { return _iter_traits<IterT>::_s_to_iter(_node); }

            template <typename IterT>
            static constexpr IterT
            _s_header_begin(_m_node_ptr_t _node)
            { return std::next(_iter_traits<IterT>::_s_to_iter(_node)); }

            /***************************************************
             * @brief   determines the next node for
             *          iterative-traversal.
             ***************************************************/
            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_next(IterT& _iter)
                requires _parent_node<_m_node_t>
            {
                _m_node_ptr_t _node = _iter_traits<IterT>::_s_to_node(_iter);
                if constexpr (Reversed)
                {
                    if (_m_node_traits_t::_s_has_children(_node))
                        return _m_node_traits_t::_s_last_child(_node);
                    while (!(_node = _m_node_traits_t::_s_prev_sibling(_node)))   
                    {
                        _node = _m_node_traits_t::_s_parent(_node);
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                    }
                    return _node;
                }
                else
                {
                    if (_m_node_traits_t::_s_has_children(_node))
                        return _m_node_traits_t::_s_first_child(_node);
                    while (!(_node = _m_node_traits_t::_s_next_sibling(_node)))   
                    {
                        _node = _m_node_traits_t::_s_parent(_node);
                        if (_m_node_traits_t::_s_is_root(_node))
                            return nullptr;
                    }
                    return _node;
                }
            }

            /***************************************************
             * @brief   determines the previous node for
             *          iterative-traversal.
             ***************************************************/
            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_prev(IterT& _iter)
                requires _parent_node<_m_node_t>
            {
                _m_node_ptr_t _node = _iter_traits<IterT>::_s_to_node(_iter);
            
                // static_assert(false, "reverse-iteration not implemented");
                if constexpr (Reversed)
                {

                }
                else
                {

                }
                return _node;
            }

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          to it's children and inserts them
             *          just after '_first', forming the depth-
             *          first-traversal, node-by-node.
             *
             * @returns an iterator to the next node in the
             *          traversal-sequence.
             ***************************************************/
            static constexpr _m_iter_t
            _s_expand_queue(_m_iter_t _first, _m_queue_t& _queue)
            {
                _first->_m_set();
                _m_iter_t _next = std::next(_first);
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
        };


        /***************************************************
         * @brief traversal-type for depth-first-post-order.
         *        
         * @details depth-first-post-order traverses the
         *          current-node's children first, and then 
         *          moves on to the current-node afterwards.
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _depth_first_post_order
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            using _m_queue_t = std::list<_m_thunk_t>;
            using _m_iter_t  = typename _m_queue_t::iterator;

            struct _traversing_iter_state
            {
                using _m_flag_t = bool;

                /***************************************************
                 * idea by kpeeter's post-order-iterator at
                 * https://github.com/kpeeters/tree.hh
                 ***************************************************/
                _m_flag_t _m_skip_children;
            };
            
            using _m_iter_state_t = _traversing_iter_state;

            template <typename IterT>
            static constexpr IterT
            _s_root_begin(_m_node_ptr_t _node)
                noexcept(std::is_nothrow_copy_constructible_v<IterT>)
            { return std::next(_iter_traits<IterT>::_s_to_iter(_node)); }

            template <typename IterT>
            static constexpr IterT
            _s_header_begin(_m_node_ptr_t _node)
            { return std::next(_iter_traits<IterT>::_s_to_iter(_node)); }

            /***************************************************
             * @brief   determines the next node for
             *          iterative-traversal.
             ***************************************************/
            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_next(IterT& _iter)
                requires _parent_node<_m_node_t>
            {
                _m_node_ptr_t _node = _iter_traits<IterT>::_s_to_node(_iter);
                _m_node_ptr_t _next;
                if constexpr (Reversed)
                {
                    
                }
                else
                {
                    if (!_m_node_traits_t::_s_is_leaf(_node) && !_iter._m_skip_children)
                        return _m_node_traits_t::_s_seek_leftmost(_node);
                    if ((_next = _m_node_traits_t::_s_next_sibling(_node)))
                    {
                        _iter._m_skip_children = false; 
                        return _next; 
                    }
                    _iter._m_skip_children = true;
                    return _m_node_traits_t::_s_parent(_node);
                }   
            }

            /***************************************************
             * @brief   determines the previous node for
             *          iterative-traversal.
             ***************************************************/
            template <typename IterT>
            static constexpr _m_node_ptr_t
            _s_prev(IterT& _iter)
                requires _parent_node<_m_node_t>
            {
                _m_node_ptr_t _node = _iter_traits<IterT>::_s_to_node(_iter);
                // static_assert(false, "reverse-iteration not implemented");
                if constexpr (Reversed)
                {

                }
                else
                {

                }
                return _node;
            }

            /***************************************************
             * @brief if _node is the last child of it's
             *        sibling-chain, the correct post-order
             *        traversal may require the next node
             *        to be expanded before handing off to
             *        the parent-node.
             ***************************************************/
            static constexpr _m_iter_t
            _s_maybe_expand_next(_m_iter_t _node, _m_queue_t& _queue)
            {
                _m_iter_t _maybe_parent = std::next(_node);
                if (_maybe_parent != _queue.end()
                    && _m_node_traits_t::_s_is_last_child_of(_maybe_parent->_m_node, _node->_m_node))
                {
                    _m_iter_t _unexpanded = std::find_if(std::next(_maybe_parent), _queue.end(),
                                                         [](const _m_thunk_t& _t) { return !_t._m_expanded; });
                    if (_unexpanded != _queue.end())
                        _s_expand_queue(_unexpanded, _queue);
                }
                return _maybe_parent;
            }

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          recursively to it's children until a
             *          leaf-node is found.
             *
             * @returns an iterator to the next node in the
             *          traversal-sequence.
             ***************************************************/
            static constexpr _m_iter_t
            _s_expand_queue(_m_iter_t _first, _m_queue_t& _queue)
            {
                _first->_m_set();
                if (_m_node_traits_t::_s_is_leaf(_first->_m_node))
                    return _s_maybe_expand_next(_first, _queue);

                // capture iterator to first-child for recursive search
                bool _first_child = true;
            #define _treelib_insert_capture_first(_varname, _where) \
                { if (_first_child) \
                { _varname = _queue.emplace(_where, _child); _first_child = false; } \
                else \
                { _queue.emplace(_where, _child); } }
                
                _m_iter_t _res;
                if constexpr (Reversed)
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node)
                         | std::views::reverse)
                        _treelib_insert_capture_first(_res, _first)
                else
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node))
                        _treelib_insert_capture_first(_res, _first)
        
            #undef _treelib_capture_first
                // keep expanding until first-child is a leaf
                if (!_m_node_traits_t::_s_is_leaf(_res->_m_node))
                    return _s_expand_queue(_res, _queue);
                return _res;
            }
        };


        /***************************************************
         * @brief traversal-type for depth-first-in-order.
         *        
         * @details depth-first-in-order traverses the
         *          first half of current-node's children first, 
         *          then the current node, and then the other
         *          half afterwards.
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _depth_first_in_order
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            using _m_queue_t = std::list<_m_thunk_t>;
            using _m_iter_t  = typename _m_queue_t::iterator;

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          to 
             *
             * @returns an iterator to the next node in the
             *          traversal-sequence.
             ***************************************************/
            static constexpr _m_iter_t
            _s_expand_queue(_m_iter_t _first, _m_queue_t& _queue)
            {
                if (_queue.empty())
                    return _first;
                
                std::size_t _child_count 
                    = _m_node_traits_t::_s_child_count(_first->_m_node);
                std::size_t _half      = _child_count / 2;

                _first->_m_set();
                _m_iter_t _res = _first;
                if constexpr (Reversed)
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(*_first)
                         | std::views::reverse)
                        _res = _queue.insert(_res, _m_thunk_t(_child));
                else
                    for (_m_node_ptr_t _child :
                         _m_node_traits_t::_s_children(_first->_m_node))
                    {
                        if (_child_count > _half)
                            _res = _queue.insert(_res, _m_thunk_t(_child));
                        else
                            _queue.insert(++_first, _m_thunk_t(_child));
                        --_child_count;
                    }
                // recursively insert, because in-order
                // visits the deepest first
                return _s_expand_queue(_res, _queue);
            }
        };


        /***************************************************
         * @brief traversal-type for breadth-first.
         ***************************************************/
        template <bool Reversed,
                  typename NodeT>
        struct _breadth_first
        {
            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;

            using _m_thunk_t = _queue_thunk<_m_node_t>;
            using _m_queue_t = std::list<_m_thunk_t>;
            using _m_iter_t  = typename _m_queue_t::iterator;

            template <typename IterT>
            static constexpr IterT
            _s_root_begin(_m_node_ptr_t _node)
                noexcept(std::is_nothrow_copy_constructible_v<IterT>)
            { return _iter_traits<IterT>::_s_to_iter(_node); }

            template <typename IterT>
            static constexpr IterT
            _s_header_begin(_m_node_ptr_t _node)
            { return std::next(_iter_traits<IterT>::_s_to_iter(_node)); }

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          to it's children and inserts them
             *          at the back of the queue, forming the 
             *          breadth-first-traversal, node-by-node.
             *
             * @returns an iterator to the next node in the
             *          traversal-sequence.
             ***************************************************/
            static constexpr _m_iter_t
            _s_expand_queue(_m_iter_t _first, _m_queue_t& _queue)
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

            using iterator_category = std::bidirectional_iterator_tag;
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
                return static_cast<_m_vnode_ptr_t>(this->_m_cur())->_m_get_value();
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

            constexpr _m_iter_t
            operator--(int)
            { 
                _m_iter_t _tmp = *this->_m_iter();
                --(*this->_m_iter());
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


        /***************************************************
         * @brief CRTP-base for traversing-iteration.
         *        
         * @note  some traversal-methods require more
         *        state than others (some booleans or pointers
         *        to certain points), this class inherits
         *        these states based on the traversal-type.
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
                  typename IterT>
        struct _traversing_iterator_base
            : public _iter_base<IsConst, ValueT, typename TraversalT::_m_node_t, IterT>
            , protected TraversalT::_m_iter_state_t
        {
            using _m_trav_t = TraversalT;
            using _m_base_t = _iter_base<IsConst, ValueT, typename TraversalT::_m_node_t, IterT>;
            using typename _m_base_t::_m_iter_t;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;

            friend _m_base_t;
            friend _m_trav_t;

            _m_node_ptr_t _m_node;

            static constexpr _m_iter_t
            _s_root_begin(_m_node_ptr_t _node)
            { return _m_trav_t::template _s_root_begin<_m_iter_t>(_node); }

            static constexpr _m_iter_t
            _s_header_begin(_m_node_ptr_t _node)
            { return _m_trav_t::template _s_header_begin<_m_iter_t>(_node); }

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
                this->_m_node = _m_trav_t::_s_next(*this);
                return *this->_m_iter();
            }

            constexpr _m_iter_t& 
            operator--()
            { 
                this->_m_node = _m_trav_t::_s_prev(*this);
                return *this->_m_iter(); 
            }
        };


        /***************************************************
         * @brief CRTP-base for queued iteration. 
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
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
            using _m_queue_t /* this type is a cutie */ 
                = std::list<_m_thunk_t>;
            using _m_queue_iter_t = typename _m_queue_t::iterator;

            friend _m_base_t;

            _m_queue_t      _m_queue;
            _m_queue_iter_t _m_queue_cur;

            static constexpr _m_iter_t
            _s_root_begin(_m_node_ptr_t _node)
            { return _m_trav_t::template _s_root_begin<_m_iter_t>(_node); }

            static constexpr _m_iter_t
            _s_header_begin(_m_node_ptr_t _node)
            { 
                _m_iter_t _res = _m_trav_t::template _s_header_begin<_m_iter_t>(_node);
                std::erase_if(_res._m_queue, [&](const _m_thunk_t& _t) { return _t._m_node == _node; });
                return _res;
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
            _m_enqueue()
            {
                this->_m_queue_cur
                    = _m_trav_t::_s_expand_queue(this->_m_queue_cur, this->_m_queue);
            }

            constexpr bool
            _m_should_enqueue()
                const noexcept
            { return !this->_m_queue_cur->_m_expanded; }

            constexpr explicit
            _queued_iterator_base(_m_node_ptr_t _node)
                : _m_queue({_m_thunk_t(_node)})
                , _m_queue_cur(_m_queue.begin())
            { }

        public:

            using traversal_type = _m_trav_t;  

            constexpr
            _queued_iterator_base()
                : _m_queue()
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

            constexpr _m_iter_t& 
            operator--()
            { 
                --this->_m_queue_cur;
                return *this->_m_iter(); 
            }
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
                  typename TraversalT>
        class _queued_iterator;

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT>
        using _queued_iter_base
            = _convertible_iter<IsConst, ValueT, typename TraversalT::_m_node_t,
                _queued_iterator_base<IsConst, ValueT, TraversalT,
                  _queued_iterator<IsConst, ValueT, TraversalT>>>;

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT>
        class _traversing_iterator;

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT>
        using _traversing_iter_base
            = _convertible_iter<IsConst, ValueT, typename TraversalT::_m_node_t,
                _traversing_iterator_base<IsConst, ValueT, TraversalT,
                  _traversing_iterator<IsConst, ValueT, TraversalT>>>;


        /***************************************************
         * @brief iterator that traverses a tree iteratively,
         *        (without forming a queue) (within limits). 
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
                  typename TraversalT>
        class _queued_iterator
            : public _queued_iter_base<IsConst, ValueT, TraversalT>
        {
        protected:

            using _m_base_t      = _queued_iter_base<IsConst, ValueT, TraversalT>;
            using _m_iter_base_t = typename _m_base_t::_m_base_t;
            using typename _m_base_t::_m_iter_t;

            friend _iter_traits<_m_iter_t>;
            friend _m_iter_base_t;

        public:

            using _m_base_t::_m_base_t;
        };
           

        // template <typename NodeT>
        // using _depth_first_pre_order_iterator 
        //     = _traversing_iterator<_depth_first_pre_order<NodeT>>;

        // template <typename NodeT>
        // using _depth_first_pre_order_queued_iterator 
        //     = _queued_iterator<_depth_first_pre_order<NodeT>>;

        // template <typename NodeT>
        // using _breadth_first_in_order_iterator 
        //     = _traversing_iterator<_breadth_first_in_order<NodeT>>;

        // template <typename NodeT>
        // using _breadth_first_in_order_queued_iterator 
        //     = _queued_iterator<_breadth_first_in_order<NodeT>>;
    }

    /***************************************************
     * @brief iterator-type aliases.
     *
     * @note  the ugly template-parameters
     *        should get deduced when constructing these
     *        types from an existing iterator.
     *
     *        these aliases exist only for the convenience of
     *        of not having to prefix the tree-type
     *        when wanting to specify an iterator.
     ***************************************************/

    template <bool IsConst, typename ValueT, typename TraversalT>
    using queued_iterator
        = _detail::_queued_iterator<IsConst, ValueT, TraversalT>;

    template <bool IsConst, typename ValueT, typename NodeT>
    using queued_depth_first_iterator
        = queued_iterator<IsConst, ValueT, _detail::_depth_first_pre_order<false, NodeT>>;

    template <bool IsConst, typename ValueT, typename NodeT>
    using queued_depth_first_post_order_iterator
        = queued_iterator<IsConst, ValueT, _detail::_depth_first_post_order<false, NodeT>>;

    // template <typename NodeT>
    // using depth_first_queued_iterator 
    //     = _detail::_depth_first_pre_order_queued_iterator<typename TreeType::node_type>;

    template <bool IsConst, typename ValueT, typename NodeT>
    using queued_breadth_first_iterator
        = queued_iterator<IsConst, ValueT, _detail::_breadth_first<false, NodeT>>;

    // template <typename NodeT>
    // using breadth_first_queued_iterator 
    //     = _detail::_breadth_first_in_order_queued_iterator<typename TreeType::node_type>;
}

#endif