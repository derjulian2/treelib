
#ifndef FLEX_TREE_HPP
#define FLEX_TREE_HPP

/**
 * @file 
 * flex_tree.hpp
 * @brief 
 * provides C++ STL-like tree-containers where each child-node can hold
 * an arbitrary number of child-nodes.
 * @author
 * Julian Benzel
 * @date
 * 10.08.2025
 * 
 * @details
 * provides the following template-types:
 * - trees::flex_tree<Type, Alloc>
 * a regular 'xml-style' tree data-structure where each node holds pointers
 * to parent/sibling/child nodes to make up the tree.
 * - trees::packed_flex_tree<Type, Alloc>
 * works the same as flex_tree, however the nodes are packed into a single, shared array
 * where traversal is implemented using mostly offsets. allows for faster iteration but slower modifications.
 * 
 * using a custom-allocator-instance with braced-initializion is not supported, as there is no way to pass
 * that allocator to the tree and then use that allocator for the initialization (as it will run recursively from the innermost list).
 *
 * @todo
 * - copy-paste node insertion logic from previous repo
 * - copy-paste algorithms from previous repo
 * - document (especially on node/builders)
 * - implement packed_flex_tree
 */

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <iterator>
#include <stdexcept>

#include "alloc_utility.hpp"
#include "traversal.hpp"

namespace trees
{
    template <typename Type, typename Allocator>
        requires (std::is_copy_constructible<Type>::value && std::is_copy_assignable<Type>::value)
    class flex_tree;

    namespace detail
    {
        /*
         * macros for iterator-composition:
         * 
         * the iterators used in the tree-data-structues consist of different access and traversal components depending on the desired use-case.
         *
         * since realizing this with inheritance isn't exactly easy/possible and composition causes space-overhead, i chose to use macros
         * and try to keep things as clean and #undef'd as possible
         */
        
        #define _tree_iterator_base(tree_t, t, a) \
            private: \
                using node_type = tree_t<t,a>::node_type; \
                friend class tree_t<t,a>; \
            public: \
                using iterator_category = std::bidirectional_iterator_tag; \
                using value_type = node_type::value_type; \
                using difference_type = std::ptrdiff_t; \
                using pointer = value_type*; \
                using reference = value_type&; \
            private: \
                node_type* _M_ptr;
            

        #define _tree_iterator_const_base(tree_t, t, a) \
            private: \
                using node_type = const tree_t<t,a>::node_type; \
                friend class tree_t<t,a>; \
            public: \
                using iterator_category = std::bidirectional_iterator_tag; \
                using value_type = node_type::value_type; \
                using difference_type = std::ptrdiff_t; \
                using pointer = value_type*; \
                using reference = value_type&; \
            private: \
                node_type* _M_ptr;

        #define _tree_iterator_access \
            public: \
                reference operator*() { return _M_ptr->_M_value; } \
                pointer operator->() { return &_M_ptr->_M_value; }

        #define _tree_iterator_constructor(iter_t) \
            private: \
                iter_t(node_type* _ptr) : _M_ptr(_ptr) { }

        #define _tree_iterator_equality_operators(iter_t) \
            public: \
                friend bool operator==(const iter_t& i, const iter_t& j) { return i._M_ptr == j._M_ptr; } \
                friend bool operator!=(const iter_t& i, const iter_t& j) { return !(i == j); } 

        #define _tree_iterator_traversal(iter_t, algorithm) \
            public: \
                iter_t& operator++() { _M_ptr = _next<algorithm, node_type>()(_M_ptr); return *this; } \
                iter_t  operator++(int) { iter_t _old(*this); ++(*this); return _old;  } \
                iter_t& operator--() { _M_ptr = _prev<algorithm, node_type>()(_M_ptr); return *this; } \
                iter_t  operator--(int) { iter_t _old(*this); --(*this); return _old; }

