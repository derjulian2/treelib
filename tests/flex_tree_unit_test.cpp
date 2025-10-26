
#include <iostream>
#include <string>
#include <algorithm>

#include "../include/treelib/flex_tree.hpp"

int main(int argc, char** argv)
{
    using namespace trl;
    using tree_type = flex_tree<std::string>;
    /* type to retrieve information about nodes */
    using traits_type = tree_type::node_traits;

    /* construction using initializer-list */
    tree_type ftr =
    {
        "hello",
    {
        "world",
        {
            "foo1",
            "foo2"
        }
    },
        "foo",
    {
        "bar",
        {
            "bogus",
            "iltam",
            "sumra"
        }
    }
    };

    

    /* construction using member-functions */
    // tree_type ftr;
    // auto hello_ = ftr.append(ftr.end(), "hello");
    // auto world_ = ftr.append(ftr.end(), "world");
    // auto foo1_ = ftr.append(world_, "foo1");
    // auto foo2_ = ftr.insert_after(foo1_, "foo2");
    // auto foo_ = ftr.insert_after(world_, "foo");
    // auto bar_ = ftr.append(ftr.end(), "bar");
    // auto bogus_ = ftr.append(bar_, "bogus");
    // auto sumra_ = ftr.append(bar_, "sumra");
    // auto iltam_ = ftr.insert_before(sumra_, "iltam");
    /* (uncomment one construction to see example-output) */

    /* regular depth-first traversal */
    std::cout << "depth-first:\n";
    for (tree_type::iterator i = ftr.begin(); i != ftr.end(); ++i)
    { std::cout << std::string(traits_type::depth(i), '-') << *i << '\n'; }

    /* breadth-first traversal */
    std::cout << "breadth-first:\n";
    for (tree_type::iterator<breadth_first_in_order> i = ftr.begin(); i != ftr.end(); ++i)
    { std::cout << std::string(traits_type::depth(i), '-') << *i << '\n'; }

    /* searching for a value and replacing it. will use depth-first as default. */
    *std::find(ftr.begin(), ftr.end(), "bogus") = "sugob";

    auto i = std::find(ftr.begin(), ftr.end(), "bar");
    /* leaf-iteration over every child of node 'bar' */
    std::cout << "leaf-iteration over 'bar':\n";
    for (tree_type::const_leaf_iterator cl = traits_type::lbegin(i); cl != traits_type::lend(i); ++cl)
    { std::cout << std::string(traits_type::depth(cl), '-') << *cl << '\n'; }

    return 0;
}