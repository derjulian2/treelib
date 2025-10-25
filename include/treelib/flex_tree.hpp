/********************************/
#ifndef TRL_FLEX_TREE_HPP
#define TRL_FLEX_TREE_HPP
/********************************/
/**
 * @file    flex_tree.hpp
 * @date    17/10/2025
 * @author  Julian Benzel
 *
 * @brief
 * C++ STL-like implementation of a flexible arbitrary-ary tree-data-structure.
 *
 * @details
 * compile-options:
 * - #define TRL_FLEX_TREE_FAST_DEPTH
 *   includes .depth as a member-field of every node in the tree. provides faster access to depth() value, but also additional bookkeeping.
 * - #define TRL_FLEX_TREE_NO_RECURSION
 *   some algorithms like node-copying or node-erasure use recursion to jump through every child-node. 
 *   this flag will make them use the same depth-first iteration algorithm that flex_tree::iterator<depth_first_post_order> uses.
 * - #define TRL_FLEX_TREE_NOEXCEPT
 *   disables exception-safety for invalid operations on a tree.
 * - #define TRL_FLEX_TREE_ITER_NOEXCEPT
 *   disables exception-safety for iterators specifically. also disabled if TRL_FLEX_TREE_NOEXCEPT is defined.
 * - #define NDEBUG (defined in release-builds)
 *   disables debug-asserts for invalid operations on a tree.
 * - #define TRL_FLEX_TREE_STL_REVERSE_ITER
 *   uses std::reverse_iterator for constructing flex_tree<>::reverse_iterator instead a custom implementation.
 *   for rationale/details see the documentation.
 * 
 * naming-schemes:
 * - 'name__' describes an implementation namespace or type used internally by the implementation.
 * - 'typename_T_' describes a private member typename used internally by the implementation.
 * - 'c_typename_T_' describes a private const member typename used internally by the implementation.
 * - 'variable_M_' or 'method_M_()' describes private member variables or methods used internally by the implementation.
 *
 * TODO:
 * - tidy up iterator-template with friend relations to hide implementation-details.
 * - tidy up iterator-base class because right now it is pretty much template-hell (but working template-hell).
 * - NO_RECURSION algorithms
 * - test a lot probably.
 * - maybe review hook()/unhook() methods?
 * - allocation-methods could maybe throw std::bad_alloc, and are used in noexcept functions. maybe review that.
 */
/********************************/
#include <concepts>
#include <cstddef>
#include <memory>
#include <iterator>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>
#include <cassert>
/********************************/
#ifndef TRL_FLEX_TREE_NOEXCEPT
    #define TRL_NOEXCEPT
#else
    #define TRL_NOEXCEPT noexcept
#endif
/********************************/
#if !defined(TRL_FLEX_TREE_ITER_NOEXCEPT) && !defined (TRL_FLEX_TREE_NOEXCEPT)
    #define TRL_ITER_NOEXCEPT
#else
    #define TRL_ITER_NOEXCEPT noexcept
#endif
/********************************/
#ifndef TRL_FLEX_TREE_DEFAULT_TRAVERSAL
    #define TRL_FLEX_TREE_DEFAULT_TRAVERSAL depth_first_pre_order
#endif
/********************************/
namespace trl
{

    enum traversal
    {
        depth_first_pre_order,
        // depth_first_in_order,
        // depth_first_post_order,

        // depth_first_reverse_pre_order,
        // depth_first_reverse_in_order,
        // depth_first_reverse_post_order,
        
        breadth_first_in_order,
        // breadth_first_reverse_order
    };

    namespace detail__
    {
        struct flex_tree_node_base__
        {
            using base_pointer_T_ = flex_tree_node_base__*;
            using c_base_pointer_T_ = const flex_tree_node_base__*;

            /**
             * @name member-fields of every node_base
             * @{
             */

            /* 
             * cyclic pointers to 'this', so they are always 'valid'.
             * less intuitive, but allows for some cleaner operations later on
             * (e.g. header->first_child should always be the .begin() node).
             */
            base_pointer_T_ parent_M_{this};
            base_pointer_T_ first_child_M_{this};
            base_pointer_T_ last_child_M_{this};
            base_pointer_T_ next_M_{this};
            base_pointer_T_ prev_M_{this};

            std::size_t child_count_M_{0ull};
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            std::size_t depth_count_M_{0ull};
        #endif 

            /**
             * @}
             */

            base_pointer_T_
            find_root_M_()
            {
                base_pointer_T_ res__{this};
                while (!res__->is_root_M_())
                { res__ = res__->parent_M_; }
                return res__;
            }

            c_base_pointer_T_
            find_root_M_() const
            { 
                c_base_pointer_T_ res__{this};
                while (!res__->is_root_M_())
                { res__ = res__->parent_M_; }
                return res__;
            }

            bool 
            is_child_of(c_base_pointer_T_ parent__) const
            {
                c_base_pointer_T_ iter__{this->parent_M_};
                do
                {
                    if (iter__ == parent__)
                    { return true; } 
                    iter__ = iter__->parent_M_; 
                }
                while (!iter__->is_root_M_());
                return false;
            } 

            std::size_t 
            depth_M_() const
            {
            #ifdef TRL_FLEX_TREE_FAST_DEPTH
                return this->depth_count_M_;
            #else
                c_base_pointer_T_ iter__{this};
                std::size_t res__{0ull};
                while (!iter__->is_root_M_()) 
                { iter__ = iter__->parent_M_; ++res__; }
                return res__;
            #endif
            }

            bool 
            is_root_M_() const 
            { return this->parent_M_ == this; }

            bool 
            is_first_child_M_() const /* why not just 'this == this->parent_M_->first_child_M_' ? (would always be false for root) */
            { return !this->has_prev_M_() || this->prev_M_->parent_M_ != this->parent_M_; }
            
            bool 
            is_last_child_M_() const
            { return !this->has_next_M_() || this->next_M_->parent_M_ != this->parent_M_; }
            
            bool 
            has_next_M_() const
            { return this->next_M_ != this;  }

            bool 
            has_prev_M_() const
            { return this->prev_M_ != this; }

            bool 
            has_children_M_() const
            { return this->first_child_M_ != this; }

            bool 
            is_only_child_M_() const 
            { return this->parent_M_->child_count_M_ == 1ull;}

            /*
             * hooking / unhooking - bookkeeping subroutines
             */
  
            void 
            update_new_child_M_(base_pointer_T_ parent__)
            {
                this->parent_M_ = parent__;
                ++parent__->child_count_M_;
            #ifdef TRL_FLEX_TREE_FAST_DEPTH
                this->depth_count_M_ = parent__->depth_count_M_ + 1;
            #endif
            }

            void 
            update_new_first_child_M_(base_pointer_T_ parent__)
            {
                update_new_child_M_(parent__);
                parent__->first_child_M_ = this;
            }

            void 
            update_new_last_child_M_(base_pointer_T_ parent__)
            {
                update_new_child_M_(parent__);
                parent__->last_child_M_ = this;
            }

            void 
            update_new_only_child_M_(base_pointer_T_ parent__)
            {
                update_new_child_M_(parent__);
                parent__->first_child_M_ = parent__->last_child_M_ = this;
            }