        #define _tree_node_property_functions(iter_t) \
            public: \
                bool is_root() const { return _M_ptr->_is_root(); } \
                bool is_first_child() const { return _M_ptr->_is_first_child(); } \
                bool is_last_child() const { return _M_ptr->_is_last_child(); } \
                bool has_children() const { return _M_ptr->_has_children(); } \
                bool has_next() const { return _M_ptr->_has_next(); } \
                bool has_previous() const { return _M_ptr->_has_prev(); } \
                bool is_sibling_of(iter_t other) const \
                {   \
                    if (this->is_root() || other.is_root()) { throw std::logic_error("parent-node does not exist for root-node"); } \
                    return this->parent() == other.parent(); \
                } \
                bool is_cousin_of(iter_t other) const \
                { \
                    if (this->is_root() || other.is_root()) { throw std::logic_error("parent-node does not exist for root-node"); } \
                    if (this->parent().is_root() || other.parent().is_root()) { throw std::logic_error("parent-node does not exist for root-node"); } \
                    return this->parent().parent() == other.parent().parent(); \
                } \
                iter_t parent() const { if (is_root()) { throw std::logic_error("parent-node does not exist for root-node"); } return _M_ptr->_parent();  } /*possibly need const-ptr*/ \
                iter_t first_child() const { if (!has_children()) { throw std::logic_error("first-child-node does not exist"); } return _M_ptr->_first_child(); } \
                iter_t last_child() const { if (!has_children()) { throw std::logic_error("first-child-node does not exist"); } return _M_ptr->_last_child(); } \
                iter_t next() const { if (!has_next()) { throw std::logic_error("next-node does not exist"); } return _M_ptr->_next(); } \
                iter_t previous() const { if (!has_previous()) { throw std::logic_error("previous-node does not exist"); } return _M_ptr->_prev(); } \
                std::size_t depth() const { return _M_ptr->_depth(); } \
                std::size_t child_count() const { return _M_ptr->_child_count(); }
                



        /* additional macros for node-properties (should be accessible via iterators) */

        /**
         * @brief a tree iterator.
         */
        template <traversal Traversal, typename Type, typename Alloc>
        class _flex_tree_iterator { };

        template <typename Type, typename Alloc>
        class _flex_tree_iterator<depth_first_pre_order, Type, Alloc>
        {
            _tree_iterator_base(::trees::flex_tree, Type, Alloc)
            _tree_iterator_access
            _tree_iterator_equality_operators(_flex_tree_iterator)
            _tree_iterator_traversal(_flex_tree_iterator, depth_first_pre_order)
            _tree_iterator_constructor(_flex_tree_iterator)
            _tree_node_property_functions(_flex_tree_iterator)
        };

        /**
         * @brief a const tree iterator.
         */
        template <traversal Traversal, typename Type, typename Alloc>
        class _flex_tree_const_iterator { };

        template <typename Type, typename Alloc>
        class _flex_tree_const_iterator<depth_first_pre_order, Type, Alloc>
        {
            _tree_iterator_const_base(::trees::flex_tree, Type, Alloc)
            _tree_iterator_access
            _tree_iterator_equality_operators(_flex_tree_const_iterator)
            _tree_iterator_traversal(_flex_tree_const_iterator, depth_first_pre_order)
            _tree_iterator_constructor(_flex_tree_const_iterator)
            _tree_node_property_functions(_flex_tree_const_iterator)
        };

        #undef _tree_iterator_base
        #undef _tree_iterator_const_base
        #undef _tree_iterator_access
        #undef _tree_iterator_traversal
        #undef _tree_iterator_equality_operators
        #undef _tree_iterator_constructor
        #undef _tree_node_property_functions

