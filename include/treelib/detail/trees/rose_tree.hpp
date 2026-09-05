
#ifndef TREELIB_ROSE_TREE_HPP
#define TREELIB_ROSE_TREE_HPP

/***************************************************
 * @file   treelib/detail/trees/rose_tree.hpp
 * @author Julian Benzel
 * @date   04.09.2026
 *
 * @brief  type-generic trees without any constraints
 *         on the number of children per node.
 ***************************************************/

#include <treelib/detail/bits/except.hpp>
#include <treelib/detail/base/node.hpp>
#include <treelib/detail/base/tree.hpp>
#include <treelib/detail/base/forest.hpp>

#include <vector>
#include <variant>
#include <ranges>
#include <cstdint>

namespace tl
{
    enum struct vrose
        : std::uint8_t
    {
        first,
        last
    };

    enum struct hrose
        : std::uint8_t
    {
        next,
        prev
    };

    namespace detail
    {
        class vecrose_node
        {
        protected:
    
            std::vector<vecrose_node*> _M_children;

        public:

            // either an index, or first/last
            using hook_type = std::variant<std::size_t, vrose>;

            vecrose_node()
                : _M_children()
            { }
            
            constexpr void
            hook_at(hook_type at, vecrose_node* node)
            {
                
            }

            constexpr vecrose_node*
            unhook_at(hook_type at)
                noexcept
            {
                
            }

            constexpr void
            unhook_if(const vecrose_node* node)
                noexcept
            {
                
            }

            constexpr std::vector<vecrose_node*>&
            children()
                noexcept
            {
                return this->_M_children;
            }

            constexpr const std::vector<vecrose_node*>&
            children()
                const noexcept
            {
                return this->_M_children;
            }

            template <typename Fn>
                requires std::invocable<Fn, hook_type, vecrose_node*, const vecrose_node*>
            constexpr void
            mimic(const vecrose_node* src, Fn&& insert_fn)
            {
                
            }
        };


        class listrose_node
        {
        protected:
    
            listrose_node* _M_next;
            listrose_node* _M_prev;

            listrose_node* _M_first;
            listrose_node* _M_last;

        public:

            using hook_type = std::variant<vrose, hrose>;

            listrose_node()
                : _M_next(nullptr)
                , _M_prev(nullptr)
                , _M_first(nullptr)
                , _M_last(nullptr)
            { }
            
            constexpr void
            hook_at(hook_type at, vecrose_node* node)
            {
                
            }

            constexpr vecrose_node*
            unhook_at(hook_type at)
                noexcept
            {
                
            }

            constexpr void
            unhook_if(const vecrose_node* node)
                noexcept
            {
                
            }

            constexpr std::vector<vecrose_node*>&
            children()
                noexcept
            {
                return this->_M_children;
            }

            constexpr const std::vector<vecrose_node*>&
            children()
                const noexcept
            {
                return this->_M_children;
            }

            template <typename Fn>
                requires std::invocable<Fn, hook_type, vecrose_node*, const vecrose_node*>
            constexpr void
            mimic(const vecrose_node* src, Fn&& insert_fn)
            {
                
            }
        };
    }




    template <typename T, 
              typename Allocator = std::allocator<T>>
    class outward_vecrose_tree
        : public weak_tree_base<weak_rose_tree_node, Allocator>
    { };


    template <typename T, 
              typename Allocator = std::allocator<T>>
    class vecrose_tree
        : public tree_base<rose_tree_node, Allocator>
    { };

        template <typename T, 
              typename Allocator = std::allocator<T>>
    class outward_listrose_tree
        : public weak_tree_base<weak_rose_tree_node, Allocator>
    { };


    template <typename T, 
              typename Allocator = std::allocator<T>>
    class listrose_tree
        : public tree_base<rose_tree_node, Allocator>
    { };

    template <typename T,
              typename Allocator = std::allocator<T>>
    using rose_tree = listrose_tree<T, Allocator>;
}


#endif