
#ifndef __BINARY_TREE_HPP
#define __BINARY_TREE_HPP

#include <memory>
#include <utility>
#include <type_traits>
#include <concepts>
#include <queue>

#define __BINARY_TREE_STORE_SIZE
#define __BINARY_TREE_NO_EXCEPTIONS

#ifdef __BINARY_TREE_NO_EXCEPTIONS
    #define __BINARY_TREE_NOEXCEPT noexcept
#else
    #define __BINARY_TREE_NOEXCEPT
#endif

namespace tl
{
    enum traversal
    {
        depth_first,
        breadth_first
    };

    enum traversal_order
    {
        pre_order,
        in_order,
        post_order
    };

    namespace __detail
    {
        /*
         * node implementations
         */

        struct __binary_tree_node_base
        {
            __binary_tree_node_base* _M_parent;
            __binary_tree_node_base* _M_left;
            __binary_tree_node_base* _M_right;

            __binary_tree_node_base()
            { this->_M_init(); }

            constexpr
            bool
            _M_root_or_header() const noexcept
            { return this->_M_parent->_M_parent == this; }

            constexpr
            bool
            _M_has_left() const noexcept
            { return this->_M_left != this; }

            constexpr
            bool
            _M_has_right() const noexcept
            { return this->_M_right != this; }

            constexpr
            void
            _M_init() noexcept
            { this->_M_parent = this->_M_left = this->_M_right = this; }

            constexpr
            void
            _M_hook_left(__binary_tree_node_base* const __x) noexcept
            {
                if (__x->_M_has_left())
                { 
                    __x->_M_left->_M_parent = this;
                    this->_M_left = __x->_M_left;
                }
                __x->_M_left = this;
                this->_M_parent = __x;
            }

            constexpr
            void
            _M_hook_right(__binary_tree_node_base* const __x) noexcept
            {
                if (__x->_M_has_right())
                { 
                    __x->_M_right->_M_parent = this;
                    this->_M_right = __x->_M_right;
                }
                __x->_M_right = this;
                this->_M_parent = __x;
            }

            constexpr
            void
            _M_unhook() noexcept
            {
                if (this->_M_parent->_M_left == this)
                { this->_M_parent->_M_left = this->_M_parent; }
                else
                { this->_M_parent->_M_right = this->_M_parent; }
                this->_M_parent = this;
            }

            constexpr
            std::size_t
            _M_node_count() const noexcept
            {
                std::size_t __res = 0;
                if (this->_M_has_left())
                { __res += 1 + this->_M_left->_M_node_count(); }
                if (this->_M_has_right())
                { __res += 1 + this->_M_right->_M_node_count(); }
                return __res;
            }

            constexpr
            std::size_t
            _M_depth() const noexcept
            {
                std::size_t __res = 0;
                const __binary_tree_node_base* __cur = this;
                while (!__cur->_M_root_or_header())
                { __cur = __cur->_M_parent; ++__res; }
                return __res;
            }
        };

        struct __binary_tree_header
            : public __binary_tree_node_base
        {
            using _M_base_t = __binary_tree_node_base;
        #ifdef __BINARY_TREE_STORE_SIZE
            std::size_t _M_size;
        #endif

            __binary_tree_header()
            { this->_M_init(); }

            constexpr
            void
            _M_init() noexcept
            {
            #ifdef __BINARY_TREE_STORE_SIZE
                this->_M_size = 0;
            #endif
                this->_M_base()->_M_init();
            }

            /*
             * technically not const-correct, but this is only violated
             * for getting const-iterators in .cend() to point
             * to the header-node.
             */
            constexpr
            _M_base_t*
            _M_base() const noexcept
            { return static_cast<_M_base_t*>(const_cast<__binary_tree_header*>(this)); }
        };

        template <typename __Tp>
        struct __binary_tree_node
            : public __binary_tree_node_base
        {
            using _M_value_t = __Tp;

            template <typename... __Args>
            __binary_tree_node(__Args&&... __args)
                : _M_value(std::forward<__Args>(__args)...)
            { }

            _M_value_t _M_value;

            constexpr 
            _M_value_t&
            _M_get_value() noexcept
            { return _M_value; }

            constexpr 
            const _M_value_t&
            _M_get_value() const noexcept
            { return _M_value; }
        };

