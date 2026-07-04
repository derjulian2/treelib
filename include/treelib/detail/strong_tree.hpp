
#ifndef TREELIB_WEAK_MULTI_TREE_HPP
#define TREELIB_WEAK_MULTI_TREE_HPP

/**
 * @file   treelib/detail/strong_tree.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  base-class for strong-trees
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
     * keeps a sentinel-node as that makes sense for 
     * strong-trees and the iterative traversal this enables.
     */
    template <typename NodeType,
              typename Allocator>
        requires strong_node<NodeType>
    struct strong_tree_base
        : protected tree_allocator_base<NodeType, Allocator>
    {
        using alloc_base = tree_allocator_base<NodeType, Allocator>;
        using typename alloc_base::node_traits;
        using typename alloc_base::node_type;
        using typename alloc_base::node_pointer;
        using typename alloc_base::base_pointer;
        using typename alloc_base::value_node_pointer;
        using typename alloc_base::value_type;


        struct tree_header_node
            : public node_base
        {
            node_type *m_first = nullptr; 
        };


        tree_header_node m_header;

        template <typename HookType, typename... Args>
            requires is_hook_type<HookType, node_type>
        value_node_pointer
        insert(HookType what, base_pointer parent, Args&&... args)
        {
            value_node_pointer new_node = this->new_node(std::forward<Args>(args)...);
            node_traits::hook_as(*new_node, what, static_cast<node_pointer>(parent));
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


        /**
         * strong-trees can erase
         * the nodes directly because
         * their structure allows the 
         * parent to be referenced from
         * the node itself
         */
        void
        erase(base_pointer node)
        {
            node->unhook();
        }



        /**
        * structurally copies the tree
        * and all associated values.
        */
        value_node_pointer
        copy(base_pointer node)
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
