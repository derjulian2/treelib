
#ifndef TREELIB_ITERATOR_HPP
#define TREELIB_ITERATOR_HPP

#include <stack>
#include <queue>


template <typename TreeType>
class depth_first_pre_order_iterator
{
public:
    using tree_type    = TreeType;
    using node_type    = typename tree_type::node_type;
    using node_pointer = node_type*;

    [[nodiscard]]
    static constexpr
    node_pointer
    next() { }


    template <typename Sequence = std::deque<node_pointer>>
    [[nodiscard]]
    static constexpr
    std::queue<node_pointer>
    enqueue(node_pointer src)
    {
        std::queue<node_pointer> res;
        std::stack<node_pointer, Sequence> stack;
        
        res.push(src);
        stack.push(src);
        while (!stack.empty())
        {
            node_pointer cur = stack.top();
            stack.pop();
            for (node_pointer c : cur->children())   
            {
                if (!c) continue;
                stack.push(c);
                res.push(c);
            }
        }
        return res;
    }

};

template <typename TreeType>
class depth_first_post_order_iterator
{

};

template <typename TreeType>
class depth_first_iterator
{

};

template <typename TreeType>
class breadth_first_iterator
{

};

template <typename TreeType>
class queued_iterator
{

};

template <typename TreeType>
class iterator
{

};

#endif