        /*
         * iterator common base-classes
         */

        template <typename __Tp, bool __Const>
        using __maybe_const = std::conditional_t<__Const, const __Tp, __Tp>;

        /*
         * iterators also provide metadata about a tree-node
         */
        template <typename __IterTp>
        struct __binary_tree_iterator_meta_base
        {
            using _M_iter_t = __IterTp;

            [[nodiscard]]
            constexpr
            bool
            has_left() const noexcept
            { return static_cast<const _M_iter_t*>(this)->_M_node->_M_has_left(); }

            [[nodiscard]]
            constexpr
            bool
            has_right() const noexcept
            { return static_cast<const _M_iter_t*>(this)->_M_node->_M_has_right(); }

            [[nodiscard]]
            constexpr
            std::size_t
            depth() const noexcept
            { return static_cast<const _M_iter_t*>(this)->_M_node->_M_depth(); }

            [[nodiscard]]
            constexpr
            std::size_t
            total_child_count() const noexcept
            { return static_cast<const _M_iter_t*>(this)->_M_node->_M_node_count(); }

            [[nodiscard]]
            constexpr
            _M_iter_t
            left() const noexcept
            { return _M_iter_t(static_cast<const _M_iter_t*>(this)->_M_node->_M_left); }

            [[nodiscard]]
            constexpr
            _M_iter_t
            right() const noexcept
            { return _M_iter_t(static_cast<const _M_iter_t*>(this)->_M_node->_M_right); }

            [[nodiscard]]
            constexpr
            _M_iter_t
            parent() const noexcept
            { return _M_iter_t(static_cast<const _M_iter_t*>(this)->_M_node->_M_parent); }

        };

        template <typename __IterTp>
        struct __binary_tree_iterator_postfix_base
        {
            using _M_iter_t = __IterTp;

            constexpr
            _M_iter_t
            operator++(int) noexcept
            {
                _M_iter_t __tmp(static_cast<_M_iter_t&>(*this));
                ++(static_cast<_M_iter_t&>(*this));
                return __tmp;
            }

            constexpr
            _M_iter_t
            operator--(int) noexcept
            {
                _M_iter_t __tmp(static_cast<_M_iter_t&>(*this));
                --(static_cast<_M_iter_t&>(*this));
                return __tmp;
            }
        };

        template <typename __Tp, bool __Const>
        struct __binary_tree_iterator_base
        {
            using _M_iter_base_t     = __binary_tree_iterator_base<__Tp, __Const>;
            using _M_node_base_t     = __binary_tree_node_base;

            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type   = typename std::pointer_traits<__binary_tree_node_base*>::difference_type;
            using value_type        = __Tp;
            using reference         = __maybe_const<value_type, __Const>&;
            using pointer           = __maybe_const<value_type, __Const>*;

            _M_node_base_t* _M_node;

            constexpr
            __binary_tree_iterator_base() noexcept 
                : _M_node() 
            { }

            constexpr
            __binary_tree_iterator_base(_M_node_base_t* __x) noexcept
                : _M_node(__x) 
            { }

            template <bool __OtherConst>
                requires (__Const || !__OtherConst)
            constexpr
            __binary_tree_iterator_base(const __binary_tree_iterator_base<__Tp, __OtherConst>& __x) noexcept
                : _M_node(__x._M_node) 
            { }

            [[nodiscard]]
            constexpr
            reference 
            operator*() const __BINARY_TREE_NOEXCEPT
            { return static_cast<__binary_tree_node<value_type>*>(this->_M_node)->_M_get_value(); }

            [[nodiscard]]
            constexpr
            pointer
            operator->() const __BINARY_TREE_NOEXCEPT
            { return std::addressof(*this); }

            [[nodiscard]]
            friend constexpr
            bool
            operator==(const __binary_tree_iterator_base& __a, 
                       const __binary_tree_iterator_base& __b) noexcept
            { return __a._M_node == __b._M_node; }

        };

        /*
         * iterator implementations
         */

        static
        constexpr
        __binary_tree_node_base*
        _S_next_depth_first(__binary_tree_node_base* __x) noexcept
        {
            if (__x->_M_has_left()) 
            { return __x->_M_left; }
            if (__x->_M_has_right()) 
            { return __x->_M_right; }
            __binary_tree_node_base* __last = __x;
            do
            { 
                __last = __x;
                __x = __x->_M_parent; 
                if (__x->_M_parent == __last)
                { return __x; } /* root found */
            }
            while ((__last == __x->_M_right) || !__x->_M_has_right());
            return __x->_M_right;
        }

