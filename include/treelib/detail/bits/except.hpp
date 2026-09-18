
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

#define _treelib_member_noexcept(_typename, _methodname) \
::tl::_detail::_is_member_noexcept(&_typename::_methodname)

#define _treelib_member_const_noexcept(_typename, _methodname) \
::tl::_detail::_is_member_const_noexcept(&_typename::_methodname)

#define _treelib_noexcept_if_member(_typename, _methodname) \
noexcept(_treelib_member_noexcept(_typename, _methodname))

#define _treelib_noexcept_if_const_member(_typename, _methodname) \
noexcept(_treelib_member_const_noexcept(_typename, _methodname))


namespace tl
{
    namespace _detail
    {
        /***************************************************
         * @brief compile-time check for noexcept-member-
         *        function without having to specify arguments.
         *        credits to this post, slightly altered:
         *
         *        Source - https://stackoverflow.com/a/56513026
         *        Posted by Artyer, modified by community. 
         *        See post 'Timeline' for change history.
         *        Retrieved 2026-09-18, License - CC BY-SA 4.0
         ***************************************************/

        template <typename RetT, typename T, typename... Args>
        consteval bool 
        _is_member_noexcept(RetT(T::*)(Args...) noexcept) 
            noexcept
        { return true; }

        template <typename RetT, typename T, typename... Args>
        consteval bool 
        _is_member_const_noexcept(RetT(T::*)(Args...) const noexcept) 
            noexcept
        { return true; }

        template <typename RetT, typename T, typename... Args>
        consteval bool 
        _is_member_noexcept(RetT(T::*)(Args...)) 
            noexcept
        { return false; }

        template <typename RetT, typename T, typename... Args>
        consteval bool 
        _is_member_const_noexcept(RetT(T::*)(Args...) const) 
            noexcept
        { return false; }
    }

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