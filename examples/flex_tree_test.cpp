#include "flex_tree_test.hpp"

#include "../include/flex_tree.hpp"
#include <algorithm>
#include <string>

int trl::tests::flex_tree_test()
{
    using namespace trl;

    
    flex_tree<std::string> str_tree_1 =
    {
        "node_1",
        "node_2",
    {   "node_3", // braces here required for the compiler to know where a node with child-nodes starts
        { 
        {   "node_4",
            {
                "node_5",
                "node_6"
            }
        },  "node_7"
        }
    } // braces here required
    };

    auto node_4 = std::find(str_tree_1.begin(), str_tree_1.end(), "node_4");

    for (flex_tree<std::string>::iterator iter = str_tree_1.begin(); iter != str_tree_1.end(); ++iter)
    {
        std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    }
    std::cout << "\n";

    flex_tree<std::string> str_tree_2 = str_tree_1;

    auto node_4_2 = std::find(str_tree_2.begin(), str_tree_2.end(), "node_4");

    *node_4 = "iltamsumra";

    for (flex_tree<std::string>::iterator iter = str_tree_1.begin(); iter != str_tree_1.end(); ++iter)
    {
        std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    }
    std::cout << "\n";

    for (flex_tree<std::string>::iterator iter = str_tree_2.begin(); iter != str_tree_2.end(); ++iter)
    {
        std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    }
    std::cout << "\n";

    str_tree_1.concatenate_after(node_4, node_4_2);

    for (flex_tree<std::string>::iterator iter = str_tree_1.begin(); iter != str_tree_1.end(); ++iter)
    {
        std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    }
    std::cout << "\n";

    for (flex_tree<std::string>::iterator iter = str_tree_2.begin(); iter != str_tree_2.end(); ++iter)
    {
        std::cout << std::string(iter.examine_node().depth(), ' ') << *iter << "\n";
    }
    std::cout << "\n";

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