            void 
            update_discard_notify_neighbours_M_()
            {
                if (this->has_next_M_() && this->has_prev_M_()) 
                { this->prev_M_->entangle_M_(this->next_M_); }
                else if (this->has_prev_M_()) 
                { this->prev_M_->next_M_ = this->prev_M_; }
                else if (this->has_next_M_()) 
                { this->next_M_->prev_M_ = this->next_M_; }
            }

            void 
            update_discard_first_child_M_()
            {
                this->parent_M_->first_child_M_ = this->next_M_;
                --this->parent_M_->child_count_M_;
            }

            void 
            update_discard_last_child_M_()
            {
                this->parent_M_->last_child_M_ = this->prev_M_;
                --this->parent_M_->child_count_M_;
            }

            void 
            update_discard_only_child_M_()
            {
                this->parent_M_->first_child_M_ = this->parent_M_->last_child_M_ = this->parent_M_;
                --this->parent_M_->child_count_M_;
            }

            /*
             * hooking / unhooking - subroutines
             */

            void 
            entangle_M_(base_pointer_T_ next__)
            {
                this->next_M_ = next__;
                next__->prev_M_ = this;
            }

            void 
            insert_between_M_(base_pointer_T_ prev__, base_pointer_T_ next__)
            {
                prev__->entangle_M_(this);
                this->entangle_M_(next__);
            }

            /*
             * hooking / unhooking
             */

            void 
            hook_as_first_child_M_(base_pointer_T_ parent__) 
            {
                if (parent__->has_children_M_())
                { 
                    if (parent__->first_child_M_->has_prev_M_())
                    { parent__->first_child_M_->prev_M_->entangle_M_(this); }
                    this->entangle_M_(parent__->first_child_M_);
                    this->update_new_first_child_M_(parent__); 
                }
                else
                { this->update_new_only_child_M_(parent__); }
            }

            void 
            hook_as_last_child_M_(base_pointer_T_ parent__)
            {  
                if (parent__->has_children_M_())
                { 
                    if (parent__->last_child_M_->has_next_M_())
                    { this->entangle_M_(parent__->last_child_M_->next_M_); }
                    parent__->last_child_M_->entangle_M_(this); 
                    this->update_new_last_child_M_(parent__); 
                }
                else
                { this->update_new_only_child_M_(parent__); }
            }

            void 
            hook_as_next_sibling_M_(base_pointer_T_ prev__)
            {
                if (prev__->is_last_child_M_()) 
                { this->hook_as_last_child_M_(prev__->parent_M_); }
                else
                {
                    this->insert_between_M_(prev__, prev__->next_M_);
                    this->update_new_child_M_(prev__->parent_M_);
                }
            }

            void 
            hook_as_prev_sibling_M_(base_pointer_T_ next__)
            {
                if (next__->is_first_child_M_())
                { this->hook_as_first_child_M_(next__->parent_M_); }
                else
                {
                    this->insert_between_M_(next__->prev_M_, next__);
                    this->update_new_child_M_(next__->parent_M_);
                }
            }

            void 
            unhook_as_first_child_M_()
            {
                this->update_discard_first_child_M_();
                this->update_discard_notify_neighbours_M_();
            }

            void 
            unhook_as_last_child_M_()
            {
                this->update_discard_last_child_M_();
                this->update_discard_notify_neighbours_M_();
            }

            void 
            unhook_as_regular_child_M_()
            {
                --this->parent_M_->child_count_M_;
                this->update_discard_notify_neighbours_M_();
            }

            void 
            unhook_as_only_child_M_()
            {
                this->update_discard_only_child_M_();
                this->update_discard_notify_neighbours_M_();
            }

            void 
            unhook_M_()
            {
                if (this->is_only_child_M_()) // has no siblings/cousins
                { this->unhook_as_only_child_M_(); }
                else if (this->has_prev_M_() && this->has_next_M_()) // has both
                { this->unhook_as_regular_child_M_(); }
                else if (this->has_prev_M_()) // has only prev
                { this->unhook_as_last_child_M_(); }
                else if (this->has_next_M_()) // has only next
                { this->unhook_as_first_child_M_(); }
            }

        };

        /**
         * @brief
         * an actual node in the tree.
         */
        template <typename ValTp__>
        struct flex_tree_node__ 
            : public flex_tree_node_base__
        {
            ValTp__ value_M_;
            
            template <typename... Args>
            flex_tree_node__(Args&&... args__) : value_M_(args__...) { }
        };

        /**
         * @brief
         * wrapper for a flex_tree_node__ and a total subnode-count (children and it's children and so on).
         *
         * @details
         * because of the recursive initialization using std::initializer_list, the nodes cannot be allocated using the
         * allocator of the tree, therefore this is only viable for allocators where every instance is interchangeable or at least default-constructible.
         * 
         * this wrapper doesn't clean up after it's resources because ownership is transferred to an associated tree that instantiated this wrapper.
         *
         */
        template <typename Alloc__>
            requires std::default_initializable<Alloc__>
        struct flex_tree_node_initializer__
        {
            using node_T_ = flex_tree_node__<typename Alloc__::value_type>;
            using node_ptr_T_ = node_T_*;
            using node_alloc_T_ = std::allocator_traits<Alloc__>::template rebind_alloc<node_T_>;

            node_ptr_T_ ptr_M_{nullptr};
            std::size_t total_child_node_count_M_{0ull};

            /* non-copyable */
            flex_tree_node_initializer__(const flex_tree_node_initializer__&) = delete;

            template <typename... Args>
            flex_tree_node_initializer__(Args&&... args__) 
            {
                node_alloc_T_ alloc__;
                this->ptr_M_ = std::allocator_traits<node_alloc_T_>::allocate(alloc__, 1);
                std::allocator_traits<node_alloc_T_>::construct(alloc__, this->ptr_M_, std::forward<Args>(args__)...);
            }

            template <typename Arg>
            flex_tree_node_initializer__(Arg&& arg__, std::initializer_list<flex_tree_node_initializer__> ilist__)
                : total_child_node_count_M_(ilist__.size())
            {
                node_alloc_T_ alloc__;
                this->ptr_M_ = std::allocator_traits<node_alloc_T_>::allocate(alloc__, 1);
                std::allocator_traits<node_alloc_T_>::construct(alloc__, this->ptr_M_, std::forward<Arg>(arg__));
                
                for (const flex_tree_node_initializer__& init__ : ilist__)
                {
                    init__.ptr_M_->hook_as_last_child_M_(this->ptr_M_);
                    total_child_node_count_M_ += init__.total_child_node_count_M_;
                }
            }
        };

        /**
         * @brief
         * top-most dummy node of any flex_tree. 
         * @details
         * does not contain an instance of the value_type of the tree,
         * but only traversal-pointers to hold the top-layer child-nodes.
         */
        struct flex_tree_header_node__ 
            : public flex_tree_node_base__
        {
            std::size_t size_M_{0ull};

            flex_tree_header_node__() = default;

            template <typename Alloc__>
            void 
            from_initializer_list_M_(std::initializer_list<flex_tree_node_initializer__<Alloc__>> ilist__)
            {
                this->size_M_ = ilist__.size();
                for (const flex_tree_node_initializer__<Alloc__>& init__ : ilist__)
                {
                    init__.ptr_M_->hook_as_last_child_M_(this);
                    this->size_M_ += init__.total_child_node_count_M_;
                }
            }
        };

