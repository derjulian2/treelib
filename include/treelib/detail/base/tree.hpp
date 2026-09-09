
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
#include <treelib/detail/base/iterator.hpp>
#include <treelib/detail/bits/except.hpp>

#include <cassert>

namespace tl
{
    namespace detail
    {
        /***************************************************
         * @brief base-class that handles
         *        memory-management of tree-nodes
         *        and associated values.
         ***************************************************/
        template <typename NodeT,
                  typename AllocT>
        class _alloc_base
        {
        protected:

            using _m_alloc_t        = AllocT;
            using _m_alloc_traits_t = std::allocator_traits<_m_alloc_t>;
            using _m_value_t        = typename _m_alloc_traits_t::value_type;
            using _m_size_t         = typename _m_alloc_traits_t::size_type;

            using _m_node_t        = NodeT;
            using _m_node_traits_t = _node_traits<_m_node_t>;
            using _m_node_ptr_t    = typename _m_node_traits_t::_m_ptr_t;
            using _m_cnode_ptr_t   = typename _m_node_traits_t::_m_cptr_t;
            
            using _m_vnode_t       = _value_node<_m_node_t, _m_value_t>;
            using _m_vnode_ptr_t   = _m_vnode_t*;
            using _m_cvnode_ptr_t  = const _m_vnode_t*;

            using _m_node_alloc_t        = _m_alloc_traits_t::template rebind_alloc<_m_vnode_t>;
            using _m_node_alloc_traits_t = std::allocator_traits<_m_node_alloc_t>;

            [[no_unique_address]]
            _m_node_alloc_t _m_node_alloc;

            /***************************************************
             * @brief allocates and constructs a fresh node-
             *        instance containing an instance of
             *        value_type, constructed from args.
             ***************************************************/
            template <typename... Args>
            [[nodiscard]]
            constexpr _m_vnode_ptr_t 
            _m_new_node(Args&&... _args)
            { 
                _m_vnode_ptr_t _res 
                    = _m_node_alloc_traits_t::allocate(this->_m_node_alloc, 1);
                _m_node_alloc_traits_t::construct(this->_m_node_alloc, _res, std::forward<Args>(_args)...);
                return _res;
            }

            /***************************************************
             * @brief destructs and deallocates a node-instance.
             ***************************************************/
            constexpr void 
            _m_put_node(_m_vnode_ptr_t _node) 
                noexcept
            { 
                _m_node_alloc_traits_t::destroy(this->_m_node_alloc, _node);
                _m_node_alloc_traits_t::deallocate(this->_m_node_alloc, _node, 1);
            }
            
        public:

            using allocator_type  = _m_alloc_t;

            using value_type      = typename _m_alloc_traits_t::value_type; 
            using pointer         = typename _m_alloc_traits_t::pointer;
            using const_pointer   = typename _m_alloc_traits_t::const_pointer;
            using reference       = value_type&;
            using const_reference = const value_type&;
            using size_type       = typename _m_alloc_traits_t::size_type;

            /***************************************************
             * @brief constructor (1).
             *        default-constructible if allocator_type
             *        is default-constructible.
             ***************************************************/
            constexpr 
            _alloc_base()
                noexcept(std::is_nothrow_default_constructible_v<_m_alloc_t>)
                requires std::default_initializable<_m_alloc_t>   
                : _m_node_alloc()
            { }

            /***************************************************
             * @brief constructor (2).
             *        constructs from a given allocator-instance.
             ***************************************************/
            constexpr
            _alloc_base(const allocator_type& alloc)
                noexcept(std::is_nothrow_copy_constructible_v<allocator_type>)
                requires std::copyable<allocator_type>
                : _m_node_alloc(alloc)
            { }

            /***************************************************
             * @returns the associated allocator.
             ***************************************************/
            [[nodiscard]]
            constexpr allocator_type
            get_allocator() 
                const noexcept 
            { return _m_alloc_t(this->_m_node_alloc); }

            /***************************************************
             * @returns the maximum possible number of elements.
             ***************************************************/
            [[nodiscard]]
            constexpr size_type
            max_size()
                const noexcept
            { return _m_node_alloc_traits_t::max_size(this->_m_node_alloc); }
        };


        /***************************************************
         * @brief base-class for trees to save their
         *        node-count in a member-variable.
         ***************************************************/
        template <typename AllocT>
        class _size_base
        {
        protected:
            
            using _m_alloc_t        = AllocT;
            using _m_alloc_traits_t = std::allocator_traits<_m_alloc_t>;
            using _m_size_t         = typename _m_alloc_traits_t::size_type;

