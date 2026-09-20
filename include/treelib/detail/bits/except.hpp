
#ifndef TREELIB_BITS_EXCEPT_HPP
#define TREELIB_BITS_EXCEPT_HPP

/***************************************************
 * @file   treelib/detail/bits/except.hpp
 * @author Julian Benzel
 * @date   03.07.2026
 *
 * @brief  named exception-types to be used
 *         in tree-related contexts.
 *
 *         use #define _treelib_no_exception_specs to
 *         disable any macroed exception-specification,
 *         just in the case that they're too verbose
 *         or redundant to the compiler (idk).
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

/*******************************************************************************************
 * @brief C++20 __VA_OPT__ recursive-macro-folding based on
 *        the awesome article by David Mazières at:
 *        https://www.scs.stanford.edu/~dm/blog/va-opt.html
 *
 *        uses this to generate noexcept-specifiers for
 *        member functions following the pattern:
 *
 *          noexcept(noexcept(std::declval<type>().method(std::declval<args>()...)))
 *******************************************************************************************/

#define _treelib_parens ()

#define _treelib_expand(...) _treelib_expand0(_treelib_expand0(_treelib_expand0(_treelib_expand0(__VA_ARGS__))))
#define _treelib_expand0(...) __VA_ARGS__

#define _treelib_for_each_again() \
    _treelib_for_each_helper
#define _treelib_for_each_helper(_macro, _sep, _arg0, ...) \
    _macro(_arg0) __VA_OPT__(_sep _treelib_parens _treelib_for_each_again _treelib_parens (_macro, _sep, __VA_ARGS__))
#define _treelib_for_each(_macro, _sep, ...) \
    __VA_OPT__(_treelib_expand(_treelib_for_each_helper(_macro, _sep, __VA_ARGS__)))

#define _treelib_comma() ,
#define _treelib_declval(_typename) \
    std::declval<_typename>()

#define _treelib_declval_list(...) \
    _treelib_for_each(_treelib_declval, _treelib_comma, __VA_ARGS__)

#ifndef _treelib_no_exception_specs
    #define _treelib_is_member_noexcept(_typename, _membername, ...) \
        noexcept(std::declval<_typename>()._membername(_treelib_declval_list(__VA_ARGS__)))

    #define _treelib_noexcept_if(_expr) \
    noexcept(noexcept(_expr))

    #define _treelib_noexcept_if_member(_typename, _methodname, ...) \
    noexcept(_treelib_is_member_noexcept(_typename, _methodname, __VA_ARGS__))
#else
    #define _treelib_is_member_noexcept(...)
    #define _treelib_noexcept_if(...)
    #define _treelib_noexcept_if_member(...)
#endif

#define _treelib_has_member(_typename, _methodname, ...) \
(requires (_typename t) { t._methodname(_treelib_declval_list(__VA_ARGS__)); })

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