        static
        constexpr
        __binary_tree_node_base*
        _S_prev_depth_first(__binary_tree_node_base* __x)
        {
            return __x;
        }

        static
        constexpr
        __binary_tree_node_base*
        _S_next_breadth_first(__binary_tree_node_base* __x)
        {
            return __x;
        }

        static
        constexpr
        __binary_tree_node_base*
        _S_prev_breadth_first(__binary_tree_node_base* __x)
        {
            return __x;
        }

        static
        void
        _S_enqueue_depth_first(std::queue<__binary_tree_node_base*>& __Q, 
            __binary_tree_node_base* __x)
        {
            __Q.push(__x);
            if (__x->_M_has_left())
            { _S_enqueue_depth_first(__Q, __x->_M_left); }
            if (__x->_M_has_right())
            { _S_enqueue_depth_first(__Q, __x->_M_right); }
        }

        static
        void
        _S_enqueue_breadth_first(std::queue<__binary_tree_node_base*>& __Q, 
            __binary_tree_node_base* __x)
        {
            std::queue<__binary_tree_node_base*> __tmp_Q;
            __tmp_Q.push(__x);
            while (!__tmp_Q.empty())
            {
                __binary_tree_node_base* __cur = __tmp_Q.front();
                __tmp_Q.pop();
                __Q.push(__cur);
                if (__cur->_M_has_left())
                { __tmp_Q.push(__cur->_M_left); }
                if (__cur->_M_has_right())
                { __tmp_Q.push(__cur->_M_right); }
            }
            /* should it enqueue until the root is found (one full [begin, end) range)? */
            /* maybe some sort of jump back to root and enqueue from there? */
        }

        template <typename __Tp, bool __Const>
        struct __binary_tree_depth_first_iterator
            : public __binary_tree_iterator_base<__Tp, __Const>
            , public __binary_tree_iterator_meta_base<__binary_tree_depth_first_iterator<__Tp, __Const>>
            , public __binary_tree_iterator_postfix_base<__binary_tree_depth_first_iterator<__Tp, __Const>>
        {
            using _M_base_t = __binary_tree_iterator_base<__Tp, __Const>;
            using _M_iter_t = __binary_tree_depth_first_iterator<__Tp, __Const>;

            using _M_base_t::_M_base_t;

            using __binary_tree_iterator_postfix_base<_M_iter_t>::operator++;
            using __binary_tree_iterator_postfix_base<_M_iter_t>::operator--;

            constexpr
            __binary_tree_depth_first_iterator(const _M_base_t& __x)
                : _M_base_t(__x) 
            { }

            constexpr
            _M_iter_t&
            operator++() noexcept
            {
                this->_M_node = _S_next_depth_first(this->_M_node);
                return *this;
            }

            constexpr
            _M_iter_t&
            operator--() noexcept
            {
                this->_M_node = _S_prev_depth_first(this->_M_node);
                return *this;
            }

        };

        template <typename __Tp, bool __Const>
        struct __binary_tree_breadth_first_iterator
            : public __binary_tree_iterator_base<__Tp, __Const>
            , public __binary_tree_iterator_meta_base<__binary_tree_breadth_first_iterator<__Tp, __Const>>
            , public __binary_tree_iterator_postfix_base<__binary_tree_breadth_first_iterator<__Tp, __Const>>
        {
            using _M_base_t = __binary_tree_iterator_base<__Tp, __Const>;
            using _M_iter_t = __binary_tree_breadth_first_iterator<__Tp, __Const>;

            using _M_base_t::_M_base_t;

            using __binary_tree_iterator_postfix_base<_M_iter_t>::operator++;
            using __binary_tree_iterator_postfix_base<_M_iter_t>::operator--;

            constexpr
            __binary_tree_breadth_first_iterator(const _M_base_t& __x)
                : _M_base_t(__x) 
            { }

            constexpr
            _M_iter_t&
            operator++() noexcept
            {
                this->_M_node = _S_next_breadth_first(this->_M_node);
                return *this;
            }

            constexpr
            _M_iter_t&
            operator--() noexcept
            {
                this->_M_node = _S_prev_breadth_first(this->_M_node);
                return *this;
            }

        };

