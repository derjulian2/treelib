
#ifndef TREELIB_BINARY_TREE_HPP
#define TREELIB_BINARY_TREE_HPP

#include <treelib/k_tree.hpp>

namespace tl
{
    template <typename T, 
            typename Allocator = std::allocator<T>>
    using binary_tree = k_tree<T, 2, Allocator>;

    template <typename T, 
            typename Allocator = std::allocator<T>>
    using weak_binary_tree = weak_k_tree<T, 2, Allocator>;
}



#endif