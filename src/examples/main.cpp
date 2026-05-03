
#include <iostream>

#include <treelib/iterator.hpp>
#include <treelib/binary_tree.hpp>
#include <treelib/k_tree.hpp>
#include <treelib/rose_tree.hpp>

#include <string>



int main(int argc, char** argv) {
    using namespace tl;
    using tree_type = weak_binary_tree<std::string>;

    tree_type tree;

    auto root = tree.insert_root("iltam");
    auto a = tree.insert<0>(root, "sumra");
    auto b = tree.insert<1>(root, "rashupti");

    auto c = tree.copy(root);

    std::queue<tree_type::node_pointer> q = depth_first_pre_order_iterator<tree_type>::enqueue(c);
    while (!q.empty())
    {
        tree_type::node_pointer node = q.front();
        std::cout << static_cast<tree_type::value_node_pointer>(node)->value() << std::endl;
        q.pop(); 
    }

    return 0;
}