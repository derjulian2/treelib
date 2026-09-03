
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
#include <treelib/detail/base/tree.hpp>
#include <treelib/detail/base/forest.hpp>

#include <memory>
#include <array>
#include <cassert>
#include <ranges>

// #define TREELIB_NO_EXCEPTIONS
// #define TREELIB_WEAK_K_TREE_NO_SHIFT

namespace tl
{

    namespace detail 
    {

        template <std::size_t K>
        struct k_node
        {
            std::array<k_node*, K> m_children;

            using out_hook_type = std::size_t;

            
            constexpr k_node*
            at_hook(out_hook_type what)
            { return this->m_children.at(what); }


            constexpr void
            hook_as(out_hook_type what, k_node& where)
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
            out_neighbours()
            {
                return this->m_children
                       | std::views::filter([](k_node* x) { return x != nullptr; });
            }

            template <typename Fn>
                requires node_copy_invocable<Fn, weak_k_tree_node>
            weak_k_tree_node* clone(Fn&& copy)
                const
            {
                weak_k_tree_node* tmp = copy(this);
                for (std::size_t i = 0; i < K; ++i)
                { 
                    if (this->m_children[i] != nullptr)
                    { 
                        tmp->m_children[i] = this->m_children[i]->clone(std::forward<Fn>(copy)); 
                    }
                }
                return tmp;
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
    class outward_k_tree
        : public detail::outward_tree_base<detail::k_node<K>, Allocator>
    { 

    };


    template <typename T, 
              std::size_t K,
              typename Allocator = std::allocator<T>>
    class k_tree
        : public detail::tree_base<detail::k_node<K>, Allocator>
    { 

    };

}

#endif