        template <typename __Tp>
        struct __binary_tree_queued_depth_first_iterator
        {

        };

        template <typename __Tp>
        struct __binary_tree_queued_breadth_first_iterator
        {

        };

        /*
         * tree implementation
         */

        template<typename __Tp,
                 typename __Alloc>
        struct __binary_tree_base
        {
            using _M_value_t         = __Tp;
            using _M_node_t          = __binary_tree_node<_M_value_t>;
            using _M_node_base_t     = __binary_tree_node_base;

            using _M_alloc_t             = typename std::allocator_traits<__Alloc>::template rebind_alloc<_M_value_t>;
            using _M_alloc_traits_t      = std::allocator_traits<_M_alloc_t>;
            using _M_node_alloc_t        = typename _M_alloc_traits_t::template rebind_alloc<_M_node_t>;
            using _M_node_alloc_traits_t = std::allocator_traits<_M_node_alloc_t>;

            /*
             * helper-class for memory-management
             */

            struct __binary_tree_impl
                : public _M_node_alloc_t
            {
                __binary_tree_header _M_header;

                constexpr
                __binary_tree_impl() noexcept(std::is_nothrow_default_constructible_v<_M_node_alloc_t>)
                    : _M_node_alloc_t() 
                { }
                
                constexpr
                __binary_tree_impl(const _M_node_alloc_t& __a) noexcept
                    : _M_node_alloc_t(__a) 
                { }

                constexpr
                __binary_tree_impl(__binary_tree_impl&&) = default;

                constexpr
                __binary_tree_impl(_M_node_alloc_t&& __a, __binary_tree_impl&& __x)
                    : _M_node_alloc_t(std::move(__a)), _M_header(std::move(__x._M_header)) 
                { }

                constexpr
                __binary_tree_impl(_M_node_alloc_t&& __a) noexcept
                    : _M_node_alloc_t(std::move(__a))
                { }
            };

            __binary_tree_impl _M_impl;

            /*
             * node creation/deletion
             */

            typename _M_node_alloc_traits_t::pointer
            _M_get_node()
            { return _M_node_alloc_traits_t::allocate(this->_M_get_node_alloc(), 1); }

            void
            _M_put_node(typename _M_node_alloc_traits_t::pointer __x) noexcept
            { _M_node_alloc_traits_t::deallocate(this->_M_get_node_alloc(), __x, 1); }

            template <typename... __Args>
            typename _M_node_alloc_traits_t::pointer
            _M_create_node(__Args&&... __args)
            {
                typename _M_node_alloc_traits_t::pointer __x = this->_M_get_node();
                _M_node_alloc_traits_t::construct(this->_M_get_node_alloc(), 
                    __x, 
                    std::forward<__Args>(__args)...
                );
                return __x;
            }

            void
            _M_destroy_node(typename _M_node_alloc_traits_t::pointer __x) noexcept
            { 
                _M_node_alloc_traits_t::destroy(this->_M_get_node_alloc(), __x);
                this->_M_put_node(__x);
            }

            /*
             * allocator
             */

            constexpr
            _M_node_alloc_t&
            _M_get_node_alloc() noexcept
            { return this->_M_impl; }

            constexpr
            const _M_node_alloc_t&
            _M_get_node_alloc() const noexcept
            { return this->_M_impl; }

            constexpr
            _M_alloc_traits_t::size_type
            _M_node_count() const noexcept
        #ifdef __BINARY_TREE_STORE_SIZE
            { return this->_M_impl._M_header._M_size; }
        #else
            { 
                if (this->_M_empty()) 
                { return 0; }
                return this->_M_root()->_M_node_count(); 
            }
        #endif

        #ifdef __BINARY_TREE_STORE_SIZE
            constexpr
            void
            _M_inc_size(typename _M_alloc_traits_t::size_type __i) noexcept
            { this->_M_impl._M_header._M_size += __i; }

            constexpr
            void
            _M_dec_size(typename _M_alloc_traits_t::size_type __i) noexcept
            { this->_M_impl._M_header._M_size -= __i; }

