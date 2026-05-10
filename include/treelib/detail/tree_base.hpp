
#ifndef TREELIB_TREE_BASE_HPP
#define TREELIB_TREE_BASE_HPP

#include <memory>
#include <type_traits>

#include <treelib/detail/utility.hpp>
#include <treelib/detail/node_base.hpp>

namespace tl
{

    template <typename NodeType,
              typename Allocator>
    struct tree_allocator_base
    {
        using node_type    = NodeType;
        using node_pointer = node_type*;
        using hook_type    = typename node_type::hook_type;

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
        get_node(Args&&... args)
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


    struct tree_header_node
        : public node_base
    {
        node_base *m_first;
    };


    template <typename NodeType>
    class tree_from_nested_factory;


    /*
    * does not keep a sentinel-node
    * as this is irrelevant for weak-trees.
    */
    template <typename NodeType,
              typename Allocator>
        requires is_weak_node<NodeType>
    struct weak_tree_base
    {

    };

    /*
    * keeps a sentinel-node as that makes sense for 
    * strong-trees and the iterative traversal this enables.
    */
    template <typename NodeType,
              typename Allocator>
        requires is_node<NodeType>
    struct tree_base
        : public tree_allocator_base<NodeType, Allocator>
    {
        using alloc_base = tree_allocator_base<NodeType, Allocator>;
        using typename alloc_base::node_type;
        using typename alloc_base::node_pointer;
        using typename alloc_base::value_node_pointer;
        using typename alloc_base::hook_type;
        using typename alloc_base::value_type;


        tree_header_node m_header;

        template <hook_type H, typename... Args>
        value_node_pointer
        insert(node_pointer parent, Args&&... args)
        {
            value_node_pointer new_node = this->m_alloc_impl.get_node(std::forward<Args>(args)...);
            new_node->template hook_as<H>(parent);
            return new_node;
        }

        template <typename... Args>
        value_node_pointer
        insert_root(Args&&... args)
        { 
            value_node_pointer new_node = this->m_alloc_impl.get_node(std::forward<Args>(args)...);
            this->m_header.m_first = new_node;
            return new_node;
        }

        template <hook_type H>
        void
        erase(node_pointer parent)
        {

        }

        void
        strong_erase(node_pointer node)
        {
            node->unhook();
        }



        /**
        * structurally copies the tree
        * and all associated values.
        */
        value_node_pointer
        copy(node_pointer node)
        {
            value_type& vref = static_cast<value_node_pointer>(node)->value();
            value_node_pointer cpy = this->m_alloc_impl.get_node(vref);
            /*
            * this basically folds over each node-type's
            * hook-slots, for example for a binary-tree this would be:
            *
            * if (node->right) {
            *     copy(node->right)->hook_as_right(cpy)
            * }
            * if (node->left) {
            *    copy(node->left)->hook_as_left(cpy)
            * }
            */
            
            node_type::for_hooks(
                [&](auto H)
                {
                    if (node->template get_hook<H>())
                    {
                        node_pointer n = cpy->template get_hook<H>(); 
                        n = copy(node->template get_hook<H>()); 
                        n->template hook_as<H>(cpy);
                    }
                });
            return cpy;
        }
    };

}

#endif