            _m_size_t _m_size;

            constexpr
            _size_base()
                : _m_size(0)
            { }

            /***************************************************
             * @brief swap specialization.
             ***************************************************/
            friend constexpr void
            swap(_size_base& a, _size_base& b)
            {
                std::swap(a._m_size, b._m_size);
            }

            constexpr
            void _m_reset()
                noexcept
            {
                this->_m_size = 0;
            }

            constexpr
            void _m_inc_size(_m_size_t _n = 1)
                noexcept
            {
                this->_m_size += _n;
            }

            constexpr
            void _m_dec_size(_m_size_t _n = 1)
                noexcept
            {
                assert(this->_m_size >= _n);
                this->_m_size -= _n;
            }

        public:

            using size_type = _m_size_t;

            /***************************************************
             * @brief returns the number of elements/nodes.
             ***************************************************/
            [[nodiscard]]
            constexpr size_type
            size() 
                const noexcept
            { return this->_m_size; }

            /***************************************************
             * @brief checks whether the container is empty.
             ***************************************************/
            [[nodiscard]]
            constexpr bool
            empty() 
                const noexcept
            { return this->_m_size == 0; }
        };


        /***************************************************
         * @brief base-class for trees which should
         *        originate from a single value-holding
         *        root-node.
         *
         *        this is convenient for e.g. binary trees, in
         *        which a construct with a header-node
         *        is kind of uncomfortable to handle, because
         *        the header-node would either require
         *        special-case handling, or your tree would
         *        always be the left/right subtree of that.
         ***************************************************/
        template <typename NodeT,
                  typename AllocT>
        class _root_base
            : public _alloc_base<NodeT, AllocT>
            , public _size_base<AllocT>
        {
        protected:

            using _m_size_base_t  = _size_base<AllocT>;
            using _m_alloc_base_t = _alloc_base<NodeT, AllocT>;

            using typename _m_alloc_base_t::_m_value_t;
            using typename _m_alloc_base_t::_m_node_t;
            using typename _m_alloc_base_t::_m_node_ptr_t;
            using typename _m_alloc_base_t::_m_cnode_ptr_t;
            using typename _m_alloc_base_t::_m_size_t;
            using typename _m_alloc_base_t::_m_node_traits_t;
            using typename _m_alloc_base_t::_m_alloc_t;

            _m_node_ptr_t _m_root;

            constexpr
            void _m_reset()
                noexcept
            {
                this->_m_root = nullptr;
                this->_m_size_base_t::_m_reset();
            }

            template <typename IterT>
            constexpr IterT
            _m_root_iter()
                const noexcept
            { return _m_node_traits_t::template _s_to_iter<IterT>(this->_m_root); }

            template <typename IterT>
            constexpr IterT
            _m_begin_iter()
                const noexcept
            // since root is value-holding, this is equivalent to root
            { return this->_m_root_iter<IterT>();}

        public:

            using typename _m_alloc_base_t::value_type;
            using typename _m_alloc_base_t::allocator_type;

            using _m_alloc_base_t::_m_alloc_base_t;

            constexpr
            _root_base()
                : _m_root(nullptr)
            { }

            /***************************************************
             * @brief constructor (3).
             *        directly insert a value 
             *        at the root of the tree.
             ***************************************************/
            constexpr
            _root_base(const value_type& value, 
                       const allocator_type& alloc = allocator_type())
                : _root_base(alloc)
            {
                this->insert_root(value);
            }

            /***************************************************
             * @brief constructor (4).
             *        directly move a value
             *        to the root of the tree.
             ***************************************************/
            constexpr
            _root_base(value_type&& value, 
                       const allocator_type& alloc = allocator_type())
                : _root_base(alloc)
            {
                this->emplace_root(value);
            }

            /***************************************************
             * @brief construct a node in-place, as
             *        the root of the tree.
             * @throws tl::modification_error if the tree
             *         already has a root-node (is non-empty). 
             ***************************************************/
            template <typename... Args>
                requires std::constructible_from<value_type, Args...>
            constexpr void
            emplace_root(Args&&... args)
            {
                if (!this->empty())
                    throw tl::modification_error("tree already has a root-node");
                this->_m_root = this->_m_new_node(std::forward<Args>(args)...);
                this->_m_inc_size();
            }

            /***************************************************
             * @brief insert to the root of the tree.
             * @throws tl::modification_error if the tree
             *         already has a root-node (is non-empty). 
             ***************************************************/
            constexpr void
            insert_root(const value_type& value)
            {
                return this->emplace_root(value);
            }
        };


