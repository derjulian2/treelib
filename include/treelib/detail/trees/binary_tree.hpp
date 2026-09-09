
#ifndef TREELIB_BINARY_TREE_HPP
#define TREELIB_BINARY_TREE_HPP

/***************************************************
 * @file   treelib/detail/trees/binary_tree.hpp
 * @author Julian Benzel
 * @date   04.09.2026
 *
 * @brief  type-generic binary-tree, implemented
 *         as a special-case for a k-tree with
 *         the parameter k = 2.
 ***************************************************/

#include <treelib/detail/trees/k_tree.hpp>

namespace tl 
{
    /*************************************************************
     * hacky scoped enum, because i want implicit conversions
     * to std::size_t (the hook-type of tl::k_tree), but also
     * scoping-rules.
     *
     * Source - https://stackoverflow.com/a/46407792
     * Posted by AndyG
     * Retrieved 2026-09-04, License - CC BY-SA 3.0
     *************************************************************/

    struct binary
    {
        static constexpr std::size_t left  = 0;
        static constexpr std::size_t right = 1;
    
    protected:
        binary() = default;
    };

    template <typename T, typename Allocator = std::allocator<T>>
    using outward_binary_tree
        = outward_k_tree<T, 2, Allocator>;

    template <typename T, typename Allocator = std::allocator<T>>
    using binary_tree
        = k_tree<T, 2, Allocator>;
} 

#endif