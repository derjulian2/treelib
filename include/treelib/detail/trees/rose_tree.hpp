
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
#include <cstdint>
#include <algorithm>

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

    namespace _detail
    {
        template <typename... Fns>
        struct _visitor : public Fns...
        { using Fns::operator()...; };

        template <typename NodeT>
        struct _vecrose_node_base
        {   
            using _m_node_t      = NodeT;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;

            // either an index, or first/last
            using _m_hook_t = std::variant<std::size_t, vrose>;

            std::vector<_m_node_ptr_t> _m_child_vec;


            _vecrose_node_base()
                : _m_child_vec()
            { }
            
            constexpr void
            _m_hook_at(_m_hook_t _at, _m_node_ptr_t _node)
            {
                std::visit(_visitor
                {
                    [&](const std::size_t& _idx) 
                        -> void
                    { this->_m_child_vec.insert(this->_m_child_vec.cbegin() + _idx, _node); },
                    [&](const vrose& _v)
                        -> void 
                    { 
                        switch (_v)
                        {
                        case (vrose::first):
                            this->_m_child_vec.insert(this->_m_child_vec.cbegin(), _node);
                            break;
                        case (vrose::last):
                            this->_m_child_vec.push_back(_node);
                            break;
                        }
                    }
                }, _at);
            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t _at)
                noexcept
            {
                return 
                std::visit(_visitor
                {
                    [&](const std::size_t& _idx) 
                        -> _m_node_ptr_t
                    { 
                        _m_node_ptr_t _res = this->_m_child_vec[_idx];
                        this->_m_child_vec.erase(this->_m_child_vec.cbegin() + _idx);
                        return _res; 
                    },
                    [&](const vrose& _v)
                        -> _m_node_ptr_t 
                    { 
                        _m_node_ptr_t _res;
                        switch (_v)
                        {
                        case (vrose::first):
                            _res = this->_m_child_vec.front();
                            this->_m_child_vec.erase(this->_m_child_vec.cbegin());
                            break;
                        case (vrose::last):
                            _res = this->_m_child_vec.back();
                            this->_m_child_vec.pop_back();
                            break;
                        }
                        return _res;
                    }
                }, _at);
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t _node)
                noexcept
            {
                std::remove(this->_m_child_vec.begin(), this->_m_child_vec.end(), _node);
            }

            constexpr std::vector<_m_node_ptr_t>&
            _m_children()
                noexcept
            { return _m_child_vec; }

            constexpr const std::vector<_m_node_ptr_t>&
            _m_children()
                const noexcept
            { return _m_child_vec; }

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_node_ptr_t, _m_cnode_ptr_t>
            constexpr void
            _m_mimic(_m_cnode_ptr_t src, Fn&& insert_fn)
            {

            }
        };


        template <typename NodeT>
        struct _listrose_node_base
        {   
            using _m_node_t      = NodeT;
            using _m_node_ptr_t  = _m_node_t*;
            using _m_cnode_ptr_t = const _m_node_t*;

            // either first/last or next/prev
            using _m_hook_t = std::variant<vrose, hrose>;
        
            _m_node_ptr_t _m_next;
            _m_node_ptr_t _m_prev;

            _m_node_ptr_t _m_first;
            _m_node_ptr_t _m_last;

            _listrose_node_base()
                : _m_next(nullptr)
                , _m_prev(nullptr)
                , _m_first(nullptr)
                , _m_last(nullptr)
            { }
            
            constexpr void
            _m_hook_at(_m_hook_t at, _m_node_ptr_t node)
            {

            }

            constexpr _m_node_ptr_t
            _m_unhook_at(_m_hook_t at)
                noexcept
            {
                return nullptr;
            }

            constexpr void
            _m_unhook_if(_m_node_ptr_t node)
                noexcept
            {

            }

            constexpr auto
            _m_children()
                noexcept
            {
                return std::vector<_m_node_ptr_t>{};
            }

            constexpr auto
            _m_children()
                const noexcept
            {
                return std::vector<_m_node_ptr_t>{};
            }

            template <typename Fn>
                requires std::invocable<Fn, _m_hook_t, _m_node_ptr_t, _m_cnode_ptr_t>
            constexpr void
            _m_mimic(_m_cnode_ptr_t src, Fn&& insert_fn)
            {

            }
        };

        struct _vecrose_node
            : public _vecrose_node_base<_vecrose_node>
        { };

        struct _bidirectional_vecrose_node
            : public _bidirectional_node<_vecrose_node_base<_bidirectional_vecrose_node>>
        { 
            using _m_base_t = _bidirectional_node<_vecrose_node_base<_bidirectional_vecrose_node>>;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;

            constexpr
            _bidirectional_vecrose_node() = default;

            constexpr bool
            _m_is_last()
                const noexcept
            {
                assert(!this->_m_is_root());
                return false;
            }

            constexpr bool
            _m_is_first()
                const noexcept
            {
                assert(!this->_m_is_root());
                return false;
            }

            constexpr _m_node_ptr_t
            _m_next_sibling()
                noexcept
            {
                return nullptr;
            }

            constexpr _m_cnode_ptr_t
            _m_next_sibling()
                const noexcept
            {
                return nullptr;
            }

            constexpr _m_node_ptr_t
            _m_prev_sibling()
                noexcept
            {
                return nullptr;
            }

            constexpr _m_cnode_ptr_t
            _m_prev_sibling()
                const noexcept
            {
                return nullptr;
            }
        };

        struct _listrose_node
            : public _listrose_node_base<_listrose_node>
        { };

        struct _bidirectional_listrose_node
            : public _bidirectional_node<_listrose_node_base<_bidirectional_listrose_node>>
        { 
            using _m_base_t = _bidirectional_node<_listrose_node_base<_bidirectional_listrose_node>>;
            using typename _m_base_t::_m_node_t;
            using typename _m_base_t::_m_node_ptr_t;
            using typename _m_base_t::_m_cnode_ptr_t;

            constexpr
            _bidirectional_listrose_node() = default;

            constexpr bool
            _m_is_last()
                const noexcept
            {
                assert(!this->_m_is_root());
                return false;
            }

            constexpr bool
            _m_is_first()
                const noexcept
            {
                assert(!this->_m_is_root());
                return false;
            }

            constexpr _m_node_ptr_t
            _m_next_sibling()
                noexcept
            {
                return nullptr;
            }

            constexpr _m_cnode_ptr_t
            _m_next_sibling()
                const noexcept
            {
                return nullptr;
            }

            constexpr _m_node_ptr_t
            _m_prev_sibling()
                noexcept
            {
                return nullptr;
            }

            constexpr _m_cnode_ptr_t
            _m_prev_sibling()
                const noexcept
            {
                return nullptr;
            }
        };
    }

    template <typename T, 
              typename Allocator = std::allocator<T>>
    using outward_vecrose_tree
        = _detail::_header_outward_tree<_detail::_vecrose_node, Allocator>;

    template <typename T, 
              typename Allocator = std::allocator<T>>
    using vecrose_tree
        = _detail::_header_tree<_detail::_bidirectional_vecrose_node, Allocator>;

    template <typename T, 
              typename Allocator = std::allocator<T>>
    using outward_listrose_tree
        = _detail::_header_outward_tree<_detail::_listrose_node, Allocator>;

    template <typename T, 
              typename Allocator = std::allocator<T>>
    using listrose_tree
        = _detail::_header_tree<_detail::_bidirectional_listrose_node, Allocator>;

    template <typename T,
              typename Allocator = std::allocator<T>>
    using rose_tree = vecrose_tree<T, Allocator>;
}


#endif