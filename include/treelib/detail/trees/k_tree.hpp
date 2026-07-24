
#ifndef TREELIB_K_TREE_HPP
#define TREELIB_K_TREE_HPP

/**
 * @file   treelib/detail/trees/k_tree.hpp
 * @author Julian Benzel
 * @date   04.07.2026
 *
 * @brief  type-generic k-ary-tree.
 */

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/strong_tree.hpp>
#include <treelib/detail/base/weak_tree.hpp>

#include <memory>
#include <concepts>
#include <utility>
#include <array>
#include <tuple>
#include <cassert>
#include <ranges>

// #define TREELIB_NO_EXCEPTIONS
// #define TREELIB_WEAK_K_TREE_NO_SHIFT

namespace tl
{

    namespace detail 
    {

        template <std::size_t K>
        struct weak_k_tree_node
        {
            std::array<weak_k_tree_node*, K> m_children;

            using out_hook_type = std::size_t;

            
            constexpr weak_k_tree_node*
            at_hook(out_hook_type what)
            { return this->m_children.at(what); }


            constexpr void
            hook_as(out_hook_type what, weak_k_tree_node& where)
            {
            #ifdef TREELIB_WEAK_K_TREE_NO_SHIFT
                #ifdef TREELIB_NO_EXCEPTIONS
                    assert(where.m_children[what] != nullptr);
                #else
                    if (where.m_children[what] != nullptr)
                    { throw modification_error("cannot insert at occupied hook"); }
                #endif
            #else
                if (where.m_children[what] != nullptr) {
                    // might explode if this->m_children[what] is
                    // also occupied by another node, needs testing?
                    where.m_children[what]->hook_as(what, *this);
                }
            #endif
                where.m_children[what] = this;
            }


            constexpr auto
            children()
            {
                return this->m_children
                       | std::views::filter([](weak_k_tree_node* x) { return x != nullptr; });
            }
        };


        template <std::size_t K>
        struct strong_k_tree_node 
        {

        };
    }

    
    template <typename T, 
            std::size_t K,
            typename Allocator = std::allocator<T>>
    class strong_k_tree
        : public strong_tree_base<detail::strong_k_tree_node<K>, Allocator>
    { };


    template <typename T, 
            std::size_t K,
            typename Allocator = std::allocator<T>>
    class weak_k_tree
        : public weak_tree_base<detail::weak_k_tree_node<K>, Allocator>
    { };


    template <typename T, 
            std::size_t K,
            typename Allocator = std::allocator<T>>
    using k_tree = weak_k_tree<T, K, Allocator>;
}

#endif