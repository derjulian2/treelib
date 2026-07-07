
#ifndef TREELIB_AVL_TREE_HPP
#define TREELIB_AVL_TREE_HPP

/**
 * @file   treelib/trees/avl_tree.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  balanced binary-search-tree.
 */

#include <treelib/trees/binary_tree.hpp>

namespace tl
{
    /* maybe implement as forest of strong_tree's? */
    template <typename T,
              typename Allocator>
    struct avl_tree
        : protected weak_binary_tree<T, Allocator>
    {
    private:

        void rotate_right() 
        {

        }

        void rotate_left()
        {
            
        }

    public:
        
    };
}

#endif