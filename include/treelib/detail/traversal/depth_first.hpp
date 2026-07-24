
#ifndef TREELIB_DEPTH_FIRST_HPP
#define TREELIB_DEPTH_FIRST_HPP

/**
 * @file   treelib/detail/queued_iterator.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  types to implement different
 *         tree-traversal-strategies using
 *         a queued approach. this is mainly used
 *         for weak-trees as these are not 
 *         iteratively traversable (e.g. because
 *         parents are not reachable from any given node).
 */

#include <treelib/detail/base/node.hpp>

#include <queue>
#include <stack>
#include <vector>

namespace tl
{
    /**
     *
     */
    template <typename NodeType>
    struct depth_first_pre_order
    {
        using node_traits  = tl::node_traits<NodeType>;
        using node_type    = typename node_traits::node_type;
        using node_pointer = typename node_traits::pointer;

        /**
         * @brief constructs an iterable sequence of node-pointers
         *        that when iterated over represents the
         *        depth-first-pre-order traversal of the underlying tree.
         */
        template <typename ResultType = std::deque<node_pointer>>
        static constexpr 
        ResultType
        enqueue(node_pointer src)
        {
            ResultType result;
            std::stack<node_pointer> stack;
            
            stack.push(src);
            while (!stack.empty())
            {
                node_pointer cur = stack.top();
                result.push_back(cur);
                stack.pop();

                for (node_pointer& c : node_traits::children(*cur))
                {
                    stack.push(c);
                }
            }
            return result;
        }


        // constexpr 
        // node_pointer
        // next(node_pointer src)
        //     requires strong_node<node_type>
        // {

        // }


        // constexpr 
        // node_pointer
        // prev(node_pointer src)
        //     requires strong_node<node_type>
        // {

        // }
    };


    template <typename NodeType>
    struct depth_first_post_order
    {
        
    };
}

#endif