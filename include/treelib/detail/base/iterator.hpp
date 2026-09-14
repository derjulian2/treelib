
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
 * @todo   - traversal-types duplicate just because
 *           of reverse-orders, maybe refactor.
 *         - other iterator-types
 *         - tree-meta info struct 'node_info'
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/traits.hpp>

#include <iterator>
#include <deque>

namespace tl
{
    namespace _detail
    {  
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
            {
                return std::addressof(this->operator*());
            }

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
         * @brief traversal-type for depth-first-pre-order.
         ***************************************************/
        template <typename NodeT>
        struct _depth_first_pre_order
        {
            using _m_node_t = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            using _m_node_ptr_t = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          to it's children and inserts them
             *          just after '_first', forming the depth-
             *          first-traversal, node-by-node.
             ***************************************************/
            static constexpr void
            _s_expand_queue(std::deque<_m_node_ptr_t>::iterator _first,
                            std::deque<_m_node_ptr_t>& _queue)
            {
                if (_queue.empty())
                    return;

                for (_m_node_ptr_t _child :
                     _m_node_traits_t::_s_children(*_first))
                    // 'drag' the iterator one down after each insert,
                    // so the order doesn't get reversed
                    _first = _queue.insert(std::next(_first), _child);
            }
        };


        /***************************************************
         * @brief traversal-type for depth-first-post-order.
         *         
         *        reverses the order in which children are
         *        enqueued, forming the mirrored traversal
         *        of depth-first-pre-order.
         ***************************************************/
        template <typename NodeT>
        struct _depth_first_post_order
        {
            using _m_node_t = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            using _m_node_ptr_t = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          to it's children and inserts them
             *          just after '_first', forming the depth-
             *          first-traversal, node-by-node.
             ***************************************************/
            static constexpr void
            _s_expand_queue(std::deque<_m_node_ptr_t>::iterator _first,
                            std::deque<_m_node_ptr_t>& _queue)
            {
                if (_queue.empty())
                    return;

                for (_m_node_ptr_t _child :
                     _m_node_traits_t::_s_children(*_first)
                     | std::views::reverse)
                    // 'drag' the iterator one down after each insert,
                    // so the order doesn't get reversed
                    _first = _queue.insert(std::next(_first), _child);
            }
        };


        /***************************************************
         * @brief traversal-type for breadth-first-in-order.
         ***************************************************/
        template <typename NodeT>
        struct _breadth_first_in_order
        {
            using _m_node_t = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            using _m_node_ptr_t = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          to it's children and inserts them
             *          at the back of the queue, forming the 
             *          breadth-first-traversal, node-by-node.
             ***************************************************/
            static constexpr void
            _s_expand_queue(std::deque<_m_node_ptr_t>::iterator _first,
                            std::deque<_m_node_ptr_t>& _queue)
            {
                if (_queue.empty())
                    return;

                for (_m_node_ptr_t _child :
                     _m_node_traits_t::_s_children(*_first))
                    _queue.push_back(_child);
            }
        };


        /***************************************************
         * @brief traversal-type for breadth-first-reverse-order.
         *
         *        reverses the order in which children are
         *        enqueued, forming the mirrored traversal
         *        of breadth-first-in-order.
         ***************************************************/
        template <typename NodeT>
        struct _breadth_first_reverse_order
        {
            using _m_node_t = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;

            using _m_node_ptr_t = typename _m_node_traits_t::_m_ptr_t;

            /***************************************************
             * @brief   expands the node pointed to by '_first'
             *          to it's children and inserts them
             *          at the back of the queue, forming the 
             *          breadth-first-traversal, node-by-node.
             *
             *          differs from in-order in the reversed
             *          order in which children are added.
             ***************************************************/
            static constexpr void
            _s_expand_queue(std::deque<_m_node_ptr_t>::iterator _first,
                            std::deque<_m_node_ptr_t>& _queue)
            {
                if (_queue.empty())
                    return;

                for (_m_node_ptr_t _child :
                     _m_node_traits_t::_s_children(*_first)
                     | std::views::reverse)
                    _queue.push_back(_child);
            }
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
         * @brief iterator that traverses a tree iteratively,
         *        (without forming a queue) (within limits). 
         ***************************************************/
        template <typename ValueT,
                  typename TraversalT>
        struct _traversing_iterator
        {

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

            using _m_queue_t /* this type is a cutie */ 
                = std::deque<_m_node_ptr_t>;
            using _m_queue_iter_t 
                = typename _m_queue_t::iterator;

            _m_queue_t      _m_queue;
            _m_queue_iter_t _m_queue_cur;
            // keep track of furthest, so no requeue 
            // happens when going backwards and then forwards again
            _m_queue_iter_t _m_queue_furthest; 


            constexpr _m_node_ptr_t 
            _m_cur()
                const noexcept
            { 
                return this->_m_queue_cur == this->_m_queue.end()
                       ? nullptr
                       : *this->_m_queue_cur;
            }

            constexpr void
            _m_enqueue_and_advance()
            {
                _m_trav_t::_s_expand_queue(this->_m_queue_cur, this->_m_queue);
                this->_m_queue_furthest = ++this->_m_queue_cur;
            }

            constexpr bool
            _m_should_enqueue()
                const noexcept
            { return this->_m_queue_cur == this->_m_queue_furthest; }

            constexpr explicit
            _queued_iterator_base(_m_node_ptr_t _node)
                : _m_queue({_node})
                , _m_queue_cur(_m_queue.begin())
                , _m_queue_furthest(_m_queue.begin())
            { }

        public:

            using traversal_type = _m_trav_t;  

            constexpr
            _queued_iterator_base()
                : _m_queue()
                , _m_queue_cur(this->_m_queue.end())
                , _m_queue_furthest(this->_m_queue.end())
            { }

            constexpr _m_iter_t& 
            operator++()
            { 
                if (this->_m_should_enqueue())
                    this->_m_enqueue_and_advance();
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

            using _m_base_t = _queued_iter_base<IsConst, ValueT, TraversalT>;
            using typename _m_base_t::_m_iter_t;

            friend _iter_traits<_m_iter_t>;

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
     * @brief traversal-type aliases.
     ***************************************************/

    template <typename NodeT>
    using depth_first
        = _detail::_depth_first_pre_order<NodeT>;

    template <typename NodeT>
    using breadth_first
        = _detail::_breadth_first_in_order<NodeT>;

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

    // template <bool IsConst, typename ValueT, typename TraversalT>
    // using queued_iterator
    //     = _detail::_queued_iterator<>;

    // template <bool IsConst, typename ValueT, typename NodeT>
    // using depth_first_queued_iterator 
    //     = _detail::;

    // template <typename NodeT>
    // using depth_first_queued_iterator 
    //     = _detail::_depth_first_pre_order_queued_iterator<typename TreeType::node_type>;

    // template <typename NodeT>
    // using breadth_first_iterator 
    //     = _detail::_breadth_first_in_order_iterator<typename TreeType::node_type>;

    // template <typename NodeT>
    // using breadth_first_queued_iterator 
    //     = _detail::_breadth_first_in_order_queued_iterator<typename TreeType::node_type>;
}

#endif