
#ifndef TREELIB_BINARY_TREE_HPP
#define TREELIB_BINARY_TREE_HPP

/**
 * @file   treelib/trees/binary_tree.hpp
 * @author Julian Benzel
 * @date   04.07.2026
 *
 * @brief  type-generic binary-tree.
 */

#include <treelib/detail/trees/k_tree.hpp>

namespace tl 
{
    template <typename T, typename Allocator = std::allocator<T>>
    struct strong_binary_tree
        : public strong_k_tree<T, 2, Allocator>
    { };

    template <typename T, typename Allocator = std::allocator<T>>
    struct weak_binary_tree
        : public weak_k_tree<T, 2, Allocator>
    { };

    template <typename T,
              typename Allocator = std::allocator<T>>
    using binary_tree = weak_binary_tree<T, Allocator>;
} 

#endif