
#include <treelib/binary>
// #include <treelib/rose>

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
     * post-order:
     * [ 1 ]
     *   ^
     * [ 2, 5, 1 ]
     *   ^
     * [ 3, 4, 2, 5, 1 ]
     *   ^
     * [ 3, 4, 2, 5, 1 ]
     *      ^ 
     * 
     *
     * [ 3, 4, 2, 6, 7, 5, 1 ]
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

        // for (const int& i : my_tree)
        //     std::println("i: {}", i);

        for (queued_iterator it 
                = my_tree.qbegin<traversal::depth_first_post_order>(); 
             it != my_tree.end();
             ++it)
            std::println("{}", *it);

        return 0;
    }

    int binary_tree_tests()
    {
        // using tree_type = binary_tree<int>;
    
        // tree_type my_tree(1);
        
        // tree_type::const_iterator it = my_tree.begin();
        // tree_type::iterator l  = my_tree.emplace(binary::left, my_tree.croot(), 2);
        // tree_type::iterator ll = my_tree.emplace(binary::left, l, 3);
        // tree_type::iterator lr = my_tree.emplace(binary::right, l, 4);

        // tree_type::iterator r  = my_tree.emplace(binary::right, my_tree.croot(), 5);
        // tree_type::iterator rl = my_tree.emplace(binary::left, r, 6);
        // tree_type::iterator rr = my_tree.emplace(binary::right, r, 7);

        // for (const int& i : my_tree)
        //     std::println("i: {}", i);

        return 0;
    }

    /***************************************************
     * @brief tests the following rose-tree:
     *
     * >-"include"
     *   >-"treelib"
     *    |-"detail"
     *    | |-"base"
     *    | | |-"node.hpp"
     *    | | |-"iterator.hpp"
     *    | | >-"tree.hpp"
     *    | >-"bits"
     *    |   |-"except.hpp"
     *    |   >-"make_tree.hpp"
     *    |-"avl"
     *    |-"binary"
     *    |-"k_ary"
     *    >-"rose"
     *
     * with a depth-first-pre-order traversal of:
     * [ "include", "treelib", "detail", "base", "node.hpp", 
     *   "iterator.hpp", "tree.hpp", "bits", "except.hpp", "make_tree.hpp",
     *   "avl", "binary", "k_ary", "rose" ]
     *
     ***************************************************/
    int rose_tree_tests()
    {
        // using tree_type = rose_tree<std::string>;

        // tree_type my_dir;

        // tree_type::iterator include = my_dir.emplace(vrose::first, my_dir.croot(), "include");
        // tree_type::iterator tl = my_dir.emplace(vrose::first, include, "treelib");
        // tree_type::iterator detail = my_dir.emplace(vrose::first, tl, "detail");
        // my_dir.emplace(vrose::last, tl, "avl");
        // my_dir.emplace(vrose::last, tl, "binary");
        // my_dir.emplace(vrose::last, tl, "k_ary");
        // my_dir.emplace(vrose::last, tl, "rose");

        // for (const std::string& s : my_dir)
        //     std::println("{}", s);

        return 0;
    }
}


int main(int argc, char** argv) 
{   
    tl::outward_binary_tree_tests();
    tl::binary_tree_tests();
    tl::rose_tree_tests();
    return 0;
}