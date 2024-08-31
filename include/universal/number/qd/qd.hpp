// quad-double floating-point arithmetic standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _QUADDOUBLE_STANDARD_HEADER_
#define _QUADDOUBLE_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/compiler.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 
#include <iostream>
#include <iomanip>

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
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/qd/exceptions.hpp>
#include <universal/number/qd/qd_fwd.hpp>
#include <universal/number/qd/qd_impl.hpp>
#include <universal/traits/qd_traits.hpp>
#include <universal/number/qd/numeric_limits.hpp>

// useful functions to work with doubledoubles
#include <universal/number/qd/manipulators.hpp>
#include <universal/number/qd/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/qd/mathlib.hpp>
#include <universal/number/qd/mathext.hpp>

#endif
