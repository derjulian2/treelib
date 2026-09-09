
#ifndef TREELIB_BASE_ITERATOR_HPP
#define TREELIB_BASE_ITERATOR_HPP

/***************************************************
 * @file   treelib/detail/base/iterator.hpp
 * @author Julian Benzel
 * @date   03.09.2026
 *
 * @brief  classes to enable various methods of
 *         tree-traversal (depth-first/breadth-first).
 *
 * @details this implementation distinguishes between
 *          'queued'-iterators, which form a queue
 *          of nodes if the user wants to iterate.
 *          
 *          this approach is useful if there are no
 *          means to iteratively traverse a tree
 *          (e.g. outward-trees, which don't have
 *           parent-references and thus cannot jump
 *           back up to higher nodes).
 *
 *          maybe one can also template iterative
 *          traversal??? need to somehow save the
 *          'child'-range and which child is next when
 *          jumping back up tho.
 *         
 ***************************************************/

#include <treelib/detail/base/node.hpp>

#include <iterator>
#include <queue>
#include <stack>

namespace tl
{
    namespace detail
    {  
        /***************************************************
         * @brief CRTP-base-class for common functionality
         *        of tree-iterators. supplies iterator-member-types,
         *        post-increment/decrement and dereferencing.
         ***************************************************/
        template <bool IsConst,
                  typename ValueT,
                  typename NodeT,
                  typename IterT>
        class _basic_iter_mixin
            : public IterT
        {
        protected:

            template <typename T>
            using _m_maybe_const_t = std::conditional_t<IsConst, const T, T>;

            using _m_iter_t       = IterT;
            using _m_value_t      = ValueT;
            using _m_node_t       = NodeT;
            using _m_node_ptr_t   = _m_node_t*;
            using _m_cnode_ptr_t  = const _m_node_t*;
            using _m_value_node_t = detail::_value_node<_m_node_t, _m_value_t>;
            using _m_vnode_ptr_t  = _m_value_node_t*;

            constexpr _m_iter_t*
            _m_iter()
                noexcept
            { return static_cast<_m_iter_t*>(this); }

            constexpr const _m_iter_t*
            _m_iter()
                const noexcept
            { return static_cast<const _m_iter_t*>(this); }

            constexpr _m_node_ptr_t
            _m_cur()
                const
            { return this->_m_iter()->_m_cur(); }

            constexpr void
            _m_set_node(_m_node_ptr_t _node)
            {
                this->_m_iter()->_m_set_node(_node);
            }

            friend _m_iter_t;
            friend class _node_traits<_m_node_t>;

            template <bool, typename, typename, typename>
            friend class _basic_iter_mixin;

        public:

            using _m_iter_t::_m_iter_t;

            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = _m_value_t;
            using pointer           = _m_maybe_const_t<value_type>*;
            using const_pointer     = const value_type*;
            using reference         = _m_maybe_const_t<value_type>&;
            using const_reference   = const value_type&;

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
            _basic_iter_mixin(const _basic_iter_mixin<OtherIsConst, _m_value_t, _m_node_t, OtherIterT>& other)
            { 
                this->_m_set_node(other._m_cur());
            }

            /***************************************************
             * @brief compare two iterators based on their
             *        current node.
             *
             * @details note that the derived-iterator-type is also
             *          a template-variable, because iterators of
             *          the same node-type of any kind (queued/
             *          traversing/leaf/sibling) should all be
             *          convertible to each other, if the constness
             *          allows it.
             ***************************************************/
            template <bool OtherIsConst, typename OtherIterT>
            [[nodiscard]]
            friend constexpr bool
            operator==(const _basic_iter_mixin& a, 
                       const _basic_iter_mixin<OtherIsConst, _m_value_t, _m_node_t, OtherIterT>& b)
                noexcept
            { return a._m_cur() == b._m_cur(); }

            [[nodiscard]]
            constexpr reference 
            operator*()
                const // noexcept(TREELIB_NO_EXCEPTIONS)
            {
            #ifdef TREELIB_NO_EXCEPTIONS
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
                const // noexcept(TREELIB_NO_EXCEPTIONS)
            {
            #ifdef TREELIB_NO_EXCEPTIONS
                assert(this->_m_cur() != nullptr)
            #else
                if (this->_m_cur() == nullptr)
                    throw std::out_of_range("cannot dereference end-iterator");
                return std::addressof(static_cast<_m_vnode_ptr_t>(this->_m_cur())->_m_get_value());
            #endif
            }

            using _m_iter_t::operator++;
            using _m_iter_t::operator--;

            constexpr _m_iter_t
            operator++(int)
            {
                _m_iter_t _tmp = *this;
                ++(*this->_m_iter());
                return _tmp;
            }

            constexpr _m_iter_t
            operator--(int)
            { 
                _m_iter_t _tmp = *this;
                --(*this->_m_iter());
                return _tmp;
            }
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

                for (_m_node_ptr_t& _child :
                     _m_node_traits_t::_s_children(*_first))
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

                for (_m_node_ptr_t& _child :
                     _m_node_traits_t::_s_children(*_first))
                    _queue.push_back(_child);
            }
        };