        template <typename Type, typename Allocator>
        class _packed_flex_tree_node final
            : public allocator_utility<Allocator>,
              public allocator_utility<typename std::allocator_traits<Allocator>::template rebind_alloc<_packed_flex_tree_node<Type, Allocator>>> // rebind alloc to node type
        {
        public:

            using value_type = Type;
            using allocator_type = Allocator;

        private:

            using node_allocator_type = std::allocator_traits<Allocator>::template rebind_alloc<_packed_flex_tree_node<Type, Allocator>>;
            using value_alloc_base = allocator_utility<Allocator>;
            using node_alloc_base = allocator_utility<node_allocator_type>;

            _packed_flex_tree_node* _M_parent; // will be used for child-node-allocation in initializer_list construction
            std::size_t _M_child_count{0};
            std::size_t _M_total_children{0};
            std::size_t _M_depth{0};

            value_type* _M_value{nullptr};

        public:

            template <typename... Args>
            _packed_flex_tree_node(Args&&... args) 
            { 
                this->_M_value = value_alloc_base::_alloc_n(allocator_type(), 1);
                value_alloc_base::_construct(allocator_type(), this->_M_value, std::forward<Args>(args)...);
            }

            template <typename Arg>
            _packed_flex_tree_node(Arg&& arg, std::initializer_list<_packed_flex_tree_node> ilist) 
                : _M_total_children(ilist.size())
            { 
                this->_M_value = value_alloc_base::_alloc_n(allocator_type(), 1); 
                value_alloc_base::_construct(allocator_type(), this->_M_value, std::forward<Arg>(arg));
                // reallocate the array in such a way that all child nodes are kept in sequence with their parents
                for (std::size_t _i = 0; _i < ilist.size(); ++_i)
                { 
                    this->_M_total_children += ilist[_i]._M_total_children;
                }

                this->_M_parent = node_alloc_base::_alloc_n(node_allocator_type(), this->_M_total_children);

                std::size_t _j = 0;
                for (std::size_t _i = 0; _i < ilist.size(); ++_i)
                {
                    node_alloc_base::_construct(node_allocator_type(), this->_M_parent + _j, std::move(*(ilist.begin() + _i)));
                    for (std::size_t _k = 0; _k < ilist[_i]._M_child_count; ++_k)
                    {
                        node_alloc_base::_construct(node_allocator_type(), this->_M_parent + _j, std::move(*(ilist.begin() + _i)._M_parent + _k));
                        node_alloc_base::_destroy(node_allocator_type(), *(ilist.begin() + _i)._M_parent + _k);
                        ++_j;
                    }
                    node_alloc_base::_dealloc_n(node_allocator_type(), *(ilist.begin() + _i)._M_parent, *(ilist.begin() + _i)._M_child_count); // dispose the old sequence
                    ++_j;
                    /* recursive increment depth here*/
                }
            }
            

            _packed_flex_tree_node(_packed_flex_tree_node&) { }
            _packed_flex_tree_node(const _packed_flex_tree_node&) { }
            
            _packed_flex_tree_node(_packed_flex_tree_node&&) { }
            _packed_flex_tree_node(const _packed_flex_tree_node&&) { }

        };
    }

    /**
     * @brief C++ STL-like arbitrary-size tree-data-structure.
     * 
     * @tparam Type your type to be stored with every node.
     * @tparam Allocator an allocator. default is std::allocator.
     */
    template <typename Type, typename Allocator = std::allocator<Type>>
        requires (std::is_copy_constructible<Type>::value && std::is_copy_assignable<Type>::value)
    class flex_tree
    {
    public:
        
        static inline constexpr traversal default_traversal = depth_first_pre_order;

        using value_type = Type;
        using allocator_type = Allocator;

        template <traversal Traversal = default_traversal>
        using iterator = detail::_flex_tree_iterator<Traversal, value_type, allocator_type>;

        template <traversal Traversal = default_traversal>
        using const_iterator = detail::_flex_tree_const_iterator<Traversal, value_type, allocator_type>;

    protected:

        /*
         * more friend declarations than necessary, but currently, you cannot befriend partially specialized templates
         * or partially specialize nested types (as the iterators would be if that were the case)
         */
        template <traversal, typename, typename> friend class detail::_flex_tree_iterator; 
        template <traversal, typename, typename> friend class detail::_flex_tree_const_iterator;
        
        /*
         * node_type definition
         */

        /*
         * this class is not meant to be used explicitly by the user. it will leak memory if not used properly. 
         *
         * it is designed to be used implictily in std::initializer_lists for intuitive tree construction. the values, that get allocated during the process,
         * will be owned and handled (and also disposed of) by a corresponding tree data-structure. do not use this without a tree.
         */
        struct _flex_tree_node final
        {
            using value_type = Type;
            using allocator_type = Allocator;

            using node_allocator_type = std::allocator_traits<Allocator>::template rebind_alloc<_flex_tree_node>;
            using value_alloc = allocator_utility<Allocator>;
            using node_alloc = allocator_utility<node_allocator_type>;

