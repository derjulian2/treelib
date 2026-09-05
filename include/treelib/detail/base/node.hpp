
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
        /*********************************************************************************
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
         *          example: consider the following node-specification.
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
         *          seperates nodes like these from the others in it's capabilities.
         *          @see tl::detail::bidirectional_node for the node's with parent-pointers.
         *********************************************************************************/
        template <typename T>
        concept _Is_Node
            = std::default_initializable<T>
            && requires (T* t, const T* ct)
            {
                /***************************************************
                 * @brief 'hooking' two nodes refers to the
                 *        action of entangling them in a way
                 *        specified by a parameter-value of
                 *        hook-type.
                 *      
                 *        e.g. t.hook_at(left, t') means 'entangle
                 *        the node t' as the left child of
                 *        node t.
                 *
                 *        unhook_at should bring the node and the
                 *        subtree at the respective recursion-point
                 *        into a seperated state.
                 *
                 *        unhook_if should check if the passed
                 *        node is sitting at one of it's recursion-
                 *        points and call unhook_at if so.
                 ***************************************************/
                typename T::_M_hook_t;
                { t->_M_hook_at(std::declval<typename T::_M_hook_t>(), t) };
                { t->_M_unhook_at(std::declval<typename T::_M_hook_t>()) }
                    -> std::convertible_to<T*>;
                { t->_M_unhook_if(t) };
                
                /***************************************************
                 * @brief accessors to all subtrees at the
                 *        respective recursion-points.
                 *        note that not all children require a 
                 *        corresponding hook-value to qualify as
                 *        a child-node @see tl::detail::vecrose_node as
                 *        an example.
                 ***************************************************/
                { t->_M_children() }
                    -> std::ranges::range;
                { ct->_M_children() }
                    -> std::ranges::input_range;
            };

        template <typename T>
        concept _Is_Copyable_Node
            = _Is_Node<T>
            && requires(T* t, const T* ct)
            {
                /***************************************************
                 * @brief because trees are non-linear
                 *        structures, we cannot just do .push_back()
                 *        for each node, but the node-type has to
                 *        specify what insertions need to be called
                 *        in which order to get to a new valid
                 *        node-state from a known node (essentially
                 *        performing a structural copy).
                 *        the implementation should also recursively
                 *        call .mimic() on all child-nodes to
                 *        ensure a full copy.
                 *
                 *        the function that will be passed to
                 *        allow the implementation to specify where
                 *        copies should go will be some form of
                 *        tree.insert(...) to ensure that if
                 *        allocation fails, the tree will remain in
                 *        a predictable state.
                 ***************************************************/
                { t->_M_mimic(ct, [](typename T::_M_hook_t, T*, const T*) -> void { }) };
            };

        /***************************************************
         * @brief class-extender that adds an instance
         *        of value-type to the passed node-type.
         *
         *        these will be the instances that will
         *        actually be allocated by a tree-container.
         ***************************************************/
        template <typename _NodeT, typename _ValueT>
            requires _Is_Node<_NodeT>
        struct _Value_Node
            : public _NodeT
        {
            using _M_value_t = _ValueT;
            using _M_ref_t   = _M_value_t&;
            using _M_cref_t  = const _M_value_t&;
            using _M_ptr_t   = _M_value_t*;
            using _M_cptr_t  = const _M_value_t*;
        
            _M_value_t _M_value;

            /***************************************************
             * @brief constructor (1).
             *        forward all args to value-type.
             ***************************************************/

            template <typename... Args>
            constexpr
            _Value_Node(Args&&... args)
                noexcept(std::is_nothrow_constructible_v<_M_value_t, Args...>)
                : _M_value(std::forward<Args>(args)...)
            { }

            /***************************************************
             * @brief value accessors.
             ***************************************************/

            [[nodiscard]]
            constexpr _M_ref_t
            _M_get_value()
                noexcept
            { return this->_M_value; }

            [[nodiscard]]
            constexpr _M_cref_t
            _M_get_value()
                const noexcept
            { return this->_M_value; }
        };


        /***************************************************
         * @brief template-CRTP-mixin that extends the
         *        passed node-base-type by an additional
         *        back-pointer to it's owning parent.
         ***************************************************/
        template <typename _NodeBaseT>
        struct _Parent_Node_Base
            : public _NodeBaseT
        {
            using _M_base_t      = _NodeBaseT;
            using _M_node_t      = typename _M_base_t::_M_node_t;
            using _M_node_ptr_t  = _M_node_t*;
            using _M_cnode_ptr_t = const _M_node_t*;

            _M_node_ptr_t _M_parent;

            constexpr void
            _M_reset()
            { this->_M_parent = nullptr; } 

            /***************************************************
             * @brief since this is a CRTP-base-class,
             *        we can assume that casting itself to
             *        to a _M_node_ptr_t is valid. 
             ***************************************************/
            
            constexpr _M_node_ptr_t
            _M_node_ptr()
                noexcept
            { return static_cast<_M_node_ptr_t>(this); }

            constexpr _M_node_ptr_t
            _M_node_ptr()
                const noexcept
            { return static_cast<_M_cnode_ptr_t>(this); }

            using _M_hook_t = typename _M_base_t::_M_hook_t;

        protected:

            friend _M_node_t;

            /***************************************************
             * constructor (1).
             * default-constructible,
             * initializes parent to nullptr.
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             *
             *         _____--_--_
             *        /    |   o |
             *       /| ___ L_\ ||  
             *        L|   L|  `||`
             *
             * this my emotional-support elephant. 
             * his name is sam.
             ***************************************************/
            constexpr
            _Parent_Node_Base()
                : _M_base_t()
                , _M_parent(nullptr)
            { }

        public:
                
            constexpr _M_node_ptr_t
            _M_get_parent() 
                noexcept
            { return this->_M_parent; }


            constexpr _M_cnode_ptr_t
            _M_get_parent()
                const noexcept
            { return this->_M_parent; }


            constexpr bool
            _M_is_root()
                const noexcept
            { return this->_M_parent == nullptr; }


            constexpr void
            _M_hook_at(_M_hook_t _at, _M_node_ptr_t _node)
            {
                this->_M_base_t::hook_at(_at, _node);
                _node->_M_parent = this->_M_node_ptr();
            }

            constexpr _M_node_ptr_t
            _M_unhook_at(_M_hook_t _at)
            {
                _M_node_ptr_t _res = this->_M_base_t::_M_unhook_at(_at);
                _res->_M_parent = nullptr;
                return _res;
            }

            constexpr void
            _M_unhook_if(_M_node_ptr_t _node)
            {
                this->_M_base_t::unhook_if(_node);
                _node->_M_parent = nullptr;
            }

            constexpr void
            _M_unhook()
            {
                this->_M_parent->_M_base_t::_M_unhook_if(this);
                this->_M_parent = nullptr;
            }

        };

        template <typename T>
        concept _Is_Parent_Node
            = requires(T* t, const T* ct)
            {
                { t->_M_unhook() };
                { t->_M_get_parent() }
                    -> std::convertible_to<T*>;
                { ct->_M_get_parent() }
                    -> std::convertible_to<const T*>;
                { ct->_M_is_root() };
                { t->_M_next_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_M_next_sibling() }
                    -> std::convertible_to<const T*>;
                { t->_M_prev_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_M_prev_sibling() }
                    -> std::convertible_to<const T*>;
            };

        /***************************************************
         * @brief additional functionality and common
         *        interface for every type satisying the
         *        requirements for a tree-node.
         ***************************************************/
        template <typename _NodeT>
            requires _Is_Node<_NodeT>
        struct _Node_Traits
        {
            using _M_node_t = _NodeT;
            using _M_ptr_t  = _M_node_t*;
            using _M_cptr_t = const _M_node_t*;
            using _M_ref_t  = _M_node_t&;
            using _M_cref_t = const _M_node_t&;

            using _M_hook_t = _M_node_t::_M_hook_t;


            static constexpr void
            _S_hook_at(_M_ptr_t _parent, _M_hook_t _at, _M_ptr_t _node)
            { _parent->_M_hook_at(_at, _node); }

            static constexpr _M_ptr_t
            _S_unhook_at(_M_ptr_t _parent, _M_hook_t _at)
            { return _parent->_M_unhook_at(_at); }

            static constexpr void
            _S_unhook_if(_M_ptr_t _parent, _M_ptr_t _node)
            { _parent->_M_unhook_if(_node); }

            static constexpr decltype(auto)
            _S_children(_M_ptr_t _node)
            { return _node->_M_children(); }

            static constexpr decltype(auto)
            _S_children(_M_cptr_t _node)
            { return _node->_M_children(); }

            template <typename Fn>
                requires std::invocable<Fn, _M_hook_t, _M_ptr_t, _M_cptr_t>
            constexpr void
            _S_mimic(_M_ptr_t _node, _M_cptr_t _src, Fn&& _insert_fn)
            { _node->_M_mimic(_src, std::forward<Fn>(_insert_fn)); }

            static constexpr bool
            _S_is_leaf(_M_cptr_t _node)
            { return std::ranges::empty(_node->_M_children()); }

            template <typename _IterT>
            static constexpr _IterT 
            _S_to_iter(_M_ptr_t _node)
            { return _IterT(_node); }

            template <typename _IterT>
            static constexpr _M_ptr_t 
            _S_from_iter(_IterT&& _iter)
            { return _iter->_M_cur(); }

            static constexpr _M_ptr_t
            _S_next_sibling(_M_ptr_t _node)
                requires _Is_Parent_Node<_M_node_t>
            { return _node->_M_next_sibling(); }

            static constexpr _M_cptr_t
            _S_next_sibling(_M_cptr_t _node)
                requires _Is_Parent_Node<_M_node_t>
            { return _node->_M_next_sibling(); }

            static constexpr _M_ptr_t
            _S_prev_sibling(_M_ptr_t _node)
                requires _Is_Parent_Node<_M_node_t>
            { return _node->_M_prev_sibling(); }

            static constexpr _M_cptr_t
            _S_prev_sibling(_M_cptr_t _node)
                requires _Is_Parent_Node<_M_node_t>
            { return _node->_M_prev_sibling(); }
        };
    }
}

#endif