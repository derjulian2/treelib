
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
        template <typename NodeType,
                  typename Allocator>
        class tree_allocator_base
        {
        protected:

            using _M_alloc_t        = Allocator;
            using _M_alloc_traits_t = std::allocator_traits<_M_alloc_t>;
            using _M_value_t        = typename _M_alloc_traits_t::value_type;
            using _M_size_t         = typename _M_alloc_traits_t::size_type;

            using _M_node_t     = NodeType;
            using _M_node_ptr_t = _M_node_t*;
            
            using _M_vnode_t     = value_node<_M_value_t, _M_node_t>;
            using _M_vnode_ptr_t = _M_vnode_t*;

            using _M_node_alloc_t        = _M_alloc_traits_t::template rebind_alloc<_M_vnode_t>;
            using _M_node_alloc_traits_t = std::allocator_traits<_M_node_alloc_t>;

            [[no_unique_address]]
            _M_node_alloc_t _M_alloc;

        protected:

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
            _M_new_node(Args&&... args)
            { 
                _M_vnode_ptr_t res = _M_node_alloc_traits_t::allocate(this->_M_get_node_alloc(), 1);
                _M_node_alloc_traits_t::construct(this->_M_get_node_alloc(), res, std::forward<Args>(args)...);
                return res;
            }


            /***************************************************
             * @brief destructs and deallocates a node-instance.
             ***************************************************/
            constexpr void 
            _M_put_node(_M_vnode_ptr_t node) 
                noexcept
            { 
                _M_node_alloc_traits_t::destroy(this->_M_get_node_alloc(), node);
                _M_node_alloc_traits_t::deallocate(this->_M_get_node_alloc(), node, 1);
            }
            

            /*****************************************************************************************************
             * @brief   copy a node and it's value from a node-pointer.
             *
             * @details this only exists to make the interface with the
             *          individual node-types easier, as they can call
             *          a clone-method by just pointing to it's descendants,
             *          without actually knowing about the value-type that
             *          the node will have in the tree.
             *          this only works if the object at that location
             *          is actually of type 'value_node_type', but as
             *          this is the only type actually allocated and
             *          constructed, the static-casting should be OK.
             *****************************************************************************************************/
            [[nodiscard]]
            constexpr _M_vnode_ptr_t
            _M_copy_node(_M_node_ptr_t node)
                requires std::copyable<_M_value_t>
            {
                return _M_new_node(static_cast<_M_vnode_ptr_t>(node)->value());
            }
            
        public:

            using allocator_type  = _M_alloc_t;

            using value_type      = typename _M_alloc_traits_t::value_type; 
            using pointer         = typename _M_alloc_traits_t::pointer;
            using const_pointer   = typename _M_alloc_traits_t::const_pointer;
            using reference       = typename _M_alloc_traits_t::reference;
            using const_reference = typename _M_alloc_traits_t::const_reference;
            using size_type       = typename _M_alloc_traits_t::size_type;

            /***************************************************
             * @brief constructors. 
             *        default-constructible if allocator_type
             *        is default-constructible.
             ***************************************************/

            constexpr 
            tree_allocator_base()
                noexcept(std::is_nothrow_default_constructible_v<_M_alloc_t>)
                requires std::default_initializable<_M_alloc_t>   
                : _M_alloc(_M_alloc_t())
            { }

            constexpr
            tree_allocator_base(const allocator_type& alloc)
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