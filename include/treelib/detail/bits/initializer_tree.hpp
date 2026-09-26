
#ifndef TREELIB_INITIALIZER_TREE_HPP
#define TREELIB_INITIALIZER_TREE_HPP

/*************************************************************
 * @file   treelib/detail/bits/initializer_tree.hpp
 * @author Julian Benzel
 * @date   25.09.2026
 *
 * @brief  'std::initializer_list' for tree-types
 *         to allow some sleek tree-construction
 *         syntax.
 *
 * @details compile-options:
 *          - #define _treelib_safe_initializers
 *            just in case it turns out that this is UB,
 *            replaces the 'std::initializer_list's with
 *            instances of 'std::vector'.
 *
 *          for trees to be constructible from
 *          'tl::initializer_tree' the respective
 *          node-type requires a method that
 *          determines how a node should be
 *          constructed from an initializer and
 *          it's children.
 * 
 *          to implement the recursiveness of the
 *          tree-initializers, this implementation
 *          stores instances of 'std::initializer_list'
 *          as a member-variable of '_initializer_node'.
 *           
 *          from cppreference:
 *              https://en.cppreference.com/cpp/language/list_initialization#List-initializing_std::initializer_list
 *
 *              The backing array has the same lifetime as 
 *              any other temporary object, except that initializing an 
 *              'std::initializer_list' object from the backing array extends 
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
 *          of 'std::initializer_list', not sure if i
 *          completely trust it, but sure.
 *************************************************************/

#include <treelib/detail/bits/except.hpp>

#include <initializer_list>
#ifdef _treelib_safe_initializers
    #include <vector>
#endif

namespace tl
{
    namespace _detail
    {
        /********************************************************
         * @brief   concept to be used for template-argument
         *          'InitT' in 'node_traits::mimic_initializer'.
         *        
         * @details because initializer-nodes require an instance
         *          of 'value_type' as a member to copy from, the 
         *          exact type is not known to the node-types.
         *          this concept constrains the type to expose
         *          access to their child-initializer-nodes.
         ********************************************************/
        template <typename T>
        concept _initializer_node_interface
            = requires (const T& ct)
            {
                { ct._m_children() }
                    -> std::ranges::range;
            };

        /********************************************************
         * @brief requirements for node-types that can build
         *        their tree-structure from an initializer-tree.
         ********************************************************/
        template <typename T, typename InitT>
        concept _initializer_compatible
            = _initializer_node_interface<InitT> 
            && requires (T* t, const InitT& inode)
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

        /********************************************************
         * @brief CRTP-base-class holding only a child-list to 
         *        allow for valueless init-nodes in header-trees.
         ********************************************************/
        template <typename InitNodeT>
        struct _initializer_node_base
        {
            using _m_init_node_t = InitNodeT;
        #ifdef _treelib_safe_initializers
            using _m_init_list_t = std::vector<_m_init_node_t>;
        #else
            using _m_init_list_t = std::initializer_list<_m_init_node_t>;
        #endif

            const _m_init_list_t _m_child_list;

            constexpr
            _initializer_node_base() = default;

            constexpr
            _initializer_node_base(_m_init_list_t _children)
                : _m_child_list(_children)
            { }

            constexpr const _m_init_list_t&
            _m_children()
                const noexcept
            { return this->_m_child_list; }
        };

        /***************************************************
         * @brief '_initialzer_node_base' extended by an
         *        instance of 'const ValueT'.
         ***************************************************/
        template <typename ValueT>
        struct _initializer_node
            : public _initializer_node_base<_initializer_node<ValueT>>
        {
            using _m_value_t = ValueT;
            using _m_base_t  = _initializer_node_base<_initializer_node<ValueT>>;
            
            // write out explicitly again for better deduction
        #ifdef _treelib_safe_initializers
            using _m_init_list_t = std::vector<_initializer_node>;
        #else
            using _m_init_list_t = std::initializer_list<_initializer_node>;
        #endif

            const _m_value_t _m_data;

            constexpr
            _initializer_node(const _m_value_t& value)
                : _m_data(value)
                , _m_base_t()
            { }

            constexpr
            _initializer_node(_m_value_t&& value)
                : _m_data(value)
                , _m_base_t()
            { }

            constexpr
            _initializer_node(const _m_value_t& value, _m_init_list_t children)
            #ifndef _treelib_safe_initializers
                noexcept(std::is_nothrow_copy_constructible_v<_m_value_t>)
            #endif
                : _m_data(value)
                , _m_base_t(children)
            { }

            constexpr
            _initializer_node(_m_value_t&& value, _m_init_list_t children)
            #ifndef _treelib_safe_initializers
                noexcept(std::is_nothrow_move_constructible_v<_m_value_t>)
            #endif
                : _m_data(value)
                , _m_base_t(children)
            { }

            constexpr const _m_value_t&
            _m_value()
                const noexcept
            { return this->_m_data; }
        };

