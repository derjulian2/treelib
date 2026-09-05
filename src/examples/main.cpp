
#include <treelib/binary>

#include <print>
#include <string>

namespace tl
{
    /***************************************************
     * @brief tests the following binary-tree:
     *
     *          1
     *         / \
     *        /   \
     *       2     5  
     *      / \   / \
     *     3   4 6   7
     *
     * with a depth-first-pre-order traversal of:
     * [ 1, 2, 3, 4, 5, 6, 7 ]
     *
     ***************************************************/
    int outward_binary_tree_tests()
    {
        using tree_type = outward_binary_tree<int>;
    
        tree_type my_tree(1);
        
        tree_type::const_iterator it = my_tree.begin();
        tree_type::iterator l  = my_tree.emplace(binary::left, my_tree.croot(), 2);
        tree_type::iterator ll = my_tree.emplace(binary::left, l, 3);
        tree_type::iterator lr = my_tree.emplace(binary::right, l, 4);

        tree_type::iterator r  = my_tree.emplace(binary::right, my_tree.croot(), 5);
        tree_type::iterator rl = my_tree.emplace(binary::left, r, 6);
        tree_type::iterator rr = my_tree.emplace(binary::right, r, 7);

        for (const int& i : my_tree)
            std::println("i: {}", i);

        return 0;
    }

    int binary_tree_tests()
    {
        // using tree_type = binary_tree<std::string>;
        // static_assert(tl::detail::node<tl::detail::bidirectional_k_node<2>>);

        // std::allocator<std::string> my_alloc;

        // tree_type my_tree_a("root_a", my_alloc);
        // tree_type my_tree_b("root_b");

        return 0;
    }
}


int main(int argc, char** argv) 
{   
    tl::outward_binary_tree_tests();
    tl::binary_tree_tests();
    return 0;
}