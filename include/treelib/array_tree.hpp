#ifndef ARRAY_TREE_HPP
#define ARRAY_TREE_HPP

/**
 * @file    array_tree.hpp
 * @date    25/08/2025
 * @author  Julian Benzel
 *
 * @brief
 * C++ STL-like implementation of an arbitrary-ary tree data-structure.
 *
 * @details
 * in contrast to flex_tree, this tree handles it's nodes like a vector and keeps
 * them all allocated in a contigous memory-sequence. this allows for the fastest traversal possible
 * at the cost of high modification costs.
 *
 */
/********************************/
#include <concepts>
#include <memory>
#include <iterator>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <cassert>
/********************************/

namespace trl
{

    namespace detail__
    {

    }

    template <typename Type, typename Allocator = std::allocator<Type>>
    class array_tree
    {

    };

}

#endif