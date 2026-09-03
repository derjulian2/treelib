
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

namespace tl
{
    namespace detail
    {

    }
    
    template <typename ValueType, typename NodeType>
        requires strong_node<NodeType>
    class traversing_iterator
    {
    public:
        using node_traits  = tl::node_traits<NodeType>;
        using node_type    = typename node_traits::node_type;
        using node_pointer = typename node_traits::pointer;
        using value_type   = ValueType;
        using pointer      = value_type*;
        using reference    = value_type&;

        operator node_pointer();

        friend 
        constexpr bool
        operator==(const traversing_iterator&, const traversing_iterator&);

        reference operator*();
        pointer operator->();
        traversing_iterator& operator++();
    };

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
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ValueType;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using const_reference   = const value_type&;
    // private:
        using traversal_type = TraversalType;
        using node_type       = typename traversal_type::node_type;
        using value_node_type = value_node<value_type, node_type>;
        using node_traits     = tl::node_traits<node_type>;
        using node_pointer    = typename node_traits::pointer;
        using queue_type      = std::deque<node_pointer>;

        node_pointer m_source;
        queue_type   m_queue;


        queued_iterator()
            : m_source(nullptr)
            , m_queue()
        { }

        
        queued_iterator(node_pointer node)
            : m_source(node)
            , m_queue()
        { this->m_queue.push_front(this->m_source); }

    
        bool
        should_enqueue()
        { return this->m_queue.size() == 1 && this->current_node() == this->m_source; }


        node_pointer
        current_node() const noexcept
        { return this->m_queue.empty() ? nullptr : this->m_queue.front(); }


        operator node_pointer()
        { return this->current_node(); }

    public:


        /**
         * @brief forces renewal of the internal
         *        node-queue to reflect changes of
         *        made in the tree during iteration.
         */
        void
        enqueue()
        {
            this->m_queue = traversal_type::enqueue(this->m_source);
        }


        friend 
        constexpr bool
        operator==(const queued_iterator& a, const queued_iterator& b)
        { return a.current_node() == b.current_node(); }


        reference operator*()
        {
        #ifdef TREELIB_NO_EXCEPTIONS
            assert(this->current_node() != nullptr)
        #else
            if (this->current_node() == nullptr)
            { throw std::out_of_range("invalid iterator-dereference"); }
            return static_cast<value_node_type*>(this->current_node())->value();
        #endif
        }


        pointer operator->()
        {
        #ifdef TREELIB_NO_EXCEPTIONS
            assert(this->current_node() != nullptr)
        #else
            if (this->current_node() == nullptr)
            { throw std::out_of_range("invalid iterator-dereference"); }
            return std::addressof(static_cast<value_node_type*>(this->current_node())->value());
        #endif
        }


        queued_iterator& operator++()
        { 
            if (this->should_enqueue())
            { this->enqueue(); }
            else
            { this->m_queue.pop_front(); }
            return *this;
        }


        // queued_iterator& operator--()
        // { 
        //     if (this->should_enqueue())
        //     { this->enqueue(); }
        //     return *this; 
        // }


        queued_iterator
        operator++(int)
        { 
            /* return (potentially) already queued iterator, just a copy */
            queued_iterator tmp = *this;
            ++(*this);
            return tmp;
        }


        // queued_iterator
        // operator--(int)
        // { 
        //     queued_iterator tmp = *this;
        //     --(*this);
        //     return tmp;
        // }
    };

    template <typename NodeType>
    struct depth_first_pre_order
    {
        using node_traits  = tl::node_traits<NodeType>;
        using node_type    = typename node_traits::node_type;
        using node_pointer = typename node_traits::pointer;

        /**
         * @brief constructs an iterable sequence of node-pointers
         *        that when iterated over represents the
         *        depth-first-pre-order traversal of the underlying tree.
         */
        template <node_kind NodeKind,
                  typename ResultType = std::deque<node_pointer>,
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
                     : node_traits::neighbours<NodeKind>(*cur))
                {
                    stack.push(c);
                }
            }
            return result;
        }
    };

        /**
     *
     */
    template <typename NodeType>
    struct breadth_first_in_order
    {
        using node_traits  = tl::node_traits<NodeType>;
        using node_type    = typename node_traits::node_type;
        using node_pointer = typename node_traits::pointer;

        /**
         * @brief constructs an iterable sequence of node-pointers
         *        that when iterated over represents the
         *        depth-first-pre-order traversal of the underlying tree.
         */
        template <node_kind NodeKind,
                  typename ContainerType,
                  typename SequenceType = std::deque<node_pointer>>
        static constexpr 
        void
        enqueue(std::back_insert_iterator<ContainerType> dest,
                node_pointer root)
        {
            std::queue<node_pointer, SequenceType> queue;
            
            queue.push(root);
            while (!queue.empty())
            {
                node_pointer& cur = queue.front();
                *dest = cur;
                queue.pop();

                for (node_pointer& c 
                     : node_traits::template neighbours<NodeKind>(*cur))
                { queue.push(c); }
            }
        }
    };


    template <typename NodeType>
    struct breadth_first_reverse_order
    {
        using node_traits  = tl::node_traits<NodeType>;
        using node_type    = typename node_traits::node_type;
        using node_pointer = typename node_traits::pointer;

        /**
         * @brief constructs an iterable sequence of node-pointers
         *        that when iterated over represents the
         *        depth-first-pre-order traversal of the underlying tree.
         */
        template <node_kind NodeKind,
                  typename ContainerType,
                  typename SequenceType = std::deque<node_pointer>>
        static constexpr 
        void
        enqueue(std::back_insert_iterator<ContainerType> dest,
                node_pointer root)
        {
            std::queue<node_pointer, SequenceType> queue;
            
            queue.push(root);
            while (!queue.empty())
            {
                node_pointer& cur = queue.front();
                *dest = cur;
                queue.pop();
                for (node_pointer& c 
                     : node_traits::template neighbours<NodeKind>(*cur)
                     | std::views::reverse)
                { queue.push(c); }
            }
        }
    };
}

#endif