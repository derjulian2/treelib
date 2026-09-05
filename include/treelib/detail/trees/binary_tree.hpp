
#ifndef TREELIB_BINARY_TREE_HPP
#define TREELIB_BINARY_TREE_HPP

/***************************************************
 * @file   treelib/detail/trees/binary_tree.hpp
 * @author Julian Benzel
 * @date   04.09.2026
 *
 * @brief  type-generic binary-tree.
 ***************************************************/

#include <treelib/detail/trees/k_tree.hpp>

namespace tl 
{
    /*************************************************************
     * hacky scoped enum, because i want implicit conversions
     * to std::size_t (the hook type of tl::k_tree), but also
     * scoping-rules.
     *
     * Source - https://stackoverflow.com/a/46407792
     * Posted by AndyG
     * Retrieved 2026-09-04, License - CC BY-SA 3.0
     *************************************************************/

    struct binary_hook
    {
        enum : std::size_t
        {
            left  = 0,
            right = 1
        };
    };


    template <typename T, typename Allocator = std::allocator<T>>
    struct outward_binary_tree
        : public outward_k_tree<T, 2, Allocator>
    { 
        using _M_base_t = outward_k_tree<T, 2, Allocator>;

    public:
        
        using _M_base_t::_M_base_t;
    };

    template <typename T, typename Allocator = std::allocator<T>>
    struct binary_tree
        : public k_tree<T, 2, Allocator>
    { 
        using _M_base_t = k_tree<T, 2, Allocator>;

    public:
        
        using _M_base_t::_M_base_t;
    };

} 

#endif