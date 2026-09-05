
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
        template <typename _NodeT>
        struct _Iter_Base
        {

        };
    }
    
    template <typename NodeType>
    struct node_info
    {
        using node_type = NodeType;

        node_info(detail::_Iter_Base<node_type>& iter)
        { }
        
    };

    template <typename NodeType>
    struct leaf_iterator
    {

    };

    template <typename NodeType>
    struct child_iterator
    {

    };

    // template <typename ValueType, typename NodeType>
    // class traversing_iterator
    // {
    // public:
    //     using node_traits  = tl::_Node_Traits<NodeType>;
    //     using node_type    = typename node_traits::node_type;
    //     using node_pointer = typename node_traits::pointer;
    //     using value_type   = ValueType;
    //     using pointer      = value_type*;
    //     using reference    = value_type&;

    //     operator node_pointer();

    //     friend 
    //     constexpr bool
    //     operator==(const traversing_iterator&, const traversing_iterator&);

    //     reference operator*();
    //     pointer operator->();
    //     traversing_iterator& operator++();
    // };

    enum struct traversal_strategy
    {
        depth_first_pre_order,
        depth_first_post_order,
        breadth_first_in_order,
        breadth_first_reverse_order
    };


    /**
     * @brief traversal-type to be used in tl::queued_iterator
     *        and tl::iterator when the iteration-strategy
     *        is dependent on a runtime-condition.
     *
     *        dispatches the .next()-calls based on the
     *        value of a 'strategy'-member-field.
     */
    template <typename NodeType>
    struct dynamic_traversal
    {
        traversal_strategy m_strategy;
    };


    /**
     * @brief requirements for types that provide the 
     *        traversing-algorithms to be used in queued-iterators.
     */
    template <typename T>
    concept queued_traversal_provider = requires ()
    {
        typename T::node_type;
        { T::enqueue(std::declval<typename T::node_type&>()) } 
        -> std::convertible_to<typename T::node_pointer>;
    };


    /**
    * @brief requirements for types that provide the 
     *       traversing-algorithms to be used in traversing-iterators.
     */
    template <typename T>
    concept iterative_traversal_provider = requires ()
    {
        typename T::node_type;
        { T::next(std::declval<typename T::node_type&>()) } 
        -> std::convertible_to<typename T::node_pointer>;
        { T::prev(std::declval<typename T::node_type&>()) } 
        -> std::convertible_to<typename T::node_pointer>;
    };


    template <typename ValueType, 
              typename TraversalType>
    class queued_iterator
    {
    public:

        using traversal_type  = TraversalType;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ValueType;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using const_reference   = const value_type&;

    protected:

        using _M_node_t       = typename traversal_type::node_type;
        using _M_node_ptr_t   = _M_node_t*;
        using _M_value_node_t = detail::_Value_Node<_M_node_t, value_type>;
        using _M_vnode_ptr_t  = _M_value_node_t*;

        using _M_queue_t      = std::deque<_M_node_ptr_t>;
        using _M_queue_iter_t = typename _M_queue_t::iterator;

        _M_node_ptr_t   _M_source;
        _M_queue_t      _M_queue;
        _M_queue_iter_t _M_iter;


        constexpr _M_node_ptr_t 
        _M_cur()
            noexcept
        { 
            return this->_M_queue.empty()
                    ? nullptr
                    : *this->_M_iter;      
        }

        constexpr void
        _M_enqueue()
        {
            this->_M_queue = traversal_type::enqueue(this->_M_source);
            this->_M_iter  = std::ranges::begin(this->_M_queue);
        }

        constexpr bool
        _M_should_enqueue()
            const noexcept
        {
            return this->_M_iter == _M_queue_iter_t();
        }

    public:

        constexpr
        queued_iterator()
            : _M_source(nullptr)
            , _M_queue()
            , _M_iter()
        { }

        constexpr
        explicit queued_iterator(_M_node_ptr_t node)
            : _M_source(node)
            , _M_queue()
            , _M_iter()
        { }

        constexpr void
        refresh()
        { this->_M_enqueue(); }

        friend 
        constexpr bool
        operator==(const queued_iterator& a, const queued_iterator& b)
            noexcept
        { return a._M_cur() == b._M_cur(); }

        constexpr reference 
        operator*()
        {
        #ifdef TREELIB_NO_EXCEPTIONS
            assert(this->_M_cur() != nullptr)
        #else
            if (this->_M_cur() == nullptr)
                throw std::out_of_range("cannot dereference end-iterator");
            return static_cast<_M_vnode_ptr_t>(this->_M_cur())->_M_get_value();
        #endif
        }

        constexpr pointer 
        operator->()
        {
        #ifdef TREELIB_NO_EXCEPTIONS
            assert(this->_M_cur() != nullptr)
        #else
            if (this->_M_cur() == nullptr)
                throw std::out_of_range("invalid iterator-dereference");
            return std::addressof(static_cast<_M_vnode_ptr_t>(this->_M_cur())->_M_get_value());
        #endif
        }

        constexpr queued_iterator& 
        operator++()
        { 
            if (this->_M_should_enqueue())
            { this->_M_enqueue(); }
            ++this->_M_iter;
            return *this;
        }

        constexpr queued_iterator& 
        operator--()
        { 
            if (this->_M_should_enqueue())
            { this->_M_enqueue(); }
            --this->_M_iter;
            return *this; 
        }

        constexpr queued_iterator
        operator++(int)
        {
            queued_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr queued_iterator
        operator--(int)
        { 
            queued_iterator tmp = *this;
            --(*this);
            return tmp;
        }
    };


    template <typename NodeType>
    struct depth_first_pre_order
    {
        using node_traits  = tl::detail::_Node_Traits<NodeType>;
        using node_type    = typename node_traits::node_type;
        using node_pointer = typename node_traits::pointer;

        /**
         * @brief constructs an iterable sequence of node-pointers
         *        that when iterated over represents the
         *        depth-first-pre-order traversal of the underlying tree.
         */
        template <typename ResultType = std::deque<node_pointer>,
                  typename SequenceType = std::deque<node_pointer>>
        static constexpr 
        ResultType
        enqueue(node_pointer src)
        {
            ResultType result;
            std::stack<node_pointer, SequenceType> stack;
            
            stack.push(src);
            while (!stack.empty())
            {
                node_pointer cur = stack.top();
                result.push_back(cur);
                stack.pop();

                for (node_pointer& c 
                     : node_traits::_S_children(cur))
                {
                    stack.push(c);
                }
            }
            return result;
        }
    };

    //     /**
    //  *
    //  */
    // template <typename NodeType>
    // struct breadth_first_in_order
    // {
    //     using node_traits  = tl::_Node_Traits<NodeType>;
    //     using node_type    = typename node_traits::node_type;
    //     using node_pointer = typename node_traits::pointer;

    //     /**
    //      * @brief constructs an iterable sequence of node-pointers
    //      *        that when iterated over represents the
    //      *        depth-first-pre-order traversal of the underlying tree.
    //      */
    //     template <node_kind NodeKind,
    //               typename ContainerType,
    //               typename SequenceType = std::deque<node_pointer>>
    //     static constexpr 
    //     void
    //     enqueue(std::back_insert_iterator<ContainerType> dest,
    //             node_pointer root)
    //     {
    //         std::queue<node_pointer, SequenceType> queue;
            
    //         queue.push(root);
    //         while (!queue.empty())
    //         {
    //             node_pointer& cur = queue.front();
    //             *dest = cur;
    //             queue.pop();

    //             for (node_pointer& c 
    //                  : node_traits::template neighbours<NodeKind>(*cur))
    //             { queue.push(c); }
    //         }
    //     }
    // };


    // template <typename NodeType>
    // struct breadth_first_reverse_order
    // {
    //     using node_traits  = tl::_Node_Traits<NodeType>;
    //     using node_type    = typename node_traits::node_type;
    //     using node_pointer = typename node_traits::pointer;

    //     /**
    //      * @brief constructs an iterable sequence of node-pointers
    //      *        that when iterated over represents the
    //      *        depth-first-pre-order traversal of the underlying tree.
    //      */
    //     template <node_kind NodeKind,
    //               typename ContainerType,
    //               typename SequenceType = std::deque<node_pointer>>
    //     static constexpr 
    //     void
    //     enqueue(std::back_insert_iterator<ContainerType> dest,
    //             node_pointer root)
    //     {
    //         std::queue<node_pointer, SequenceType> queue;
            
    //         queue.push(root);
    //         while (!queue.empty())
    //         {
    //             node_pointer& cur = queue.front();
    //             *dest = cur;
    //             queue.pop();
    //             for (node_pointer& c 
    //                  : node_traits::template neighbours<NodeKind>(*cur)
    //                  | std::views::reverse)
    //             { queue.push(c); }
    //         }
    //     }
    // };
}

#endif