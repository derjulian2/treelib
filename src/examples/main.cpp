
#include <treelib/k_tree>

#include <iostream>
#include <string>

using namespace tl;

int main(int argc, char** argv) {

    weak_k_tree<std::string, 2> my_tree;

    (void)my_tree.insert("root");
    queued_iterator left1  = my_tree.insert(0, my_tree.root(), "left1");
    queued_iterator right1 = my_tree.insert(1, my_tree.root(), "right1");

    // weak_k_tree<std::string, 2> my_tree_2 = make_tree<weak_k_tree<std::string, 2>>({
    //     ("root"), {
    //         ("left1"),
    //         ("left2")
    //     }
    // });

    for (const std::string& s : my_tree)
    {
        std::cout << s << std::endl;
    }

    /*
     * >>> root
     * >>> left1
     * >>> right1
     */

    return 0;
}