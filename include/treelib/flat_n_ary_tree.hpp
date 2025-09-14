#ifndef TRL_FLAT_N_ARY_TREE_HPP
#define TRL_FLAT_N_ARY_TREE_HPP

/**
 * @file    flat_n_ary_tree.hpp
 * @date    25/08/2025
 * @author  Julian Benzel
 *
 * @brief
 * C++ STL-like implementation of an n-ary tree data-structure.
 *
 * @details
 * 
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

    template <typename Type, std::size_t Size, 
            typename Allocator = std::allocator<Type>>
    class flat_n_ary_tree
    {

    };

}

#endif