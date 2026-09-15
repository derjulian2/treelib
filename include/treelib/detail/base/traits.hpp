
#ifndef TREELIB_BASE_TRAITS_HPP
#define TREELIB_BASE_TRAITS_HPP

/*******************************************************************************************
 * @file   treelib/detail/base/traits.hpp
 * @author Julian Benzel
 * @date   14.09.2026
 *
 * @brief  requirements and unified interfaces
 *         for various tree-related objects 
 *         (nodes, iterators).
 *
 * @details nodes consist of recursion-points
 *          that somehow reference the root-node of
 *          a subtree (e.g. .left/.right in a binary-tree).
 *          
 *          these recursion-points are owning references, meaning
 *          the lifetime of a node that is referenced by a recursion-point 
 *          of another node will be dependent on the lifetime of that parent-node.
 *
 *          example. consider the following node-specification:
 *
 *              BTree ::= BNil | BNode x (BTree) (BTree)
 *
 *          which could be implemented by the following structure:
 *
 *              struct bnode {
 *                  bnode *left;
 *                  bnode *right;
 *                  
 *                  typedef bool hook_type;
 *              };
 *
 *          which would mean that this node has two recursion-points
 *          which are addressed with a boolean true;(left)/false;(right) value.
 *
 *          when trying to erase a node here, there would be a 'hole' in the
 *          parent's recursion-point for that node, because it would point to
 *          the memory of the deallocated node and there would be no way
 *          to securely delete a node without some '.parent'-back-reference to
 *          notify the relevant nodes about the change.
 *          
 *          this property is something that seperates nodes like these from 
 *          others regarding it's capabilities.
 *
 * @todo    noexcept-specifiers.
 *******************************************************************************************/

#include <treelib/detail/bits/except.hpp>

#include <concepts>
#include <ranges>

namespace tl
{
    namespace _detail
    {
        /*****************************************
         * @brief requirements of a node-type to
         *        be used in a tree-container.
         *****************************************/
        template <typename T>
        concept _node
            = std::default_initializable<T>
            && requires (T* t, const T* ct)
            {
                /***************************************************
                 * @brief 'hooking' two nodes refers to
                 *        entangling them in a way specified 
                 *        by a parameter-value of hook-type.
                 *      
                 *        e.g. t.hook_at(left, t') means entangle
                 *        the node t' as the left child of
                 *        node t.
                 *
                 *        unhook_at should bring the node and the
                 *        subtree at the respective recursion-point
                 *        into a seperated state and return the
                 *        node that was unhooked.
                 *
                 *        unhook_if should check if the passed
                 *        node is sitting at one of it's recursion-
                 *        points and seperate them if so.
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
                 *        a child-node @see tl::_detail::_listrose_node
                 *        as an example.
                 ***************************************************/
                { t->_m_children() }
                    -> std::ranges::range;
                { ct->_m_children() }
                    -> std::ranges::input_range;
            };


        /***************************************************
         * @brief   requirements of a node-type to
         *          with copy-semantics.
         ***************************************************/
        template <typename T>
        concept _copyable_node
            = _node<T>
            && requires(T* t, const T* ct)
            {
                /***************************************************
                 * @brief because trees are non-linear
                 *        structures, we cannot just do .push_back()
                 *        for each node to copy, but the node-type has to
                 *        specify what insertions need to be called
                 *        in which order to get to a new valid
                 *        node-state from a known node (essentially
                 *        performing a 'structural-copy'), which is
                 *        what the interface-method 'mimic' should
                 *        expose to the tree.
                 *        the implementation should also recursively
                 *        call '.mimic()' on all child-nodes to
                 *        ensure a full copy.
                 *
                 *        '.mimic()' should essentially bring a
                 *        null-initialized node into a state
                 *        where it is structuraly identical
                 *        to the source-node.          
                 *
                 *        the function that will be passed to
                 *        allow the implementation to specify where
                 *        copies should go will be some form of
                 *        tree.insert(...) to ensure that if
                 *        allocation fails, the tree will remain in
                 *        a predictable state.
                 ***************************************************/
                { t->_m_mimic(ct, [](typename T::_m_hook_t _at, T* _parent, const T* _src) -> void { }) };
            };
    
        
        /***************************************************
         * @brief requirements for a bidirectional node-type,
         *        i.e. a node-type with a back-reference to
         *        it's owning node.
         ***************************************************/
        template <typename T>
        concept _parent_node
            = requires(T* t, const T* ct)
            {   
                /***************************************************
                 * @brief parent accessors.
                 ***************************************************/
                { t->_m_get_parent() }
                    -> std::convertible_to<T*>;
                { ct->_m_get_parent() }
                    -> std::convertible_to<const T*>;
                
                /***************************************************
                 * @brief sibling accessors.
                 ***************************************************/
                { t->_m_next_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_m_next_sibling() }
                    -> std::convertible_to<const T*>;
                { t->_m_prev_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_m_prev_sibling() }
                    -> std::convertible_to<const T*>;

                /***************************************************
                 * @brief unhooking the target-node itself,
                 *        not just a child at a specific hook.
                 *
                 *            _____--_--_
                 *           /    |   o |
                 *          /| ___ L_\ ||  
                 *           L|   L|  `||`
                 *
                 *        this my emotional-support elephant. 
                 *        his name is sam.
                 ***************************************************/
                { t->_m_unhook() };
            };


