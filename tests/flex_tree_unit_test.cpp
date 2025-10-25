
#include <iostream>
#include <string>
#include <algorithm>

#include "../include/treelib/flex_tree.hpp"

int main(int argc, char** argv)
{
    using namespace trl;

    flex_tree<int> ftr
    {
        1,
        2,
        4920,
    {
        6942,
        {
            6943,
            6944,
            6945
        }
    }
    };

    std::cout << "depth-first iteration:\n";
    for (flex_tree<int>::iterator<> i = ftr.begin(); i != ftr.end(); ++i)
    {
        std::cout << std::string(node_traits::depth(i), '=') << ' ' << *i << '\n';
    }

    std::cout << "breadth-first iteration:\n";
    for (flex_tree<int>::iterator<breadth_first_in_order> i = ftr.begin(); i != ftr.end(); ++i)
    {
        std::cout << std::string(node_traits::depth(i), '=') << ' ' << *i << '\n';
    }

    // std::cout << "reverse iteration:\n";
    // for (flex_tree<int>::reverse_iterator<> ri = ftr.rbegin(); ri != ftr.rend(); ++ri)
    // {
    //     std::cout << std::string(node_traits::depth(ri), '=') << ' ' << *ri << '\n';
    // }
    
    flex_tree<int>::iterator<> i = std::find(ftr.begin(), ftr.end(), 6942);

    // std::cout << "leaf iteration:\n";
    // for (leaf_iterator<decltype(i)> j = node_traits::lbegin(i); j != node_traits::lend(i); ++j)
    // {
    //     std::cout << std::string(node_traits::depth(j), '=') << ' ' << *j << '\n';
    // }

    return 0;
}