            _flex_tree_node* _M_parent{nullptr};
            _flex_tree_node* _M_first_child{nullptr};
            _flex_tree_node* _M_last_child{nullptr};
            _flex_tree_node* _M_next{nullptr};
            _flex_tree_node* _M_prev{nullptr};

            std::size_t _M_child_count{0};
            std::size_t _M_depth{0};

            value_type _M_value;

            _flex_tree_node() { }

            /*
             * only available when allocator_type is stateless/default-constructible
             */

            template <typename... Args>
                requires std::is_default_constructible<allocator_type>::value
            _flex_tree_node(Args&&... args) 
                : _M_value(args...)
            { 

            }

            template <typename Arg>
                requires std::is_default_constructible<allocator_type>::value
            _flex_tree_node(Arg&& arg, std::initializer_list<_flex_tree_node> ilist)
                : _M_value(arg)
            { 
                allocator_type alloc;
                node_allocator_type node_alloc;
                this->_child_nodes_from_ilist(node_alloc, ilist);
            }

            template <typename... Args>
            void _make_value(allocator_type& alloc, Args&&... args) 
            {
                this->_M_value = value_alloc::_alloc_n(alloc, 1); 
                value_alloc::_construct(alloc, this->_M_value, std::forward<Args>(args)...);
            }

            void _hook_dangling_child_nodes()
            {
                if (_has_children())
                {
                    _flex_tree_node* _iter = _first_child();
                    do
                    {
                        _iter->_M_parent = this;
                        _iter = _iter->_is_last_child() ? nullptr : _iter->_next();
                    }
                    while (_iter);
                }
            }

            void _child_nodes_from_ilist(node_allocator_type& node_alloc, std::initializer_list<_flex_tree_node> ilist)
            {
                for (std::size_t _i = 0; _i < ilist.size(); ++_i)
                { 
                    _flex_tree_node* _new = node_alloc::_alloc_n(node_alloc, 1);
                    node_alloc::_construct(node_alloc, _new, *(ilist.begin() + _i));
                    _new->_hook_as_last_child(this);
                    _new->_hook_dangling_child_nodes();
                }
            }

            _flex_tree_node(_flex_tree_node&) = default;
            _flex_tree_node(const _flex_tree_node&) = default;
            
            _flex_tree_node(_flex_tree_node&&) = default;

            /*
             * graceful insert/erase utility and subroutines
             */

            void _entangle(_flex_tree_node* _next)
            {
                this->_M_next = _next;
                _next->_M_prev = this;
            }

            void _update_new_first_child(_flex_tree_node* _parent)
            {
                _parent->_M_first_child = this;
            }

            void _update_new_last_child(_flex_tree_node* _parent)
            {
                _parent->_M_last_child = this;
            }

            void _update_new_only_child(_flex_tree_node* _parent)
            {
                _parent->_M_first_child = _parent->_M_last_child = this;
            }

            void _update_new_child(_flex_tree_node* _parent)
            {
                this->_M_parent = _parent;
                this->_M_depth = _parent->_M_depth + 1;
                ++_parent->_M_child_count;
            }

            _flex_tree_node* _find_next_cousin(_flex_tree_node* _parent)
            {
                if (_parent->_has_next())
                {
                    if (_parent->_next()->_has_children())
                    { return _parent->_next()->_first_child(); }
                }
                return nullptr;
            }

            _flex_tree_node* _find_prev_cousin(_flex_tree_node* _parent)
            {
                if (_parent->_has_prev())
                {
                    if (_parent->_prev()->_has_children())
                    { return _parent->_prev()->_last_child(); }
                }
                return nullptr;
            }

            void _update_discard_only_child()
            {
                this->_parent()->_M_first_child = this->_parent()->_M_last_child = nullptr;
                --this->_parent()->_M_child_count;
            }

            void _update_discard_first_child()
            {
                this->_parent()->_M_first_child = this->_next();
                --this->_parent()->_M_child_count;
            }

            void _update_discard_last_child()
            {
                this->_parent()->_M_last_child = this->_prev();
                --this->_parent()->_M_child_count;
            }

            /*
             * methods for graceful insert into/erase out of the tree-structure
             */

