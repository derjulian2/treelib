
#ifndef TREELIB_ROSE_TREE_HPP
#define TREELIB_ROSE_TREE_HPP

/**
 * @file   treelib/trees/rose_tree.hpp
 * @author Julian Benzel
 * @date   04.07.2026
 *
 * @brief  type-generic tree without any constraints
 *         on the number of children per node.
 */

#include <treelib/detail/node_base.hpp>
#include <treelib/detail/tree_base.hpp>

#include <vector>
#include <utility>

namespace tl
{
    struct weak_vecrose_tree_node
    {
        std::vector<weak_vecrose_tree_node*> m_children;
    };

    struct vecrose_tree_node
    {
        vecrose_tree_node               *m_parent;
        std::vector<vecrose_tree_node*>  m_children;
    };

    struct weak_rose_tree_node
        : public weak_node_base
    {
        weak_rose_tree_node *m_first_child = nullptr;
        weak_rose_tree_node *m_next        = nullptr;

        enum struct hook_type : std::uint8_t
        {
            first_child,
            next
        };

        weak_rose_tree_node*&
        from_hook(const hook_type& h)
        { 
            if (h == hook_type::first_child)
                return m_first_child;
            else
                return m_next;
        }

        void
        hook_as(const hook_type& h, weak_rose_tree_node* parent)
        { 
            parent->from_hook(h) = this;
        }

        template <typename Fn>
        static constexpr
        void for_hooks(Fn&& fn)
        { fn(hook_type::first_child); fn(hook_type::next); }

        std::vector<weak_rose_tree_node*>
        children()
        {
            std::vector<weak_rose_tree_node*> res;
            weak_rose_tree_node* iter = m_first_child;
            while (iter)
            {
                res.push_back(iter);
                iter = iter->m_next;
            }
            return res;
        }    
    };

    struct rose_tree_node
        : public node_base
    {
        rose_tree_node *m_last_child;
        rose_tree_node *m_prev;

        rose_tree_node *m_parent;
    };


    template <typename T, 
              typename Allocator = std::allocator<T>>
    class weak_rose_tree
        : public weak_tree_base<weak_rose_tree_node, Allocator>
    { };


    template <typename T, 
              typename Allocator = std::allocator<T>>
    class rose_tree
        : public tree_base<rose_tree_node, Allocator>
    { };
}


#endif