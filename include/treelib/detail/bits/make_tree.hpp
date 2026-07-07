
#ifndef TREELIB_MAKE_TREE_HPP
#define TREELIB_MAKE_TREE_HPP

/**
 * @file    treelib/detail/make_tree.hpp
 * @author  Julian Benzel
 * @date    04.07.2026
 *
 * @brief   helper-types for tree-construction
 *          from initializer-lists for some
 *          sleek inline-syntax.
 *
 * @details trees that should be constructible
 *          like this need to satisfy certain
 *          requirements (see tl::constructible_from_tree_initializer).
 *          these basically define how syntax like:
 *
 *          binary_tree b = make_tree {
 *              "root" {
 *                  "l1",
 *                  "r1"
 *              }
 *          } 
 *
 *          is translated into a valid tree-structure.
 */

#include <utility>
#include <ranges>
#include <initializer_list>


namespace tl
{

    template <typename T>
    concept constructible_from_tree_initializer = requires()
    { true; };

    template <typename T>
    struct tree_initializer_traits;

    template <typename TreeType>
    struct node_initializer 
    { 

    };

    template <typename TreeType>
    [[nodiscard]]
    constexpr 
    TreeType&& make_tree(std::initializer_list<node_initializer<TreeType>>) {
        TreeType result;
        for (;; /* iterate recursively through initializer-lists*/){
            result.insert();
        }
        return std::move(result);
    }
}

#endif