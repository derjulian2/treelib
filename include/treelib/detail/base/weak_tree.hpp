
#ifndef TREELIB_WEAK_TREE_HPP
#define TREELIB_WEAK_TREE_HPP

/**
 * @file   treelib/detail/base/weak_tree.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  base-class for weak-trees
 *         that have a single starting-point
 *         (e.g. root/leaf).
 */

#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/tree_alloc.hpp>
#include <treelib/detail/traversal/queued_iterator.hpp>

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
    class weak_tree_base
        : protected tree_allocator_base<NodeType, Allocator>
    {
    public:
        using alloc_base = tree_allocator_base<NodeType, Allocator>;
        using typename alloc_base::node_traits;
        using typename alloc_base::node_type;
        using typename alloc_base::node_pointer;
        using typename alloc_base::base_pointer;
        using typename alloc_base::value_node_pointer;
        using typename alloc_base::value_type;

        using default_traversal_type = depth_first_pre_order<node_type>;

        using iterator               = tl::queued_iterator<value_type, default_traversal_type>;
        using const_iterator         = tl::queued_iterator<const value_type, default_traversal_type>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    private:

        value_node_pointer m_root = nullptr;
    #ifdef TREELIB_TRACK_TREE_SIZE
        std::size_t        m_size = 0;
    #endif

    public:

        weak_tree_base()
            : m_root(nullptr)
            , m_size(0)
        { }


        weak_tree_base(const weak_tree_base& other)
            : m_root(other.empty() ? nullptr : recursive_copy(other.m_root))
            , m_size(other.m_size)
        { }


        weak_tree_base(weak_tree_base&& other)
            : m_root(other.m_root)
            , m_size(other.m_size)
        { }


        ~weak_tree_base()
        { this->clear(); }


        weak_tree_base& operator=(const weak_tree_base& other)
        {
            if (!this->empty())
            { this->clear(); }

        }

        
        weak_tree_base& operator=(weak_tree_base&& other)
        {

        }


        static constexpr
        void
        swap(weak_tree_base& a, weak_tree_base& b)
        {
            std::swap(a.m_root, b.m_root);
            std::swap(a.m_size, b.m_size);
        }



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
            if (!this->empty())
            { this->recursive_erase(this->m_root); }
            this->m_root = nullptr;
        #ifdef TREELIB_TRACK_TREE_SIZE
            this->m_size = 0;
        #endif
        }

        iterator root()
        { return iterator(this->m_root); }


        iterator begin() 
        { return iterator(this->m_root); }


        iterator end()
        { return iterator(nullptr); }


        template <typename... Args>
        iterator
        insert(Args&&... args)
        {
            this->m_root = this->new_node(std::forward<Args>(args)...);
            this->m_size++;
            return iterator(this->m_root);
        }


        template <typename HookType, typename... Args>
            requires is_hook_type<HookType, node_type>
        iterator
        insert(HookType what, iterator parent, Args&&... args)
        {
            value_node_pointer new_node = this->new_node(std::forward<Args>(args)...);
            node_traits::hook_as(*new_node, what, *static_cast<node_pointer>(parent));
            this->m_size++;
            return iterator(new_node);
        }

        /**
         * @brief   erase a value from the tree, along with all
         *          values associated with it's child-nodes.
         * 
         * @details weak trees can only erase 'adjacent' nodes
         *          addressed by hooks, not the referenced nodes
         *          themselves because updating the parent-node
         *          accordingly would be impossible (there are no
         *          back-pointers to properly unhook a node).
         *
         * @param   what 
         * @param   parent
         */
        template <typename HookType>
            requires is_hook_type<HookType, node_type>
        void
        erase(HookType what, iterator parent)
        {
            node_pointer node = node_traits::at_hook(*static_cast<node_pointer>(parent), what);
            /* requires some form of forgetting the old node (unhooking?)*/
            if (node != nullptr)
            { recursive_erase(node); }
        }

    private:
        
        void
        recursive_erase(node_pointer node)
        {
            for (node_pointer c : node_traits::children(*node))
            { recursive_erase(c); }
            this->put_node(static_cast<value_node_pointer>(node));
        }


        base_pointer
        recursive_copy(base_pointer node)
        {

        }
        
    };
}

#endif