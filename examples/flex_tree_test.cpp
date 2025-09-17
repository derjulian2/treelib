#include "flex_tree_test.hpp"

#include "../include/treelib/flex_tree.hpp"
#include "../include/treelib/node_traits.hpp"
#include <algorithm>
#include <string>

int trl::tests::flex_tree_test()
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

    // for (flex_tree<std::string>::const_iterator<> i = str_tree_1.cbegin<>(); i != str_tree_1.cend<>(); ++i)
    // {

    // }

    // auto node_4 = std::find(str_tree_1.begin(), str_tree_1.end(), "node_4");

    // for (flex_tree<std::string>::iterator iter = str_tree_1.begin(); iter != str_tree_1.end(); ++iter)
    // {
    //     std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    // }
    // std::cout << "\n";

    // flex_tree<std::string> str_tree_2 = str_tree_1;

    // auto node_4_2 = std::find(str_tree_2.begin(), str_tree_2.end(), "node_4");

    // *node_4 = "iltamsumra";

    // for (flex_tree<std::string>::iterator iter = str_tree_1.begin(); iter != str_tree_1.end(); ++iter)
    // {
    //     std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    // }
    // std::cout << "\n";

    // for (flex_tree<std::string>::iterator iter = str_tree_2.begin(); iter != str_tree_2.end(); ++iter)
    // {
    //     std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    // }
    // std::cout << "\n";

    // str_tree_1.concatenate_after(node_4, node_4_2);

    // for (flex_tree<std::string>::iterator iter = str_tree_1.begin(); iter != str_tree_1.end(); ++iter)
    // {
    //     std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    // }
    // std::cout << "\n";

    // for (flex_tree<std::string>::iterator iter = str_tree_2.begin(); iter != str_tree_2.end(); ++iter)
    // {
    //     std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    // }
    // std::cout << "\n";

    return 0;
}

int trl::examples::flex_tree_example_1()
{

    return 0;
}

int trl::examples::flex_tree_example_2()
{

    return 0;
}

int trl::examples::flex_tree_example_3()
{

    return 0;
}
