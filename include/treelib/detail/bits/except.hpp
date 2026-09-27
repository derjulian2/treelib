
#ifndef TREELIB_BITS_EXCEPT_HPP
#define TREELIB_BITS_EXCEPT_HPP

/*********************************************************
 * @file   treelib/detail/bits/except.hpp
 * @author Julian Benzel
 * @date   25.09.2026
 *
 * @brief  named exception-types to be used
 *         in tree-related contexts and helper-macros
 *         for noexcept-specs.
 *
 * @details compile-options:
 *          - #define _treelib_no_exceptions
 *            toggle exception- safety of some tree-
 *            operations such as insertion/erasure or 
 *            traversal.
 *          - #define _treelib_no_exception_specs
 *            to disable any macroed exception-
 *            specifications just in case that they're
 *            too verbose or redundant to the compiler.
 *********************************************************/

#include <stdexcept>
#include <ranges>

/***************************************************
 * @brief helper-macroes noexcept-specifiers.
 ***************************************************/

#ifdef _treelib_no_exceptions
    #define _treelib_noexcept noexcept
#else
    #define _treelib_noexcept
#endif

/*************************************************************
 * @brief C++20 __VA_OPT__ recursive-macro-folding based on
 *        the awesome article by David Mazières at:
 *        https://www.scs.stanford.edu/~dm/blog/va-opt.html
 *************************************************************/

#define _treelib_parens ()

#define _treelib_expand(...) \
_treelib_expand0(_treelib_expand0(_treelib_expand0(_treelib_expand0(__VA_ARGS__))))

#define _treelib_expand0(...) __VA_ARGS__

#define _treelib_for_each_again() \
    _treelib_for_each_helper
#define _treelib_for_each_helper(_macro, _sep, _arg0, ...) \
    _macro(_arg0) __VA_OPT__(_sep _treelib_parens _treelib_for_each_again _treelib_parens (_macro, _sep, __VA_ARGS__))
#define _treelib_for_each(_macro, _sep, ...) \
    __VA_OPT__(_treelib_expand(_treelib_for_each_helper(_macro, _sep, __VA_ARGS__)))

#define _treelib_comma() ,

/*********************************************************************************
 * @brief use of macro-folding to generate member-
 *        function noexcept-checks of the pattern:
 *          noexcept(noexcept(declval<type>().member(declval<args>()...)))
 *********************************************************************************/

#define _treelib_declval(_typename) \
    std::declval<_typename>()

#define _treelib_declval_list(...) \
    _treelib_for_each(_treelib_declval, _treelib_comma, __VA_ARGS__)

#ifndef _treelib_no_exception_specs
    #define _treelib_member_noexcept(_typename, _membername, ...) \
        noexcept(std::declval<_typename>()._membername(_treelib_declval_list(__VA_ARGS__)))

    #define _treelib_noexcept_if(_expr) \
    noexcept(noexcept(_expr))

    #define _treelib_noexcept_if_member(_typename, _methodname, ...) \
    noexcept(_treelib_member_noexcept(_typename, _methodname, __VA_ARGS__))

    #define _treelib_noexcept_if_member_iterable(_typename, _methodname, ...) \
    noexcept( \
        _treelib_member_noexcept(_typename, _methodname, __VA_ARGS__) \
        && ::tl::_detail::_noexcept_iterable<decltype(std::declval<_typename>()._methodname(_treelib_declval_list(__VA_ARGS__)))> \
    )

    #define _treelib_noexcept_if_member_iterable_no_deref(_typename, _methodname, ...) \
    noexcept( \
        _treelib_member_noexcept(_typename, _methodname, __VA_ARGS__) \
        && ::tl::_detail::_noexcept_iterable_no_deref<decltype(std::declval<_typename>()._methodname(_treelib_declval_list(__VA_ARGS__)))> \
    )

    #define _treelib_noexcept_iterable(_expr) \
        ::tl::_detail::_noexcept_iterable<decltype(_expr)>

    #define _treelib_noexcept_iterable_no_deref(_expr) \
        ::tl::_detail::_noexcept_iterable_no_deref<decltype(_expr)>

    #define _treelib_noexcept_first_readable(_expr) \
        ::tl::_detail::_noexcept_first_readable<decltype(_expr)>

    #define _treelib_noexcept_last_readable(_expr) \
        ::tl::_detail::_noexcept_last_readable<decltype(_expr)>
#else
    #define _treelib_member_noexcept(...) (false)
    #define _treelib_noexcept_if(...)
    #define _treelib_noexcept_if_member(...)
    #define _treelib_noexcept_if_member_iterable(...)
    #define _treelib_noexcept_first_readable(_expr) (false)
    #define _treelib_noexcept_last_readable(_expr) (false)
#endif

#define _treelib_has_member(_typename, _methodname, ...) \
(requires (_typename t) { t._methodname(_treelib_declval_list(__VA_ARGS__)); })

#define _treelib_has_member_type(_typename, _membername) \
(requires () { typename _typename::_membername; })

namespace tl
{
    namespace _detail
    {
        /***************************************************
         * @brief helper-concepts for noexcept-specs.
         *        if this evaluates to true the iteration
         *        'for (it = begin; begin != end; ++it)'
         *        is noexcept.
         ***************************************************/
        template <typename R>
        concept _noexcept_iterable_no_deref
            = std::ranges::range<R>
            && noexcept(std::ranges::begin(std::declval<R&>()))
            && noexcept(std::ranges::end(std::declval<R&>()))
            && noexcept(++std::declval<std::ranges::iterator_t<R>>())
            && noexcept(std::declval<std::ranges::iterator_t<R>>() 
                        == std::declval<std::ranges::iterator_t<R>>());

        /********************************************************
         * @brief additionally requires '*it' to be noexcept.
         ********************************************************/
        template <typename R>
        concept _noexcept_iterable
            = _noexcept_iterable_no_deref<R>
            && noexcept(*std::declval<std::ranges::iterator_t<R>>());

        /********************************************************
         * @brief true if reading the first/last element of
         *        the range-type is noexcept.
         ********************************************************/
        template <typename R>
        concept _noexcept_first_readable
            = std::ranges::range<R>
            && noexcept(*std::ranges::begin(std::declval<R&>()));

        template <typename R>
        concept _noexcept_last_readable
            = std::ranges::range<R>
            && noexcept(*std::prev(std::ranges::end(std::declval<R&>())));
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