            constexpr
            void
            _M_set_size(typename _M_alloc_traits_t::size_type __i) noexcept
            { this->_M_impl._M_header._M_size = __i; }
        #endif

            /*
             * constructors
             */

            __binary_tree_base() = default;

            __binary_tree_base(const _M_node_alloc_t& __a)
                : _M_impl(__a)
            { }

            __binary_tree_base(__binary_tree_base&&) = default;

            __binary_tree_base(_M_node_alloc_t&& __a, __binary_tree_base&& __x)
                : _M_impl(std::move(__a), std::move(__x._M_impl))
            { }

            __binary_tree_base(_M_node_alloc_t&& __a)
                : _M_impl(std::move(__a))
            { }

            /*
             * destructor
             */

            ~__binary_tree_base()
            { this->_M_clear(); }

            /*
             * node accessors
             */

            constexpr
            _M_node_base_t*
            _M_root() noexcept
            { return this->_M_impl._M_header._M_parent; }

            constexpr
            const _M_node_base_t*
            _M_root() const noexcept
            { return this->_M_impl._M_header._M_parent; }

            constexpr
            bool
            _M_empty() const noexcept
            { return this->_M_root() == this->_M_impl._M_header._M_base(); }

            /*
             * low-level node-operations
             */

            template <typename... __Args>
            _M_node_base_t*
            _M_insert_root(__Args&&... __args)
            {
                _M_node_base_t* __tmp = this->_M_create_node(std::forward<__Args>(__args)...);
                /* if root already present, hook to the left of the new root */
                if (!this->_M_empty())
                { this->_M_root()->_M_hook_left(__tmp); }
                this->_M_impl._M_header._M_parent = __tmp; 
                __tmp->_M_parent = this->_M_impl._M_header._M_base();
            #ifdef __BINARY_TREE_STORE_SIZE
                this->_M_inc_size(1);
            #endif
                return __tmp;
            }

            template <typename... __Args>
            constexpr
            _M_node_base_t*
            _M_insert(bool __left, _M_node_base_t* __x, __Args&&... __args)
            { 
                if (__x == this->_M_impl._M_header._M_base())
                { return this->_M_insert_root(std::forward<__Args>(__args)...); }
                
                _M_node_base_t* __tmp = this->_M_create_node(std::forward<__Args>(__args)...);
                if (__left)
                { __tmp->_M_hook_left(__x); }
                else
                { __tmp->_M_hook_right(__x); }
            #ifdef __BINARY_TREE_STORE_SIZE
                this->_M_inc_size(1);
            #endif
                return __tmp;
            }

            constexpr
            _M_node_base_t*
            _M_clone(const _M_node_base_t* __x)
            {
                _M_node_base_t* __tmp = this->_M_create_node(static_cast<const _M_node_t*>(__x)->_M_get_value());
                if (__x->_M_has_left())
                { this->_M_clone(__x->_M_left)->_M_hook_left(__tmp); }
                if (__x->_M_has_right())
                { this->_M_clone(__x->_M_right)->_M_hook_right(__tmp); }
                return __tmp;
            }

            constexpr
            void
            _M_erase_children(_M_node_base_t* __x) noexcept
            {
                if (__x->_M_has_left())
                { 
                    this->_M_erase_children(__x->_M_left);
                    this->_M_destroy_node(static_cast<_M_node_t*>(__x->_M_left));
                #ifdef __BINARY_TREE_STORE_SIZE
                    this->_M_dec_size(1);
                #endif
                }
                if (__x->_M_has_right())
                { 
                    this->_M_erase_children(__x->_M_right);
                    this->_M_destroy_node(static_cast<_M_node_t*>(__x->_M_right));
                #ifdef __BINARY_TREE_STORE_SIZE
                    this->_M_dec_size(1);
                #endif
                }
            }

            constexpr
            void
            _M_erase(_M_node_base_t* __x) noexcept
            {
                if (__x == this->_M_root())
                { this->_M_impl._M_header._M_init(); }
                else
                { __x->_M_unhook(); }
                this->_M_erase_children(__x);
                this->_M_destroy_node(static_cast<_M_node_t*>(__x));
            #ifdef __BINARY_TREE_STORE_SIZE
                this->_M_dec_size(1);
            #endif
            }

            constexpr
            void
            _M_clear() noexcept
            { 
                if (!this->_M_empty())
                {
                    this->_M_erase(this->_M_root()); 
                    this->_M_init();
                }
            }

