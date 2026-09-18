
#include <treelib/binary>
#include <treelib/rose>

#include <print>
#include <string>
#include <cassert>

namespace tl
{
    /***************************************************
     * @brief tests the following outward-binary-tree:
     *
     *          1
     *         / \
     *        /   \
     *       2     5  
     *      / \   / \
     *     3   4 6   7
     *
     ***************************************************/
    void outward_binary_tree_tests()
    {
        using tree_type = outward_binary_tree<int>;
    
        tree_type my_tree(1);
        
        tree_type::iterator l  = my_tree.emplace(binary::left, my_tree.croot(), 2);
        tree_type::iterator ll = my_tree.emplace(binary::left, l, 3);
        tree_type::iterator lr = my_tree.emplace(binary::right, l, 4);

        tree_type::iterator r  = my_tree.emplace(binary::right, my_tree.croot(), 5);
        tree_type::iterator rl = my_tree.emplace(binary::left, r, 6);
        tree_type::iterator rr = my_tree.emplace(binary::right, r, 7);

        /***************************************************
         * @brief traversal-tests.
         ***************************************************/

        std::vector pre_order     { 1, 2, 3, 4, 5, 6, 7 };
        std::vector post_order    { 3, 4, 2, 6, 7, 5, 1 };
        std::vector breadth_first { 1, 2, 5, 3, 4, 6, 7 };

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first>(), my_tree.end(),
                                            pre_order.cbegin(), pre_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first_pre_order>(), my_tree.end(),
                                            pre_order.cbegin(), pre_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first_post_order>(), my_tree.qend<traversal::depth_first_post_order>(),
                                            post_order.cbegin(), post_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::breadth_first>(), my_tree.qend<traversal::breadth_first>(),
                                            breadth_first.cbegin(), breadth_first.cend(), std::equal_to()));

        std::println("{} :: traversal-tests passed", __FUNCTION__);

        /***************************************************
         * @brief modifier-tests.
         ***************************************************/

        std::println("{} :: modifier-tests passed", __FUNCTION__);

        /***************************************************
         * @brief constructor/assignment-operator-tests.
         ***************************************************/

        // tree_type my_tree_copy = my_tree;

        std::println("{} :: constructor/assignment-operator passed", __FUNCTION__);

        /***************************************************
         * @brief information-tests.
         ***************************************************/

        // assert(my_tree_copy == my_tree);

        std::println("{} :: information-tests passed", __FUNCTION__);
    }

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
     ***************************************************/
    void binary_tree_tests()
    {
        using tree_type = binary_tree<int>;
    
        tree_type my_tree(1);
        
        tree_type::const_iterator it = my_tree.begin();
        tree_type::iterator l  = my_tree.emplace(binary::left, my_tree.croot(), 2);
        tree_type::iterator ll = my_tree.emplace(binary::left, l, 3);
        tree_type::iterator lr = my_tree.emplace(binary::right, l, 4);

        tree_type::iterator r  = my_tree.emplace(binary::right, my_tree.croot(), 5);
        tree_type::iterator rl = my_tree.emplace(binary::left, r, 6);
        tree_type::iterator rr = my_tree.emplace(binary::right, r, 7);

        /***************************************************
         * @brief traversal-tests.
         ***************************************************/

        std::vector pre_order     { 1, 2, 3, 4, 5, 6, 7 };
        std::vector post_order    { 3, 4, 2, 6, 7, 5, 1 };
        std::vector breadth_first { 1, 2, 5, 3, 4, 6, 7 };

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first>(), my_tree.end(),
                                            pre_order.cbegin(), pre_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first_pre_order>(), my_tree.end(),
                                            pre_order.cbegin(), pre_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first_post_order>(), my_tree.qend<traversal::depth_first_post_order>(),
                                            post_order.cbegin(), post_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::breadth_first>(), my_tree.qend<traversal::breadth_first>(),
                                            breadth_first.cbegin(), breadth_first.cend(), std::equal_to()));

        std::println("{} :: traversal-tests passed", __FUNCTION__);

        /***************************************************
         * @brief modifier-tests.
         ***************************************************/

        std::println("{} :: modifier-tests passed", __FUNCTION__);

        /***************************************************
         * @brief constructor/assignment-operator-tests.
         ***************************************************/

        // tree_type my_tree_copy = my_tree;

        std::println("{} :: constructor/assignment-operator passed", __FUNCTION__);

        /***************************************************
         * @brief information-tests.
         ***************************************************/

        // assert(my_tree_copy == my_tree);

        std::println("{} :: information-tests passed", __FUNCTION__);
    }

    /***************************************************
     * @brief tests the following rose-tree:
     *
     * >-"include"
     *   >-"treelib"
     *    |-"detail"
     *    | >-"base"
     *    |   |-"node.hpp"
     *    |   |-"iterator.hpp"
     *    |   >-"tree.hpp"
     *    |-"avl"
     *    |-"binary"
     *    |-"k_ary"
     *    >-"rose"
     *
     ***************************************************/
    void rose_tree_tests()
    {
        using tree_type = rose_tree<std::string>;

        tree_type my_tree;

        tree_type::iterator include = my_tree.emplace(vrose::first, my_tree.croot(), "include");
        tree_type::iterator tl      = my_tree.emplace(vrose::first, include, "treelib");
        tree_type::iterator detail  = my_tree.emplace(vrose::first, tl, "detail");
        tree_type::iterator base    = my_tree.emplace(vrose::first, detail, "base");
        
        my_tree.emplace(vrose::last, base, "node.hpp");
        my_tree.emplace(vrose::last, base, "iterator.hpp");
        my_tree.emplace(vrose::last, base, "tree.hpp");

        my_tree.emplace(vrose::last, tl, "avl");
        my_tree.emplace(vrose::last, tl, "binary");
        my_tree.emplace(vrose::last, tl, "k_ary");
        my_tree.emplace(vrose::last, tl, "rose");

        /***************************************************
         * @brief traversal-tests.
         ***************************************************/

        std::vector pre_order     { "include", "treelib", "detail", "base", "node.hpp", 
                                    "iterator.hpp", "tree.hpp", "avl", "binary", "k_ary", "rose" };
        std::vector post_order    { "node.hpp", "iterator.hpp", "tree.hpp", "base", "detail",
                                    "avl", "binary", "k_ary", "rose", "treelib", "include" };
        std::vector breadth_first { "include", "treelib", "detail", "avl", "binary", "k_ary", "rose",
                                    "base", "node.hpp", "iterator.hpp", "tree.hpp" };

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first>(), my_tree.end(),
                                            pre_order.cbegin(), pre_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first_pre_order>(), my_tree.end(),
                                            pre_order.cbegin(), pre_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::depth_first_post_order>(), my_tree.qend<traversal::depth_first_post_order>(),
                                            post_order.cbegin(), post_order.cend(), std::equal_to()));

        assert(std::lexicographical_compare(my_tree.qbegin<traversal::breadth_first>(), my_tree.qend<traversal::breadth_first>(),
                                            breadth_first.cbegin(), breadth_first.cend(), std::equal_to()));

        std::println("{} :: traversal-tests passed", __FUNCTION__);

        /***************************************************
         * @brief modifier-tests.
         ***************************************************/

        std::println("{} :: modifier-tests passed", __FUNCTION__);

        /***************************************************
         * @brief constructor/assignment-operator-tests.
         ***************************************************/

        // tree_type my_tree_copy = my_tree;

        std::println("{} :: constructor/assignment-operator passed", __FUNCTION__);

        /***************************************************
         * @brief information-tests.
         ***************************************************/

        // assert(my_tree_copy == my_tree);

        std::println("{} :: information-tests passed", __FUNCTION__);

    }
}


int main(int argc, char** argv) 
{   
    tl::outward_binary_tree_tests();
    tl::binary_tree_tests();
    tl::rose_tree_tests();
    return 0;
}