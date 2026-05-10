
#ifndef _TREELIB_UTILITY_HPP_
#define _TREELIB_UTILITY_HPP_

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <utility>


namespace tl
{
    /**
     * @brief abbreviated integral and floating-point types.
     */

    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using f32  = float;
    using f64  = double;
    using f128 = long double;

    static_assert(sizeof(f32)  ==  32 / 8);
    static_assert(sizeof(f64)  ==  64 / 8);
    static_assert(sizeof(f128) == 128 / 8);


    /**
     * @brief   folds the passed function over 
     *          every argument of the template-parameter-pack,
     *          forcing the 'loop' to be unfolded at compile-time. 
     *
     * @example the following code can compile to
     *          several different programs if the
     *          constexpr-variables ´a´ and ´b´ are of certain values:
     *          @code
     *          constexpr int a = ..., b = ... ;
     *          tl::for_pack<int, a, b>(
     *              [&](auto i)
     *              {
     *                  if constexpr (i == 2)
     *                  { ... }
     *                  else
     *                  { ... }
     *          });
     *          @endcode
     *
     * @details i believe C++26 has 'template for (...)' which is something like this,
     *          but as a language feature.
     *          also see this stack-overflow post that taught me how
     *          to encode values with types using std::integral_constant:
     *          https://stackoverflow.com/questions/56937863/constexpr-lambda-argument
     */
    template <typename T, T... Args, typename Fn>
        requires (std::invocable<Fn, std::integral_constant<T, Args>> && ...)
    constexpr
    void for_pack(Fn&& fn) 
    noexcept((... && noexcept(fn(std::integral_constant<T, Args>()))))
    { (fn(std::integral_constant<T, Args>()), ...); }


    /**
     * @brief   folds the passed function over
     *          each element of the passed integer-sequence.
     *          forcing the 'loop' to be unfolded at compile-time.
     * @details (see tl::for_pack(...) for details)
     */
    template <typename T, T... Ints, typename Fn>
    constexpr 
    void for_iseq(Fn&& fn, std::integer_sequence<T, Ints...>)
    noexcept(noexcept(for_pack<T, Ints...>(fn)))
    { for_pack<T, Ints...>(fn); }


    /**
     * @brief   folds the passed function over
     *          an index-sequence from 0 to N-1,
     *          forcing the 'loop' to be unfolded at compile-time.
     * @details (see tl::for_pack(...) for details)
     */
    template <std::size_t N, typename Fn>
    constexpr 
    void for_n(Fn&& fn)
    noexcept(noexcept(for_iseq(fn, std::make_index_sequence<N>())))
    { for_iseq(fn, std::make_index_sequence<N>()); }


}

#endif