            void _hook_as_first_child(_flex_tree_node* _parent)
            {
                if (_parent->_has_children())
                {
                    if (_parent->_first_child()->_has_prev()) { _parent->_first_child()->_prev()->_entangle(this); }
                    _entangle(_parent->_first_child());
                    _update_new_first_child(_parent);
                }
                else 
                {
                    _update_new_only_child(_parent);
                    if (!_parent->_is_root())
                    {
                        this->_M_prev = _find_prev_cousin();
                        this->_M_next = _find_next_cousin();
                    }
                }
                _update_new_child(_parent);
                if (_has_children()) { _recursive_update_depth(); }
            }

            void _hook_as_last_child(_flex_tree_node* _parent)
            {
                if (_parent->_has_children())
                {
                    if (_parent->_last_child()->_has_next()) { _entangle(_parent->_last_child()->_next()); }
                    _parent->_last_child()->_entangle(this);
                    _update_new_last_child(_parent);
                }
                else 
                {
                    _update_new_only_child(_parent);
                    if (!_parent->_is_root())
                    {
                        this->_M_prev = _find_prev_cousin(_parent);
                        this->_M_next = _find_next_cousin(_parent);
                    }
                }
                _update_new_child(_parent);
                if (_has_children()) { _recursive_update_depth(); }
            }

            void _hook_as_next_sibling(_flex_tree_node* _prev_sibling)
            {
                if (_prev_sibling->_is_last_child()) { _hook_as_last_child(_prev_sibling->_parent()); }
                else
                {
                    this->_entangle(_prev_sibling->_next());
                    _prev_sibling->_entangle(this);
                    _update_new_child(_prev_sibling->_parent()); 
                }
                if (_has_children()) { _recursive_update_depth(); }
            }

            void _hook_as_prev_sibling(_flex_tree_node* _next_sibling)
            {
                if (_next_sibling->_is_first_child()) { _hook_as_first_child(_next_sibling->_parent()); }
                else
                {
                    _next_sibling->_prev()->_entangle(this);
                    this->_entangle(_next_sibling);
                    _update_new_child(_next_sibling->_parent()); 
                }
                if (_has_children()) { _recursive_update_depth(); }
            }
            
            void _unhook()
            {
                if (_is_first_child() && !_is_last_child()) // only has next sibling
                {
                    _update_discard_first_child();
                    if (_has_prev()) { _prev()->_entangle(_next()); }
                    else { _next()->_M_prev = nullptr; }
                }
                else if (!_is_first_child() && _is_last_child()) // only has prev sibling
                {
                    _update_discard_last_child();
                    if (_has_next()) { _prev()->_entangle(_next()); }
                    else { _prev()->_M_next = nullptr; }
                }
                else if (_is_first_child() && _is_last_child()) // only child
                {
                    _update_discard_only_child();
                    if (_has_next() && _has_prev()) { _prev()->_entangle(_next()); }
                    else if (_has_next() && !_has_prev()) { _next()->_M_prev = nullptr; }
                    else if (!_has_next() && _has_prev()) { _prev()->_M_next = nullptr; }
                }
                else if (!_is_first_child() && !_is_last_child()) // has both siblings
                {
                    _prev()->_entangle(_next());
                    --_parent()->_M_child_count;
                }
            }

            void _recursive_update_depth()
            {
                node_type* _iter = _M_first_child;
                do
                {
                    _iter->_M_depth = this->_M_depth + 1;
                    if (_iter->_has_children()) { _iter->_recursive_update_depth(); }
                    _iter = _iter->_is_last_child() ? nullptr : _iter->_M_next;
                }
                while (_iter);
            }

            /*
            * @returns number of erased nodes
            */
            std::size_t _recursive_erase_children(node_allocator_type& node_alloc)
            {
                std::size_t _counter = 0;
                node_type* _iter = _M_first_child;
                node_type* _next;
                allocator_type alloc(node_alloc);
                while (_iter)
                {
                    if (_iter->_has_children()) { _counter += _iter->_recursive_erase_children(node_alloc); }
                    _next = _iter->_next();
                    delete _iter;
                    ++_counter;
                    _iter = _next;
                }
                return _counter;
            }

