
#ifndef TREELIB_WEAK_MULTI_TREE_HPP
#define TREELIB_WEAK_MULTI_TREE_HPP

/**
 * @file   treelib/detail/weak_multi_tree.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  base-class for weak-trees
 *         that have multiple starting-points
 *         (e.g. roots/leaves).
 */

#include <treelib/detail/node.hpp>
#include <treelib/detail/tree_alloc.hpp>
#include <treelib/detail/queued_iterator.hpp>

#define TREELIB_TRACK_TREE_SIZE

namespace tl
{
    /* maybe implement as forest of weak_tree's? */
    template <typename NodeType,
              typename Allocator>
        requires weak_node<NodeType>
    struct weak_multi_tree_base
        : protected tree_allocator_base<NodeType, Allocator>
    { };
}

#endif