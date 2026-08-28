// quad-double floating-point arithmetic standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(QUADDOUBLE_ENABLE_LITERALS)
// default is to enable them
#define QUADDOUBLE_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#define QUADDOUBLE_EXCEPT noexcept
#else
#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
#define QUADDOUBLE_EXCEPT 
#else
#define QUADDOUBLE_EXCEPT noexcept
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// configure the library implementation
#if !defined(QUADDOUBLE_NATIVE_SQRT)
#define QUADDOUBLE_NATIVE_SQRT 1
#endif
#if !defined(QUADDOUBLE_NATIVE_TRIGONOMETRY)
#define QUADDOUBLE_NATIVE_TRIGONOMETRY 1
#endif
#if !defined(QUADDOUBLE_NATIVE_HYPERBOLIC)
#define QUADDOUBLE_NATIVE_HYPERBOLIC 1
#endif
#if !defined(QUADDOUBLE_NATIVE_MINMAX)
#define QUADDOUBLE_NATIVE_MINMAX 1
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
/// INCLUDE FILES that make up the library
///
/// Layered per #1334:
///     core.hpp          arithmetic, no streams
///     manipulators.hpp  type_tag / to_quad / to_triple / to_binary / to_native / to_components
///     iostream.hpp      operator<< / operator>>
///
/// A translation unit that only computes can include core.hpp directly and skip the
/// stream headers entirely. to_string() and parse() are in the CORE, not the text
/// layers: assign(const std::string&) calls parse(), and to_string() concatenates a
/// std::string without ever touching a stream.
#include <universal/number/qd/core.hpp>
#include <universal/number/qd/manipulators.hpp>
#include <universal/number/qd/iostream.hpp>
#include <universal/number/qd/attributes.hpp>

/// the report builders stay at the umbrella: they pull <sstream>/<iomanip> by design
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

#include <universal/number/qd/math/constants/qd_constants.hpp>
#include <universal/number/qd/mathlib.hpp>
#include <universal/number/qd/mathext.hpp>
