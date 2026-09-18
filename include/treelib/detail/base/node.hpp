
#ifndef TREELIB_BASE_NODE_HPP
#define TREELIB_BASE_NODE_HPP

 /***********************************************************************
  * @file   treelib/detail/base/node.hpp
  * @author Julian Benzel
  * @date   14.09.2026
  *
  * @brief  mixins/CRTP-mixins for node-types.
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
  *          NodeBaseT::_m_node_t, which is just a
  *          sort of 'indirect'-CRTP. it is always assumed
  *          that a cast to the final/derived type '_m_node_t*' 
  *          is valid, making these types CRTP-classes AND mixins.
  ***********************************************************************/

#include <treelib/detail/bits/except.hpp>

#include <type_traits>
#include <utility>

namespace tl
{
    namespace _detail
    {
        /***************************************************
         * @brief mixin that adds an instance
         *        of value-type to the passed node-type.
         *
         *        these will be the instances that will
         *        actually be allocated by a tree-container.
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
        
        protected:

            _m_value_t _m_value;

        public:

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
         * @brief CRTP-mixin that extends the
         *        passed node-base-type by an additional
         *        back-pointer to it's owning parent.
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

            _m_node_ptr_t _m_parent;

            // unused code
            // constexpr void
            // _m_reset()
            //     noexcept
            // { this->_m_parent = nullptr; } 

            /***************************************************
             * @brief since this is a CRTP-base,
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

            /***************************************************
             * constructor (1).
             * default-constructible,
             * initializes parent to nullptr.
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             ***************************************************/
            constexpr
            _bidirectional_node()
                _treelib_noexcept_if(_m_base_t())
                : _m_base_t()
                , _m_parent(nullptr)
            { }

        public:
                
            /***************************************************
             * @brief parent accessors.
             ***************************************************/

            constexpr _m_node_ptr_t
            _m_get_parent() 
                noexcept
            { return this->_m_parent; }


            constexpr _m_cnode_ptr_t
            _m_get_parent()
                const noexcept
            { return this->_m_parent; }

            /***************************************************
             * @brief forward hooking/unhooking to base-class
             *        but also set/reset the hooked-node's parent-
             *        pointer to this/nullptr.
             ***************************************************/

            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
            //    _treelib_noexcept_if_member(_m_base_t, _m_hook_at)
            {
                this->_m_base_t::_m_hook_at(_at, _node);
                _node->_m_parent = this->_m_node_ptr();
            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t _at)
            //    _treelib_noexcept_if_member(_m_base_t, _m_unhook_at)
            {
                _m_node_ptr_t _res = this->_m_base_t::_m_unhook_at(_at);
                _res->_m_parent = nullptr;
                return _res;
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t _node)
            //    _treelib_noexcept_if_member(_m_base_t, _m_unhook_if)
            {
                this->_m_base_t::unhook_if(_node);
                _node->_m_parent = nullptr;
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
            //    _treelib_noexcept_if_member(_m_base_t, _m_unhook_if)
            {
                this->_m_parent->_m_base_t::_m_unhook_if(this);
                this->_m_parent = nullptr;
            }

        };
        

        /***************************************************
         * @brief CRTP-mixin that extends the
         *        passed node-base-type by a member-variable
         *        keeping track of the node's depth in the
         *        tree (i.e. the distance from the root-node).
         ***************************************************/
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

            _m_depth_t _m_depth;

            /***************************************************
             * constructor (1).
             * default-constructible,
             * initializes depth to 0.
             *
             * this constructor is marked protected because
             * this CRTP-base should not be instantiated
             * on it's own.
             ***************************************************/
            constexpr
            _depth_node()
                _treelib_noexcept_if(_m_base_t())
                : _m_base_t()
                , _m_depth(0)
            { }

        public:

            /***************************************************
             * @brief set the depth-member to the desired
             *        value and recursively update all child-
             *        nodes.
             ***************************************************/
            constexpr void
            _m_update_depth(_m_depth_t _depth)
                noexcept
            {
                this->_m_depth = _depth;
                for (_m_node_ptr_t _child
                     : this->_m_base_t::_m_children())
                     _child->_m_update_depth(_depth + 1);
            }
        
            /***************************************************
             * @brief depth accessor.
             ***************************************************/

            constexpr _m_depth_t
            _m_get_depth()
                const noexcept
            { return this->_m_depth; }

            /***************************************************
             * @brief recursively update the depth-values when
             *        hooking another node.
             ***************************************************/

            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
                _treelib_noexcept_if_member(_m_base_t, _m_hook_at)
            {
                this->_m_base_t::_m_hook_at(_at, _node);
                _node->_m_update_depth(this->_m_depth + 1);
            }
        };
    }
}

#endif