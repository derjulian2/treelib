
#ifndef TREELIB_BASE_NODE_HPP
#define TREELIB_BASE_NODE_HPP

 /***********************************************************************
  * @file   treelib/detail/base/node.hpp
  * @author Julian Benzel
  * @date   25.09.2026
  *
  * @brief   mixins/CRTP-mixins and traits for node-types.
  *
  * @details some of these classes are designed to be inherited
  *          from a third 'unifying'-type that will
  *          pass itself as a template-parameter to
  *          the respective CRTP-base-classes. 
  *          this way we get the type with all required 
  *          features into the node-base and into the extender,
  *          keeping '_m_node_t' fixed to the final node-type.
  *
  *          @see tl::_detail::_bidirectional_k_node as
  *          an example of this:
  *
  *              k_node_base<bidirectional_k_node>
  *                             |
  *              bidirectional_node<bidirectional_k_node>
  *                             |
  *     (optional) depth_node<bidirectional_k_node>
  *                             |
  *                 bidirectional_k_node 
  *
  *          with the final type injected throughout
  *          the linear inheritance-chain.
  *
  *          to avoid specifying the final type twice,
  *          the classes will get the final type via
  *          'NodeBaseT::_m_node_t', which is just a
  *          sort of 'indirect'-CRTP. it is always assumed
  *          that a cast to the final/derived type '_m_node_t*' 
  *          is valid, making these types CRTP-classes AND mixins.
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

        /***************************************************
         * @brief   requirements of a node-type to
         *          with copying-capabilities.
         ***************************************************/
        template <typename T>
        concept _copyable_tree_node
            = requires(T* t, const T* ct)
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

        /*********************************************************************************
         * @brief   requirements of a node-type to
         *          be used in a tree-container.
         *
         * @details nodes consist of recursion-points
         *          that somehow reference the root-node of
         *          a subtree (e.g. .left/.right in a binary-tree).
         *          
         *          these recursion-points are owning references, meaning the
         *          lifetime of a node that is referenced by such a recursion-point 
         *          of another node will be dependent on the lifetime of that parent-node.
         *
         *          this 'owning' hierarchy between nodes is seperate from the
         *          the actual tree-hierarchy though, to allow for nodes that
         *          own other nodes on the same 'level' as themselves. 
         *          these are therefore not considered children of that node, but
         *          their lifetime still depends on the non-parent node. 
         *********************************************************************************/
        template <typename T>
        concept _tree_node
            = (_dynamic_tree_node<T> || std::default_initializable<T>)
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
                 * @brief accessors to all nodes whose lifetime
                 *        should be considered dependent of that of
                 *        the current-node. 
                 *
                 *        these can be either children or
                 *        siblings of the current-node, but should
                 *        not be parent-nodes.
                 ***************************************************/
                { t->_m_subordinates() }
                    -> std::ranges::range;
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(t->_m_subordinates())>,
                            T*>;
                { ct->_m_subordinates() }
                    -> std::ranges::range;
                requires std::convertible_to<
                            std::ranges::range_value_t<decltype(ct->_m_subordinates())>,
                            T*>;

                /***************************************************
                 * @brief accessors to all nodes that should be
                 *        considered a child of the current node,
                 *        implying that it is 'structurally deeper'
                 *        inside the tree's hierarchy.
                 *
                 * @note  that not all children require a 
                 *        corresponding hook-value at all times
                 *        to qualify as a child-node. 
                 *        @see tl::_detail::_listrose_node
                 *        as an example.
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
                /***************************************************
                 * @brief parent accessors.
                 ***************************************************/
                { t->_m_parent() }
                    -> std::convertible_to<T*>;
                { ct->_m_parent() }
                    -> std::convertible_to<T*>;
                
                /***************************************************
                 * @brief sibling accessors.
                 ***************************************************/
                { t->_m_next_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_m_next_sibling() }
                    -> std::convertible_to<T*>;
                { t->_m_prev_sibling() }
                    -> std::convertible_to<T*>;
                { ct->_m_prev_sibling() }
                    -> std::convertible_to<T*>;

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
         * @brief mixin that adds an instance
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
                noexcept(std::is_nothrow_constructible_v<_m_value_t, ArgsTs...>)
                requires std::default_initializable<_m_node_t>
                : _m_node_t()
                , _m_data(std::forward<ArgsTs>(_args)...)
            { }

            /***************************************************
             * @brief constructor (2).
             *        dynamic-node constructor. 
             ***************************************************/

            template <typename... ArgsTs>
            constexpr
            _value_node(const auto& _alloc, 
                        ArgsTs&&... _args)
                noexcept(std::is_nothrow_constructible_v<_m_value_t, ArgsTs...>
                         && std::is_nothrow_constructible_v<_m_node_t, const typename _m_node_t::_m_alloc_t&>)
                requires (_dynamic_tree_node<_m_node_t>
                          && std::convertible_to<decltype(_alloc), typename _m_node_t::_m_alloc_t>)
                : _m_node_t(_alloc)
                , _m_data(std::forward<ArgsTs>(_args)...)
            { }

            /***************************************************
             * @brief value accessors.
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
         * @brief CRTP-mixin that extends the
         *        passed node-base-type by an additional
         *        back-pointer to it's owning parent-node.
         ***************************************************/
        template <typename NodeBaseT>
        class _bidirectional_node
            : public NodeBaseT
        {
        public:

            using _m_base_t      = NodeBaseT;
            using _m_node_t      = typename _m_base_t::_m_node_t;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;
            using typename _m_base_t::_m_hook_t;

        protected:

            _m_node_ptr_t _m_parent_node;

            /***************************************************
             * @brief since this is a CRTP-base,
             *        we can assume that casting itself to
             *        to a _m_node_ptr_t is valid. 
             ***************************************************/
            
            constexpr _m_node_ptr_t
            _m_node_ptr()
                noexcept
            { return static_cast<_m_node_ptr_t>(this); }

            constexpr _m_cnode_ptr_t
            _m_node_ptr()
                const noexcept
            { return static_cast<_m_cnode_ptr_t>(this); }

            /***************************************************
             * constructor (1).
             * default-constructible,
             * initializes parent to nullptr.
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             ***************************************************/
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
             * @brief forward hooking/unhooking to base-class
             *        but also set/reset the hooked-node's parent-
             *        pointer to this/nullptr.
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

            /***************************************************
             * @brief a unique feature of bidirectional nodes
             *        is being able to not just unhook a child, 
             *        but also itself with only a this pointer,
             *        since the parent-node can be notified 
             *        of the desired change. 
             ***************************************************/
            constexpr void
            _m_unhook()
                _treelib_noexcept_if(this->_m_parent_node->_m_base_t::_m_unhook_if(this->_m_node_ptr()))
            {
                this->_m_parent_node->_m_base_t::_m_unhook_if(this->_m_node_ptr());
                this->_m_parent_node = nullptr;
            }
        };
        
        /********************************************************
         * @brief CRTP-mixin that extends the
         *        passed node-base-type by a member-variable
         *        keeping track of the node's depth in the
         *        tree (the distance from the root-node).
         ********************************************************/
        template <typename NodeBaseT>
        class _depth_node
            : public NodeBaseT
        {
        public:

            using _m_base_t      = NodeBaseT;
            using _m_node_t      = typename _m_base_t::_m_node_t;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;
            using typename _m_base_t::_m_hook_t;
            using _m_depth_t     = std::size_t;

        protected:

            _m_depth_t _m_node_depth;

            /***************************************************
             * constructor (1).
             * default-constructible,
             * initializes depth to 0.
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             ***************************************************/
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
             * @brief recursively update the depth-values when
             *        hooking another node.
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
         * @brief CRTP-mixin that extends the
         *        passed node-base-type by a member-variable
         *        keeping track of the tree's-height, rooted
         *        at the current-node.
         *        (the distance to the furthest child-node).
         ********************************************************/
        template <typename NodeBaseT>
            requires _bidirectional_tree_node<NodeBaseT>
        class _height_node
            : public NodeBaseT
        {
        public:

            using _m_base_t      = NodeBaseT;
            using _m_node_t      = typename _m_base_t::_m_node_t;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;
            using typename _m_base_t::_m_hook_t;
            using _m_height_t    = std::size_t;

        protected:

            _m_height_t _m_node_height;

            /***************************************************
             * constructor (1).
             * default-constructible,
             * initializes height to 0.
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             ***************************************************/
            template <typename... ArgsTs>
            constexpr
            _height_node(ArgsTs&&... _args)
                _treelib_noexcept_if(_m_base_t())
                : _m_base_t(std::forward<ArgsTs>(_args)...)
                , _m_node_height(0)
            { }

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
             * @brief recursively update the height-values when
             *        hooking another node.
             ***************************************************/
            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
                noexcept(noexcept(this->_m_base_t::_m_hook_at(_at, _node))
                         && noexcept(this->_m_parent()))
            {
                this->_m_base_t::_m_hook_at(_at, _node);
                this->_m_update_height(_node->_m_height() + 1);
            }

            /***************************************************
             * @brief recursively update the height-values when
             *        hooking another node.
             ***************************************************/
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
             * @brief information.
             ***************************************************/

            static constexpr bool 
            _s_is_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_is_leaf())
                requires _treelib_has_member(_m_cref_t, _m_is_leaf)
            // forward call to member-function if present
            { return _node->_m_is_leaf(); }

            static constexpr bool
            _s_is_leaf(_m_cptr_t _node)
                _treelib_noexcept_if(std::ranges::empty(_node->_m_children()))
            // SFINAE fallback
            { return std::ranges::empty(_node->_m_children()); }

            static constexpr bool
            _s_is_root(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_parent())
                requires _bidirectional_tree_node<_m_node_t>
            { return _node->_m_parent() == nullptr; }

            static constexpr std::size_t
            _s_child_count(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_child_count())
                requires _treelib_has_member(_m_cref_t, _m_child_count)
            // forward call to member-function if present
            { return _node->_m_child_count(); }

            static constexpr std::size_t
            _s_child_count(_m_cptr_t _node)
                _treelib_noexcept_if_member_iterable_no_deref(_m_cref_t, _m_children)
            // SFINAE fallback
            { 
                auto _children = _node->_m_children();
                return std::ranges::distance(std::ranges::cbegin(_children),
                                             std::ranges::cend(_children)); 
            }

            static constexpr _m_height_t
            _s_height(_m_cptr_t _node)
        #ifdef _treelib_store_height
                _treelib_noexcept_if(_node->_m_height())
            { return _node->_m_height() }
        #else
                _treelib_noexcept_if(_s_children(_node))
                requires _bidirectional_tree_node<_m_node_t>
            {
                _m_height_t _res {0};
                for (_m_cptr_t _child
                     : _s_children(_node))
                    _res = std::max(_s_height(_child) + 1, _res);
                return _res;
            }
        #endif

            static constexpr _m_depth_t
            _s_depth(_m_cptr_t _node)
        #ifdef _treelib_store_depth
                _treelib_noexcept_if(_node->_m_depth())
            { return _node->_m_depth() }
        #else
                _treelib_noexcept_if(_node->_m_parent())
                requires _bidirectional_tree_node<_m_node_t>
            {
                _m_depth_t _res {0};
                while ((_node = _node->_m_parent()))
                    ++_res;
                return _res;
            }
        #endif

            /***************************************************
             * @brief hook-functionality.
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

            /***************************************************
             * @brief child accessors.
             ***************************************************/

            static constexpr decltype(auto)
            _s_children(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_children())
            { return _node->_m_children(); }

            // forward call to member-function if present
            static constexpr decltype(auto)
            _s_subordinates(_m_cptr_t _node)
                _treelib_noexcept_if(_node->_m_subordinates())
                requires _treelib_has_member(_m_node_t, _m_subordinates)
            { return _node->_m_subordinates(); }

            // SFINAE fallback, default is same as children
            static constexpr decltype(auto)
            _s_subordinates(_m_cptr_t _node)
                _treelib_noexcept_if(_s_children(_node))
            { return _s_children(_node); }

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
                if (_s_is_leaf(_node))
                    return nullptr;
                auto _children = _s_children(_node);
                return *std::ranges::begin(_children);
            }

            static constexpr _m_ptr_t
            _s_last_child(_m_cptr_t _node)
                noexcept(noexcept(_s_is_leaf(_node))
                         && _treelib_noexcept_last_readable(_s_children(_node)))
            {
                if (_s_is_leaf(_node))
                    return nullptr;
                auto _children = _s_children(_node);
                return *(std::prev(std::ranges::end(_children)));
            }

            static constexpr _m_cptr_t
            _s_seek_leftmost(_m_cptr_t _node)
                noexcept(noexcept(_s_is_leaf(_node))
                         && _treelib_noexcept_first_readable(_s_children(_node)))
            {
                while (!_s_is_leaf(_node))
                {
                    auto _children = _s_children(_node);
                    _node = *std::ranges::begin(_children);
                }
                return _node;
            }

            static constexpr _m_cptr_t
            _s_seek_rightmost(_m_cptr_t _node)
                noexcept(noexcept(_s_is_leaf(_node))
                         && _treelib_noexcept_last_readable(_s_children(_node)))
            {
                while (!_s_is_leaf(_node))
                {
                    auto _children = _s_children(_node);
                    _node = *std::prev(std::ranges::end(_children));
                }
                return _node;
            }

            static constexpr _m_ptr_t
            _s_seek_leftmost(_m_ptr_t _node)
            {
                while (!_s_is_leaf(_node))
                {
                    auto _children = _s_children(_node);
                    _node = *std::ranges::begin(_children);
                }
                return _node;
            }

            static constexpr _m_ptr_t
            _s_seek_rightmost(_m_ptr_t _node)
            {
                while (!_s_is_leaf(_node))
                {
                    auto _children = _s_children(_node);
                    _node = *std::prev(std::ranges::end(_children));
                }
                return _node;
            }

            /***************************************************
             * @brief child-information.
             ***************************************************/

            static constexpr bool
            _s_is_first_child_of(_m_cptr_t _parent, _m_cptr_t _node)
                _treelib_noexcept_if(_s_first_child(_parent))
            { return _s_first_child(_parent) == _node; }

            // '_node' cannot be a root-node here
            static constexpr bool
            _s_is_first_child(_m_cptr_t _node)
                _treelib_noexcept_if(_s_is_first_child_of(_s_parent(_node), _node))
                requires _bidirectional_tree_node<_m_node_t>
            { return _s_is_first_child_of(_s_parent(_node), _node); }

            static constexpr bool
            _s_is_last_child_of(_m_cptr_t _parent, _m_cptr_t _node)
                _treelib_noexcept_if(_s_last_child(_parent))
            { return _s_last_child(_parent) == _node; }

            // '_node' cannot be a root-node here 
            static constexpr bool
            _s_is_last_child(_m_cptr_t _node)
                _treelib_noexcept_if(_s_is_last_child_of(_s_parent(_node), _node))
                requires _bidirectional_tree_node<_m_node_t>
            { return _s_is_last_child_of(_s_parent(_node), _node); }

            /***************************************************
             * @brief sibling accessors.
             ***************************************************/

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
                if (_s_is_root(_node) || _s_is_last_child(_node))
                    return nullptr;
                auto _children = _s_children(_s_parent(_node));
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
                if (_s_is_root(_node) || _s_is_first_child(_node))
                    return nullptr;
                auto _children = _s_children(_s_parent(_node));
                return *(std::find(std::ranges::cbegin(_children),
                                   std::ranges::cend(_children),
                                   _node) - 1);
            }

            /***************************************************
             * @brief structural-copy from other nodes
             *        or initializer-nodes.
             ***************************************************/

            template <typename FnT>
                requires _copyable_tree_node<_m_node_t>
                         && std::invocable<FnT, _m_hook_t, _m_ptr_t, _m_cptr_t>
            static constexpr void
            _s_mimic(_m_ptr_t _node, _m_cptr_t _src, FnT&& _insert_fn)
            { _node->_m_mimic(_src, std::forward<FnT>(_insert_fn)); }

            template <typename InitT, typename FnT>
                requires _initializer_compatible<_m_node_t, InitT>
                         && std::invocable<FnT, _m_hook_t, _m_ptr_t, const InitT&>
            static constexpr void
            _s_mimic_initializer(_m_ptr_t _node, const InitT& _init, FnT&& _insert_fn)
            { _node->_m_mimic_initializer(_init, std::forward<FnT>(_insert_fn)); }

        };
    }
}

#endif