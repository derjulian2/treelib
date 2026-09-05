
#ifndef TREELIB_BASE_ALLOC_HPP
#define TREELIB_BASE_ALLOC_HPP

/***************************************************
 * @file   treelib/detail/base/alloc.hpp
 * @author Julian Benzel
 * @date   03.09.2026
 *
 * @brief  base-class for memory-management
 *         of a tree-data-structure.
 ***************************************************/

#include <memory>
#include <cassert>
#include <treelib/detail/base/node.hpp>

namespace tl
{
    namespace detail
    {
        /***************************************************
         * @brief base-class that handles low-level-
         *        memory-management of tree-nodes.
         ***************************************************/
        template <typename _NodeT,
                  typename _AllocT>
        class _Tree_Alloc_Base
        {
        protected:

            using _M_alloc_t        = _AllocT;
            using _M_alloc_traits_t = std::allocator_traits<_M_alloc_t>;
            using _M_value_t        = typename _M_alloc_traits_t::value_type;
            using _M_size_t         = typename _M_alloc_traits_t::size_type;

            using _M_node_t        = _NodeT;
            using _M_node_traits_t = _Node_Traits<_M_node_t>;
            using _M_node_ptr_t    = typename _M_node_traits_t::_M_ptr_t;
            using _M_cnode_ptr_t   = typename _M_node_traits_t::_M_cptr_t;
            
            using _M_vnode_t       = _Value_Node<_M_node_t, _M_value_t>;
            using _M_vnode_ptr_t   = _M_vnode_t*;
            using _M_cvnode_ptr_t  = const _M_vnode_t*;

            using _M_node_alloc_t        = _M_alloc_traits_t::template rebind_alloc<_M_vnode_t>;
            using _M_node_alloc_traits_t = std::allocator_traits<_M_node_alloc_t>;

            [[no_unique_address]]
            _M_node_alloc_t _M_alloc;

        protected:

            /***************************************************
             * @brief accessors to allocator-instance.
             ***************************************************/

            constexpr _M_node_alloc_t& 
            _M_get_node_alloc() 
                noexcept
            { return this->_M_alloc; }

            constexpr const _M_node_alloc_t& 
            _M_get_node_alloc() 
                const noexcept
            { return this->_M_alloc; }


            /***************************************************
             * @brief allocates and constructs a fresh node-
             *        instance containing an instance of
             *        value_type, constructed from args.
             ***************************************************/
            template <typename... Args>
            [[nodiscard]]
            constexpr _M_vnode_ptr_t 
            _M_new_node(Args&&... _args)
            { 
                _M_vnode_ptr_t _res = _M_node_alloc_traits_t::allocate(this->_M_get_node_alloc(), 1);
                _M_node_alloc_traits_t::construct(this->_M_get_node_alloc(), _res, std::forward<Args>(_args)...);
                return _res;
            }

            /***************************************************
             * @brief destructs and deallocates a node-instance.
             ***************************************************/
            constexpr void 
            _M_put_node(_M_vnode_ptr_t _node) 
                noexcept
            { 
                _M_node_alloc_traits_t::destroy(this->_M_get_node_alloc(), _node);
                _M_node_alloc_traits_t::deallocate(this->_M_get_node_alloc(), _node, 1);
            }
            
        public:

            using allocator_type  = _M_alloc_t;

            using value_type      = typename _M_alloc_traits_t::value_type; 
            using pointer         = typename _M_alloc_traits_t::pointer;
            using const_pointer   = typename _M_alloc_traits_t::const_pointer;
            using reference       = value_type&;
            using const_reference = const value_type&;
            using size_type       = typename _M_alloc_traits_t::size_type;

            /***************************************************
             * @brief constructor (1).
             *        default-constructible if allocator_type
             *        is default-constructible.
             ***************************************************/
            constexpr 
            _Tree_Alloc_Base()
                noexcept(std::is_nothrow_default_constructible_v<_M_alloc_t>)
                requires std::default_initializable<_M_alloc_t>   
                : _M_alloc(_M_alloc_t())
            { }

            /***************************************************
             * @brief constructor (2).
             *        constructs from a given allocator-instance.
             ***************************************************/
            constexpr
            _Tree_Alloc_Base(const allocator_type& alloc)
                noexcept(std::is_nothrow_copy_constructible_v<allocator_type>)
                requires std::copyable<allocator_type>
                : _M_alloc(alloc)
            { }

            /***************************************************
             * @returns the associated allocator.
             ***************************************************/
            [[nodiscard]]
            constexpr allocator_type
            get_allocator() 
                const noexcept 
            { return _M_alloc_t(this->_M_get_node_alloc()); }

            /***************************************************
             * @returns the maximum possible number of elements.
             ***************************************************/
            [[nodiscard]]
            constexpr size_type
            max_size()
                const noexcept
            { return _M_node_alloc_traits_t::max_size(this->_M_get_node_alloc()); }
        };
    }
}

#endif