        /***************************************************
         * @brief base-class for trees which should
         *        originate from a valueless-header-node.
         *
         *        this is convenient for e.g. rose-trees, in which
         *        a construct without a header-node could
         *        lead to an invalid tree-state or node-leaks.
         *
         *        for example, if you would insert a
         *        next-sibling at the value-holding root-node,
         *        there would be no parent-node holding
         *        ownership for that node.
         *
         *        the only real difference is in the .root() and
         *        .begin() iterator-accessors.
         ***************************************************/
        template <typename NodeT,
                  typename AllocT>
        class _header_base
            : public _alloc_base<NodeT, AllocT>
            , public _size_base<AllocT>
        {
        protected:

            using _m_alloc_base_t = _alloc_base<NodeT, AllocT>;
            using _m_size_base_t  = _size_base<AllocT>;

            using typename _m_alloc_base_t::_m_node_t;
            using typename _m_alloc_base_t::_m_node_ptr_t;
            using typename _m_alloc_base_t::_m_cnode_ptr_t;
            using typename _m_alloc_base_t::_m_vnode_ptr_t;
            using typename _m_alloc_base_t::_m_cvnode_ptr_t;
            using typename _m_alloc_base_t::_m_size_t;
            using typename _m_alloc_base_t::_m_node_traits_t;

            using _m_header_t = _m_node_t;

            _m_header_t _m_header;

            constexpr
            void _m_reset()
                noexcept
            {
                this->_m_header->_m_reset();
                this->_m_size_base_t::_m_reset();
            }

            template <typename IterT>
            constexpr IterT
            _m_root_iter()
                const noexcept
            { return _m_node_traits_t::template _s_to_iter<IterT>(this->_m_header); }

            template <typename IterT>
            constexpr IterT
            _m_begin_iter()
                const noexcept
            // advance once to move to first-child, if any
            { return std::next(this->_m_root_iter<IterT>());}
        
        public:

            using _m_alloc_base_t::_m_alloc_base_t;

            constexpr
            _header_base()
                : _m_header()
            { }
        };


