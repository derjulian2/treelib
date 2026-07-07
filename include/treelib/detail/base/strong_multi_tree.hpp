
#ifndef TREELIB_MULTI_TREE_HPP
#define TREELIB_MULTI_TREE_HPP

/**
 * @file   treelib/detail/strong_multi_tree.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  base-class for strong-trees
 *         that have multiple starting-points
 *         (e.g. roots/leaves).
 */

#include <treelib/detail/node.hpp>
#include <treelib/detail/tree_alloc.hpp>
#include <treelib/detail/queued_iterator.hpp>

#define TREELIB_TRACK_TREE_SIZE

namespace tl
{
    /* maybe implement as forest of strong_tree's? */
    template <typename NodeType,
              typename Allocator>
        requires strong_node<NodeType>
    struct strong_multi_tree_base
        : protected tree_allocator_base<NodeType, Allocator>
    { };
}

#endif