
#ifndef TREELIB_TRAVERSING_ITERATOR_HPP
#define TREELIB_TRAVERSING_ITERATOR_HPP

/**
 * @file   treelib/detail/traversing_iterator.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  types to implement different
 *         tree-traversal-strategies using
 *         an iterative approach. this is
 *         reserved for strong-nodes as only here
 *         there is a path from any given node to
 *         any other node in the tree.
 */

#include <treelib/detail/node.hpp>

namespace tl
{

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

}

#endif