            constexpr
            void
            _M_init() noexcept
            { this->_M_impl._M_header._M_init(); }

            constexpr
            void
            _M_transfer() noexcept
            {

            }
        };
    }

    enum binary_tree_hook_type
    {
        left,
        right
    };

    /*
     * @brief 
     */
    template <typename T, 
              typename Allocator = std::allocator<T>>
    class binary_tree
        : public __detail::__binary_tree_base<T, Allocator>
    {
        using _M_base_t = __detail::__binary_tree_base<T, Allocator>;

        using _M_alloc_t        = Allocator;
        using _M_alloc_traits_t = std::allocator_traits<_M_alloc_t>;

        using typename _M_base_t::_M_node_base_t;
        using typename _M_base_t::_M_node_alloc_t;
        using typename _M_base_t::_M_node_alloc_traits_t;
        
    public:
        using value_type      = T;
        using pointer         = typename _M_alloc_traits_t::pointer;
        using const_pointer   = typename _M_alloc_traits_t::const_pointer;
        using reference       = value_type&;
        using const_reference = const value_type&;

        using size_type       = typename _M_alloc_traits_t::size_type;
        using difference_type = typename _M_alloc_traits_t::difference_type;
        using allocator_type  = Allocator;

        using iterator               = __detail::__binary_tree_depth_first_iterator<value_type, false>;
        using const_iterator         = __detail::__binary_tree_depth_first_iterator<value_type, true>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using hook_type              = binary_tree_hook_type; 

        /*
         * constructors
         */

        binary_tree() noexcept(noexcept(allocator_type())) 
            : binary_tree(allocator_type()) { }

        explicit constexpr
        binary_tree(const allocator_type& allocator) noexcept
        { }

        constexpr
        binary_tree(const binary_tree& other) noexcept
        { }

        constexpr
        binary_tree(binary_tree&& other) noexcept
        { }

        /*
         * destructor
         */

        constexpr
        ~binary_tree()
        { }

        /*
         * assignment
         */

        constexpr
        binary_tree& operator=(const binary_tree&)
        { }

        constexpr
        binary_tree& operator=(binary_tree&&) noexcept
        { }

        /*
         * information
         */

        [[nodiscard]]
        constexpr
        allocator_type
        get_allocator() const noexcept
        { return this->_M_get_node_alloc(); }

        [[nodiscard]]
        constexpr
        bool
        empty() const noexcept
        { return this->_M_empty(); }

        [[nodiscard]]
        constexpr
        size_type
        size() const noexcept
        { return this->_M_node_count(); }

        [[nodiscard]]
        constexpr
        size_type
        max_size() const noexcept
        { return _M_node_alloc_traits_t::max_size(this->_M_get_node_alloc()); }

        /*
         * iterators
         */

        constexpr
        iterator
        begin() noexcept
        { return iterator(this->_M_root()); }

        constexpr
        iterator
        end() noexcept
        { return iterator(this->_M_impl._M_header._M_base()); }

        constexpr
        reverse_iterator
        rbegin() noexcept
        { return reverse_iterator(this->begin()); }

        constexpr
        reverse_iterator
        rend() noexcept
        { return reverse_iterator(this->end()); }

        constexpr
        const_iterator
        cbegin() const noexcept
        { return const_iterator(this->_M_impl._M_header._M_parent); }

        constexpr
        const_iterator
        cend() const noexcept
        { return const_iterator(this->_M_impl._M_header._M_base()); }

        constexpr
        const_reverse_iterator
        crbegin() const noexcept
        { return const_reverse_iterator(this->cbegin()); }

        constexpr
        const_reverse_iterator
        crend() const noexcept
        { return const_reverse_iterator(this->cend()); }

        /*
         * element access
         */

        constexpr
        reference
        root() noexcept
        { return *this->begin(); }

        constexpr
        const_reference
        root() const noexcept
        { return *this->cbegin(); }

        /*
         * modifiers 
         */

        constexpr
        void 
        clear() noexcept
        { this->_M_clear(); }
        
        constexpr
        iterator
        insert(const value_type& value)
        { return this->_M_insert_root(value); }

        constexpr
        iterator
        insert(value_type&& value)
        { return this->_M_insert_root(std::move(value)); }

        constexpr
        iterator
        insert(const hook_type& how, const_iterator pos, const value_type& value)
        {
            if (how == left) 
            { return iterator(this->_M_insert(true, pos._M_node, value)); }
            return iterator(this->_M_insert(false, pos._M_node, value));
        }

        constexpr
        iterator
        insert(const hook_type& how, const_iterator pos, value_type&& value)
        { 
            if (how == left) 
            { return iterator(this->_M_insert(true, pos._M_node, std::move(value))); }
            return iterator(this->_M_insert(false, pos._M_node, std::move(value)));
        }

        template <typename... Args>
            requires std::constructible_from<value_type, Args...>
        constexpr
        iterator
        emplace(Args&&... args)
        { return this->_M_insert_root(std::forward<Args>(args)...); }

        template <typename... Args>
        constexpr
        iterator
        emplace(const hook_type& how, const_iterator pos, Args&&... args)
        {
            if (how == left) 
            { return iterator(this->_M_insert(true, pos._M_node, std::forward<Args>(args)...)); }
            return iterator(this->_M_insert(false, pos._M_node, std::forward<Args>(args)...));
        }

        constexpr
        iterator
        erase(const_iterator pos)
        { 
            _M_node_base_t* __next = pos._M_node->_M_parent;
            this->_M_erase(pos._M_node);
            return iterator(__next);
        }

        constexpr
        void
        swap(binary_tree& other) noexcept
        { 
            
        }

        /*
         * operations
         */

        constexpr
        void
        splice(const_iterator pos, binary_tree& other)
        { this->splice(pos, std::move(other)); }

        constexpr
        void
        splice(const_iterator pos, binary_tree&& other)
        { 
            if (!other.empty())
            {
                this->_M_transfer(pos, other.begin(), other.end());
            #ifdef __BINARY_TREE_STORE_SIZE
                this->_M_inc_size(other.size());
                other._M_set_size(0);
            #endif
            }
        }

        constexpr
        void
        splice(const_iterator pos, binary_tree& other, const_iterator it)
        { }

        constexpr
        void
        splice(const_iterator pos, binary_tree&& other, const_iterator it)
        { }

        constexpr
        void
        splice(const_iterator pos, binary_tree& other, 
               const_iterator first, const_iterator last)
        { }

        constexpr
        void
        splice(const_iterator pos, binary_tree&& other, 
               const_iterator first, const_iterator last)
        { }

    };

