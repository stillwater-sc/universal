// arbitrary configuration decimal floating-point arithmetic standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _DOUBLEDOUBLE_STANDARD_HEADER_
#define _DOUBLEDOUBLE_STANDARD_HEADER_

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
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/dd/exceptions.hpp>
#include <universal/number/dd/dd_fwd.hpp>
#include <universal/number/dd/dd_impl.hpp>
#include <universal/traits/dd_traits.hpp>
#include <universal/number/dd/numeric_limits.hpp>

// useful functions to work with doubledoubles
#include <universal/number/dd/manipulators.hpp>
#include <universal/number/dd/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/dd/mathlib.hpp>

#endif