            /*
             * utility methods for the iterator to expose to the user (iterator-code may be reused for other trees)
             */

            std::size_t _child_count() const { return _M_child_count; }
            std::size_t _depth() const { return _M_depth; }
            _flex_tree_node* _parent() const { return _M_parent; }
            _flex_tree_node* _first_child() const { return _M_first_child; }
            _flex_tree_node* _last_child() const { return _M_last_child; }
            _flex_tree_node* _next() const { return _M_next; }
            _flex_tree_node* _prev() const { return _M_prev; }
            bool _is_root() const { return !static_cast<bool>(this->_M_parent); }
            bool _has_children() const { return this->_M_first_child; }
            bool _is_first_child() const { return this->_is_root() ? false : (this->_M_parent->_M_first_child == this); }
            bool _is_last_child() const { return this->_is_root() ? false :  (this->_M_parent->_M_last_child == this); }
            bool _has_next() const { return this->_M_next; }
            bool _has_prev() const { return this->_M_prev; }
        };

        using node_type = _flex_tree_node;
        using node_allocator_type = std::allocator_traits<allocator_type>::template rebind_alloc<node_type>;

        /*
         * nested class that inherits from node_allocator_type for base-class-optimization
         */
        struct _flex_tree_impl 
            : public node_allocator_type
        {
            node_type* _M_header{nullptr};
            std::size_t _M_size{0};

            _flex_tree_impl(const node_allocator_type& alloc)
                : node_allocator_type(alloc) 
            { 
                this->_M_header = new node_type();
            }

            ~_flex_tree_impl()
            {
                if (_M_header)
                {
                    delete this->_M_header;
                }
            }

            node_allocator_type& _get_node_alloc() { return *this; }
            const node_allocator_type& _get_node_alloc() const { return *this; }

            allocator_type _get_alloc() const { return allocator_type(*this); }
        };

        _flex_tree_impl _M_impl; // can be used as alloc for both value and nodes bc of inheritance
        
        void _recursive_copy()
        {
            node_type* _iter;
            do
            {

            }
            while (_iter);
        }

        template <traversal Traversal = default_traversal>
        void _recursive_determine_size()
        {
            this->_M_impl._M_size = 0;
            for (iterator<Traversal> _iter = begin<Traversal>(); _iter != end<Traversal>(); ++_iter)
            {
                ++this->_M_impl._M_size;
            }
        }

    public:

        /*
         * constructors and assignment operators
         */

        flex_tree(const allocator_type& alloc = allocator_type()) : _M_impl(alloc) { }

        flex_tree(std::initializer_list<node_type> ilist) : flex_tree(allocator_type()) 
        { 
            this->_M_impl._M_header->_child_nodes_from_ilist(this->_M_impl, ilist);
            this->_M_impl._M_header->_hook_dangling_child_nodes(); 
            this->_M_impl._M_header->_recursive_update_depth();
            this->_recursive_determine_size(); 
        }
        
        flex_tree& operator=(std::initializer_list<node_type> ilist) 
        { flex_tree temp(ilist); swap(*this, temp); return *this; } /* temp's destructor should clean up potentially allocated resources */

        flex_tree(flex_tree& other) { other._recursive_copy(); }
        flex_tree(flex_tree&& other) : flex_tree() { swap(*this, other); }
        flex_tree& operator=(flex_tree other) { swap(*this, other); return *this; }
        flex_tree& operator=(flex_tree&& other) { swap(*this, other); return *this; }
        ~flex_tree() { clear(); }

        friend void swap(flex_tree& a, flex_tree& b) { std::swap(a._M_impl, b._M_impl); }

        /*
         * iterators
         */

        /**
         * @brief provides iterators to traverse the tree.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @returns iterator to the first valid node of the tree. returns end-iterator if tree is empty.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> begin() { return iterator<Traversal>(this->_M_impl._M_header->_has_children() ? this->_M_impl._M_header->_first_child() : this->_M_impl._M_header); }
        
        /**
         * @brief provides const-iterators to traverse the tree.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @returns const-iterator to the first valid node of the tree. returns end-iterator if tree is empty.
         */
        template <traversal Traversal = default_traversal>
        const_iterator<Traversal> cbegin() const { return const_iterator<Traversal>(this->_M_impl._M_header->_has_children() ? this->_M_impl._M_header->_first_child() : this->_M_impl._M_header); }

