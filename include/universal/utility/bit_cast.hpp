#pragma once
// bit_cast.hpp provides sw::bit_cast, a backport of C++20 std::bit_cast
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// C++20 introduced std::bit_cast, with clang, gcc and msvc implementing
// it with a builtin, __builtin_bit_cast(T,v), portable between compilers
// and available in < c++20 language modes.

// If compiler-provided constexpr-capable bit_cast is detected it's used,
// otherwise this header defines a non-constexpr bit_cast, useful as a
// UB-free alternative to 'type-punning', for trivially copyable types.

// The BIT_CAST_CONSTEXPR preprocessor symbol is defined as constexpr if
// sw::bit_cast is constexpr, otherwise it's defined as empty, allowing
// clients to propagate constexpr on functions that depend on bit_cast,
// and BIT_CAST_IS_CONSTEXPR symbol is defined as true or false.

// The earlier BIT_CAST_SUPPORT, and related CONSTEXPRESSION, symbols are
// now redundant and can be deprecated.

#if defined (BIT_CAST_SUPPORT) || defined (BIT_CAST)
static_assert(false, "BIT_CAST_SUPPORT is deprecated; bit_cast.hpp now defines "
                     "BIT_CAST_IS_CONSTEXPR and BIT_CAST_CONSTEXPR symbols, "
                     "and sw::is_bit_cast_constexpr_v bool variable");
#endif

#if defined __has_include && __cplusplus >= 202002L
#  if __has_include (<bit>)
#    include <bit>
#  endif
#endif

#include <cstring>
#include <type_traits>

#if __cpp_lib_bit_cast
#  define BIT_CAST using std::bit_cast
#  define BIT_CAST_CONSTEXPR constexpr
#  define BIT_CAST_IS_CONSTEXPR true
#elif defined(__has_builtin)
#  if __has_builtin(__builtin_bit_cast)
#    define BIT_CAST template <typename T, typename F> constexpr \
     T bit_cast(F v) noexcept { return __builtin_bit_cast(T,v); }
#    define BIT_CAST_CONSTEXPR constexpr
#    define BIT_CAST_IS_CONSTEXPR true
#  else
#    define BIT_CAST using non_builtin::bit_cast
#    define BIT_CAST_CONSTEXPR
#    define BIT_CAST_IS_CONSTEXPR false
#  endif
#else
#  define BIT_CAST using non_builtin::bit_cast
#  define BIT_CAST_CONSTEXPR
#  define BIT_CAST_IS_CONSTEXPR false
#endif

// If a compiler-provided constexpr bit_cast isn't detected then define
// non-constexpr non_builtin::bit_cast using memcpy
//
#if ! BIT_CAST_IS_CONSTEXPR
namespace non_builtin {

template <class From, class To>
using bit_castable_to_t = std::enable_if_t<
                          sizeof(From) == sizeof(To)
                          && std::is_trivially_copyable_v<From>
                          && std::is_trivially_copyable_v<To>, To>;

// Non-constexpr implementation of bit_cast, calls std::memcpy
// c.f. https://en.cppreference.com/w/cpp/numeric/bit_cast
//
template <class T, class F>
bit_castable_to_t<F,T> bit_cast(const F& src) noexcept
{
  static_assert(std::is_trivially_constructible_v<T>,
    "non_builtin::bitcast requires trivially constructible destination type");

  T dst;
  std::memcpy(&dst, &src, sizeof(T));
  return dst;
}

} // non_builtin
#endif

namespace sw {

BIT_CAST;

inline constexpr bool is_bit_cast_constexpr_v = BIT_CAST_IS_CONSTEXPR;

} // sw

#undef BIT_CAST

// BIT_CAST_SUPPORT drives the algorithm selection of ieee-754 decode

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */

#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION BIT_CAST_CONSTEXPR
#endif

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */

#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION BIT_CAST_CONSTEXPR
#endif

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */

#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION BIT_CAST_CONSTEXPR
#endif

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