        template <traversal Trav__, typename ValTp__, bool Const__>
        struct flex_tree_iterator_base__
        { 
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = typename std::conditional<Const__, const ValTp__, ValTp__>::type;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;
            
            using node_ptr_T_ = typename std::conditional<Const__, const flex_tree_node__<ValTp__>*, flex_tree_node__<ValTp__>*>::type;
            using base_ptr_T_ = typename std::conditional<Const__, const flex_tree_node_base__*, flex_tree_node_base__*>::type;

            base_ptr_T_ ptr_M_{nullptr};

            flex_tree_iterator_base__() = default;

            explicit flex_tree_iterator_base__(base_ptr_T_ ptr__) noexcept
                : ptr_M_(ptr__)
            { }

            template <traversal OTrav__>
                requires Const__
            flex_tree_iterator_base__(const flex_tree_iterator_base__<OTrav__, ValTp__, false>& other) noexcept
                : ptr_M_(other.ptr_M_)
            { }

            template <traversal OTrav__>
            flex_tree_iterator_base__(const flex_tree_iterator_base__<OTrav__, ValTp__, Const__>& other) noexcept
                : ptr_M_(other.ptr_M_)
            { }

            operator base_ptr_T_() noexcept
            { return ptr_M_; }

            base_ptr_T_
            node_ptr_M_() const noexcept
            { return ptr_M_; }

            [[nodiscard]]
            reference 
            operator*() const TRL_ITER_NOEXCEPT
            {
            #if !defined(TRL_FLEX_TREE_NOEXCEPT) && !defined(TRL_FLEX_TREE_ITER_NOEXCEPT)
                if (this->ptr_M_->is_root_M_()) 
                { throw std::logic_error("cannot dereference end()-iterator"); }
            #else
                assert(!this->ptr_M_->is_root_M_()); /* downcast will cause UB on end()-node. check only in debug. */
            #endif
                return static_cast<node_ptr_T_>(this->ptr_M_)->value_M_; 
            }
            
            [[nodiscard]]
            pointer 
            operator->() const TRL_ITER_NOEXCEPT
            { 
            #if !defined(TRL_FLEX_TREE_NOEXCEPT) && !defined(TRL_FLEX_TREE_ITER_NOEXCEPT)
                if (this->ptr_M_->is_root_M_()) { throw std::logic_error("cannot dereference end()-iterator"); }
            #else
                assert(!this->ptr_M_->is_root_M_()); /* downcast will cause UB on end()-node. check only in debug. */
            #endif
                return std::addressof(static_cast<node_ptr_T_>(this->ptr_M_)->value_M_); 
            }

            /* iterators should be comparable even if they differ in algorithm or constness */
            template <traversal OTrav__, bool OConst__>
            friend bool 
            operator==(const flex_tree_iterator_base__<Trav__, ValTp__, Const__>& i, 
                       const flex_tree_iterator_base__<OTrav__, ValTp__, OConst__>& j) noexcept
            { return i.ptr_M_ == j.ptr_M_; }
           
            template <traversal OTrav__, bool OConst__>
            friend bool 
            operator!=(const flex_tree_iterator_base__<Trav__, ValTp__, Const__>& i, 
                       const flex_tree_iterator_base__<OTrav__, ValTp__, OConst__>& j) noexcept
            { return !(i == j); } 

        };

        /**
         * @brief an iterator to a flex_tree.
         * @tparam Traversal the algorithm used to traverse the tree.
         */
        template <traversal Trav__, typename ValTp__, bool Const__>
        struct flex_tree_iterator__; /* primary template. not to be instantiated. */

        template <typename ValTp__, bool Const__>
        struct flex_tree_iterator__<depth_first_pre_order, ValTp__, Const__>
            : public flex_tree_iterator_base__<depth_first_pre_order, ValTp__, Const__>
        {
            using self_T_ = flex_tree_iterator__<depth_first_pre_order, ValTp__, Const__>;
            using base_T_ = flex_tree_iterator_base__<depth_first_pre_order, ValTp__, Const__>;
            using base_T_::base_T_; /* use constructors of base-class */

            self_T_& 
            operator++() noexcept
            { 
                if (this->ptr_M_->has_children_M_()) 
                { this->ptr_M_ = this->ptr_M_->first_child_M_; return *this; }
                while (this->ptr_M_->is_last_child_M_() && !this->ptr_M_->is_root_M_()) 
                { this->ptr_M_ = this->ptr_M_->parent_M_;  }
                this->ptr_M_ = this->ptr_M_->next_M_;
                return *this;
            }

            self_T_&
            operator--() noexcept
            { 
                if (this->ptr_M_->is_first_child_M_() && !this->ptr_M_->is_root_M_())
                { this->ptr_M_ = this->ptr_M_->parent_M_; return *this; }
                this->ptr_M_ = this->ptr_M_->prev_M_;
                while (this->ptr_M_->has_children_M_())
                { this->ptr_M_ = this->ptr_M_->last_child_M_; }
                return *this;
            }

            self_T_
            operator++(int) noexcept
            { self_T_ old{*this}; ++(*this); return old; }

            self_T_
            operator--(int) noexcept
            { self_T_ old{*this}; --(*this); return old; }

        };

        template <typename ValTp__, bool Const__>
        struct flex_tree_iterator__<breadth_first_in_order, ValTp__, Const__>
            : public flex_tree_iterator_base__<breadth_first_in_order, ValTp__, Const__>
        {
            using self_T_ = flex_tree_iterator__<breadth_first_in_order, ValTp__, Const__>;
            using base_T_ = flex_tree_iterator_base__<breadth_first_in_order, ValTp__, Const__>;
            using base_T_::base_T_; /* use constructors of base-class */

            bool direction_M_{false};

            self_T_& 
            operator++() noexcept
            { 
                if (this->ptr_M_->depth_M_() % 2)
                {
                    if (this->ptr_M_->has_next_M_())
                    { this->ptr_M_ = this->ptr_M_->next_M_; return *this; }
                    while (!this->ptr_M_->has_children_M_())
                    {
                        if (!this->ptr_M_->has_prev_M_()) 
                        { this->ptr_M_ = this->ptr_M_->find_root_M_(); return *this; }
                        this->ptr_M_ = this->ptr_M_->prev_M_; 
                    }
                    this->ptr_M_ = this->ptr_M_->last_child_M_;
                }
                else
                {
                    if (this->ptr_M_->has_prev_M_())
                    { this->ptr_M_ = this->ptr_M_->prev_M_; return *this; }
                    while (!this->ptr_M_->has_children_M_())
                    {
                        if (!this->ptr_M_->has_next_M_()) 
                        { this->ptr_M_ = this->ptr_M_->find_root_M_(); return *this; }
                        this->ptr_M_ = this->ptr_M_->next_M_; 
                    }
                    this->ptr_M_ = this->ptr_M_->first_child_M_;
                }
                return *this;
            }

            self_T_&
            operator--() noexcept
            {   
                if (this->ptr_M_->is_root_M_())
                {
                    /* 
                     * extremely slow if operator-- is called on root, because the last
                     * node in the deepest layer needs to be found. maybe there
                     * is a smarter way to do this.
                     *
                     * maybe just don't implement this at all? as with depth-first?
                     */
                    auto iter = this->ptr_M_;
                    ++(*this);
                    while (!this->ptr_M_->is_root_M_())
                    { iter = this->ptr_M_; ++(*this); }
                    this->ptr_M_ = iter;
                    return *this;
                }              
                if (this->ptr_M_->depth_M_() % 2)
                {
                    if (this->ptr_M_->has_prev_M_())
                    { this->ptr_M_ = this->ptr_M_->prev_M_; return *this; }
                    this->ptr_M_ = this->ptr_M_->parent_M_;
                    while (this->ptr_M_->has_next_M_())
                    { this->ptr_M_ = this->ptr_M_->next_M_; }
                }
                else
                {
                    if (this->ptr_M_->has_next_M_())
                    { this->ptr_M_ = this->ptr_M_->next_M_; return *this; }
                    this->ptr_M_ = this->ptr_M_->parent_M_;
                    while (this->ptr_M_->has_prev_M_())
                    { this->ptr_M_ = this->ptr_M_->prev_M_; }

                }
                return *this;
            }

            self_T_
            operator++(int) noexcept
            { self_T_ old{*this}; ++(*this); return old; }

            self_T_
            operator--(int) noexcept
            { self_T_ old{*this}; --(*this); return old; }
        };

