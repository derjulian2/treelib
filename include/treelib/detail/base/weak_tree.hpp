
#ifndef TREELIB_WEAK_TREE_HPP
#define TREELIB_WEAK_TREE_HPP

/**
 * @file   treelib/detail/weak_tree.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  base-class for weak-trees
 *         that have a single starting-point
 *         (e.g. root/leaf).
 */

#include <treelib/detail/node.hpp>
#include <treelib/detail/tree_alloc.hpp>
#include <treelib/detail/queued_iterator.hpp>

#define TREELIB_TRACK_TREE_SIZE

namespace tl
{
/*
     * does not keep a sentinel-node
     * as this is irrelevant for weak-trees.
     */
    template <typename NodeType,
              typename Allocator>
        requires weak_node<NodeType>
    struct weak_tree_base
        : protected tree_allocator_base<NodeType, Allocator>
    {
        using alloc_base = tree_allocator_base<NodeType, Allocator>;
        using typename alloc_base::node_traits;
        using typename alloc_base::node_type;
        using typename alloc_base::node_pointer;
        using typename alloc_base::base_pointer;
        using typename alloc_base::value_node_pointer;
        using typename alloc_base::value_type;


        using iterator               = tl::queued_iterator<value_type, node_type>;
        using const_iterator         = tl::queued_iterator<const value_type, node_type>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;



        value_node_pointer m_root = nullptr;
    #ifdef TREELIB_TRACK_TREE_SIZE
        std::size_t        m_size = 0;
    #endif

        [[nodiscard]]
        constexpr std::size_t
        size() const noexcept
        {
        #ifdef TREELIB_TRACK_TREE_SIZE
            return this->m_size;
        #else
            return this->count_nodes();
        #endif
        }


        [[nodiscard]]
        constexpr bool
        empty() const noexcept
        { return this->m_root == nullptr; }


        void
        clear() noexcept
        {
            for (;;/* all hooks*/)
            { this->erase(this->m_root); }
            this->put_node(this->m_root);
        #ifdef TREELIB_TRACK_TREE_SIZE
            this->m_size = 0;
        #endif
        }

        iterator root()
        { return begin(); }

        iterator begin() 
        { return iterator(this->m_root); }

        iterator end()
        { return iterator(nullptr); }


        template <typename... Args>
        iterator
        insert(Args&&... args)
        {
            this->m_root = this->new_node(std::forward<Args>(args)...);
            return iterator(this->m_root);
        }

        template <typename HookType, typename... Args>
            requires is_hook_type<HookType, node_type>
        iterator
        insert(HookType what, iterator parent, Args&&... args)
        {
            value_node_pointer new_node = this->new_node(std::forward<Args>(args)...);
            node_traits::hook_as(*new_node, what, static_cast<node_pointer>(parent));
            return iterator(new_node);
        }

        /**
         * weak trees can only erase
         * adjacent nodes referenced by hooks, not the
         * referenced nodes themselves
         * because updating the parent-node would
         * be impossible
         */
        template <typename HookType>
            requires is_hook_type<HookType, node_type>
        void
        erase(HookType what, base_pointer parent)
        {

        }
    };
}

#endif