
#ifndef TREELIB_BASE_TREE_HPP
#define TREELIB_BASE_TREE_HPP

/***************************************************
 * @file   treelib/detail/base/tree.hpp
 * @author Julian Benzel
 * @date   15.09.2026
 *
 * @brief  base-classes/mixins for common tree-types
 *         operating on the respective node-type.
 *
 * @todo   - maybe make _size_base an optional mixin.
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/bits/initializer_tree.hpp>

#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/traits.hpp>
#include <treelib/detail/base/iterator.hpp>

#include <cassert>

namespace tl
{
    namespace _detail
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
             * creation/destruction of nodes.
             ***************************************************/

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

            _m_size_t _m_node_count;

            constexpr
            _size_base()
                : _m_node_count(0)
            { }

            constexpr
            void _m_reset()
                noexcept
            {
                this->_m_node_count = 0;
            }

            /***************************************************
             * size modifiers.
             ***************************************************/

            constexpr
            void _m_inc_size(_m_size_t _n = 1)
                noexcept
            {
                this->_m_node_count += _n;
            }

            constexpr
            void _m_dec_size(_m_size_t _n = 1)
                noexcept
            {
                assert(this->_m_node_count >= _n);
                this->_m_node_count -= _n;
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
            { return this->_m_node_count; }

            /***************************************************
             * @brief checks whether the container is empty.
             ***************************************************/
            [[nodiscard]]
            constexpr bool
            empty() 
                const noexcept
            { return this->_m_node_count == 0; }
        };


        /***************************************************
         * @brief supplies basic copying/erasure/comparing
         *        methods that recursively operate on nodes.
         ***************************************************/
        template <typename NodeT,
                  typename AllocT>
        class _basic_tree_base
            : public _alloc_base<NodeT, AllocT>
            , public _size_base<AllocT>
        {
        protected:

            using _m_size_base_t  = _size_base<AllocT>;
            using _m_alloc_base_t = _alloc_base<NodeT, AllocT>;

            using typename _m_alloc_base_t::_m_value_t;
            using typename _m_alloc_base_t::_m_node_t;
            using typename _m_alloc_base_t::_m_node_ptr_t;
            using typename _m_alloc_base_t::_m_vnode_ptr_t;
            using typename _m_alloc_base_t::_m_cvnode_ptr_t;
            using typename _m_alloc_base_t::_m_cnode_ptr_t;
            using typename _m_alloc_base_t::_m_size_t;
            using typename _m_alloc_base_t::_m_node_traits_t;
            using typename _m_alloc_base_t::_m_alloc_t;
            
            using _m_init_node_t = _initializer_node<_m_value_t>;
            using _m_init_node_ref_t = const _m_init_node_t&;
            using _m_hook_t = typename _m_node_traits_t::_m_hook_t;

            /***************************************************
             * recursive node-operations.
             ***************************************************/

            /***************************************************
             * @brief recursively erases all child-nodes
             *        of the passed node. 
             * @note  _node itself is not erased and can 
             *        therefore be a valueless header-node.
             *        all children must be static_cast-able to
             *        _m_vnode_ptr_t.
             ***************************************************/
            constexpr
            void _m_erase_children(_m_node_ptr_t _node)
                noexcept
            {
                assert(_node != nullptr);
                for (_m_node_ptr_t _child 
                     : _m_node_traits_t::_s_children(_node))
                    this->_m_erase_children(_child);
                this->_m_put_node(static_cast<_m_vnode_ptr_t>(_node));
                this->_m_dec_size();
            }

            /***************************************************
             * @brief helper function to construct and hook
             *        a new node from the value of an
             *        initializer-node.
             ***************************************************/
            constexpr void 
            _m_insert_init(_m_hook_t _at,
                           _m_node_ptr_t _where, 
                           _m_init_node_ref_t _src)
            {
                _m_node_ptr_t _new_node = this->_m_new_node(_src._m_value);
                _m_node_traits_t::_s_hook_at(_where, _at, _new_node);
                this->_m_inc_size();
            }
            
            /***************************************************
             * @brief helper function to construct and hook
             *        a new node from the value of an existing
             *        node. used in _m_copy_children.
             ***************************************************/
            constexpr void 
            _m_insert_copy(_m_hook_t _at,
                            _m_node_ptr_t _where, 
                            _m_cnode_ptr_t _src)
            {
                _m_node_ptr_t _new_node = this->_m_new_node(static_cast<_m_cvnode_ptr_t>(_src)->_m_value());
                _m_node_traits_t::_s_hook_at(_where, _at, _new_node);
                this->_m_inc_size();
            }

            /***************************************************
             * @brief recursively copies all child-nodes
             *        of the passed node and mimics the structure
             *        of the source-node.
             * @note  _dest itself is not copied and can 
             *        therefore be a valueless header-node.
             *        all children must be static_cast-able to
             *        _m_vnode_ptr_t. the destination-node will
             *        have to satisfy _copyable_node because it
             *        needs to provide .mimic() to build the
             *        copied structure.
             ***************************************************/
            constexpr void
            _m_copy_children(_m_node_ptr_t _dest, _m_cnode_ptr_t _src)
                requires _copyable_node<_m_node_t>
            {
                // capture this-pointer to insert into this tree
                auto _insert_fn = [&](_m_hook_t _at,
                                      _m_node_ptr_t _where, 
                                      _m_cnode_ptr_t _src) 
                                  { this->_m_insert_copy(_at, _where, _src); };
                _m_node_traits_t::_s_mimic(_dest, _src, _insert_fn);
            }

            static constexpr bool 
            _s_compare(_m_cvnode_ptr_t _a, _m_cvnode_ptr_t _b)
                requires std::equality_comparable<_m_value_t>
            { return _a->_m_value() == _b->_m_value(); }

            /***************************************************
             * @brief recursively compares all values of 
             *        the child-nodes of the passed nodes.
             * @note  _a and _b themselves are not compared and can 
             *        therefore be valueless header-nodes.
             *        all children must be static_cast-able to
             *        _m_vnode_ptr_t.
             ***************************************************/
            static constexpr bool 
            _s_compare_children(_m_cnode_ptr_t _a, _m_cnode_ptr_t _b)
            {
                decltype(auto) _a_children = _m_node_traits_t::_s_children(_a);
                decltype(auto) _b_children = _m_node_traits_t::_s_children(_b);
                
                if (_m_node_traits_t::_s_child_count(_a)
                    != _m_node_traits_t::_s_child_count(_b))
                    return false;

                auto _a_beg = std::ranges::begin(_a_children);
                auto _a_end = std::ranges::end(_a_children);
                auto _b_beg = std::ranges::begin(_b_children);
                auto _b_end = std::ranges::end(_b_children);

                for (;_a_beg != _a_end && _b_beg != _b_end; ++_a_beg, ++_b_beg)
                {
                    _m_cvnode_ptr_t _a_child = static_cast<_m_cvnode_ptr_t>(*_a_beg);
                    _m_cvnode_ptr_t _b_child = static_cast<_m_cvnode_ptr_t>(*_b_beg);
                    if (!_s_compare(_a_child, _b_child)
                        || !_s_compare_children(_a_child, _b_child))
                        return false;
                }
                return true;
            }

        public:
            
            using _m_alloc_base_t::_m_alloc_base_t;
        };


        /***************************************************
         * @brief base-class for trees which should
         *        originate from a single value-holding
         *        root-node.
         *
         *        this is convenient for e.g. binary trees, in
         *        which a construct with a header-node
         *        is kind of uncomfortable to handle, because
         *        the header-node would make any tree the
         *        left/right subtree of that node.
         ***************************************************/
        template <typename NodeT,
                  typename AllocT>
        class _root_base
            : public _basic_tree_base<NodeT, AllocT>
        {
        protected:

            using _m_base_t = _basic_tree_base<NodeT, AllocT>;

            using typename _m_base_t::_m_value_t;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;
            using typename _m_base_t::_m_cvnode_ptr_t;
            using typename _m_base_t::_m_size_t;
            using typename _m_base_t::_m_node_traits_t;
            using typename _m_base_t::_m_alloc_t;

            using _m_init_tree_t = _root_initializer_tree<_m_value_t>;
            using typename _m_base_t::_m_hook_t;
            using typename _m_base_t::_m_init_node_t;
            using typename _m_base_t::_m_init_node_ref_t;

            _m_node_ptr_t _m_root;

            constexpr
            void _m_reset()
                noexcept
            {
                this->_m_root = nullptr;
                this->_m_size_base_t::_m_reset();
            }

            constexpr _m_node_ptr_t
            _m_root_node()
                const noexcept
            { return this->_m_root; }

            template <typename IterT>
            constexpr IterT
            _m_begin()
                const
            { return _iter_traits<IterT>::_s_root_begin(this->_m_root_node()); }
            
        public:

            using typename _m_base_t::value_type;
            using typename _m_base_t::allocator_type;
            using initializer_tree_type = _m_init_tree_t;

            using _m_base_t::_m_base_t;

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
             * @brief constructor (5).
             *        build from an initializer-tree.
             ***************************************************/
            constexpr
            _root_base(initializer_tree_type&& init,
                       const allocator_type& alloc = allocator_type())
                requires _initializable_node<_m_node_t, _m_init_node_t>
            {
                // capture this-pointer to insert into this tree
                auto _insert_fn = [&](_m_hook_t _at,
                                      _m_node_ptr_t _where, 
                                      _m_init_node_ref_t _src) 
                                  { this->_m_insert_init(_at, _where, _src); };
                this->emplace_root(init._m_root._m_value);
                this->_m_root_node()->_m_mimic_initializer(init._m_root, _insert_fn);
            }

            /***************************************************
             * @brief copy-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _root_base(const _root_base& other)
                : _root_base(other.get_allocator())
            { 
                if (other.empty())
                    return;
                this->emplace_root(static_cast<_m_cvnode_ptr_t>(other._m_root)->_m_value());
                this->_m_copy_children(this->_m_root, other._m_root);
            }

            constexpr _root_base&
            operator=(const _root_base& other)
            {
                this->clear(); 
                if (other.empty())
                    return *this;
                this->emplace_root(static_cast<_m_cvnode_ptr_t>(other._m_root)->_m_value());
                this->_m_copy_children(this->_m_root, other._m_root);
                return *this;
            }

            /***************************************************
             * @brief move-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _root_base(_root_base&& other)
                : _root_base(other.get_allocator())
            { 
                // refactor to somehow automatically swap stuff
                std::swap(this->_m_root, other._m_root);
                std::swap(this->_m_node_count, other._m_node_count);
            }

            constexpr _root_base&
            operator=(_root_base&& other)
            {
                this->clear();
                std::swap(this->_m_root, other._m_root);
                std::swap(this->_m_node_count, other._m_node_count);
                return *this;
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

            /***************************************************
             * @brief clears the contents.
             ***************************************************/
            constexpr void 
            clear() 
                noexcept
            {
                this->_m_erase_children(this->_m_root);
                this->_m_reset();
            }

            /***************************************************
             * @brief compares trees based on their structure
             *        and values held inside the nodes.
             *        
             *        trees compare equal if their structure
             *        is equal and each node has the same value
             *        than it's 'structural counterpart' in the
             *        other tree (i.e. rootA same rootB, first-childA
             *        same as first-childB, ...).
             ***************************************************/
            template <typename OtherAllocT>
            friend constexpr bool
            operator==(const _root_base& a,
                       const _root_base<_m_node_t, OtherAllocT>& b)
            { 
                return _m_base_t::_s_compare(
                            static_cast<_m_cvnode_ptr_t>(a._m_root_node()), 
                            static_cast<_m_cvnode_ptr_t>(b._m_root_node()))
                    && _m_base_t::_s_compare_children(a._m_root_node(), b._m_root_node()); 
            }
        };


        /***************************************************
         * @brief base-class for trees which should
         *        originate from a valueless-header-node.
         *
         *        this is convenient for e.g. rose-trees, in 
         *        which a construct without a header-node could
         *        lead to an invalid tree-state/node-leaks.
         *
         *        for example, if you would insert a
         *        next-sibling at the value-holding root-node,
         *        there would be no parent-node holding
         *        ownership for that node.
         ***************************************************/
        template <typename NodeT,
                  typename AllocT>
        class _header_base
            : public _basic_tree_base<NodeT, AllocT>
        {
        protected:

            using _m_base_t = _basic_tree_base<NodeT, AllocT>;

            using typename _m_base_t::_m_value_t;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;
            using typename _m_base_t::_m_size_t;
            using typename _m_base_t::_m_node_traits_t;
            using typename _m_base_t::_m_alloc_t;

            using _m_header_t = _m_node_t;

            _m_header_t _m_header;

            constexpr
            void _m_reset()
                noexcept
            {
                this->_m_size_base_t::_m_reset();
            }

            constexpr _m_node_ptr_t
            _m_root_node()
                const noexcept
            { 
                // casting constness away here is OK because if a mutable
                // iterator is constructed, this is non-const anyway and
                // if a const-iterator is constructed, the iterator does not
                // expose the node directly, so constness is restored (i hope).
                return const_cast<_m_node_ptr_t>(std::addressof(this->_m_header)); 
            }
        
            template <typename IterT>
            constexpr IterT
            _m_begin()
                const
            { return _iter_traits<IterT>::_s_header_begin(this->_m_root_node()); }

        public:

            using _m_base_t::_m_base_t;

            constexpr
            _header_base()
                : _m_header()
            { }

            /***************************************************
             * @brief copy-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _header_base(const _header_base& other)
                : _header_base(other.get_allocator())
            { 
                if (other.empty())
                    return;
                this->_m_copy_children(this->_m_root_node(), 
                                       other._m_root_node());
            }

            constexpr _header_base&
            operator=(const _header_base& other)
            {
                this->clear(); 
                if (other.empty())
                    return *this;
                this->_m_copy_children(this->_m_root_node(), 
                                       other._m_root_node());
                return *this;
            }

            /***************************************************
             * @brief move-constructor and assignment-operator.
             ***************************************************/
            constexpr
            _header_base(_header_base&& other)
                : _header_base(other.get_allocator())
            { 
                // refactor to somehow automatically swap stuff
                std::swap(this->_m_header, other._m_header);
                std::swap(this->_m_node_count, other._m_node_count);
            }

            constexpr _header_base&
            operator=(_header_base&& other)
            {
                this->clear();
                std::swap(this->_m_header, other._m_header);
                std::swap(this->_m_node_count, other._m_node_count);
                return *this;
            }

            /***************************************************
             * @brief clears the contents.
             ***************************************************/
            constexpr void 
            clear() 
                noexcept
            {
                for (_m_node_ptr_t _child 
                     : _m_node_traits_t::_s_children(this->_m_root_node()))
                    this->_m_erase_children(_child);
                this->_m_reset();
            }

            /***************************************************
             * @brief compares trees based on their structure
             *        and values held inside the nodes.
             *        
             *        trees compare equal if their structure
             *        is equal and each node has the same value
             *        than it's 'structural counterpart' in the
             *        other tree (i.e. rootA same rootB, first-childA
             *        same as first-childB, ...).
             ***************************************************/
            template <typename OtherAllocT>
            friend constexpr bool
            operator==(const _header_base& a,
                       const _header_base<_m_node_t, OtherAllocT>& b)
            { return _m_base_t::_s_compare_children(a._m_root_node(), b._m_root_node()); }
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

            template <traversal Trav>
            using _m_to_trav_t = _to_traversal_t<Trav, _m_node_t>;

        public:

            using typename _m_base_t::allocator_type;
            using size_type = _m_size_t;
            using typename _m_base_t::value_type;

            static constexpr traversal
                default_traversal = traversal::depth_first;

            template <traversal Trav>
            using queued_iterator       
                = _queued_iterator<false, _m_value_t, _m_to_trav_t<Trav>>;

            template <traversal Trav>
            using const_queued_iterator 
                = _queued_iterator<true, _m_value_t, _m_to_trav_t<Trav>>;

            // using leaf_iterator       = _leaf_iterator<_m_value_t, _m_node_t>;
            
            // using child_iterator      = _child_iterator<_m_value_t, _m_node_t>;
            
            // using node_info           = _node_info<_m_node_t>;

            using iterator       = queued_iterator<default_traversal>;
            using const_iterator = const_queued_iterator<default_traversal>;
            using hook_type      = _m_node_t::_m_hook_t;

            /***************************************************
             * constructors / special-member-functions.
             ***************************************************/

            using _m_base_t::_m_base_t;

            /***************************************************
             * @brief destructor.
             ***************************************************/
            constexpr
            ~_outward_tree_mixin()
                noexcept
            { this->clear(); }

            /***************************************************
             * iterators.
             ***************************************************/

            /***************************************************
             * @returns an iterator to the root-node of the tree.
             ***************************************************/
            constexpr iterator 
            root()
            { return _iter_traits<iterator>::_s_to_iter(this->_m_root_node()); }

            /***************************************************
             * @returns an iterator to the root-node of the tree.
             ***************************************************/
            constexpr const_iterator 
            croot()
                const
            { return _iter_traits<const_iterator>::_s_to_iter(this->_m_root_node()); }

            /***************************************************
             * @returns a queued iterator to the beginning, 
             *          using the specified traversal-type.
             ***************************************************/

            template <traversal Trav>
            constexpr queued_iterator<Trav>
            qbegin()
            { return this->template _m_begin<queued_iterator<Trav>>(); }

            template <traversal Trav>
            constexpr const_queued_iterator<Trav>
            cqbegin()
                const
            { return this->template _m_begin<const_queued_iterator<Trav>>(); }

            /***************************************************
             * @returns an iterator to the beginning, using
             *          the default-traversal and iterator-type.
             ***************************************************/
            constexpr iterator 
            begin() 
            { return this->template qbegin<default_traversal>(); }

            /***************************************************
             * @returns an iterator to the beginning, using
             *          the default-traversal and iterator-type.
             ***************************************************/
            constexpr const_iterator
            cbegin()
                const
            { return this->template cqbegin<default_traversal>(); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr iterator 
            end()
                noexcept
            { return _iter_traits<iterator>::_s_to_iter(nullptr); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr const_iterator
            cend()
                const noexcept
            { return _iter_traits<const_iterator>::_s_to_iter(nullptr); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            template <traversal Trav>
            constexpr queued_iterator<Trav>
            qend()
                noexcept
            { return _iter_traits<queued_iterator<Trav>>::_s_to_iter(nullptr); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            template <traversal Trav>
            constexpr const_queued_iterator<Trav>
            cqend()
                const noexcept
            { return _iter_traits<const_queued_iterator<Trav>>::_s_to_iter(nullptr); }

            /***************************************************
             * modifiers.
             ***************************************************/

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
                    _iter_traits<const_iterator>::_s_to_node(where), as, _new_node
                );
                this->_m_inc_size();
                return _iter_traits<iterator>::_s_to_iter(_new_node);
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
        };


        template <typename BaseT>
        struct _tree_mixin
            : public _outward_tree_mixin<BaseT>
        {
        protected:

            using _m_base_t = _outward_tree_mixin<BaseT>;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_value_t;

            template <traversal Trav>
            using _m_to_trav_t = _to_traversal_t<Trav, _m_node_t>;

        public:

            using _m_base_t::_m_base_t;

            template <traversal Trav>
            using traversing_iterator = _traversing_iterator<false, _m_value_t, _m_to_trav_t<Trav>>;
            template <traversal Trav>
            using const_traversing_iterator = _traversing_iterator<true, _m_value_t, _m_to_trav_t<Trav>>;

            using _m_base_t::default_traversal;

            using typename _m_base_t::hook_type;

            using iterator = traversing_iterator<default_traversal>;
            using const_iterator = traversing_iterator<default_traversal>;
            
            /***************************************************
             * additional iterators.
             ***************************************************/

            /***************************************************
             * @returns a traversing iterator to the beginning, 
             *          using the specified traversal-type.
             ***************************************************/

            template <traversal Trav>
            constexpr traversing_iterator<Trav>
            tbegin()
            { return this->template _m_begin<traversing_iterator<Trav>>(); }

            template <traversal Trav>
            constexpr traversing_iterator<Trav>
            ctbegin()
                const
            { return this->template _m_begin<const_traversing_iterator<Trav>>(); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            template <traversal Trav>
            constexpr traversing_iterator<Trav>
            tend()
                noexcept
            { return _iter_traits<traversing_iterator<Trav>>::_s_to_iter(nullptr); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            template <traversal Trav>
            constexpr const_traversing_iterator<Trav>
            ctend()
                const noexcept
            { return _iter_traits<const_traversing_iterator<Trav>>::_s_to_iter(nullptr); }

            /***************************************************
             * @returns an iterator to the beginning, using
             *          the default-traversal and iterator-type.
             ***************************************************/
            constexpr iterator 
            begin() 
            { return this->template tbegin<default_traversal>(); }

            /***************************************************
             * @returns an iterator to the beginning, using
             *          the default-traversal and iterator-type.
             ***************************************************/
            constexpr const_iterator
            cbegin()
                const
            { return this->template ctbegin<default_traversal>(); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr iterator 
            end()
                noexcept
            { return _iter_traits<iterator>::_s_to_iter(nullptr); }

            /***************************************************
             * @returns an iterator to the end.
             ***************************************************/
            constexpr const_iterator
            cend()
                const noexcept
            { return _iter_traits<const_iterator>::_s_to_iter(nullptr); }

            /***************************************************
             * additional modifiers.
             ***************************************************/

            /***************************************************
             * @brief move a tree (or parts of it) 
             *        to another location in this tree.
             ***************************************************/
            constexpr void
            splice(hook_type to, const_iterator pos, _tree_mixin&& other, const_iterator src)
                noexcept
            {

            }

            /***************************************************
             * @brief erases the subtree at the specified hook.
             ***************************************************/
            void
            erase(hook_type at, iterator where)
            {

            }
        };


        /***************************************************
         * @brief type-aliases for base/mixin combinations.
         ***************************************************/

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