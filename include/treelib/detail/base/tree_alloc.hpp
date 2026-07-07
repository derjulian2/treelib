
#ifndef TREELIB_TREE_ALLOC_HPP
#define TREELIB_TREE_ALLOC_HPP

/**
 * @file   treelib/detail/tree_alloc.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  base-class for memory-management
 *         of a tree-structure.
 */

#include <memory>
#include <treelib/detail/node.hpp>

namespace tl
{
    template <typename NodeType,
              typename Allocator>
    struct tree_allocator_base
    {
        using node_traits  = tl::node_traits<NodeType>;
        using node_type    = typename node_traits::node_type;
        using node_pointer = typename node_traits::pointer;
        using base_pointer = node_base*;

        using allocator_type      = Allocator;
        using alloc_traits        = std::allocator_traits<allocator_type>;

        using value_type          = typename alloc_traits::value_type; 
        using value_node_type     = value_node<value_type, node_type>;
        using value_node_pointer  = value_node_type*;

        using node_allocator_type = alloc_traits::template rebind_alloc<value_node_type>;
        using node_alloc_traits   = std::allocator_traits<node_allocator_type>;


        [[no_unique_address]]
        node_allocator_type m_alloc;

        
        [[nodiscard]]
        constexpr
        node_allocator_type&
        get_node_alloc() noexcept
        { return this->m_alloc; }


        [[nodiscard]]
        constexpr
        allocator_type
        get_allocator() const noexcept 
        { return allocator_type(this->m_alloc); }


        template <typename... Args>
        [[nodiscard]]
        constexpr
        value_node_pointer
        new_node(Args&&... args)
        { 
            value_node_pointer res = node_alloc_traits::allocate(this->get_node_alloc(), 1);
            node_alloc_traits::construct(this->get_node_alloc(), res, std::forward<Args>(args)...);
            return res;
        }

        constexpr
        void
        put_node(value_node_pointer node) noexcept
        { 
            node_alloc_traits::destroy(this->get_node_alloc(), node);
            node_alloc_traits::deallocate(this->get_node_alloc(), node, 1);
        }
        
    };

}

#endif