        /*
         * custom reverse-iterator adaptor for flex_tree::iterator.
         * for STL std::reverse_iterator-compliance, use TRL_FLEX_TREE_STL_REVERSE_ITER.
         *
         * the rationale behind this custom adaptor is the way std::reverse_iterator handles dereferences
         * and works internally. std::reverse_iterator(tree.end()) would hold an instance of an end()-iterator
         * internally, but on dereference, it would return *(end() - 1), which in my case is undesired behaviour,
         * as this dereference requires invoking the iteration-algorithm each time, potentially causing large overhead.
         */
        template <typename IterTp__>
        struct flex_tree_reverse_iterator__
        {
            using base_type = IterTp__;
            using iterator_category = typename base_type::iterator_category;
            using value_type = typename base_type::value_type;
            using difference_type = typename base_type::difference_type;
            using pointer = typename base_type::pointer;
            using reference = typename base_type::reference;

            using self_T_ = flex_tree_reverse_iterator__;
            using self_ref_T_ = self_T_&;
            using c_self_ref_T_ = const self_T_&;

            base_type instance_M_;

            explicit flex_tree_reverse_iterator__(const base_type& iter__)
                : instance_M_(iter__)
            { }

            base_type& 
            base()
            { this->instance_M_; }

            const base_type& 
            base() const
            { return this->instance_M_; }

            self_ref_T_
            operator++() noexcept
            { --this->instance_M_; return *this; }

            self_T_
            operator++(int) noexcept
            { this->instance_M_--; return *this; }

            self_ref_T_
            operator--() noexcept
            { ++this->instance_M_; return *this; }

            self_T_
            operator--(int) noexcept
            { this->instance_M_++; return *this; }

            [[nodiscard]]
            reference 
            operator*() const TRL_ITER_NOEXCEPT
            { return *this->instance_M_; }
            
            [[nodiscard]]
            pointer 
            operator->() const TRL_ITER_NOEXCEPT
            { return std::addressof(*this->instance_M_); }

            friend bool
            operator==(c_self_ref_T_ a, c_self_ref_T_ b)
            { return a.instance_M_ == b.instance_M_; }

            friend bool
            operator!=(c_self_ref_T_ a, c_self_ref_T_ b)
            { return a.instance_M_ != b.instance_M_; }

        };

        /**
         * contains all allocation/deallocation logic for nodes in the flex-tree 
         * (with the exception of the node-initializer, see that for more info).
         */
        template <typename ValTp__, typename Alloc__>
        struct flex_tree_base__
        {
            using alloc_T_ = Alloc__;
            using node_T_ = flex_tree_node__<ValTp__>;
            using base_ptr_T_ = flex_tree_node_base__*;
            using c_base_ptr_T_ = const flex_tree_node_base__*;
            using node_ptr_T_ = node_T_*;
            using c_node_ptr_T_ = const node_T_*;
            using node_alloc_T_ = typename std::allocator_traits<alloc_T_>::template rebind_alloc<node_T_>;

            struct flex_tree_impl__
                : public node_alloc_T_
            {
                flex_tree_header_node__ header_M_;

                template <typename... Args__>
                node_ptr_T_
                get_node_M_(Args__&&... args__)
                {
                    node_ptr_T_ new_ = std::allocator_traits<node_alloc_T_>::allocate(this->get_node_alloc_M_(), 1);
                    std::allocator_traits<node_alloc_T_>::construct(this->get_node_alloc_M_(), new_, std::forward<Args__>(args__)...);
                    return new_;
                }

                void
                put_node_M_(node_ptr_T_ node__)
                {
                    std::allocator_traits<node_alloc_T_>::destroy(this->get_node_alloc_M_(), node__);
                    std::allocator_traits<node_alloc_T_>::deallocate(this->get_node_alloc_M_(), node__, 1);
                }

                node_alloc_T_& 
                get_node_alloc_M_()
                { return *this; }
                
                const node_alloc_T_& 
                get_node_alloc_M_() const 
                { return *this; }

                alloc_T_ 
                get_alloc_M_() const 
                { return alloc_T_(*this); }

                flex_tree_impl__(const node_alloc_T_& node_alloc_)
                    : node_alloc_T_(node_alloc_) 
                { }
            };

        #ifdef TRL_FLEX_TREE_FAST_DEPTH

            /**
             * recursively update depth-member variable of node__ and all of it's descendants.
             * maybe review semantics of this method to only change depth of child-nodes ?
             */
            std::size_t
            update_depth_M_(base_ptr_T_ node__, std::size_t depth__)
            {
                assert(node__->has_children_M_());

                node__->depth_count_M_ = depth__;
                base_ptr_T_ iter__ = node__->first_child_M_;
                std::size_t nodes_affected__{0ull};
            #ifdef TRL_FLEX_TREE_NO_RECURSION

            #else
                while (true)
                {
                    if (iter__->has_children_M_())
                    { nodes_affected__ += update_depth_M_(iter__, depth__ + 1); }
                    else
                    { iter__->depth_count_M_ = depth__ + 1; }
                    ++nodes_affected__;
                    if (!iter__->has_next_M_())
                    { break; }
                    iter__ = iter__->next_M_;
                }
            #endif
                return nodes_affected__;
            }

        #endif

            /**
             * OK to be called on any value-node or on the header as the header will never be a child-node of any other node.
             * expects node__ to definitely have child-nodes.
             */        
            std::size_t
            copy_children_M_(base_ptr_T_ new_parent__, c_base_ptr_T_ node__)
            {
                assert(node__->has_children_M_());

                c_base_ptr_T_ iter__{node__->first_child_M_}; /* possibly iter__ == node__*/
                std::size_t nodes_affected__{0ull};

            #ifdef TRL_FLEX_TREE_NO_RECURSION
                _node* _copy_iter{_this_copy};
                while (_iter != (this->_has_next() ? this->_M_next : this))
                {
                    // make copy and hook it to _copy_iter
                    _flex_tree_node* _copy = _new(_alloc, _iter->_M_value);
                    _copy->_hook_as_last_child(_copy_iter);
                    ++_nodes_affected;
                    // depth-first-post-order, but for two iterator-pointers:
                    if (_iter->_has_children()) { _iter = _iter->_M_first_child; _copy_iter = _copy_iter->_M_first_child; }
                    while (_iter->_is_last_child()) { _iter = _iter->_M_parent; _copy_iter = _copy_iter->_M_parent; }
                    if (!_iter->_is_root()) { _iter = _iter->_M_next; _copy_iter = _copy_iter->_M_next; }
                }
            #else
                while (true)
                {
                    node_ptr_T_ copy__ = this->impl_M_.get_node_M_(static_cast<c_node_ptr_T_>(iter__)->value_M_);
                    copy__->hook_as_last_child_M_(new_parent__);

                    if (iter__->has_children_M_())
                    { nodes_affected__ += copy_children_M_(copy__, iter__); }

                    ++nodes_affected__;

                    if (!iter__->has_next_M_())
                    { break; }

                    iter__ = iter__->next_M_;
                }
            #endif
                return nodes_affected__;
            }