        /***************************************************
         * @brief additional functionality and unified
         *        interface for tree-node-types.
         ***************************************************/
        template <typename NodeT>
            requires _node<NodeT>
        struct _node_traits
        {
            using _m_node_t = NodeT;
            using _m_ptr_t  = _m_node_t*;
            using _m_cptr_t = const _m_node_t*;
            using _m_ref_t  = _m_node_t&;
            using _m_cref_t = const _m_node_t&;

            using _m_hook_t  = _m_node_t::_m_hook_t;
            using _m_depth_t = std::size_t;

            /***************************************************
             * @brief information.
             ***************************************************/

            static constexpr bool
            _s_is_leaf(_m_cptr_t _node)
                _treelib_noexcept_if_member(_m_node_t, _m_children)
            { return std::ranges::empty(_node->_m_children()); }

            static constexpr bool
            _s_is_root(_m_cptr_t _node)
                _treelib_noexcept_if_member(_m_node_t, _m_get_parent)
                requires _parent_node<_m_node_t>
            { return _node->_m_get_parent() == nullptr; }

            static constexpr _m_depth_t
            _s_depth(_m_cptr_t _node)
        #ifdef _treelib_store_depth
                _treelib_noexcept_if_member(_m_node_t, _m_get_depth)
            { return _node->_m_get_depth() }
        #else
                _treelib_noexcept_if_member(_m_node_t, _m_get_parent)
            {
                _m_depth_t _res { 0 };
                while ((_node = _node->_m_get_parent()))
                    ++_res;
                return _res;
            }
        #endif

            static constexpr std::size_t
            _s_child_count(_m_cptr_t _node)
                noexcept
            { return std::ranges::size(_node->_m_children()); }

            /***************************************************
             * @brief hook-functionality.
             ***************************************************/

            static constexpr void
            _s_hook_at(_m_ptr_t _parent, _m_hook_t _at, _m_ptr_t _node)
            { _parent->_m_hook_at(_at, _node); }

            static constexpr _m_ptr_t
            _s_unhook_at(_m_ptr_t _parent, _m_hook_t _at)
            { return _parent->_m_unhook_at(_at); }

            static constexpr void
            _s_unhook_if(_m_ptr_t _parent, _m_ptr_t _node)
            { _parent->_m_unhook_if(_node); }

            /***************************************************
             * @brief child accessors.
             ***************************************************/

            static constexpr decltype(auto)
            _s_children(_m_ptr_t _node)
            { return _node->_m_children(); }

            static constexpr decltype(auto)
            _s_children(_m_cptr_t _node)
            { return _node->_m_children(); }

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

            /***************************************************
             * @brief sibling accessors.
             ***************************************************/

            static constexpr _m_ptr_t
            _s_next_sibling(_m_ptr_t _node)
                requires _parent_node<_m_node_t>
            { return _node->_m_next_sibling(); }

            static constexpr _m_cptr_t
            _s_next_sibling(_m_cptr_t _node)
                requires _parent_node<_m_node_t>
            { return _node->_m_next_sibling(); }

            static constexpr _m_ptr_t
            _s_prev_sibling(_m_ptr_t _node)
                requires _parent_node<_m_node_t>
            { return _node->_m_prev_sibling(); }

            static constexpr _m_cptr_t
            _s_prev_sibling(_m_cptr_t _node)
                requires _parent_node<_m_node_t>
            { return _node->_m_prev_sibling(); }

            /***************************************************
             * @brief structural-copy.
             ***************************************************/

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_ptr_t, _m_cptr_t>
            constexpr void
            _s_mimic(_m_ptr_t _node, _m_cptr_t _src, Fn&& _insert_fn)
                requires _copyable_node<_m_node_t>
            { _node->_m_mimic(_src, std::forward<Fn>(_insert_fn)); }

        };


        /***************************************************
         * @brief uniform interface for tree-iterators.
         ***************************************************/
        template <typename IterT>
        struct _iter_traits
        {
            using _m_iter_t = IterT;
            using _m_node_t = typename _m_iter_t::_m_node_t;
            using _m_node_ptr_t = typename _m_iter_t::_m_node_ptr_t;

            using _m_value_t = typename IterT::value_type;
            using _m_ref_t   = typename IterT::reference;
            using _m_ptr_t   = typename IterT::pointer;

            static constexpr bool
            _s_constness = std::is_const_v<std::remove_reference_t<_m_ref_t>>;

            static constexpr _m_iter_t
            _s_to_iter(_m_node_ptr_t _node)
                noexcept(std::is_nothrow_constructible_v<_m_iter_t, _m_node_ptr_t>)
            { return IterT(_node); }

            static constexpr _m_node_ptr_t 
            _s_to_node(const _m_iter_t& _iter)
                noexcept
            { 
                static_assert(_treelib_member_noexcept(_m_iter_t, _m_cur),
                    "current-node accessor should not throw");
                return _iter._m_cur(); 
            }
        };
    }
}

#endif