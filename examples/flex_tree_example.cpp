
#include "../include/treelib/flex_tree.hpp"
#include "../include/treelib/node_traits.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    using namespace trl;

    
    flex_tree<std::string> str_tree_1
    {
        "first_node"
    };

    std::size_t reps{7ull};

    for (std::size_t i = 0ull; i < reps; ++i)
    {
        /* double the tree in size each time by prepending a new node in front of every node */
        for (flex_tree<std::string>::iterator it = str_tree_1.begin(); it != str_tree_1.end();)
        {
            it = str_tree_1.prepend(it, std::to_string(i));
            ++it;
        }
    }

    for (flex_tree<std::string>::iterator it = str_tree_1.begin(); it != str_tree_1.end(); ++it)
    {
        std::cout << std::string(node_traits::depth(it), '-') << *it << "\n";
    }
    std::cout << "tree size of " << str_tree_1.size() << "\n";

    return 0;
}