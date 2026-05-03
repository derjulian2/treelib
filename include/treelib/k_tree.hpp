
#ifndef TREELIB_K_TREE_HPP
#define TREELIB_K_TREE_HPP

#include <treelib/detail/node_base.hpp>
#include <treelib/detail/tree_base.hpp>

#include <memory>
#include <concepts>
#include <utility>
#include <array>
#include <tuple>

namespace tl
{
    template <std::size_t K>
    struct weak_k_tree_node 
        : public node_base
    {
        std::array<weak_k_tree_node*, K> m_children;

        using hook_type = std::size_t;

        template <std::size_t... Idxs>
        static constexpr
        auto
        hooks()
        { return std::tuple<decltype(Idxs)...>(Idxs...); } 

        template <hook_type H>
        constexpr
        weak_k_tree_node*& get_hook()
        {
            static_assert(H < K, "hook-index out-of-range"); 
            return m_children[H];
        }

        template <hook_type H>
        constexpr 
        void hook_as(weak_k_tree_node* parent)
        { parent->get_hook<H>() = this; }

        template <typename Fn>
        static constexpr
        void for_hooks(Fn&& fn)
        { tl::for_iseq(fn, std::make_index_sequence<K>()); }

        std::array<weak_k_tree_node*, K>&
        children()
        { return m_children; }
    };


    template <std::size_t K>
    class k_tree_node
        : public weak_k_tree_node<K>
    {
        k_tree_node *m_parent;

        using hook_type = std::size_t;
    };


    template <typename T, 
            std::size_t K,
            typename Allocator = std::allocator<T>>
    class k_tree
        : public tree_base<k_tree_node<K>, Allocator>
    { };


    template <typename T, 
            std::size_t K,
            typename Allocator = std::allocator<T>>
    class weak_k_tree
        : public tree_base<weak_k_tree_node<K>, Allocator>
    { };

}

#endif