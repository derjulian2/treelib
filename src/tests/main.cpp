
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

        /***************************************************
         * initializer-tests.
         ***************************************************/

        using namespace initializers;

        tree_type my_tree(
            root_tree(
                node(1,
                {
                    node(2,
                    {
                        node(3),
                        node(4)
                    }),
                    node(5,
                    {
                        node(6),
                        node(7)
                    })
                })
            )
        );

        std::println("{} :: initializer-tests passed", __FUNCTION__);

        /***************************************************
         * traversal-tests.
         ***************************************************/

        std::vector pre_order     { 1, 2, 3, 4, 5, 6, 7 };
        std::vector post_order    { 3, 4, 2, 6, 7, 5, 1 };
        std::vector in_order      { 3, 2, 4, 1, 6, 5, 7 };
        std::vector breadth_first { 1, 2, 5, 3, 4, 6, 7 };

        assert(std::ranges::equal(my_tree.cbegin(), 
                                  my_tree.cend(), 
                                  pre_order.cbegin(), 
                                  pre_order.cend()));

        // assert(std::ranges::equal(my_tree.cqbegin<traversal::depth_first_post_order>(), 
        //                           my_tree.cqend<traversal::depth_first_post_order>(), 
        //                           post_order.cbegin(), 
        //                           post_order.cend()));

        // assert(std::ranges::equal(my_tree.cqbegin<traversal::depth_first_in_order>(), 
        //                           my_tree.cqend<traversal::depth_first_in_order>(), 
        //                           in_order.cbegin(), 
        //                           in_order.cend()));

        assert(std::ranges::equal(my_tree.cqbegin<traversal::breadth_first>(), 
                                  my_tree.cqend<traversal::breadth_first>(), 
                                  breadth_first.cbegin(), 
                                  breadth_first.cend()));

        std::println("{} :: traversal-tests passed", __FUNCTION__);

        /***************************************************
         * modifier-tests.
         ***************************************************/

        std::println("{} :: modifier-tests passed", __FUNCTION__);

        /***************************************************
         * constructor/assignment-operator-tests.
         ***************************************************/

        tree_type my_tree_copy(my_tree);

        assert(my_tree_copy == my_tree);

        tree_type my_tree_move = std::move(my_tree_copy);

        assert(my_tree_move == my_tree);

        std::println("{} :: constructor/assignment-operator passed", __FUNCTION__);

        /***************************************************
         * information-tests.
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
    
        /***************************************************
         * initializer-tests.
         ***************************************************/

        using namespace initializers;

        tree_type my_tree(
            root_tree(
                node(1,
                {
                    node(2,
                    {
                        node(3),
                        node(4)
                    }),
                    node(5,
                    {
                        node(6),
                        node(7)
                    })
                })
            )
        );

        std::println("{} :: initializer-tests passed", __FUNCTION__);

        /***************************************************
         * traversal-tests.
         ***************************************************/

        std::vector pre_order     { 1, 2, 3, 4, 5, 6, 7 };
        std::vector post_order    { 3, 4, 2, 6, 7, 5, 1 };
        std::vector breadth_first { 1, 2, 5, 3, 4, 6, 7 };

        assert(std::ranges::equal(my_tree.cbegin(), 
                                  my_tree.cend(), 
                                  pre_order.cbegin(), 
                                  pre_order.cend()));

        // assert(std::ranges::equal(my_tree.ctbegin<traversal::depth_first_post_order>(), 
        //                           my_tree.ctend<traversal::depth_first_post_order>(), 
        //                           post_order.cbegin(), 
        //                           post_order.cend()));

        // assert(std::ranges::equal(my_tree.cqbegin<traversal::breadth_first>(), 
        //                           my_tree.cqend<traversal::breadth_first>(), 
        //                           breadth_first.cbegin(), 
        //                           breadth_first.cend()));

        std::println("{} :: traversal-tests passed", __FUNCTION__);

        /***************************************************
         * modifier-tests.
         ***************************************************/

        std::println("{} :: modifier-tests passed", __FUNCTION__);

        /***************************************************
         * constructor/assignment-operator-tests.
         ***************************************************/

        tree_type my_tree_copy(my_tree);

        assert(my_tree_copy == my_tree);

        tree_type my_tree_move = std::move(my_tree_copy);

        assert(my_tree_move == my_tree);

        std::println("{} :: constructor/assignment-operator passed", __FUNCTION__);

        /***************************************************
         * information-tests.
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
    void outward_rose_tree_tests()
    {
        using tree_type = outward_rose_tree<std::string>;

        using namespace initializers;
        using snode = node<std::string>;

        tree_type t;

        tree_type my_tree(
            header_tree({
                snode("include",
                {
                    snode("treelib",
                    {
                        snode("detail",
                        {
                            snode("base",
                            {
                                snode("node.hpp"),
                                snode("iterator.hpp"),
                                snode("tree.hpp")
                            })
                        }),
                        snode("avl"),
                        snode("binary"),
                        snode("k_ary"),
                        snode("rose")
                    })
                })
            })
        );

        /***************************************************
         * traversal-tests.
         ***************************************************/

        std::vector pre_order     { "include", "treelib", "detail", "base", "node.hpp", 
                                    "iterator.hpp", "tree.hpp", "avl", "binary", "k_ary", "rose" };
        std::vector post_order    { "node.hpp", "iterator.hpp", "tree.hpp", "base", "detail",
                                    "avl", "binary", "k_ary", "rose", "treelib", "include" };
        std::vector breadth_first { "include", "treelib", "detail", "avl", "binary", "k_ary", "rose",
                                    "base", "node.hpp", "iterator.hpp", "tree.hpp" };

        assert(std::ranges::equal(my_tree.cbegin(), 
                                  my_tree.cend(), 
                                  pre_order.cbegin(), 
                                  pre_order.cend()));

        // assert(std::ranges::equal(my_tree.cqbegin<traversal::depth_first_post_order>(), 
        //                           my_tree.cqend<traversal::depth_first_post_order>(), 
        //                           post_order.cbegin(), 
        //                           post_order.cend()));

        assert(std::ranges::equal(my_tree.cqbegin<traversal::breadth_first>(), 
                                  my_tree.cqend<traversal::breadth_first>(), 
                                  breadth_first.cbegin(), 
                                  breadth_first.cend()));
                                            
        std::println("{} :: traversal-tests passed", __FUNCTION__);

        /***************************************************
         * modifier-tests.
         ***************************************************/

        std::println("{} :: modifier-tests passed", __FUNCTION__);

        /***************************************************
         * constructor/assignment-operator-tests.
         ***************************************************/

        tree_type my_tree_copy(my_tree);

        assert(my_tree_copy == my_tree);

        tree_type my_tree_move = std::move(my_tree_copy);

        assert(my_tree_move == my_tree);

        std::println("{} :: constructor/assignment-operator passed", __FUNCTION__);

        /***************************************************
         * information-tests.
         ***************************************************/

        assert(my_tree_copy == my_tree);

        std::println("{} :: information-tests passed", __FUNCTION__);
    }
}


int main(int argc, char** argv) 
{   
    tl::outward_binary_tree_tests();
    tl::binary_tree_tests();
    tl::outward_rose_tree_tests();
    return 0;
}