
#ifndef TREELIB_EXCEPT_HPP
#define TREELIB_EXCEPT_HPP

/**
 * @file   treelib/detail/except.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  named exception-types to be used
 *         in tree-related contexts.
 */

#include <stdexcept>

namespace tl
{
    /**
     * @brief error-type to be thrown on invalid
     *        navigation-operations during tree-traversal.
     */
    struct traversal_error 
        : public std::runtime_error
    { using std::runtime_error::runtime_error; };


    /**
     * @brief error-type to be thrown on invalid
     *        operations when modifying the structure
     *        of a tree.
     */
    struct modification_error 
        : public std::runtime_error
    { using std::runtime_error::runtime_error; };
}

#endif