
#ifndef TREELIB_BITS_EXCEPT_HPP
#define TREELIB_BITS_EXCEPT_HPP

/***************************************************
 * @file   treelib/detail/bits/except.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  named exception-types to be used
 *         in tree-related contexts.
 ***************************************************/

#include <stdexcept>

/***************************************************
 * @brief compile-time option to toggle exception-
 *        safety of some tree-operations such as
 *        insertion/erasure or traversal.
 *        usually there will be assertions as well
 *        which will be removed in release-builds.
 ***************************************************/

#ifdef _treelib_no_exceptions
    #define _treelib_noexcept noexcept
#else
    #define _treelib_noexcept
#endif

#define _treelib_noexcept_if(_expr) \
noexcept(noexcept(_expr))

#define _treelib_noexcept_if_member(_basename, _methodname) \
noexcept(noexcept(std::declval<_basename>()._methodname()))

namespace tl
{
    /***************************************************
     * @brief error-type to be thrown on invalid
     *        navigation-operations during tree-traversal
     *        (e.g. trying to find the parent of the
     *         root-node).
     ***************************************************/
    struct traversal_error 
        : public std::runtime_error
    { using std::runtime_error::runtime_error; };


    /***************************************************
     * @brief error-type to be thrown on invalid
     *        operations when modifying the structure
     *        of a tree (e.g. trying to erase a sub-branch
     *        where there is no actual node there and
     *        _treelib_no_exceptions is not defined).
     ***************************************************/
    struct modification_error 
        : public std::runtime_error
    { using std::runtime_error::runtime_error; };
}

#endif