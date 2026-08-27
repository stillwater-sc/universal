// double-double floating-point arithmetic standard header
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
#if !defined(DOUBLEDOUBLE_ENABLE_LITERALS)
// default is to enable them
#define DOUBLEDOUBLE_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#define DOUBLEDOUBLE_EXCEPT noexcept
#else
#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
#define DOUBLEDOUBLE_EXCEPT 
#else
#define DOUBLEDOUBLE_EXCEPT noexcept
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable native sqrt implementation
// 
#if !defined(DOUBLEDOUBLE_NATIVE_SQRT)
#define DOUBLEDOUBLE_NATIVE_SQRT 1
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
/// INCLUDE FILES that make up the library
///
/// Layered per #1334:
///     core.hpp          arithmetic, no streams
///     manipulators.hpp  type_tag / to_binary / to_triple
///     iostream.hpp      operator<< / operator>>
///
/// A translation unit that only computes can include core.hpp directly and skip the
/// stream headers entirely. to_string() and parse() are in the CORE, not the text
/// layers: assign(const std::string&) calls parse(), and to_string() concatenates a
/// std::string without ever touching a stream.
#include <universal/number/dd/core.hpp>
#include <universal/number/dd/manipulators.hpp>
#include <universal/number/dd/iostream.hpp>
#include <universal/number/dd/attributes.hpp>

/// the report builders stay at the umbrella: they pull <sstream>/<iomanip> by design
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

#include <universal/number/dd/math/constants/dd_constants.hpp>
#include <universal/number/dd/mathlib.hpp>
#include <universal/number/dd/mathext.hpp>