    /*
     * general binary-tree-iterators
     */

    template <typename T>
    using depth_first_iterator         = __detail::__binary_tree_depth_first_iterator<T, false>;
    template <typename T>
    using depth_first_const_iterator   = __detail::__binary_tree_depth_first_iterator<T, true>;
    
    template <typename T>
    using breadth_first_iterator       = __detail::__binary_tree_breadth_first_iterator<T, false>;
    template <typename T>
    using breadth_first_const_iterator = __detail::__binary_tree_breadth_first_iterator<T, true>;

    namespace __detail
    {
        template <typename __Tp, traversal __Trav>
        struct __binary_tree_iterator_traits;

        template <typename __Tp>
        struct __binary_tree_iterator_traits<__Tp, traversal::depth_first>
        { using _M_iter_t = depth_first_iterator<__Tp>; };

        template <typename __Tp>
        struct __binary_tree_iterator_traits<__Tp, traversal::breadth_first>
        { using _M_iter_t = breadth_first_iterator<__Tp>; };

        template <typename __Tp, traversal __Trav>
        struct __binary_tree_const_iterator_traits;

        template <typename __Tp>
        struct __binary_tree_const_iterator_traits<__Tp, traversal::depth_first>
        { using _M_iter_t = depth_first_const_iterator<__Tp>; };

        template <typename __Tp>
        struct __binary_tree_const_iterator_traits<__Tp, traversal::breadth_first>
        { using _M_iter_t = breadth_first_const_iterator<__Tp>; };
    }

    template<typename T, traversal Traversal>
    using iterator       = typename __detail::__binary_tree_iterator_traits<T, Traversal>::_M_iter_t;

    template<typename T, traversal Traversal>
    using const_iterator = typename __detail::__binary_tree_const_iterator_traits<T, Traversal>::_M_iter_t;

}

#endif