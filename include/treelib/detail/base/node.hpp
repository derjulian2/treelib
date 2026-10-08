
#ifndef TREELIB_BASE_NODE_HPP
#define TREELIB_BASE_NODE_HPP

 /***********************************************************************
  * @file   treelib/detail/base/node.hpp
  * @author Julian Benzel
  * @date   04.10.2026
  *
  * @brief   requirements, traits and extensions for node-types.
  *
  * @details some of these classes are designed to be inherited
  *          from a third 'unifying'-type that will pass itself 
  *          as a template-parameter to he respective CRTP-base. 
  *
  *          when used this way, the CRTP-base can use functionality
  *          of the final node-type as well as extend/encapsulate
  *          functionality from the base-type.
  *
  *          @see tl::_detail::_bidirectional_k_node as
  *          an example of this:
  *
  *              k_node_base<bidirectional_k_node>
  *                (providing .hook_at(...))
  *                             |
  *              bidirectional_node<bidirectional_k_node>
  *                (which is able to:
  *                   1. assign .parent = (node*)this (CRTP)
  *                   2. and encapsulate .base::hook_at(...) (mixin))
  *                             |
  *                        [optionally]
  *              depth_node<bidirectional_k_node>
  *                             |
  *                  bidirectional_k_node 
  *
  *          with the final type unifying struct 'bidirectional_k_node' 
  *          injected throughout the linear inheritance-chain.
  *
  *          to avoid specifying the final type twice,
  *          the classes will get the final type via
  *          'NodeBaseT::_m_node_t', which is just a
  *          sort of 'indirect'-CRTP. it is always assumed
  *          that a cast to the final/derived type '_m_node_t*' is valid, 
  *          making these types sort of CRTP-classes and mixins.
  ***********************************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/bits/initializer_tree.hpp>

#include <type_traits>
#include <utility>
#include <concepts>
#include <ranges>
#include <cassert>

namespace tl
{
    namespace _detail
    {
        /******************************************************************
         * @brief   requirements for node that require
         *          dynamic-memory allocation-capabilities.
         *
         * @details some node-types requires additional 
         *          dynamic-memory allocation-functionality to
         *          properly function (e.g. rose-trees storing
         *          their child-pointers in a vector).
         *  
         *          this is similiar to e.g. std::unordered_set 
         *          which allocates not only it's nodes, but also the
         *          bucket-arrays, which are of a different type.
         *
         *          any allocators used internally by the node
         *          should be copy-constructed from the allocator
         *          of the associated tree-instance, which will
         *          get passed by @see tl::_detail::_alloc_base during
         *          node-construction.
         ******************************************************************/
        template <typename T>
        concept _dynamic_tree_node
            = std::constructible_from<T, const typename T::_m_alloc_t&>;

        /********************************************************
         * @brief requirements of a node-type with
         *        ordered semantics on it's child-nodes.
         ********************************************************/
        template <typename T>
        concept _ordered_tree_node
            = requires(T* t, const T* ct)
            {
                /***************************************************
                 * an ordered node considers some of
                 * it's child-nodes as left and
                 * some as right of itself.
                 * this is used to implement in-order-traversal
                 * for any node that supports it.
                 ***************************************************/
                { t->_m_left_children() };
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(t->_m_left_children())>,
                            T*>;
                { ct->_m_left_children() };
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(t->_m_left_children())>,
                            T*>;
                { t->_m_right_children() };
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(t->_m_left_children())>,
                            T*>;
                { ct->_m_right_children() };
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(t->_m_left_children())>,
                            T*>;
                
            };

        /***************************************************
         * @brief requirements of a node-type
         *        with copying-capabilities.
         ***************************************************/
        template <typename T>
        concept _mimicable_tree_node
            = requires(T* t, const T* ct)
            {
                /***************************************************
                 * because trees are non-linear
                 * structures, we cannot just do .push_back()
                 * for each node to copy, but the node-type has to
                 * specify what insertions need to be called
                 * in which order to get to a new valid
                 * node-state from a known node (essentially
                 * performing a 'structural-copy'), which is
                 * what the interface-method 'mimic' should
                 * expose to the tree.
                 * the implementation should also recursively
                 * call '.mimic()' on all child-nodes to
                 * ensure a full copy.
                 *
                 * '.mimic()' should essentially bring a
                 * node into a state where it is structuraly 
                 * identical to the source-node, requesting
                 * insertions for new nodes, assignments
                 * for existing nodes (as in
                 * value-assignments, so that e.g. moving can
                 * reuse existing nodes) and deletions
                 * as required.    
                 *
                 * the function that will be passed to
                 * allow the implementation to specify where
                 * copies should go will be some form of
                 * tree.insert(...) to ensure that if
                 * allocation fails, the tree will remain in
                 * a predictable state.
                 ***************************************************/
                { t->_m_mimic(ct,
                              // insertion-function 
                              [](const T* _src) -> T* { return nullptr; },
                              // erasure-function
                              [](T* _node) -> void { },
                              // assignment-function
                              [](T* _dest, const T* _src) -> void { }) };
            };

        /*********************************************************************************
         * @brief   requirements of a node-type to
         *          be used in a tree-container.
         *
         * @details nodes consist of recursion-points
         *          that somehow reference the root-node of
         *          a subtree (e.g. .left/.right in a binary-tree).
         *          here, these are all represented as pointers.
         *          
         *          these recursion-points are owning references, meaning the
         *          lifetime of a node that is referenced by such a recursion-point 
         *          of another node will be dependent on the lifetime of that parent-node.
         *********************************************************************************/
        template <typename T>
        concept _tree_node
            = (_dynamic_tree_node<T> || std::default_initializable<T>)
            && requires (T* t, const T* ct)
            {
                /***************************************************
                 * 'hooking' two nodes refers to
                 * entangling them in a way specified 
                 * by a parameter-value of hook-type.
                 *      
                 * e.g. t.hook_at(left, t') means entangle
                 * the node t' as the left child of
                 * node t.
                 *
                 * unhook_at should bring the node and the
                 * subtree at the respective recursion-point
                 * into a seperated state and return the
                 * node that was unhooked.
                 *
                 * unhook_if should check if the passed
                 * node is sitting at one of it's recursion-
                 * points and seperate them if so.
                 ***************************************************/
                typename T::_m_hook_t;
                { t->_m_hook_at(std::declval<typename T::_m_hook_t>(), t) };
                { t->_m_unhook_at(std::declval<typename T::_m_hook_t>()) }
                    -> std::convertible_to<T*>;
                { t->_m_unhook_if(t) };

                /***************************************************
                 * accessors to all nodes that should be
                 * considered a child of the current node,
                 * implying that it is 'structurally deeper'
                 * inside the tree's hierarchy.
                 ***************************************************/
                { t->_m_children() }
                    -> std::ranges::range;
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(t->_m_children())>,
                            T*>;
                { ct->_m_children() }
                    -> std::ranges::range;
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(ct->_m_children())>,
                            T*>;
            };

        /********************************************************
         * @brief requirements for a bidirectional node-type,
         *        i.e. a node-type with a back-reference to
         *        it's owning node.
         ********************************************************/
        template <typename T>
        concept _bidirectional_tree_node
            = requires(T* t, const T* ct)
            {   
                /*********************
                 * parent accessors.
                 *********************/
                { t->_m_parent() }
                    -> std::convertible_to<T*>;
                { ct->_m_parent() }
                    -> std::convertible_to<T*>;
            };

        /***************************************************
         * @brief template-class that adds an instance
         *        of value-type to the passed node-type.
         *
         *        instances of this class will actually be 
         *        allocated by the tree-container.
         ***************************************************/
        template <typename NodeT, 
                  typename ValueT>
        class _value_node
            : public NodeT
        {
        public:
        
            using _m_value_t = ValueT;
            using _m_ref_t   = _m_value_t&;
            using _m_cref_t  = const _m_value_t&;
            using _m_ptr_t   = _m_value_t*;
            using _m_cptr_t  = const _m_value_t*;
        
            using _m_node_t = NodeT;

        protected:

            _m_value_t _m_data;

        public:

            /***************************************************
             * @brief constructor (1).
             *        forward all args to value-type.
             ***************************************************/
            template <typename... ArgsTs>
            constexpr
            _value_node(ArgsTs&&... _args)
                noexcept(std::is_nothrow_constructible_v<_m_value_t, ArgsTs...>
                         && std::is_nothrow_constructible_v<_m_node_t>)
                requires std::default_initializable<_m_node_t>
                : _m_node_t()
                , _m_data(std::forward<ArgsTs>(_args)...)
            { }

            /***************************************************
             * @brief constructor (2).
             *        dynamic-node constructor. 
             ***************************************************/
            template <typename... ArgsTs, typename T = _m_node_t>
            constexpr
            _value_node(const T::_m_alloc_t& _alloc, 
                        ArgsTs&&... _args)
                noexcept(std::is_nothrow_constructible_v<_m_value_t, ArgsTs...>
                         && std::is_nothrow_constructible_v<_m_node_t, const typename T::_m_alloc_t&>)
                requires _dynamic_tree_node<_m_node_t>
                : _m_node_t(_alloc)
                , _m_data(std::forward<ArgsTs>(_args)...)
            { }

            /***************************************************
             * value accessors.
             ***************************************************/

            [[nodiscard]]
            constexpr _m_ref_t
            _m_value()
                noexcept
            { return this->_m_data; }

            [[nodiscard]]
            constexpr _m_cref_t
            _m_value()
                const noexcept
            { return this->_m_data; }
        };

        /***************************************************
         * @brief template-class that extends the
         *        passed node-base-type by an additional
         *        back-pointer to it's owning parent-node.
         ***************************************************/
        template <typename NodeBaseT>
        class _bidirectional_node
            : public NodeBaseT
        {
        protected:

            using _m_base_t = NodeBaseT;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;
            using typename _m_base_t::_m_hook_t;

            _m_node_ptr_t _m_parent_node;

            /***************************************************
             * CRTP-base-class can assume that a 
             * cast of itself to derived-class is valid.
             ***************************************************/
            
            constexpr _m_node_ptr_t
            _m_node_ptr()
                noexcept
            { return static_cast<_m_node_ptr_t>(this); }

            constexpr _m_cnode_ptr_t
            _m_node_ptr()
                const noexcept
            { return static_cast<_m_cnode_ptr_t>(this); }

            /********************************************************
             * @brief constructor (1).
             *        default-constructible,
             *        initializes parent to nullptr.
             *
             *        this constructor is marked protected because
             *        this CRTP-base-class  should not be 
             *        instantiated on it's own.
             ********************************************************/
            template <typename... ArgsTs>
            constexpr
            _bidirectional_node(ArgsTs&&... _args)
                noexcept(std::is_nothrow_constructible_v<_m_base_t>)
                : _m_base_t(std::forward<ArgsTs>(_args)...)
                , _m_parent_node(nullptr)
            { }

        public:
                
            /***************************************************
             * @brief parent accessor.
             ***************************************************/
            constexpr _m_node_ptr_t
            _m_parent()
                const noexcept
            { return this->_m_parent_node; }

            /***************************************************
             * forward hooking/unhooking to base-class
             * but also set/reset the hooked-node's parent-
             * pointer to this/nullptr.
             ***************************************************/

            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
                _treelib_noexcept_if(this->_m_base_t::_m_hook_at(_at, _node))
            {
                this->_m_base_t::_m_hook_at(_at, _node);
                _node->_m_parent_node = this->_m_node_ptr();
            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t _at)
                _treelib_noexcept_if(this->_m_base_t::_m_unhook_at(_at))
            {
                _m_node_ptr_t _res = this->_m_base_t::_m_unhook_at(_at);
                _res->_m_parent_node = nullptr;
                return _res;
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t _node)
                _treelib_noexcept_if(this->_m_base_t::_m_unhook_if(_node))
            {
                this->_m_base_t::unhook_if(_node);
                _node->_m_parent_node = nullptr;
            }

            /**********************************************
             * a unique feature of bidirectional nodes
             * is being able to not just unhook a child, 
             * but also itself with only a this pointer,
             * since the parent-node can be notified 
             * of the desired change. 
             **********************************************/
            constexpr void
            _m_unhook()
                _treelib_noexcept_if(this->_m_parent_node->_m_base_t::_m_unhook_if(this->_m_node_ptr()))
            {
                this->_m_parent_node->_m_base_t::_m_unhook_if(this->_m_node_ptr());
                this->_m_parent_node = nullptr;
            }
        };
        
        /********************************************************
         * @brief template-class that extends the
         *        passed node-base-type by a member-variable
         *        keeping track of the node's depth in the
         *        tree (the distance from the root-node).
         ********************************************************/
        template <typename NodeBaseT>
        class _depth_node
            : public NodeBaseT
        {
        protected:

            using _m_base_t = NodeBaseT;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;
            using typename _m_base_t::_m_hook_t;
            using _m_depth_t = std::size_t;

            _m_depth_t _m_node_depth;

            /********************************************************
             * @brief constructor (1).
             *        default-constructible,
             *        initializes depth to 0.
             *
             *        this constructor is marked protected because
             *        this CRTP-base-class  should not be 
             *        instantiated on it's own.
             ********************************************************/
            template <typename... ArgsTs>
            constexpr
            _depth_node(ArgsTs&&... _args)
                _treelib_noexcept_if(_m_base_t())
                : _m_base_t(std::forward<ArgsTs>(_args)...)
                , _m_node_depth(0)
            { }

        public:

            /***************************************************
             * @brief set the depth-member to the desired
             *        value and recursively update all child-
             *        nodes.
             ***************************************************/
            constexpr void
            _m_update_depth(_m_depth_t _depth)
                noexcept(_treelib_noexcept_iterable(this->_m_children()))
            {
                this->_m_node_depth = _depth;
                for (_m_node_ptr_t _child
                     : this->_m_children())
                     _child->_m_update_depth(_depth + 1);
            }
        
            /***************************************************
             * @brief depth accessor.
             ***************************************************/
            constexpr _m_depth_t
            _m_depth()
                const noexcept
            { return this->_m_node_depth; }

            /***************************************************
             * recursively update the depth-values when
             * hooking another node.
             ***************************************************/
            
            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
                _treelib_noexcept_if(this->_m_base_t::_m_hook_at(_at, _node))
            {
                this->_m_base_t::_m_hook_at(_at, _node);
                _node->_m_update_depth(this->_m_node_depth + 1);
            }
        };

        /********************************************************
         * @brief template-class that extends the
         *        passed node-base-type by a member-variable
         *        keeping track of the tree's-height, rooted
         *        at the current-node (the distance to the 
         *        furthest child-node).
         ********************************************************/
        template <typename NodeBaseT>
        class _height_node
            : public NodeBaseT
        {
        protected:

            using _m_base_t = NodeBaseT;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;
            using typename _m_base_t::_m_hook_t;
            using _m_height_t = std::size_t;

            _m_height_t _m_node_height;

            /********************************************************
             * @brief constructor (1).
             *        default-constructible,
             *        initializes height to 0.
             *
             *        this constructor is marked protected because
             *        this CRTP-base-class  should not be 
             *        instantiated on it's own.
             ********************************************************/
            template <typename... ArgsTs>
            constexpr
            _height_node(ArgsTs&&... _args)
                _treelib_noexcept_if(_m_base_t())
                : _m_base_t(std::forward<ArgsTs>(_args)...)
                , _m_node_height(0)
            { }

            /***************************************************
             * find the maximum height of all children,
             * or 0 if this node is a leaf.
             ***************************************************/

            constexpr _m_height_t
            _m_determine_height()
                const noexcept(_treelib_noexcept_iterable(this->_m_children()))
            {
                _m_height_t _res {0};
                for (_m_node_ptr_t _child
                     : this->_m_children())
                    _res = std::max(_child->_m_height(), _res);
                return _res;
            }

        public:

            /***************************************************
             * @brief sets the height-value of this node to
             *        the desired value and recursively updates
             *        the height value upwards in the tree.
             ***************************************************/
            constexpr void
            _m_update_height(_m_height_t _height)
                _treelib_noexcept_if(this->_m_parent())
            {
                this->_m_node_height = _height;
                if (this->_m_parent() != nullptr)
                    this->_m_parent()->_m_update_height(
                        std::max(this->_m_parent()->_m_height(), this->_m_height() + 1)
                    );
            }

            /***************************************************
             * @brief height accessor.
             ***************************************************/
            constexpr _m_height_t
            _m_height()
                const noexcept
            { return this->_m_node_height; }

            /***************************************************
             * recursively update the height-values when
             * hooking another node or unhooking another node.
             ***************************************************/

            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
                noexcept(noexcept(this->_m_base_t::_m_hook_at(_at, _node))
                         && noexcept(this->_m_parent()))
            {
                this->_m_base_t::_m_hook_at(_at, _node);
                this->_m_update_height(_node->_m_height() + 1);
            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t _at)
                noexcept(noexcept(this->_m_base_t::_m_unhook_at(_at))
                         && noexcept(this->_m_update_height(this->_m_determine_height())))
            {
                _m_node_ptr_t _res = this->_m_base_t::_m_unhook_at(_at);
                this->_m_update_height(this->_m_determine_height());
                return _res;
            }
        };

        /***************************************************
         * @brief additional functionality and unified
         *        interface for tree-node-types.
         ***************************************************/
        template <typename NodeT>
            requires _tree_node<NodeT>
        struct _node_traits
        {
            using _m_node_t = NodeT;
            using _m_ptr_t  = _m_node_t*;
            using _m_cptr_t = const _m_node_t*;
            using _m_ref_t  = _m_node_t&;
            using _m_cref_t = const _m_node_t&;

            using _m_hook_t   = _m_node_t::_m_hook_t;
            using _m_depth_t  = std::size_t;
            using _m_height_t = std::size_t;

            template <typename ValueT>
            using _m_vnode_t      = _value_node<_m_node_t, ValueT>;
            template <typename ValueT>
            using _m_vnode_ptr_t  = _m_vnode_t<ValueT>*;
            template <typename ValueT>
            using _m_cvnode_ptr_t = const _m_vnode_t<ValueT>*;

            /***************************************************
             * information about a node's location in the tree.
             ***************************************************/

            // forward call to member-function if present
            static constexpr bool 
            _s_is_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_is_leaf())
                requires _treelib_has_member(_m_cref_t, _m_is_leaf)
            { return _node->_m_is_leaf(); }

            // SFINAE fallback
            static constexpr bool
            _s_is_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(std::ranges::empty(_node->_m_children()))
            { return std::ranges::empty(_node->_m_children()); }

            // forward call to member-function if present
            static constexpr bool 
            _s_is_left_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_is_left_leaf())
                requires (_ordered_tree_node<_m_node_t> 
                          && _treelib_has_member(_m_cref_t, _m_is_left_leaf))
            { return _node->_m_is_left_leaf(); }

            // SFINAE fallback
            static constexpr bool
            _s_is_left_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(std::ranges::empty(_node->_m_left_children()))
                requires _ordered_tree_node<_m_node_t>
            { return std::ranges::empty(_node->_m_left_children()); }

            // forward call to member-function if present
            static constexpr bool 
            _s_is_right_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_is_right_leaf())
                requires (_treelib_has_member(_m_cref_t, _m_is_right_leaf)
                          && _ordered_tree_node<_m_node_t>)
            { return _node->_m_is_right_leaf(); }

            // SFINAE fallback
            static constexpr bool
            _s_is_right_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(std::ranges::empty(_node->_m_right_children()))
                requires _ordered_tree_node<_m_node_t>
            { return std::ranges::empty(_node->_m_right_children()); }

            static constexpr bool
            _s_is_root(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_parent())
                requires _bidirectional_tree_node<_m_node_t>
            { return _node->_m_parent() == nullptr; }

            static constexpr bool
            _s_is_first_child_of(_m_cptr_t _parent, _m_cptr_t _node)
                _treelib_noexcept_if(_s_first_child(_parent))
            { return _s_first_child(_parent) == _node; }

            static constexpr bool
            _s_is_first_child(_m_cptr_t _node)
                _treelib_noexcept_if(_s_is_first_child_of(_s_parent(_node), _node))
                requires _bidirectional_tree_node<_m_node_t>
            { 
                assert(!_s_is_root(_node));
                return _s_is_first_child_of(_s_parent(_node), _node); 
            }

            static constexpr bool
            _s_is_last_child_of(_m_cptr_t _parent, _m_cptr_t _node)
                _treelib_noexcept_if(_s_last_child(_parent))
            { return _s_last_child(_parent) == _node; }

            static constexpr bool
            _s_is_last_child(_m_cptr_t _node)
                _treelib_noexcept_if(_s_is_last_child_of(_s_parent(_node), _node))
                requires _bidirectional_tree_node<_m_node_t>
            { 
                assert(!_s_is_root(_node));
                return _s_is_last_child_of(_s_parent(_node), _node); 
            }

            // forward call to member-function if present
            static constexpr std::size_t
            _s_child_count(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_child_count())
                requires _treelib_has_member(_m_cref_t, _m_child_count)
            { return _node->_m_child_count(); }

            // SFINAE fallback
            static constexpr std::size_t
            _s_child_count(_m_cptr_t _node)
                _treelib_noexcept_if_member_iterable_no_deref(_m_cref_t, _m_children)
            { 
                decltype(auto) _children = _node->_m_children();
                return std::ranges::distance(std::ranges::cbegin(_children),
                                             std::ranges::cend(_children)); 
            }

            // forward call to member-function if present
            static constexpr _m_height_t
            _s_height(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_height())
                requires (_bidirectional_tree_node<_m_node_t>
                          && _treelib_has_member(_m_cref_t, _m_height))
            { return _node->_m_height(); }

            // SFINAE fallback
            static constexpr _m_height_t
            _s_height(_m_cptr_t _node)
                _treelib_noexcept_if(_s_children(_node))
                requires _bidirectional_tree_node<_m_node_t>
            {
                _m_height_t _res {0};
                for (_m_cptr_t _child
                     : _s_children(_node))
                    _res = std::max(_s_height(_child) + 1, _res);
                return _res;
            }

            // forward call to member-function if present
            static constexpr _m_depth_t
            _s_depth(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_depth())
                requires (_bidirectional_tree_node<_m_node_t>
                          && _treelib_has_member(_m_cref_t, _m_depth))
            { return _node->_m_depth(); }

            // SFINAE fallback
            static constexpr _m_depth_t
            _s_depth(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_parent())
                requires _bidirectional_tree_node<_m_node_t>
            {
                _m_depth_t _res {0};
                while ((_node = _node->_m_parent()))
                    ++_res;
                return _res;
            }

            /***************************************************
             * hook-functionality.
             ***************************************************/

            static constexpr void
            _s_hook_at(_m_ptr_t _parent, _m_hook_t _at, _m_ptr_t _node)
                _treelib_noexcept_if(_parent->_m_hook_at(_at, _node))
            { _parent->_m_hook_at(_at, _node); }

            static constexpr _m_ptr_t
            _s_unhook_at(_m_ptr_t _parent, _m_hook_t _at)
                _treelib_noexcept_if(_parent->_m_unhook_at(_at))
            { return _parent->_m_unhook_at(_at); }

            static constexpr void
            _s_unhook_if(_m_ptr_t _parent, _m_ptr_t _node)
                _treelib_noexcept_if(_parent->_m_unhook_if(_node))
            { _parent->_m_unhook_if(_node); }

            // forward call to member-function if present
            static constexpr void
            _s_unhook(_m_ptr_t _node)
                _treelib_noexcept_if(_node->_m_unhook())    
                requires (_bidirectional_tree_node<_m_node_t>
                          && _treelib_has_member(_m_node_t, _m_unhook))
            { _node->_m_unhook(); }
        
            // SFINAE fallback
            static constexpr void
            _s_unhook(_m_ptr_t _node)
                _treelib_noexcept_if(_s_unhook_if(_s_parent(_node), _node))
                requires _bidirectional_tree_node<_m_node_t>
            { _s_unhook_if(_s_parent(_node), _node); }

            /***************************************************
             * related-node accessors.
             ***************************************************/

            static constexpr decltype(auto)
            _s_children(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_children())
            { return _node->_m_children(); }

            static constexpr decltype(auto)
            _s_left_children(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_left_children())
                requires _ordered_tree_node<_m_node_t>
            { return _node->_m_left_children(); }

            static constexpr decltype(auto)
            _s_right_children(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_right_children())
                requires _ordered_tree_node<_m_node_t>
            { return _node->_m_right_children(); }

            static constexpr _m_ptr_t
            _s_parent(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_parent())
                requires _bidirectional_tree_node<_m_node_t>
            { return _node->_m_parent(); }

            static constexpr _m_ptr_t
            _s_first_child(_m_cptr_t _node)
                noexcept(noexcept(_s_is_leaf(_node))
                         && _treelib_noexcept_first_readable(_s_children(_node)))
            {
                assert(!_s_is_leaf(_node));
                return std::views::all(_s_children(_node)).front();
            }

            static constexpr _m_ptr_t
            _s_first_left_child(_m_cptr_t _node)
                noexcept(noexcept(_s_is_left_leaf(_node))
                         && _treelib_noexcept_first_readable(_s_left_children(_node)))
                requires _ordered_tree_node<_m_node_t>
            {
                assert(!_s_is_left_leaf(_node));
                return std::views::all(_s_left_children(_node)).front();
            }

            static constexpr _m_ptr_t
            _s_first_right_child(_m_cptr_t _node)
                noexcept(noexcept(_s_is_right_leaf(_node))
                         && _treelib_noexcept_first_readable(_s_right_children(_node)))
                requires _ordered_tree_node<_m_node_t>
            {
                assert(!_s_is_right_leaf(_node));
                return std::views::all(_s_right_children(_node)).front();
            }

            static constexpr _m_ptr_t
            _s_last_child(_m_cptr_t _node)
                noexcept(noexcept(_s_is_leaf(_node))
                         && _treelib_noexcept_last_readable(_s_children(_node)))
            {
                assert(!_s_is_leaf(_node));
                return std::views::all(_s_children(_node)).back();
            }

            static constexpr _m_ptr_t
            _s_last_left_child(_m_cptr_t _node)
                noexcept(noexcept(_s_is_left_leaf(_node))
                         && _treelib_noexcept_last_readable(_s_left_children(_node)))
                requires _ordered_tree_node<_m_node_t>
            {
                assert(!_s_is_left_leaf(_node));
                return std::views::all(_s_left_children(_node)).back();
            }

            static constexpr _m_ptr_t
            _s_last_right_child(_m_cptr_t _node)
                noexcept(noexcept(_s_is_right_leaf(_node))
                         && _treelib_noexcept_last_readable(_s_right_children(_node)))
                requires _ordered_tree_node<_m_node_t>
            {
                assert(!_s_is_right_leaf(_node));
                return std::views::all(_s_right_children(_node)).back();;
            }

            // forward call to member-function if present
            static constexpr _m_ptr_t
            _s_next_sibling(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_next_sibling())
                requires (_bidirectional_tree_node<_m_node_t> 
                          && _treelib_has_member(_m_cref_t, _m_next_sibling))
            { return _node->_m_next_sibling(); }

            // SFINAE fallback
            static constexpr _m_ptr_t
            _s_next_sibling(_m_cptr_t _node)
                noexcept(_treelib_noexcept_iterable(_s_children(_node))
                         && noexcept(_s_is_root(_node))
                         && noexcept(_s_is_last_child(_node))
                         && noexcept(_s_parent(_node)))
                requires _bidirectional_tree_node<_m_node_t>
            {
                assert(!(_s_is_root(_node) || _s_is_last_child(_node)));
                decltype(auto) _children = _s_children(_s_parent(_node));
                return *(std::find(std::ranges::cbegin(_children),
                                   std::ranges::cend(_children),
                                   _node) + 1);
            }

            // forward call to member-function if present
            static constexpr _m_ptr_t
            _s_prev_sibling(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_prev_sibling())
                requires (_bidirectional_tree_node<_m_node_t> 
                          && _treelib_has_member(_m_cref_t, _m_prev_sibling))
            { return _node->_m_prev_sibling(); }

            // SFINAE fallback
            static constexpr _m_ptr_t
            _s_prev_sibling(_m_cptr_t _node)
                noexcept(_treelib_noexcept_iterable(_s_children(_node))
                         && noexcept(_s_is_root(_node))
                         && noexcept(_s_is_first_child(_node))
                         && noexcept(_s_parent(_node)))
                requires _bidirectional_tree_node<_m_node_t>
            {
                assert(!(_s_is_root(_node) || _s_is_first_child(_node)));
                decltype(auto) _children = _s_children(_s_parent(_node));
                return *(std::find(std::ranges::cbegin(_children),
                                   std::ranges::cend(_children),
                                   _node) - 1);
            }

            static constexpr _m_ptr_t
            _s_seek_leftmost(_m_cptr_t _node)
                noexcept(noexcept(_s_is_leaf(_node)) && noexcept(_s_first_child(_node)))
            {
                assert(!_s_is_leaf(_node));
                _m_ptr_t _res;
                do 
                { _res = _s_first_child(_node); }
                while (!_s_is_leaf(_node));
                return _res;
            }

            static constexpr _m_ptr_t
            _s_seek_rightmost(_m_cptr_t _node)
                noexcept(noexcept(_s_is_leaf(_node)) && noexcept(_s_last_child(_node)))
            {
                assert(!_s_is_leaf(_node));
                _m_ptr_t _res;
                do 
                { _res = _s_last_child(_node); }
                while (!_s_is_leaf(_node));
                return _res;
            }

            /***************************************************
             * structural-imitation to implement copy and
             * construction from initializer-nodes.
             ***************************************************/

            template <typename InsertFnT, typename EraseFnT, typename AssignFnT>
                requires _mimicable_tree_node<_m_node_t>
            static constexpr void
            _s_mimic(_m_ptr_t _node, 
                     _m_cptr_t _src, 
                     InsertFnT&& _insert_fn,
                     EraseFnT&& _erase_fn,
                     AssignFnT&& _assign_fn)
            { _node->_m_mimic(_src, _treelib_fwd(_insert_fn), _treelib_fwd(_erase_fn), _treelib_fwd(_assign_fn)); }

            template <typename InitT, typename FnT>
                requires _initializer_compatible<_m_node_t, InitT>
            static constexpr void
            _s_mimic_initializer(_m_ptr_t _node, const InitT& _init, FnT&& _insert_fn)
            { _node->_m_mimic_initializer(_init, std::forward<FnT>(_insert_fn)); }

        };
    }
}

#endif