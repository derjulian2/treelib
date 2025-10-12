
#include <iostream>
#include <string>



#include "../include/treelib/node_traits.hpp"
#include "../include/treelib/flex_tree.hpp"


int main(int argc, char** argv)
{
    using namespace trl;

    flex_tree<int> tree =
    {
        1,
        2,
    {
        3,
        {
            4,
        {
            5,
            {
                6,
                7,
                8
            }
        },
            9,
            10
        }
    }
    };

    std::cout << "forward depth-first:\n";
    for (flex_tree<int>::iterator i = tree.begin(); i != tree.end(); ++i)
    {
        std::cout << std::string(node_traits::depth(i), '-') << *i << "\n";
    }

    std::cout << "backward depth-first:\n";
    for (flex_tree<int>::reverse_iterator ri = tree.rbegin(); ri != tree.rend(); ++ri)
    {
        std::cout << std::string(node_traits::depth(ri), '-') << *ri << "\n";
    }

    /* maybe introduce a leaf-iterator algorithm as a shortcut ? */
    flex_tree<int>::iterator<breadth_first_in_order> five = std::find(tree.begin<breadth_first_in_order>(), tree.end<breadth_first_in_order>(), 5);
    std::cout << "iterating over leaf-nodes:\n";
    for (flex_tree<int>::iterator<breadth_first_in_order> i = node_traits::first_child(five); i != ++node_traits::last_child(five); ++i)
    {

    }

    return 0;
}