        /**
         * @brief provides iterators to traverse the tree.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @returns iterator to the header-node of the tree marking the end-node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> end() { return iterator<Traversal>(this->_M_impl._M_header); }

        /**
         * @brief provides const-iterators to traverse the tree.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @returns const-iterator to the header-node of the tree marking the end-node.
         */
        template <traversal Traversal = default_traversal>
        const_iterator<Traversal> cend() const { return const_iterator<Traversal>(this->_M_impl._M_header); }

        /*
         * modifiers
         */

        /**
         * @brief insert a new child-node at the front of a node's child-node-chain.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param value the value that the node will initially hold.
         * @param where iterator to the new node's parent node.
         * @returns iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> prepend(iterator<Traversal> where, const value_type& value)
        {
            node_type* _new = node_type::node_alloc::_create(this->_M_impl._get_node_alloc(), value);
            _new->_hook_as_first_child(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /**
         * @brief emplace a new child-node at the front of a node's child-node-chain.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param args constructor arguments for the new-nodes value-type.
         * @param where iterator to the new node's parent node.
         * @returns iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> emplace_prepend(iterator<Traversal> where, Args&&... args)
        {
            node_type* _new = node_type::node_alloc::_create(this->_M_impl._get_node_alloc(), std::forward<Args>(args)...);
            _new->_hook_as_first_child(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /**
         * @brief insert a new child-node at the back of a node's child-node-chain.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param value the value that the node will initially hold.
         * @param where iterator to the new node's parent node.
         * @returns iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> append(iterator<Traversal> where, const value_type& value)
        {
            node_type* _new = new node_type(value);
            _new->_hook_as_last_child(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /**
         * @brief emplaces a new child-node at the back of a node's child-node-chain.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param args constructor arguments for the new-nodes value-type.
         * @param where iterator to the new node's parent node.
         * @returns iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> emplace_append(iterator<Traversal> where, Args&&... args)
        {
            node_type* _new = node_type::node_alloc::_create(this->_M_impl._get_node_alloc(), std::forward<Args>(args)...);
            _new->_hook_as_last_child(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /**
         * @brief insert a new node as the next sibling of the another node.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param value the value that the node will initially hold.
         * @param where iterator to the new node's previous sibling.
         * @returns iterator to the newly created node.
         * @throws logic_error if where is an iterator to the root-node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> insert_after(iterator<Traversal> where, const value_type& value)
        {
            if (where == this->begin<Traversal>()) { throw std::logic_error("cannot insert after the root-node"); }
            node_type* _new = node_type::node_alloc::_create(this->_M_impl._get_node_alloc(), value);
            _new->_hook_as_next_sibling(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /**
         * @brief insert a new node as the next sibling of the another node.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param args constructor arguments for the new-nodes value-type.
         * @param where iterator to the new node's previous sibling.
         * @returns iterator to the newly created node.
         * @throws logic_error if where is an iterator to the root-node.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> emplace_after(iterator<Traversal> where, Args&&... args)
        {
            if (where == this->begin<Traversal>()) { throw std::logic_error("cannot insert after the root-node"); }
            node_type* _new = node_type::node_alloc::_create(this->_M_impl._get_node_alloc(), std::forward<Args>(args)...);
            _new->_hook_as_next_sibling(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /**
         * @brief insert a new node as the previous sibling of the another node.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param value the value that the node will initially hold.
         * @param where iterator to the new node's next sibling.
         * @returns iterator to the newly created node.
         * @throws logic_error if where is an iterator to the root-node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> insert_before(iterator<Traversal> where, const value_type& value)
        {
            if (where == this->begin<Traversal>()) { throw std::logic_error("cannot insert before the root-node"); }
            node_type* _new = node_type::node_alloc::_create(this->_M_impl._get_node_alloc(), value);
            _new->_hook_as_prev_sibling(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /**
         * @brief insert a new node as the previous sibling of the another node.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param args constructor arguments for the new-nodes value-type.
         * @param where iterator to the new node's next sibling.
         * @returns iterator to the newly created node.
         * @throws logic_error if where is an iterator to the root-node.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> emplace_before(iterator<Traversal> where, Args&&... args)
        {
            if (where == this->begin<Traversal>()) { throw std::logic_error("cannot insert before the root-node"); }
            node_type* _new = node_type::node_alloc::_create(this->_M_impl._get_node_alloc(), std::forward<Args>(args)...);
            _new->_hook_as_prev_sibling(where._M_ptr);
            ++this->_M_impl._M_size;
            return iterator<Traversal>(_new);
        }

        /** REQUIRES COPYING
         * @brief concatenate all root-nodes of one tree to a given position of another.
         *
         */
        // template <traversal Traversal = default_traversal>
        // iterator<Traversal> concatenate_prepend(iterator<Traversal> where, const flex_tree& other)
        // {