            /**
             * OK to be called on any value-node or on the header as the header will never be a child-node of any other node.
             * expects node__ to definitely have child-nodes.
             */
            std::size_t 
            erase_children_M_(base_ptr_T_ node__)
            {
                assert(node__->has_children_M_());

                base_ptr_T_ iter__{node__->first_child_M_};
                std::size_t nodes_affected__{0ull};

            #ifdef TRL_FLEX_TREE_NO_RECURSION
                while (_iter != (this->_has_next() ? this->_M_next : this))
                {
                    _node* _iter_next{_next<depth_first_pre_order>(_iter)};
                    _iter->_unhook();
                    _delete(_alloc, _iter);
                    ++_nodes_affected;
                    _iter = _iter_next;
                }
            #else
                while (true)
                { 
                    if (iter__->has_children_M_())
                    { nodes_affected__ += erase_children_M_(iter__); }

                    base_ptr_T_ iter_next__{iter__->next_M_}; // save next node before deletion
                    iter__->unhook_M_();
                    this->impl_M_.put_node_M_(static_cast<node_ptr_T_>(iter__)); /* static_cast should not fail as it is never called on root */
                    ++nodes_affected__;

                    if (!iter__->has_next_M_())
                    { break; }

                    iter__ = iter_next__;
                }
            #endif
                return nodes_affected__;
            }

            flex_tree_base__(const alloc_T_& alloc__)
                : impl_M_(alloc__)
            { }

            flex_tree_impl__ impl_M_;
        };
    }

    /**
     * @brief
     * iterator-adaptor to iterate through the child-nodes of a specific tree-node, essentially
     * defining `[first_child, parent)` as a subrange in the tree.
     *
     * this template is to be used with `trl::node_traits::lbegin()`/`lend()` 
     * as in leaf-begin and leaf-end that define the bounds of the range.
     */
    template <typename IteratorType>
    struct leaf_iterator
    {
        using base_type = IteratorType;
        using iterator_category = typename base_type::iterator_category;
        using value_type = typename base_type::value_type;
        using difference_type = typename base_type::difference_type;
        using pointer = typename base_type::pointer;
        using reference = typename base_type::reference;

        base_type instance_M_;

        explicit leaf_iterator(const base_type& iter)
            : instance_M_(iter)
        { }

        base_type& 
        base()
        { return static_cast<base_type&>(*this); }

        const base_type& 
        base() const
        { return static_cast<const base_type&>(*this); }

        leaf_iterator& 
        operator++()
        { 
            this->instance_M_.ptr_M_ = 
            this->instance_M_.ptr_M_->is_last_child_M_() ? 
            this->instance_M_.ptr_M_->parent_M_ : this->instance_M_.ptr_M_->next_M_; 
            return *this; 
        }

        leaf_iterator 
        operator++(int)
        { leaf_iterator old{*this}; ++(*this); return old; }

        leaf_iterator& 
        operator--()
        { 
            this->instance_M_.ptr_M_ = 
            this->instance_M_.ptr_M_->is_first_child_M_() ? 
            this->instance_M_.ptr_M_->parent_M_ : this->instance_M_.ptr_M_->prev_M_; 
            return *this; 
        }

        leaf_iterator 
        operator--(int)
        { leaf_iterator old{*this}; --(*this); return old; }

        [[nodiscard]]
        reference 
        operator*() const TRL_ITER_NOEXCEPT
        { return *this->instance_M_; }
        
        [[nodiscard]]
        pointer 
        operator->() const TRL_ITER_NOEXCEPT
        { return std::addressof(*this->instance_M_); }

        friend bool
        operator==(const leaf_iterator& a, const leaf_iterator& b)
        { return a.instance_M_ == b.instance_M_; }

        friend bool
        operator!=(const leaf_iterator& a, const leaf_iterator& b)
        { return a.instance_M_ != b.instance_M_; }

    };

    /**
     * @brief
     * provides (optionally exception-safe) information about a node's placement in a tree.
     */
    struct node_traits
    {

        template <typename IteratorType>
        static IteratorType
        parent(IteratorType iter) TRL_NOEXCEPT
        {
            auto ptr__ = iter.ptr_M_;
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (ptr__->is_root_M_()) { throw std::logic_error("root-node cannot have a parent-node"); }
        #else
            assert(!ptr__->is_root_M_()); 
        #endif
            return IteratorType(ptr__);
        }

        template <typename IteratorType>
        static IteratorType
        next(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_next_M_()) { throw std::logic_error("node does not have a next node"); }
        #else
            assert(ptr__->has_next_M_()); 
        #endif
            return IteratorType(ptr__->next_M_);
        }