        /***************************************************
         * @brief lightweight recursive initializer-list
         *        to construct trees from. originates from
         *        a value-holding root-node.
         ***************************************************/
        template <typename ValueT>
        struct _root_initializer_tree
        {
            using _m_value_t     = ValueT;
            using _m_init_node_t = _initializer_node<_m_value_t>;
            using _m_init_list_t = typename _m_init_node_t::_m_init_list_t;

            const _m_init_node_t _m_root;

            constexpr
            _root_initializer_tree(const _m_init_node_t& root)
                : _m_root(root)
            { }
        };

        /***************************************************
         * @brief lightweight recursive initializer-list
         *        to construct trees from. originates from
         *        a valueless header-node.
         ***************************************************/
        template <typename ValueT>
        struct _header_initializer_tree
        {
            using _m_value_t     = ValueT;
            using _m_init_node_t = _initializer_node<_m_value_t>;
            using _m_init_node_base_t = _initializer_node_base<_m_init_node_t>;
            
            // write out explicitly again for better deduction
        #ifdef _treelib_safe_initializers
            using _m_init_list_t = std::vector<_initializer_node>;
        #else
            using _m_init_list_t = std::initializer_list<_m_init_node_t>;
        #endif

            const _m_init_node_base_t _m_header;

            constexpr
            _header_initializer_tree(_m_init_list_t header_children)
                : _m_header(header_children)
            { }
        };
    }

    namespace initializers
    {
        /***************************************************
         * @brief lightweight recursive initializer-list
         *        to construct trees from. represents a
         *        single node with an arbitrary amount
         *        of child-nodes.
         *        
         * @details the well-formedness of the initializer
         *          -tree's structure is checked when the
         *          node-types mimic the structure of the 
         *          init-nodes.
         *          these methods may throw or silently
         *          ignore errors and try their best to
         *          construct a valid tree.
         ***************************************************/
        template <typename ValueT>
        using node = ::tl::_detail::_initializer_node<ValueT>;

        /***************************************************
         * @brief lightweight recursive initializer-list
         *        to construct trees from. originates from
         *        a value-holding root-node.
         *
         * @example construction of a 'tl::binary_tree<int>'.
         *
         * @code
         *          using namespace tl::initializers;
         *          binary_tree<int> my_tree(
         *              root_tree(
         *                  node(1,
         *                  {
         *                      node(2,
         *                      {
         *                          node(3),
         *                          node(4)
         *                      }),
         *                      node(5,
         *                      {
         *                          node(6),
         *                          node(7)
         *                      })
         *                  })
         *              )
         *          );
         * @endcode
         *
         *          constructing the tree:
         *
         *                    1
         *                   / \
         *                  /   \
         *                 2     5  
         *                / \   / \
         *               3   4 6   7
         *
         ***************************************************/
        template <typename T>
        using root_tree 
            = _detail::_root_initializer_tree<T>;

        /********************************************************
         * @brief lightweight recursive initializer-list
         *        to construct trees from. originates from
         *        a valueless header-node.
         *
         * @example construction of a 'tl::rose_tree<string>'.
         *
         * @code
         *          using namespace tl::initializers;
         *          using snode = node<string>;
         *          rose_tree<string> my_tree(
         *              header_tree({
         *                  snode("include",
         *                  {
         *                      snode("treelib",
         *                      {
         *                          snode("detail",
         *                          {
         *                              snode("node.hpp"),
         *                              snode("iterator.hpp"),
         *                              snode("tree.hpp")
         *                          }),
         *                          snode("avl"),
         *                          snode("binary"),
         *                          snode("k_ary"),
         *                          snode("rose")
         *                      })
         *                  })
         *              })
         *          );
         * @endcode
         *
         *          constructing the tree:
         *
         *              >-"include"
         *                >-"treelib"
         *                 |-"detail"
         *                 | >-"base"
         *                 |   |-"node.hpp"
         *                 |   |-"iterator.hpp"
         *                 |   >-"tree.hpp"
         *                 |-"avl"
         *                 |-"binary"
         *                 |-"k_ary"
         *                 >-"rose"
         *
         ********************************************************/
        template <typename T>
        using header_tree 
            = _detail::_header_initializer_tree<T>;
        }
}

#endif