
#ifndef TREELIB_BASE_NODE_HPP
#define TREELIB_BASE_NODE_HPP

 /***************************************************
  * @file   treelib/detail/base/node.hpp
  * @author Julian Benzel
  * @date   03.09.2026
  *
  * @brief   requirements and traits
  *          for node-types used in trees.
  ***************************************************/

#include <ranges>
#include <concepts>

namespace tl
{
    namespace detail
    {
        /*****************************************************************************************************
         * @brief   requirements of a node-type to
         *          be used in a tree-container.
         *
         * @details nodes consist of recursion-points
         *          that somehow reference the root-node of
         *          a subtree (e.g. .left/.right in a binary-tree).
         *          
         *          these recursion-points are owning references, meaning
         *          the lifetime of a node that is referenced by a recursion-point 
         *          of another node will be dependent on the lifetime of the parent-node.
         *
         *          example. consider the following node-specification:
         *
         *          BTree ::= BNil | BNode x (BTree) (BTree)
         *
         *          which could be implemented by the following structure:
         *
         *          struct bnode {
         *              bnode *left;
         *              bnode *right;
         *          };
         *
         *          when trying to erase a node, there would be a 'hole' in the
         *          parent's recursion-point for that node, and there would not be a way
         *          to securely delete a node without some '.parent'-back-reference, which
         *          seperates nodes like these from these in it's capabilities.
         *****************************************************************************************************/
        template <typename T>
        concept node
            = requires (T t, const T ct)
            {
                typename T::hook_type;
                { t.children() }
                    -> std::ranges::range;
                { ct.children() }
                    -> std::ranges::range;
                { t.hook_as(std::declval<typename T::hook_type>(), std::addressof(t)) };
                { t.clone() }
                    -> std::same_as<T*>;
                { t.erase_at(std::declval<typename T::hook_type>()) }
                    -> std::convertible_to<std::size_t>;
            };


        /***************************************************
         * @brief template-mixin that extends the
         *        passed node-type by an instance of
         *        value-type.
         ***************************************************/
        template <typename NodeType, typename ValueType>
            requires node<NodeType>
        class value_node
            : public NodeType
        {
        public:
            
            using value_type      = ValueType;
            using reference       = value_type&;
            using const_reference = const value_type&;
            using pointer         = value_type*;
            using const_pointer   = const value_type*;
        
        protected:
            
            value_type _M_value;

        public:

            template <typename... Args>
            constexpr
            value_node(Args&&... args)
                noexcept(std::is_nothrow_constructible_v<value_type, Args...>)
                : _M_value(std::forward<Args>(args)...)
            { }

            [[nodiscard]]
            constexpr reference
            value()
                noexcept
            { return this->_M_value; }

            [[nodiscard]]
            constexpr const_reference
            value()
                const noexcept
            { return this->_M_value; }
        };


        /***************************************************
         * @brief additional functionality and common
         *        interface for every type satisying the
         *        requirements for a tree-node.
         ***************************************************/
        template <typename NodeType>
            requires node<NodeType>
        struct node_traits
        {
            using node_type            = NodeType;
            using node_pointer         = node_type*;
            using const_node_pointer   = const node_type*;
            using node_reference       = node_type&;
            using const_node_reference = const node_type&;

            using typename node_type::hook_type;

            static constexpr void
            hook_as(node_reference node, hook_type as, node_reference parent)
            {
                node.hook_as(as, parent);
            }

            static constexpr decltype(auto)
            children(node_reference node)
            { 
                return node.children(); 
            }

            static constexpr bool
            is_leaf(const_node_reference node)
            { 
                return std::ranges::empty(node.children());
            }
        };


        /***************************************************
         * @brief template-mixin that extends the
         *        passed node-type by an additional
         *        back-pointer to it's owning parent.
         ***************************************************/
        template <typename NodeType>
            requires node<NodeType>
        struct bidirectional_node
            : public NodeType
        {
            using node_type            = NodeType;
            using node_pointer         = node_type*;
            using const_node_pointer   = const node_type*;
            using node_reference       = node_type&;
            using const_node_reference = const node_type&;

            using typename node_type::hook_type;

            node_pointer _M_parent;

            constexpr void
            _M_reset()
            { this->_M_parent = nullptr; } 

            constexpr
            bidirectional_node()
                : _M_parent(nullptr)
            { }
                
            constexpr node_pointer
            parent() 
                noexcept
            { return this->_M_parent; }


            constexpr const_node_pointer
            parent()
                const noexcept
            { return this->_M_parent; }


            constexpr bool
            is_root()
                const noexcept
            { return this->_M_parent == nullptr; }


            constexpr void
            hook_as(hook_type as, node_pointer where)
            {
                this->node_type::hook_as(std::forward<hook_type>(as), std::forward<node_pointer>(where));
                this->_M_parent = where;
            }
        };
    }
}

#endif