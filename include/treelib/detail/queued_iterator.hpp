
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

#include <treelib/detail/node.hpp>

namespace tl
{
    template <typename ValueType, typename NodeType>
        requires node<NodeType>
    class queued_depth_first_pre_order_iterator
    {

    };

    template <typename ValueType, typename NodeType>
        requires node<NodeType>
    class queued_depth_first_in_order_iterator
    {

    };

    template <typename ValueType, typename NodeType>
        requires node<NodeType>
    class queued_depth_first_post_order_iterator
    {

    };


    template <typename ValueType, typename NodeType>
        requires node<NodeType>
    class queued_breadth_first_in_order_iterator
    {

    };

    template <typename ValueType, typename NodeType>
        requires node<NodeType>
    class queued_breadth_first_reverse_order_iterator
    {

    };


    template <typename ValueType, typename NodeType>
        requires node<NodeType>
    class queued_iterator
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
        operator==(const queued_iterator&, const queued_iterator&);

        reference operator*();
        pointer operator->();
        queued_iterator& operator++();
    };
}

#endif