        // }

        // template <traversal Traversal = default_traversal>
        // iterator<Traversal> concatenate_append(iterator<Traversal> where, const flex_tree& other)
        // {

        // }

        // template <traversal Traversal = default_traversal>
        // iterator<Traversal> concatenate_after(iterator<Traversal> where, const flex_tree& other)
        // {
        //     if (where == this->begin()) { throw std::logic_error("cannot insert after the root-node"); }
        // }

        // template <traversal Traversal = default_traversal>
        // iterator<Traversal> concatenate_before(iterator<Traversal> where, const flex_tree& other)
        // {
        //     if (where == this->begin()) { throw std::logic_error("cannot insert before the root-node"); }
        // }

        /**
         * @brief erases a node from the tree.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @param where the node to be erased.
         * @returns iterator to the next valid node in the tree.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> erase(iterator<Traversal> where)
        {
            if (where.is_root()) { return this->end<Traversal>(); }
            
            iterator<Traversal> _next_valid = this->end<Traversal>();
            if (where.has_next()) { _next_valid = where.next(); }
            else { _next_valid = where.parent(); }

            allocator_type alloc(this->_M_impl._get_alloc());
            if (where.has_children()) { this->_M_impl._M_size -= 1 + where._M_ptr->_recursive_erase_children(this->_M_impl._get_node_alloc()); }
            where._M_ptr->_unhook();
            node_type::value_alloc::_discard(alloc, where._M_ptr->_M_value);
            node_type::node_alloc::_discard(this->_M_impl._get_node_alloc(), where._M_ptr);
            return _next_valid;
        }

        /**
         * @brief erases every node in the tree.
         */
        void clear()
        {
            if (this->size())
            { this->_M_impl._M_size -= this->_M_impl._M_header->_recursive_erase_children(this->_M_impl._get_node_alloc()); } // should always return size()
        }

        /**
         * @brief determines the depth of the deepest node in the tree via full iteration.
         * @tparam Traversal the algorithm used to traverse the tree. default is depth-first.
         * @returns the depth of the deepest node in the tree.
         */
        template <traversal Traversal = default_traversal>
        std::size_t maximum_depth() const 
        {
            if (this->_M_impl._M_size)
            {
                std::size_t _current_deepest = 1;
                for (const_iterator<Traversal> _iter = this->cbegin<Traversal>(); _iter != this->cend<Traversal>(); ++_iter)
                {
                    std::size_t _iter_depth = _iter.depth();
                    if (_iter_depth > _current_deepest) { _current_deepest = _iter_depth; }
                }
                return _current_deepest;
            }
            else { return 0ULL; }
        }

        /**
         * @brief get the associated allocator object.
         * @return allocator_type 
         */
        allocator_type get_allocator() const { return this->_M_impl._get_alloc(); }

        /**
         * @return total node-count of the tree.
         */
        std::size_t size() const { return this->_M_impl._M_size; }
    };

    /**
     * behaves just like a flex_tree, except that nodes are not stored as a graph of nodes but rather packed
     * into an array and iteration happens using offsets. this significantly speeds up iteration and reduces node-size,
     * but also significantly raises modification/insertion/erasure costs as the array could require reallocation.
     *
     */
    template <typename Type, typename Alloc = std::allocator<Type>>
        requires (std::is_copy_constructible<Type>::value && std::is_copy_assignable<Type>::value)
    class packed_flex_tree
    {

    };

}

#endif