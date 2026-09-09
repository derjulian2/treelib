
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
        concept _is_node
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
                typename T::_m_hook_t;
                { t->_m_hook_at(std::declval<typename T::_m_hook_t>(), t) };
                { t->_m_unhook_at(std::declval<typename T::_m_hook_t>()) }
                    -> std::convertible_to<T*>;
                { t->_m_unhook_if(t) };
                
                /***************************************************
                 * @brief accessors to all subtrees at the
                 *        respective recursion-points.
                 *        note that not all children require a 
                 *        corresponding hook-value to qualify as
                 *        a child-node @see tl::detail::vecrose_node as
                 *        an example.
                 ***************************************************/
                { t->_m_children() }
                    -> std::ranges::range;
                { ct->_m_children() }
                    -> std::ranges::input_range;
            };

        template <typename T>
        concept _Is_Copyable_Node
            = _is_node<T>
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
                { t->_m_mimic(ct, [](typename T::_m_hook_t, T*, const T*) -> void { }) };
            };

        /***************************************************
         * @brief class-extender that adds an instance
         *        of value-type to the passed node-type.
         *
         *        these will be the instances that will
         *        actually be allocated by a tree-container.
         ***************************************************/
        template <typename NodeT, typename ValueT>
            requires _is_node<NodeT>
        struct _value_node
            : public NodeT
        {
            using _m_value_t = ValueT;
            using _m_ref_t   = _m_value_t&;
            using _m_cref_t  = const _m_value_t&;
            using _m_ptr_t   = _m_value_t*;
            using _m_cptr_t  = const _m_value_t*;
        
            _m_value_t _m_value;

            /***************************************************
             * @brief constructor (1).
             *        forward all args to value-type.
             ***************************************************/

            template <typename... Args>
            constexpr
            _value_node(Args&&... args)
                noexcept(std::is_nothrow_constructible_v<_m_value_t, Args...>)
                : _m_value(std::forward<Args>(args)...)
            { }

            /***************************************************
             * @brief value accessors.
             ***************************************************/

            [[nodiscard]]
            constexpr _m_ref_t
            _m_get_value()
                noexcept
            { return this->_m_value; }

            [[nodiscard]]
            constexpr _m_cref_t
            _m_get_value()
                const noexcept
            { return this->_m_value; }
        };


        /***************************************************
         * @brief template-CRTP-mixin that extends the
         *        passed node-base-type by an additional
         *        back-pointer to it's owning parent.
         ***************************************************/
        template <typename NodeBaseT>
        struct _bidirectional_node_base
            : public NodeBaseT
        {
            using _m_base_t      = NodeBaseT;
            using _m_node_t      = typename _m_base_t::_m_node_t;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;

            _m_node_ptr_t _m_parent;

            constexpr void
            _m_reset()
            { this->_m_parent = nullptr; } 

            /***************************************************
             * @brief since this is a CRTP-base-class,
             *        we can assume that casting itself to
             *        to a _m_node_ptr_t is valid. 
             ***************************************************/
            
            constexpr _m_node_ptr_t
            _m_node_ptr()
                noexcept
            { return static_cast<_m_node_ptr_t>(this); }

            constexpr _m_node_ptr_t
            _m_node_ptr()
                const noexcept
            { return static_cast<_m_cnode_ptr_t>(this); }

            using _m_hook_t = typename _m_base_t::_m_hook_t;

        protected:

            friend _m_node_t;

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
            _bidirectional_node_base()
                : _m_base_t()
                , _m_parent(nullptr)
            { }

        public:
                
            constexpr _m_node_ptr_t
            _m_get_parent() 
                noexcept
            { return this->_m_parent; }


            constexpr _m_cnode_ptr_t
            _m_get_parent()
                const noexcept
            { return this->_m_parent; }


            constexpr bool
            _m_is_root()
                const noexcept
            { return this->_m_parent == nullptr; }


            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
            {
                this->_m_base_t::hook_at(_at, _node);
                _node->_m_parent = this->_m_node_ptr();
            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t _at)
            {
                _m_node_ptr_t _res = this->_m_base_t::_m_unhook_at(_at);
                _res->_m_parent = nullptr;
                return _res;
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t _node)
            {
                this->_m_base_t::unhook_if(_node);
                _node->_m_parent = nullptr;
            }

            constexpr void
            _m_unhook()
            {
                this->_m_parent->_m_base_t::_m_unhook_if(this);
                this->_m_parent = nullptr;
            }

        };

        template <typename T>
        concept _is_parent_node
            = requires(T* t, const T* ct)
            {
                { t->_m_unhook() };
                { t->_m_get_parent() }
                    -> std::convertible_to<T*>;
                { ct->_m_get_parent() }
                    -> std::convertible_to<const T*>;
                { ct->_m_is_root() };
                { t->_m_next_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_m_next_sibling() }
                    -> std::convertible_to<const T*>;
                { t->_m_prev_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_m_prev_sibling() }
                    -> std::convertible_to<const T*>;
            };

        /***************************************************
         * @brief additional functionality and common
         *        interface for every type satisying the
         *        requirements for a tree-node.
         ***************************************************/
        template <typename NodeT>
            requires _is_node<NodeT>
        struct _node_traits
        {
            using _m_node_t = NodeT;
            using _m_ptr_t  = _m_node_t*;
            using _m_cptr_t = const _m_node_t*;
            using _m_ref_t  = _m_node_t&;
            using _m_cref_t = const _m_node_t&;

            using _m_hook_t = _m_node_t::_m_hook_t;


            static constexpr void
            _s_hook_at(_m_ptr_t _parent, _m_hook_t _at, _m_ptr_t _node)
            { _parent->_m_hook_at(_at, _node); }

            static constexpr _m_ptr_t
            _s_unhook_at(_m_ptr_t _parent, _m_hook_t _at)
            { return _parent->_m_unhook_at(_at); }

            static constexpr void
            _s_unhook_if(_m_ptr_t _parent, _m_ptr_t _node)
            { _parent->_m_unhook_if(_node); }

            static constexpr decltype(auto)
            _s_children(_m_ptr_t _node)
            { return _node->_m_children(); }

            static constexpr decltype(auto)
            _s_children(_m_cptr_t _node)
            { return _node->_m_children(); }

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_ptr_t, _m_cptr_t>
            constexpr void
            _s_mimic(_m_ptr_t _node, _m_cptr_t _src, Fn&& _insert_fn)
            { _node->_m_mimic(_src, std::forward<Fn>(_insert_fn)); }

            static constexpr bool
            _s_is_leaf(_m_cptr_t _node)
            { return std::ranges::empty(_node->_m_children()); }

            template <typename IterT>
            static constexpr IterT 
            _s_to_iter(_m_ptr_t _node)
            { return IterT(_node); }

            template <typename IterT>
            static constexpr _m_ptr_t 
            _s_from_iter(const IterT& _iter)
            { return _iter._m_cur(); }

            static constexpr _m_ptr_t
            _s_next_sibling(_m_ptr_t _node)
                requires _is_parent_node<_m_node_t>
            { return _node->_m_next_sibling(); }

            static constexpr _m_cptr_t
            _s_next_sibling(_m_cptr_t _node)
                requires _is_parent_node<_m_node_t>
            { return _node->_m_next_sibling(); }

            static constexpr _m_ptr_t
            _s_prev_sibling(_m_ptr_t _node)
                requires _is_parent_node<_m_node_t>
            { return _node->_m_prev_sibling(); }

            static constexpr _m_cptr_t
            _s_prev_sibling(_m_cptr_t _node)
                requires _is_parent_node<_m_node_t>
            { return _node->_m_prev_sibling(); }

            static constexpr _m_ptr_t
            _s_first_child(_m_ptr_t _node)
            {
                if (_s_is_leaf(_node))
                    return nullptr;
                return *std::ranges::begin(_s_children(_node));
            }

            static constexpr _m_cptr_t
            _s_first_child(_m_cptr_t _node)
            {
                if (_s_is_leaf(_node))
                    return nullptr;
                return *std::ranges::begin(_s_children(_node));
            }

            static constexpr _m_ptr_t
            _s_last_child(_m_ptr_t _node)
            {
                if (_s_is_leaf(_node))
                    return nullptr;
                return *(std::ranges::end(_s_children(_node)) - 1);
            }

            static constexpr _m_cptr_t
            _s_last_child(_m_cptr_t _node)
            {
                if (_s_is_leaf(_node))
                    return nullptr;
                return *(std::ranges::end(_s_children(_node)) - 1);
            }
        };
    }
}

#endif