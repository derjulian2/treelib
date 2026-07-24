
#ifndef TREELIB_QUEUED_ITERATOR_HPP
#define TREELIB_QUEUED_ITERATOR_HPP

/**
 * @file   treelib/detail/queued_iterator.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  types to implement different
 *         tree-traversal-strategies using
 *         a queued approach. this is mainly used
 *         for weak-trees as these are not 
 *         iteratively traversable (e.g. because
 *         parents are not reachable from any given node).
 */

#include <queue>
#include <stack>
#include <vector>
#include <iterator>
#include <stdexcept>

#include <treelib/detail/base/node.hpp>
#include <treelib/detail/traversal/depth_first.hpp>


namespace tl
{

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
}

#endif