        /*************************************************************
         * @brief template-mixin for trees where the node-type
         *        does not hold a back-reference to
         *        it's own parent-node. 
         *        similiar to the difference of std::list
         *        and std::forward_list (next/prev-pointers
         *        vs only next-pointer).
         *        
         *        this mixin is written with the intention
         *        that BaseT is either _header_base or
         *        _root_base, depending on the desired
         *        properties of the final tree.
         *************************************************************/
        template <typename BaseT>
        class _outward_tree_mixin
            : public BaseT
        {
        protected:

            using _m_base_t = BaseT;

            using typename _m_base_t::_m_value_t;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;
            using typename _m_base_t::_m_vnode_ptr_t;
            using typename _m_base_t::_m_cvnode_ptr_t;
            using typename _m_base_t::_m_size_t;
            using typename _m_base_t::_m_node_traits_t;

            using _m_hook_t = typename _m_node_traits_t::_m_hook_t;

            constexpr
            void _m_do_erase(_m_node_ptr_t _node)
                noexcept
            {
                assert(_node != nullptr);
                for (_m_node_ptr_t _child 
                     : _m_node_traits_t::_s_children(_node))
                    this->_m_do_erase(_child);
                this->_m_put_node(static_cast<_m_vnode_ptr_t>(_node));
                this->_m_dec_size();
            }

            constexpr void 
            _m_insert_from_copy(_m_hook_t _at,
                                _m_node_ptr_t _where, 
                                _m_cnode_ptr_t _src)
            {
                _m_node_ptr_t _new_node = this->_m_new_node(static_cast<_m_cvnode_ptr_t>(_src)->_m_get_value());
                _m_node_traits_t::_s_hook_at(_where, _at, _new_node);
                this->_m_inc_size();
            }

            constexpr void // probably refactor.
            _m_copy_nodes(_m_node_ptr_t* dest, _m_cnode_ptr_t src)
            {
                *dest = this->_m_new_node(static_cast<_m_cvnode_ptr_t>(src)->value());
                _m_node_traits_t::_s_mimic((*dest), src, [&](auto... args) { this->_m_insert_from_copy(std::forward(args)...); });
            }

        public:

            using typename _m_base_t::allocator_type;
            using size_type = _m_size_t;
            using typename _m_base_t::value_type;

            using default_traversal_type = _depth_first_pre_order<_m_node_t>;

            using queued_iterator       
                = _queued_iterator<false, _m_value_t, default_traversal_type>;
            using const_queued_iterator 
                = _queued_iterator<true, _m_value_t, default_traversal_type>;
            // using traversing_iterator = _traversing_iterator<_m_value_t, default_traversal_type>;
            // using leaf_iterator       = _leaf_iterator<_m_value_t, _m_node_t>;
            // using child_iterator      = _child_iterator<_m_value_t, _m_node_t>;
            // using node_info           = _node_info<_m_node_t>;

            using iterator       = queued_iterator;
            using const_iterator = const_queued_iterator;
            using hook_type      = _m_node_t::_m_hook_t;

            using _m_base_t::_m_base_t;

            /***************************************************
             * @brief copy-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _outward_tree_mixin(const _outward_tree_mixin& other)
                : _outward_tree_mixin(other.get_allocator())
            { 
                if (other.empty())
                    return;
                this->_m_copy_nodes(&this->_m_root, other._m_root);
            }

            constexpr _outward_tree_mixin&
            operator=(const _outward_tree_mixin& other)
            {
                if (!this->empty())
                { this->clear(); }
                if (other.empty())
                    return *this;
                this->_m_copy_nodes(&this->_m_root, other._m_root);
                return *this;
            }

            /***************************************************
             * @brief move-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _outward_tree_mixin(_outward_tree_mixin&& other)
            { 
                std::swap(*this, other);
            }

            constexpr _outward_tree_mixin&
            operator=(_outward_tree_mixin&& other)
            {

            }

            /***************************************************
             * @brief destructor.
             ***************************************************/
            constexpr
            ~_outward_tree_mixin()
                noexcept
            { this->clear(); }

            /***************************************************
             * @returns an iterator to the valueless
             *          root-node of the tree.
             ***************************************************/
            constexpr iterator 
            root()
                noexcept
            { return this->template _m_root_iter<iterator>(); }

            /***************************************************
             * @returns an iterator to the valueless
             *          root-node of the tree.
             ***************************************************/
            constexpr const_iterator 
            croot()
                const noexcept
            { return this->template _m_root_iter<const_iterator>(); }

            /***************************************************
             * @returns an iterator to the beginning.
             ***************************************************/
            constexpr iterator 
            begin() 
                noexcept
            { return this->template _m_begin_iter<iterator>(); }

            /***************************************************
             * @returns an iterator to the beginning.
             ***************************************************/
            constexpr const_iterator
            cbegin()
                const noexcept
            { return this->template _m_begin_iter<const_iterator>(); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr iterator 
            end()
                noexcept
            { return _m_node_traits_t::template _s_to_iter<iterator>(nullptr); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr const_iterator
            cend()
                const noexcept
            { return _m_node_traits_t::template _s_to_iter<const_iterator>(nullptr); }

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
                _m_node_ptr_t _new_node = this->_m_new_node(std::forward<Args>(args)...);
                _m_node_traits_t::_s_hook_at(
                    _m_node_traits_t::_s_from_iter(where), as, _new_node
                );
                this->_m_inc_size();
                return _m_node_traits_t::template _s_to_iter<iterator>(_new_node);
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
            splice_to(hook_type to, const_iterator pos, _outward_tree_mixin&& other)
                noexcept
            {

            }

            /***************************************************
             * @brief move a tree (or parts of it) 
             *        to another location in this tree.
             ***************************************************/
            constexpr void
            splice_to(hook_type to, const_iterator pos, _outward_tree_mixin&& other, const_iterator src)
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
                this->_m_do_erase(this->_m_root);
                this->_m_reset();
            }

            /***************************************************
             * @brief lexicographically compares the 
             *        values of two trees.
             * @note  this ignores the tree's actual hierarchical
             *        structure. need to implement a true tree-compare.
             ***************************************************/
        
            friend constexpr bool
            operator==(const _outward_tree_mixin& a,
                       const _outward_tree_mixin& b)
            {
                return std::lexicographical_compare(a.cbegin(), a.cend(),
                                                    b.cbegin(), b.cend());
            }
        };


        template <typename BaseT>
        struct _tree_mixin
            : public BaseT
        {

        };


        template <typename NodeT,
                  typename AllocT>
        using _root_outward_tree
            = _outward_tree_mixin<_root_base<NodeT, AllocT>>;

        template <typename NodeT,
                  typename AllocT>
        using _header_outward_tree
            = _outward_tree_mixin<_header_base<NodeT, AllocT>>;

        template <typename NodeT,
                  typename AllocT>
        using _root_tree
            = _tree_mixin<_root_base<NodeT, AllocT>>;

        template <typename NodeT,
                  typename AllocT>
        using _header_tree
            = _tree_mixin<_header_base<NodeT, AllocT>>;
    }


}

#endif