        template <typename NodeT>
        struct _breadth_first_reverse_order
        {

        };

        template <typename NodeT>
        struct _node_info
        {
            using node_type = NodeT;

            // node_info(detail::_Iter_Base<node_type>& iter)
            // { }
            
        };

        template <typename ValueT,
                  typename NodeT>
        struct _leaf_iterator
        {

        };

        template <typename ValueT,
                  typename NodeT>
        struct _child_iterator
        {

        };

        template <typename ValueT,
                  typename TraversalT>
        struct _traversing_iterator
        {

        };

        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT,
                  typename IterT>
        class _queued_iterator_base
        {
        protected:
    
            using _m_iter_t     = IterT;
            using _m_trav_t     = TraversalT;
            using _m_node_t     = typename _m_trav_t::_m_node_t;
            using _m_node_ptr_t = _m_node_t*;
            using _m_value_t    = ValueT;

            using _m_queue_t      = std::deque<_m_node_ptr_t>;
            using _m_queue_iter_t = typename _m_queue_t::iterator;

            _m_queue_t      _m_queue;
            _m_queue_iter_t _m_queue_cur;
            // keep track of furthest, so no requeue 
            // happens when going backwards and then forwards again
            _m_queue_iter_t _m_queue_furthest; 

            template <bool, typename, typename, typename>
            friend class _basic_iter_mixin;

            constexpr _m_iter_t*
            _m_iter()
                noexcept
            { return static_cast<_m_iter_t*>(this); }

            constexpr _m_iter_t*
            _m_iter()
                const noexcept
            { return static_cast<const _m_iter_t*>(this); }

            constexpr _m_node_ptr_t 
            _m_cur()
                const noexcept
            { 
                return this->_m_queue_cur == this->_m_queue.end()
                       ? nullptr
                       : *this->_m_queue_cur;
            }

            constexpr void
            _m_set_node(_m_node_ptr_t _node)
            {
                this->_m_queue.assign({_node});
                this->_m_queue_cur = this->_m_queue_furthest = this->_m_queue.begin();
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
            {
                return this->_m_queue_cur == this->_m_queue_furthest;
            }

            constexpr explicit
            _queued_iterator_base(_m_node_ptr_t _node)
                : _m_queue({_node})
                , _m_queue_cur(_m_queue.begin())
                , _m_queue_furthest(_m_queue.begin())
            { }

            friend detail::_node_traits<_m_node_t>;

        public:

            using traversal_type  = TraversalT;  

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


        template <bool IsConst,
                  typename ValueT,
                  typename TraversalT>
        class _queued_iterator
            : public _basic_iter_mixin<IsConst, 
                                       ValueT,
                                       typename TraversalT::_m_node_t,
                                       _queued_iterator_base<IsConst, ValueT, TraversalT, _queued_iterator<IsConst, ValueT, TraversalT>>>
        { 
        protected:

            using _m_base_t = _basic_iter_mixin<IsConst, 
                                       ValueT,
                                       typename TraversalT::_m_node_t,
                                       _queued_iterator_base<IsConst, ValueT, TraversalT, _queued_iterator<IsConst, ValueT, TraversalT>>>;

            friend class _node_traits<typename TraversalT::_m_node_t>;

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

    // template <typename TreeType>
    // using depth_first_iterator 
    //     = detail::_depth_first_pre_order_iterator<typename TreeType::node_type>;

    // template <typename TreeType>
    // using depth_first_queued_iterator 
    //     = detail::_depth_first_pre_order_queued_iterator<typename TreeType::node_type>;

    // template <typename TreeType>
    // using breadth_first_iterator 
    //     = detail::_breadth_first_in_order_iterator<typename TreeType::node_type>;

    // template <typename TreeType>
    // using breadth_first_queued_iterator 
    //     = detail::_breadth_first_in_order_queued_iterator<typename TreeType::node_type>;

    // template <typename NodeType>
    // using depth_first = depth_first_pre_order<NodeType>;

    // template <typename NodeType>
    // using breadth_first = breadth_first_in_order<NodeType>;
}

#endif