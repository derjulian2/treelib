
#ifndef TREELIB_INITIALIZER_TREE_HPP
#define TREELIB_INITIALIZER_TREE_HPP

/***************************************************
 * @file   treelib/detail/bits/initializer_tree.hpp
 * @author Julian Benzel
 * @date   21.9.2026
 *
 * @brief  std::initializer_list for tree-types
 *         to allow some sleek tree-construction
 *         syntax.
 *
 * @details for trees to be constructible from
 *          tl::initializer_tree the respective
 *          node-type requires a constructor that
 *          determines how a node should be
 *          constructed from an initializer and it's
 *          children.
 * 
 *          to implement the recursiveness of the
 *          tree-initializers, this implementation
 *          stores instances of std::initializer_list
 *          as a member-variable of _initializer_node.
 *           
 *          from cppreference:
 *              https://en.cppreference.com/cpp/language/list_initialization#List-initializing_std::initializer_list
 *
 *              The backing array has the same lifetime as 
 *              any other temporary object, except that initializing an 
 *              std::initializer_list object from the backing array extends 
 *              the lifetime of the array exactly like binding 
 *              a reference to a temporary. 
 *
 *          which i guess means that storing these
 *          temporary member-variables extends the
 *          lifetime of the backing-array ???
 *          which should be enough to use the recursive
 *          structures inside the tree-constructors.
 *
 *          i actually went ahead and asked github-copilot
 *          about this, it said this was a 'reasonable use'
 *          of std::initializer_list, not sure if i
 *          completely trust it, but sure.
 * 
 *          just in case:
 *          if it turns out that this is UB, use
 *          #define _treelib_safe_initializers to
 *          replace the std::initializer_list's with
 *          instances of std::vector.
 ***************************************************/

#include <treelib/detail/bits/except.hpp>

#include <initializer_list>
#ifdef _treelib_safe_initializers
    #include <vector>
#endif

namespace tl
{
    namespace _detail
    {
        // template <typename T>
        // concept _header_tree
        //     = true;

        // template <typename T>
        // concept _root_tree
        //     = true;

        template <typename T, typename InitT>
        concept _initializable_node
            = requires (T* t, const T* ct, const InitT& inode)
            {
                /***************************************************
                 * @brief '.mimic_initializer()' should bring a
                 *        null-initialized node into a state
                 *        where it is structuraly identical
                 *        to the initializer-node.          
                 *
                 *        the function that will be passed to
                 *        allow the implementation to specify where
                 *        copies should go will be some form of
                 *        tree.insert(...) to ensure that if
                 *        allocation fails, the tree will remain in
                 *        a predictable state.
                 *
                 *        @see tl::_detail::_copyable_node for more.
                 ***************************************************/
                { t->_m_mimic_initializer(inode, [](typename T::_m_hook_t _at, 
                                                    T* _parent, 
                                                    const InitT& _src) -> void { }) };
            };

        /***************************************************
         * @brief lightweight recursive initializer-list
         *        to construct trees from.
         ***************************************************/
        template <typename ValueT>
        struct _initializer_node
        {
            using _m_value_t = ValueT;
        #ifdef _treelib_safe_initializers
            using _m_init_list_t = std::vector<_initializer_node>;
        #else
            using _m_init_list_t = std::initializer_list<_initializer_node>;
        #endif

            const _m_value_t _m_value;
            const _m_init_list_t _m_children;

            constexpr
            _initializer_node(const _m_value_t& value, _m_init_list_t&& children = { })
                noexcept(std::is_nothrow_copy_constructible_v<_m_value_t>)
                : _m_value(value)
                , _m_children(children)
            { }

            constexpr
            _initializer_node(_m_value_t&& value, _m_init_list_t&& children = { })
                noexcept(std::is_nothrow_move_constructible_v<_m_value_t>)
                : _m_value(value)
                , _m_children(children)
            { }
        };

        template <typename ValueT>
        struct _header_initializer_tree
        {
            using _m_value_t     = ValueT;
            using _m_init_node_t = _initializer_node<_m_value_t>;
            using _m_init_list_t = typename _m_init_node_t::_m_init_list_t;

            const _m_init_list_t _m_header_children;

            constexpr
            _header_initializer_tree(_m_init_list_t&& header_children)
                : _m_header_children(header_children)
            { }
        };

        template <typename ValueT>
        struct _root_initializer_tree
        {
            using _m_value_t     = ValueT;
            using _m_init_node_t = _initializer_node<_m_value_t>;
            using _m_init_list_t = typename _m_init_node_t::_m_init_list_t;

            _m_init_node_t _m_root;

            constexpr
            _root_initializer_tree(const _m_init_node_t& root)
                : _m_root(root)
            { }
        };
    }

    namespace initializer
    {
        template <typename ValueT>
        using node = ::tl::_detail::_initializer_node<ValueT>;
    }

    template <typename ValueT>
    //    requires _detail::_root_tree<NodeT>
    using initializer_tree 
        = _detail::_root_initializer_tree<ValueT>;

    // template <typename TreeT>
    //     requires _detail::_header_tree<TreeT>
    // using initializer_tree 
    //     = _detail::_header_initializer_tree<TreeT>;
}

#endif