        template <typename IteratorType>
        static IteratorType
        previous(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_prev_M_()) { throw std::logic_error("node does not have a previous node"); }
        #else
            assert(ptr__->has_prev_M_()); 
        #endif
            return IteratorType(ptr__->prev_M_);
        }

        template <typename IteratorType>
        static IteratorType
        first_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_children_M_()) { throw std::logic_error("node does not have any child-nodes"); }
        #else
            assert(ptr__->has_children_M_()); 
        #endif
            return IteratorType(ptr__->first_child_M_);
        }

        template <typename IteratorType>
        static IteratorType
        last_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (!ptr__->has_children_M_()) { throw std::logic_error("node does not have any child-nodes"); }
        #else
            assert(ptr__->has_children_M_()); 
        #endif
            return IteratorType(ptr__->last_child_M_);
        }

        template <typename IteratorType>
        static std::size_t 
        depth(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->depth_M_(); 
        }

        template <typename IteratorType>
        static std::size_t 
        child_count(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->child_count_M_; 
        }

        template <typename IteratorType>
        static bool 
        is_root(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->is_root_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_first_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->is_first_child_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_last_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->is_last_child_M_();
        }

        template <typename IteratorType>
        static bool 
        has_next(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->has_next_M_();
        }

        template <typename IteratorType>
        static bool 
        has_previous(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->has_prev_M_();
        }

        template <typename IteratorType>
        static bool 
        has_children(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
            return ptr__->has_children_M_(); 
        }

        template <typename IteratorType>
        static bool 
        is_only_child(IteratorType iter) TRL_NOEXCEPT
        { 
            auto ptr__ = iter.ptr_M_;
        #ifndef TRL_NODE_TRAITS_NOEXCEPT
            if (ptr__->is_root_M_()) { throw std::logic_error("root-node cannot be an only-child"); }
        #else
            assert(!ptr__->is_root_M_());
        #endif
            return ptr__->is_only_child_M_(); 
        }

        template <typename IteratorType>
        static leaf_iterator<IteratorType>
        lbegin(IteratorType iter) TRL_NOEXCEPT
        { return leaf_iterator<IteratorType>(first_child(iter)); }

        template <typename IteratorType>
        static leaf_iterator<IteratorType>
        lend(IteratorType iter) TRL_NOEXCEPT
        { return leaf_iterator<IteratorType>(iter); }

    };

    /**
     * @brief C++ STL-like implementation of a flexible arbitrary-ary tree-data-structure.
     * @tparam Type the type that every node should contain.
     * @tparam Allocator an allocator type.
     */
    template <typename Type, typename Allocator = std::allocator<Type>>
    class flex_tree
        : protected detail__::flex_tree_base__<Type, Allocator>
    {
    public:

        static constexpr traversal default_traversal = TRL_FLEX_TREE_DEFAULT_TRAVERSAL;

        using value_type = Type;
        using allocator_type = Allocator;
        using initializer_type = detail__::flex_tree_node_initializer__<Allocator>;
        

        template <traversal Traversal = default_traversal>
        using iterator = detail__::flex_tree_iterator__<Traversal, value_type, false>;

        template <traversal Traversal = default_traversal>
        using const_iterator = detail__::flex_tree_iterator__<Traversal, value_type, true>;
    
    #ifndef TRL_FLEX_TREE_STL_REVERSE_ITER
        template <traversal Traversal = default_traversal>
        using reverse_iterator = detail__::flex_tree_reverse_iterator__<iterator<Traversal>>;

        template <traversal Traversal = default_traversal>
        using const_reverse_iterator = detail__::flex_tree_reverse_iterator__<const_iterator<Traversal>>;
    #else
        template <traversal Traversal = default_traversal>
        using reverse_iterator = std::reverse_iterator<iterator<Traversal>>;

        template <traversal Traversal = default_traversal>
        using const_reverse_iterator = std::reverse_iterator<const_iterator<Traversal>>;
    #endif
        
    protected:

        using node_T_ = detail__::flex_tree_node__<value_type>;
        using node_ptr_T_ = node_T_*;
        using base_ptr_T_ = detail__::flex_tree_node_base__*;
        using node_initializer_T_ = detail__::flex_tree_node_initializer__<allocator_type>;
        using node_alloc_T_ = typename std::allocator_traits<allocator_type>::template rebind_alloc<node_T_>;

    public:

        /**
         * @name constructors and special member functions
         * @{
         */

        /**
         * @brief 
         * initializes the tree using a recursively constructed initializer-list.
         * @details
         * this constructor is only supported for default-constructible allocator-types such as std::allocator.
         * because the initializer-list constructs the nodes recursively the tree is basically built up from the inside and before this constructor is called.
         * this requires node-allocation and i found no way to do this using the allocator contained in the tree itself (as it doesn't exist at the time the inner nodes are constructed).
         * @param ilist the initializer-list containing the values and node-hierarchy. see examples for a reference-usage.
         */
        flex_tree(std::initializer_list<node_initializer_T_> ilist, const allocator_type& allocator = allocator_type()) noexcept
            : flex_tree(allocator)
        {
            this->impl_M_.header_M_.from_initializer_list_M_(ilist);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(&this->impl_M_.header_M_, 0);
        #endif
        }

        /**
         * @brief initializer-list assignment.
         */
        flex_tree& 
        operator=(std::initializer_list<node_initializer_T_> ilist) noexcept 
        { 
            this->clear();
            this->impl_M_.header_M_.from_initializer_list_M_(ilist);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(&this->impl_M_.header_M_, 0);
        #endif
        }

        /**
         * @brief subtree constructor from an iterator.
         * @param where an iterator to the node to be copied with all of it's descendants. 
         * @tparam Traversal the algorithm used to traverse the tree.
         * @note exceptions are thrown / the behaviour is undefined if:
         * - `where` is an `end()`-iterator.
         */
        template <traversal Traversal>
        explicit flex_tree(const_iterator<Traversal> where, const allocator_type& allocator = allocator_type()) TRL_NOEXCEPT
            : flex_tree(allocator)
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(*where);
            if (where.node_ptr_M_()->has_children_M_())
            { this->copy_children_M_(new__, where); }
            new__->hook_as_last_child_M_(&this->impl_M_.header_M_);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(&this->impl_M_.header_M_, 0);
        #endif
        }

        /**
         * @brief subtree assignment.
         */
        template <traversal Traversal>
        flex_tree& operator=(const_iterator<Traversal> where) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
        #endif
            this->clear();
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(*where);
            if (where.node_ptr_M_()->has_children_M_())
            { this->copy_children_M_(new__, where); }
            new__->hook_as_last_child_M_(&this->impl_M_.header_M_);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(&this->impl_M_.header_M_, 0);
        #endif
        }

        /**
         * @brief default constructor.
         */
        flex_tree(const allocator_type& allocator = allocator_type()) noexcept 
            : detail__::flex_tree_base__<Type, Allocator>(allocator)
        { }
        
        /**
         * @brief destructor. clears up remaining nodes.
         */
        ~flex_tree() noexcept
        { this->clear(); }

        /**
         * @brief copy constructor.
         */
        flex_tree(const flex_tree& other) noexcept
            : flex_tree(other.get_allocator())
        { 
            if (other.impl_M_.header_M_.has_children_M_())
            { this->copy_children_M_(&this->impl_M_.header_M_, &other.impl_M_.header_M_); }
        }

        /**
         * @brief move constructor.
         */
        flex_tree(flex_tree&& other) noexcept
            : flex_tree()
        { swap(*this, other); }

        /**
         * @brief copy assignment using copy-swap-idiom.
         */
        flex_tree& 
        operator=(flex_tree other) noexcept
        { this->clear(); swap(*this, other); return *this; }

        /**
         * @brief move assignment.
         */
        flex_tree& 
        operator=(flex_tree&& other) noexcept 
        { swap(*this, other); return *this; }

        /**
         * @brief std::swap specialization.
         */
        friend void 
        swap(flex_tree& a, flex_tree& b) noexcept
        { std::swap(a.impl_M_, b.impl_M_); }

        /**
         * @}
         */

        /**
         * @name iteration
         * @{
         */

        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the first-child-node of the root, or end() if the tree is empty.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        begin() noexcept
        { return iterator<Traversal>(this->impl_M_.header_M_.first_child_M_); }
        
        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return a const-iterator to the first-child-node of the root, or end() if the tree is empty.
         */
        template <traversal Traversal = default_traversal>
        const_iterator<Traversal> 
        cbegin() const noexcept
        { return const_iterator<Traversal>(this->impl_M_.header_M_.first_child_M_); }

        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the true root-node of the tree, acting as a valueless sentinel-node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        end() noexcept
        { return iterator<Traversal>(&this->impl_M_.header_M_); }

        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return a const-iterator to the true root-node of the tree, acting as a valueless sentinel-node.
         */
        template <traversal Traversal = default_traversal>
        const_iterator<Traversal> 
        cend() const noexcept
        { return const_iterator<Traversal>(&this->impl_M_.header_M_); }

        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return a reverse-iterator to the first-child-node of the root, or end() if the tree is empty.
         */
        template <traversal Traversal = default_traversal>
        reverse_iterator<Traversal> 
        rbegin() noexcept
    #ifdef TRL_FLEX_TREE_STL_REVERSE_ITER
        { return reverse_iterator<Traversal>(this->end<Traversal>()); }
    #else
        { return reverse_iterator<Traversal>(--this->end<Traversal>()); }
    #endif
        
        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return a const-reverse-iterator to the first-child-node of the root, or end() if the tree is empty.
         */
        template <traversal Traversal = default_traversal>
        const_reverse_iterator<Traversal> 
        crbegin() const noexcept
    #ifdef TRL_FLEX_TREE_STL_REVERSE_ITER
        { return reverse_iterator<Traversal>(this->end<Traversal>()); }
    #else
        { return const_reverse_iterator<Traversal>(--this->cend<Traversal>()); }
    #endif

        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return a reverse-iterator to the true root-node of the tree, acting as a valueless sentinel-node.
         */
        template <traversal Traversal = default_traversal>
        reverse_iterator<Traversal> 
        rend() noexcept
    #ifdef TRL_FLEX_TREE_STL_REVERSE_ITER
        { return reverse_iterator<Traversal>(this->begin<Traversal>()); }
    #else
        { return reverse_iterator<Traversal>(this->end<Traversal>()); }
    #endif

        /**
         * @tparam Traversal the algorithm used to traverse the tree.
         * @returns a const-reverse-iterator to the true root-node of the tree, acting as a valueless sentinel-node.
         */
        template <traversal Traversal = default_traversal>
        const_reverse_iterator<Traversal> 
        crend() const noexcept
    #ifdef TRL_FLEX_TREE_STL_REVERSE_ITER
        { return const_reverse_iterator<Traversal>(this->cbegin<Traversal>()); }
    #else
        { return const_reverse_iterator<Traversal>(this->cend<Traversal>()); }
    #endif

        /**
         * @}
         */

        /** 
         * @name single-node modifiers
         * @{
         */

        /**
         * @brief insert a new child-node as `where`'s first-child.
         * @param where an iterator to the new node's parent node.
         * @param value the value that the node will initially hold.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        prepend(iterator<Traversal> where, const value_type& value) noexcept
        {
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(value);
            new__->hook_as_first_child_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }
    
        /**
         * @brief emplace a new child-node as `where`'s first-child.
         * @param where an iterator to the new node's parent node.
         * @param args constructor arguments that are forwarded into the value of the new node.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> 
        emplace_prepend(iterator<Traversal> where, Args&&... args) noexcept
        {
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(std::forward<Args>(args)...);
            new__->hook_as_first_child_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }
        
        /**
         * @brief insert a new child-node as `where`'s last-child.
         * @param where an iterator to the new node's parent node.
         * @param value the value that the node will initially hold.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        append(iterator<Traversal> where, const value_type& value) noexcept
        {
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(value); 
            new__->hook_as_last_child_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }

        /**
         * @brief emplace a new child-node as `where`'s last-child.
         * @param where an iterator to the new node's parent node.
         * @param args constructor arguments that are forwarded into the value of the new node.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> 
        emplace_append(iterator<Traversal> where, Args&&... args) noexcept
        {
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(std::forward<Args>(args)...);
            new__->hook_as_last_child_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }

        /**
         * @brief insert a new node as the next sibling of `where`.
         * @param where an iterator to the new node's previous sibling.
         * @param value the value that the node will initially hold.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `where` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        insert_after(iterator<Traversal> where, const value_type& value) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(value);
            new__->hook_as_next_sibling_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }
            
        /**
         * @brief emplace a new node as the next sibling of `where`.
         * @param where an iterator to the new node's previous sibling.
         * @param args constructor arguments that are forwarded into the value of the new node.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `where` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> 
        emplace_after(iterator<Traversal> where, Args&&... args) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(std::forward<Args>(args)...);
            new__->hook_as_next_sibling_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }

        /**
         * @brief insert a new node as the previous sibling of `where`.
         * @param where an iterator to the new node's next sibling.
         * @param value the value that the node will initially hold.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `where` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        insert_before(iterator<Traversal> where, const value_type& value) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(value);
            new__->hook_as_prev_sibling_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }
    
        /**
         * @brief emplace a new node as the previous sibling of `where`.
         * @param where an iterator to the new node's next sibling.
         * @param args constructor arguments that are forwarded into the value of the new node.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `where` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal, typename... Args>
        iterator<Traversal> 
        emplace_before(iterator<Traversal> where, Args&&... args) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(std::forward<Args>(args)...); 
            new__->hook_as_prev_sibling_M_(where); ++this->impl_M_.header_M_.size_M_;
            return iterator<Traversal>(new__);
        }
        
        /**
         * @}
         */

        /**
         * @name concatenation modifiers
         * copying tree-sections from one place to another.
         * @{
         */

        /**
         * @brief inserts an entire section of a tree with all of it's descendants at a given position by copying.
         * @param where an iterator to the node where the to-be-inserted tree should follow as the last-child.
         * @param src an iterator to the source-node that should be copied with all it's descendants. can be the same iterator as `where`.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `src` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        concatenate_append(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
        #else
            assert(!src.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(*src);
            if (src.node_ptr_M_()->has_children_M_())
            { this->copy_children_M_(new__, src); }
            new__->hook_as_last_child_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(new__, new__->parent_M_->depth_M_());
        #endif
            return iterator<Traversal>(new__);
        }

        /**
         * @brief inserts an entire section of a tree with all of it's descendants at a given position by copying.
         * @param where an iterator to the node where the to-be-inserted tree should follow as the first-child.
         * @param src an iterator to the source-node that should be copied with all it's descendants. can be the same iterator as `where`.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `src` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        concatenate_prepend(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
        #else
            assert(!src.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(*src);
            if (src.node_ptr_M_()->has_children_M_())
            { this->copy_children_M_(new__, src); }
            new__->hook_as_first_child_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(new__, new__->parent_M_->depth_M_());
        #endif
            return iterator<Traversal>(new__);
        }

        /**
         * @brief inserts an entire section of a tree with all of it's descendants at a given position by copying.
         * @param where an iterator to the node where the to-be-inserted tree should follow as the next sibling.
         * @param src an iterator to the source-node that should be copied with all it's descendants. can be the same iterator as `where`.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `where` or `src` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        concatenate_after(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
            assert(!src.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(*src);
            if (src.node_ptr_M_()->has_children_M_())
            { this->copy_children_M_(new__, src); }
            new__->hook_as_next_sibling_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(new__, new__->parent_M_->depth_M_());
        #endif
            return iterator<Traversal>(new__);
        }

        /**
         * @brief inserts an entire section of a tree with all of it's descendants at a given position by copying.
         * @param where an iterator to the node where the to-be-inserted tree should follow as the previous sibling.
         * @param src an iterator to the source-node that should be copied with all it's descendants. can be the same iterator as `where`.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the newly created node.
         * @note exceptions are thrown / behaviour is undefined if:
         * - `where` or `src` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        concatenate_before(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
            assert(!src.node_ptr_M_()->is_root_M_());
        #endif
            node_ptr_T_ new__ = this->impl_M_.get_node_M_(*src);
            if (src.node_ptr_M_()->has_children_M_())
            { this->copy_children_M_(new__, src); }
            new__->hook_as_prev_sibling_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(new__, new__->parent_M_->depth_M_());
        #endif
            return iterator<Traversal>(new__);
        }

        /**
         * @}
         */

        /**
         * @name splicing-operations 
         *
         * concatenating trees by moving nodes from one place to another.
         * @{
         */

        /**
         * @brief moves nodes from `src` behind the last-child of `where`, or insert's it as `where`'s first-child, if there are none.
         * @param where the node that should have `src` as it's last-child. 
         * @param src the node to be put behind `where`'s last-child, with all of it's descendants.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @note exceptions are thrown / the behaviour is undefined if:
         * - `src` is an `end()`-iterator.
         * - `where` and `src` point to the same node.
         * - `where` is a child-node of `src`.
         */
        template <traversal Traversal = default_traversal>
        void
        splice_append(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
            if (where.node_ptr_M_()->is_child_of(src.node_ptr_M_())) { throw std::invalid_argument("'where' cannot be a child-node of 'src'"); }
            if (where == src) { throw std::invalid_argument("cannot splice to the same node"); }
        #else
            assert(!src.node_ptr_M_()->is_root_M_())
            assert(!where.node_ptr_M_()->is_child_of(src.node_ptr_M_()))
            assert(where != src);
        #endif
            src.ptr_M_->unhook_M_();
            src.ptr_M_->hook_as_last_child_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(src, src.ptr_M_->parent_M_->depth_M_());
        #endif
        }

        /**
         * @brief moves nodes from `src` in front of the first-child of `where`, or insert's it as `where`'s first-child, if there are none.
         * @param where the node that should have `src` as it's first-child.
         * @param src the node to be put before `where`'s first-child, with all of it's descendants.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @note exceptions are thrown / the behaviour is undefined if:
         * - `src` is an `end()`-iterator.
         * - `where` and `src` point to the same node.
         * - `where` is a child-node of `src`.
         */
        template <traversal Traversal = default_traversal>
        void
        splice_prepend(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
            if (where.node_ptr_M_()->is_child_of(src.node_ptr_M_())) { throw std::invalid_argument("'where' cannot be a child-node of 'src'"); }
            if (where == src) { throw std::invalid_argument("cannot splice to the same node"); }
        #else
            assert(!src.node_ptr_M_()->is_root_M_())
            assert(!where.node_ptr_M_()->is_child_of(src.node_ptr_M_()))
            assert(where != src);
        #endif
            src.ptr_M_->unhook_M_();
            src.ptr_M_->hook_as_first_child_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(src, src.ptr_M_->parent_M_->depth_M_());
        #endif
        }

        /**
         * @brief moves nodes from `src` behind `where`.
         * @param where the node that `src` should go after.
         * @param src the node to be put behind `where`, with all of it's descendants.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @note exceptions are thrown / the behaviour is undefined if:
         * - `where` or `src` is an `end()`-iterator.
         * - `where` and `src` point to the same node.
         * - `where` is a child-node of `src`.
         */
        template <traversal Traversal = default_traversal>
        void
        splice_after(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
            if (where.node_ptr_M_()->is_child_of(src.node_ptr_M_())) { throw std::invalid_argument("'where' cannot be a child-node of 'src'"); }
            if (where == src) { throw std::invalid_argument("cannot splice to the same node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_() && !src.node_ptr_M_()->is_root_M_());
            assert(!where.node_ptr_M_()->is_child_of(src.node_ptr_M_()))
            assert(where != src);
        #endif
            src.ptr_M_->unhook_M_();
            src.ptr_M_->hook_as_next_sibling_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(src, src.ptr_M_->parent_M_->depth_M_());
        #endif
        }

        /**
         * @brief moves nodes from `src` in front of `where`.
         * @param where the node that `src` should go before.
         * @param src the node to be put before `where`, with all of it's descendants.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @note exceptions are thrown / the behaviour is undefined if:
         * - `where` or `src` is an `end()`-iterator.
         * - `where` and `src` point to the same node.
         * - `where` is a child-node of `src`.
         */
        template <traversal Traversal = default_traversal>
        void
        splice_before(iterator<Traversal> where, iterator<Traversal> src) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
            if (src.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'src' cannot point to the root-node"); }
            if (where.node_ptr_M_()->is_child_of(src.node_ptr_M_())) { throw std::invalid_argument("'where' cannot be a child-node of 'src'"); }
            if (where == src) { throw std::invalid_argument("cannot splice to the same node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_() && !src.node_ptr_M_()->is_root_M_());
            assert(!where.node_ptr_M_()->is_child_of(src.node_ptr_M_()))
            assert(where != src);
        #endif
            src.ptr_M_->unhook_M_();
            src.ptr_M_->hook_as_prev_sibling_M_(where);
        #ifdef TRL_FLEX_TREE_FAST_DEPTH
            this->update_depth_M_(src, src.ptr_M_->parent_M_->depth_M_());
        #endif
        }
        
        /**
         * @}
         */

        /**
         * @name erasure modifiers
         * @{
         */

        /**
         * @brief erases a node and all of it's descendants from the tree.
         * @param where the node to be erased.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return an iterator to the next valid node in the tree.
         * @note exceptions are thrown / the behaviour is undefined if:
         * - `where` is an `end()`-iterator.
         */
        template <traversal Traversal = default_traversal>
        iterator<Traversal> 
        erase(iterator<Traversal> where) TRL_NOEXCEPT
        {
        #ifndef TRL_FLEX_TREE_NOEXCEPT
            if (where.node_ptr_M_()->is_root_M_()) { throw std::invalid_argument("'where' cannot point to the root-node"); }
        #else
            assert(!where.node_ptr_M_()->is_root_M_());
        #endif
            if (where.node_ptr_M_()->has_children_M_())
            { this->impl_M_.header_M_.size_M_ -= this->erase_children_M_(where); }
            iterator<Traversal> next__ = std::next(where);
            where.node_ptr_M_()->unhook_M_();
            this->impl_M_.put_node_M_(static_cast<node_ptr_T_>(where.node_ptr_M_()));
            return next__;
        }
        
        /**
         * @brief erases every node in the tree.
         */
        void 
        clear() noexcept
        { 
            if (this->size()) 
            { this->impl_M_.header_M_.size_M_ -= this->erase_children_M_(&this->impl_M_.header_M_); } 
        }

        /**
         * @}
         */

        /**
         * @name container-information
         * @{
         */

        /**
         * @brief determines the depth of the deepest node in the tree via a full iteration.
         * @tparam Traversal the algorithm used to traverse the tree.
         * @return the depth of the deepest node in the tree.
         */
        template <traversal Traversal = default_traversal>
        std::size_t 
        maximum_depth() const noexcept
        {
            std::size_t res__{0ull};
            for (iterator<Traversal> it__ = this->begin<Traversal>(); it__ != end<Traversal>(); ++it__)
            { std::size_t depth__ = it__.ptr_M_->depth_M_(); if (depth__) { res__ = depth__; } }
            return res__;
        }

        /**
         * @brief get the associated allocator object.
         * @return instance of `allocator_type` 
         */
        constexpr allocator_type 
        get_allocator() const noexcept
        { return this->impl_M_.get_alloc_M_(); }

        /**
         * @return the total node-count of the tree.
         */
        constexpr std::size_t 
        size() const noexcept
        { return this->impl_M_.header_M_.size_M_; }

        /**
         * @return true if the tree is empty.
         */
        constexpr bool
        empty() const noexcept
        { return !this->impl_M_.header_M_.size_M_; }

        /**
         * @}
         */

    };

}




#endif