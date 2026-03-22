
#ifndef __TREE_HPP
#define __TREE_HPP

#include <memory>

namespace tl
{

    namespace __detail
    {
        struct __tree_node_base
        {
            __tree_node_base* _M_parent;
            __tree_node_base* _M_child;

            __tree_node_base* _M_next;
            __tree_node_base* _M_prev;
        };

        struct __tree_header
            : public __tree_node_base
        {

        };

        template <typename __Tp, bool __Const>
        struct __tree_iterator_base
        {

        };

        template <typename __Tp,
                  typename __Alloc>
        struct __tree_base
        {

        };
    }

    template <typename T, typename Allocator = std::allocator<T>>
    class tree
        : protected __detail::__tree_base<T, Allocator>
    {

    };
}

#endif