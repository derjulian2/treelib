#include "flex_tree_test.hpp"

#include "../include/treelib/flex_tree.hpp"
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

    auto beg = str_tree_1.begin();
    auto beg2 = str_tree_1.begin<breadth_first_in_order>();
    auto rbeg = str_tree_1.rbegin();
    auto cbeg = str_tree_1.cbegin();
    //auto crbeg = str_tree_1.crbegin();

    auto end = str_tree_1.end();
    auto cend = str_tree_1.cend();
    auto rend = str_tree_1.rend();
    //auto crend = str_tree_1.crend();

    beg2 = beg;
    cbeg = beg;
    bool b = (beg == beg2);

    /* conversions possible */
    trl::node_traits::parent(beg);
    trl::flex_tree<std::string>::reverse_iterator<breadth_first_in_order> r = trl::node_traits::parent(rbeg);

    static_assert(std::input_iterator<flex_tree<std::string>::iterator<>>);
    static_assert(std::input_iterator<flex_tree<std::string>::const_iterator<>>);
    static_assert(std::input_iterator<flex_tree<std::string>::reverse_iterator<>>);
    static_assert(std::input_iterator<flex_tree<std::string>::const_reverse_iterator<>>);

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
