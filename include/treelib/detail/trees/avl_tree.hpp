
#ifndef TREELIB_AVL_TREE_HPP
#define TREELIB_AVL_TREE_HPP

/***************************************************
 * @file   treelib/trees/avl_tree.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  balanced binary-search-tree implemented
 *         using a flat vector-based binary-tree.
 ***************************************************/

#include <compare>
#include <functional>

namespace tl
{
    namespace _detail
    {
        template <typename T,
                  typename AllocT>
        struct _flat_binary_tree
        {
            using _m_alloc_t = AllocT;
            using _m_vec_t   = std::vector<T, AllocT>;

            _m_vec_t _m_data;
        };
    }

    /***************************************************
     * @brief balanced binary-search-tree with
     *        iterator-stability upon insertion/erasure.
     ***************************************************/
    template <typename T,
              typename Allocator>
        requires std::three_way_comparable<T, std::less<>>
    struct avl_tree
        : protected _detail::_flat_binary_tree<T, Allocator>
    {
    private:

        void rotate_right() 
        {

        }

        void rotate_left()
        {
            
        }

    public:
        
        template <typename... Args>
        constexpr iterator 
        emplace(Args&&... args)
        {

        }

        constexpr iterator
        insert(const value_type& value)
        {

        }

        constexpr iterator
        lookup(const value_type& value)
        {
            
        }

        constexpr void
        remove(const value_type& value)
            noexcept
        {

        }

        constexpr void
        merge(const avl_tree& other)
        {
            
        }
    };
}

#endif