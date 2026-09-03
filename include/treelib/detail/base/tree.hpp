
#ifndef TREELIB_BASE_TREE_HPP
#define TREELIB_BASE_TREE_HPP

/***************************************************
 * @file   treelib/detail/base/tree.hpp
 * @author Julian Benzel
 * @date   03.09.2026
 *
 * @brief  base-class for common tree-operations
 *         operating on the respective node-type.
 ***************************************************/

#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/alloc.hpp>

namespace tl
{
    namespace detail
    {
        /***************************************************
         * @brief base-class for trees where the node-type
         *        does not hold a back-reference 
         *        to it's parent-node. 
         *
         *        similiar to the difference of std::list
         *        and std::forward_list (next/prev vs next).
         ***************************************************/
        template <typename NodeType,
                typename Allocator>
        class outward_tree_base
            : public tree_allocator_base<NodeType, Allocator>
        {
        protected:
            using _M_alloc_base_t = tree_allocator_base<NodeType, Allocator>;

            using typename _M_alloc_base_t::_M_node_ptr_t;
            using typename _M_alloc_base_t::_M_vnode_ptr_t;
            using typename _M_alloc_base_t::_M_size_t;

            _M_vnode_ptr_t _M_root;
            _M_size_t      _M_size;


            constexpr
            void _M_reset()
            {
                this->_M_root = nullptr;
                this->_M_size = 0;
            }

            constexpr
            void _M_inc_size(_M_size_t n = 1)
            {
                this->_M_size += n;
            }

            constexpr
            void _M_dec_size(_M_size_t n = 1)
            {
                assert(n > 0);
                this->_M_size -= n;
            }

            constexpr
            void _M_do_erase(_M_node_ptr_t node)
            {
                assert(node != nullptr);
                for (;;)
                    _M_do_erase(child);
                this->_M_put_node(node);
                this->_M_dec_size();
            }

        public:

            using typename _M_alloc_base_t::allocator_type;
            using typename _M_alloc_base_t::size_type;
            using typename _M_alloc_base_t::value_type;

            using iterator = int*;
            using const_iterator = int*;
            using hook_type = int;

            outward_tree_base()
                noexcept(std::is_nothrow_default_constructible_v<_M_alloc_base_t>)
                requires std::default_initializable<_M_alloc_base_t> 
                : _M_alloc_base_t()
                , _M_root(nullptr)
                , _M_size(0)
            { }

            outward_tree_base(const allocator_type& alloc)
                noexcept(std::is_nothrow_copy_constructible_v<allocator_type>)
                requires std::copyable<allocator_type>
                : _M_alloc_base_t(std::forward<allocator_type>(alloc))
                , _M_root(nullptr)
                , _M_size(0)
            { }

            outward_tree_base(const outward_tree_base& other)
            { }


            outward_tree_base(outward_tree_base&& other)
            { 
                std::swap(*this, other);
            }


            ~outward_tree_base()
            { this->clear(); }


            outward_tree_base& operator=(const outward_tree_base& other)
            {
                if (!this->empty())
                { this->clear(); }

            }

            
            outward_tree_base& operator=(outward_tree_base&& other)
            {

            }


            static constexpr void
            swap(outward_tree_base& a, outward_tree_base& b)
            {
                std::swap(a._M_root, b._M_root);
                std::swap(a._M_size, b._M_size);
            }


            [[nodiscard]]
            constexpr size_type
            size() 
                const noexcept
            { return this->_M_size; }


            [[nodiscard]]
            constexpr bool
            empty() 
                const noexcept
            { return this->_M_root == nullptr && this->_M_size == 0; }




            constexpr iterator 
            root()
                noexcept
            { return iterator(this->_M_root); }

            constexpr const_iterator 
            croot()
                const noexcept
            { return const_iterator(this->_M_root); }

            constexpr iterator 
            begin() 
                noexcept
            { return iterator(this->_M_root); }

            constexpr const_iterator
            cbegin()
                const noexcept
            { return const_iterator(this->_M_root); }

            constexpr iterator 
            end()
                noexcept
            { return iterator(nullptr); }

            constexpr const_iterator
            cend()
                const noexcept
            { return const_iterator(nullptr); }

            /***************************************************
             * @brief insert a node as the root of the tree.
             ***************************************************/
            template <typename... Args>
            constexpr iterator
            emplace(Args&&... args)
            {
                this->_M_root = this->_M_new_node(std::forward<Args>(args)...);
                this->_M_inc_size();
                return iterator(this->_M_root);
            }

            constexpr iterator
            insert(const value_type& value)
            {
                return this->emplace(value);
            }

            /***************************************************
             * @brief insert a node as a relative 
             *        of an existing node.
             ***************************************************/
            template <typename... Args>
            constexpr iterator
            emplace(hook_type as, const_iterator where, Args&&... args)
            {
                _M_node_ptr_t new_node = this->_M_new_node(std::forward<Args>(args)...);
                new_node->hook_as(as, where);
                this->_M_inc_size();
                return iterator(new_node);
            }

            constexpr iterator
            insert(hook_type as, const_iterator where, const value_type& value)
            {
                return this->emplace(as, where, value);
            }

            /***************************************************
             * @brief move a tree (or parts of it) 
             *        to another location in this tree.
             ***************************************************/
            constexpr void
            splice_to(hook_type to, const_iterator pos, outward_tree_base&& other)
                noexcept
            {

            }

            /***************************************************
             * @brief move a tree (or parts of it) 
             *        to another location in this tree.
             ***************************************************/
            constexpr void
            splice_to(hook_type to, const_iterator pos, outward_tree_base&& other, const_iterator src)
                noexcept
            {

            }

            /***************************************************
             * @brief insert a tree (or parts of it)
             *        to a specified location in this tree.
             ***************************************************/
            constexpr void
            insert_range(hook_type as, const_iterator dest, const_iterator src)
            {

            }

            /***************************************************
             * @brief 
             ***************************************************/
            void
            erase_at(hook_type at, iterator where)
            //    noexcept
            // maybe throws if nothing there at hook 'at'
            {

            }

            /***************************************************
             * @brief clears the contents.
             ***************************************************/
            constexpr void 
            clear() 
                noexcept
            {
                if (this->empty())
                    return;
                this->_M_do_erase(this->_M_root);
                this->_M_reset();
            }

        };


        /***************************************************
         * @brief base-class for trees where the node-type
         *        is extended by a back-reference to it's
         *        parent-node.
         *
         *        similiar to the difference of std::list
         *        and std::forward_list (next/prev vs next).
         *
         *        outward-trees are a strict subset of
         *        regular, bidirectional trees (just how
         *        technically std::forward_list is a
         *        subset of std::list), so this base can
         *        reuse the capabilities of outward-trees.
         ***************************************************/
        template <typename NodeType,
                typename Allocator>
        struct tree_base
            : public outward_tree_base<bidirectional_node<NodeType>, Allocator>
        {

        };
    }


}

#endif