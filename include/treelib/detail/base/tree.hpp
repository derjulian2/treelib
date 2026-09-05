
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
#include <treelib/detail/base/iterator.hpp>
#include <treelib/detail/bits/except.hpp>

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
        template <typename _NodeT,
                  typename _AllocT>
        class _Outward_Tree_Base
            : public _Tree_Alloc_Base<_NodeT, _AllocT>
        {
        protected:

            using _M_base_t      = _Tree_Alloc_Base<_NodeT, _AllocT>;

            using typename _M_base_t::_M_node_t;
            using typename _M_base_t::_M_node_ptr_t;
            using typename _M_base_t::_M_cnode_ptr_t;
            using typename _M_base_t::_M_vnode_ptr_t;
            using typename _M_base_t::_M_cvnode_ptr_t;
            using typename _M_base_t::_M_size_t;
            using typename _M_base_t::_M_node_traits_t;

            using _M_hook_t = typename _M_node_traits_t::_M_hook_t;

            _M_vnode_ptr_t _M_root;
            _M_size_t      _M_size;


            constexpr
            void _M_reset()
                noexcept
            {
                this->_M_root = nullptr;
                this->_M_size = 0;
            }

            constexpr
            void _M_inc_size(_M_size_t _n = 1)
                noexcept
            {
                this->_M_size += _n;
            }

            constexpr
            void _M_dec_size(_M_size_t _n = 1)
                noexcept
            {
                assert(this->_M_size >= _n);
                this->_M_size -= _n;
            }

            constexpr
            void _M_do_erase(_M_node_ptr_t _node)
                noexcept
            {
                assert(_node != nullptr);
                for (_M_node_ptr_t _child 
                     : _M_node_traits_t::_S_children(_node))
                    this->_M_do_erase(_child);
                this->_M_put_node(static_cast<_M_vnode_ptr_t>(_node));
                this->_M_dec_size();
            }

            constexpr void 
            _M_insert_from_copy(_M_hook_t _at,
                                _M_node_ptr_t _where, 
                                _M_cnode_ptr_t _src)
            {
                _M_node_ptr_t _new_node = this->_M_new_node(static_cast<_M_cvnode_ptr_t>(_src)->_M_get_value());
                _M_node_traits_t::_S_hook_at(_where, _at, _new_node);
                this->_M_inc_size();
            }

            constexpr void // probably refactor.
            _M_copy_nodes(_M_node_ptr_t* dest, _M_cnode_ptr_t src)
            {
                *dest = this->_M_new_node(static_cast<_M_cvnode_ptr_t>(src)->value());
                _M_node_traits_t::_S_mimic((*dest), src, [&](auto... args) { this->_M_insert_from_copy(std::forward(args)...); });
            }

        public:

            using typename _M_base_t::allocator_type;
            using typename _M_base_t::size_type;
            using typename _M_base_t::value_type;

            using iterator       = tl::queued_iterator<value_type, _M_node_t>;
            using const_iterator = tl::queued_iterator<const value_type, _M_node_t>;
            using hook_type      = _M_node_t::_M_hook_t;

            /***************************************************
             * @brief constructor (1).
             *        default-initializable if allocator
             *        is default-initializable.
             ***************************************************/
            constexpr
            _Outward_Tree_Base()
                noexcept(std::is_nothrow_default_constructible_v<_M_base_t>)
                requires std::default_initializable<_M_base_t> 
                : _M_base_t()
                , _M_root(nullptr)
                , _M_size(0)
            { }

            /***************************************************
             * @brief constructor (2).
             *        use the given allocator instance.
             ***************************************************/
            constexpr
            _Outward_Tree_Base(const allocator_type& alloc)
                noexcept(std::is_nothrow_copy_constructible_v<allocator_type>)
                requires std::copyable<allocator_type>
                : _M_base_t(alloc)
                , _M_root(nullptr)
                , _M_size(0)
            { }

            /***************************************************
             * @brief constructor (3).
             *        directly insert the value 
             *        at the root of the tree.
             ***************************************************/
            constexpr
            _Outward_Tree_Base(const value_type& value, 
                               const allocator_type& alloc = allocator_type())
                : _Outward_Tree_Base(alloc)
            {
                this->insert(value);
            }

            /***************************************************
             * @brief constructor (4).
             *        directly move the value
             *        to the root of the tree.
             ***************************************************/
            constexpr
            _Outward_Tree_Base(value_type&& value, 
                               const allocator_type& alloc = allocator_type())
                : _Outward_Tree_Base(alloc)
            {
                this->emplace(value);
            }

            /***************************************************
             * @brief copy-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _Outward_Tree_Base(const _Outward_Tree_Base& other)
                : _Outward_Tree_Base(other.get_allocator())
            { 
                if (other.empty())
                    return;
                this->_M_copy_nodes(&this->_M_root, other._M_root);
            }

            constexpr _Outward_Tree_Base&
            operator=(const _Outward_Tree_Base& other)
            {
                if (!this->empty())
                { this->clear(); }
                if (other.empty())
                    return *this;
                this->_M_copy_nodes(&this->_M_root, other._M_root);
                return *this;
            }

            /***************************************************
             * @brief move-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _Outward_Tree_Base(_Outward_Tree_Base&& other)
            { 
                std::swap(*this, other);
            }

            constexpr _Outward_Tree_Base&
            operator=(_Outward_Tree_Base&& other)
            {

            }

            /***************************************************
             * @brief destructor.
             ***************************************************/
            constexpr
            ~_Outward_Tree_Base()
                noexcept
            { this->clear(); }

            /***************************************************
             * @brief std::swap specialization.
             ***************************************************/
            friend constexpr void
            swap(_Outward_Tree_Base& a, _Outward_Tree_Base& b)
            {
                std::swap(a._M_root, b._M_root);
                std::swap(a._M_size, b._M_size);
            }

            /***************************************************
             * @brief returns the number of elements/nodes.
             ***************************************************/
            [[nodiscard]]
            constexpr size_type
            size() 
                const noexcept
            { return this->_M_size; }

            /***************************************************
             * @brief checks whether the container is empty 
             ***************************************************/
            [[nodiscard]]
            constexpr bool
            empty() 
                const noexcept
            { return this->_M_root == nullptr && this->_M_size == 0; }

            /***************************************************
             * @returns an iterator to the root of the tree.
             *          equivalent to .begin().
             ***************************************************/
            constexpr iterator 
            root()
                noexcept
            { return iterator(this->_M_root); }

            /***************************************************
             * @returns an iterator to the root of the tree.
             *          equivalent to .cbegin().
             ***************************************************/
            constexpr const_iterator 
            croot()
                const noexcept
            { return const_iterator(this->_M_root); }

            /***************************************************
             * @returns an iterator to the beginning.
             *          equivalent to .root().
             ***************************************************/
            constexpr iterator 
            begin() 
                noexcept
            { return iterator(this->_M_root); }

            /***************************************************
             * @returns an iterator to the beginning.
             *          equivalent to .croot().
             ***************************************************/
            constexpr const_iterator
            cbegin()
                const noexcept
            { return const_iterator(this->_M_root); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr iterator 
            end()
                noexcept
            { return iterator(nullptr); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr const_iterator
            cend()
                const noexcept
            { return const_iterator(nullptr); }

            /***************************************************
             * @brief construct a node in-place, as
             *        the root of the tree.
             * @throws tl::modification_error if the tree
             *         already has a root-node (is non-empty). 
             ***************************************************/
            template <typename... Args>
            constexpr iterator
            emplace(Args&&... args)
            {
                if (!this->empty())
                    throw tl::modification_error("tree already has a root-node");
                this->_M_root = this->_M_new_node(std::forward<Args>(args)...);
                this->_M_inc_size();
                return iterator(this->_M_root);
            }

            /***************************************************
             * @brief insert to the root of the tree.
             * @throws tl::modification_error if the tree
             *         already has a root-node (is non-empty). 
             ***************************************************/
            constexpr iterator
            insert(const value_type& value)
            {
                return this->emplace(value);
            }

            /***************************************************
             * @brief construct a node in-place, as a relative 
             *        of an existing node.
             * @param as : the 'position' of the new node.
             *             @see tl::node for more information.
             ***************************************************/
            template <typename... Args>
            constexpr iterator
            emplace(hook_type as, const_iterator where, Args&&... args)
            {
                _M_node_ptr_t _new_node = this->_M_new_node(std::forward<Args>(args)...);
                static_cast<_M_node_ptr_t>(where)->hook_at(as, _new_node);
                this->_M_inc_size();
                return iterator(_new_node);
            }

            /***************************************************
             * @brief insert an element as a relative 
             *        of an existing node.
             * @param as : the 'position' of the new node.
             *             @see tl::node for more information. 
             ***************************************************/
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
            splice_to(hook_type to, const_iterator pos, _Outward_Tree_Base&& other)
                noexcept
            {

            }

            /***************************************************
             * @brief move a tree (or parts of it) 
             *        to another location in this tree.
             ***************************************************/
            constexpr void
            splice_to(hook_type to, const_iterator pos, _Outward_Tree_Base&& other, const_iterator src)
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
             * @brief erases the subtree at the specified hook.
             ***************************************************/
            void
            erase_at(hook_type at, iterator where)
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

            /***************************************************
             * @brief lexicographically compares the 
             *        values of two trees.
             * @note  this ignores the tree's actual hierarchical
             *        structure. need to implement a true tree-compare.
             ***************************************************/
        
            friend constexpr bool
            operator==(const _Outward_Tree_Base& a,
                       const _Outward_Tree_Base& b)
            {
                return std::lexicographical_compare(a.cbegin(), a.cend(),
                                                    b.cbegin(), b.cend());
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
        template <typename _NodeT,
                  typename _AllocT>
        struct _Tree_Base
            : public _Outward_Tree_Base<_NodeT, _AllocT>
        {
            using _M_base_t = _Outward_Tree_Base<_NodeT, _AllocT>;

            using _M_base_t::_M_base_t;
        };
    }


}

#endif