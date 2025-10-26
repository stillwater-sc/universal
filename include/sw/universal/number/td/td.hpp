// triple-double floating-point arithmetic standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
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
#if !defined(TRIPLEDOUBLE_ENABLE_LITERALS)
// default is to enable them
#define TRIPLEDOUBLE_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(TRIPLEDOUBLE_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define TRIPLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#define TRIPLEDOUBLE_EXCEPT noexcept
#else
#if TRIPLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
#define TRIPLEDOUBLE_EXCEPT 
#else
#define TRIPLEDOUBLE_EXCEPT noexcept
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable native sqrt implementation
// 
#if !defined(TRIPLEDOUBLE_NATIVE_SQRT)
#define TRIPLEDOUBLE_NATIVE_SQRT 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/td/exceptions.hpp>
#include <universal/number/td/td_fwd.hpp>
#include <universal/number/td/td_impl.hpp>
#include <universal/number/td/numeric_limits.hpp>
#include <universal/traits/td_traits.hpp>

// useful functions to work with triple-doubles
#include <universal/number/td/manipulators.hpp>
#include <universal/number/td/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/td/math/constants/td_constants.hpp>
#include <universal/number/td/mathlib.hpp>
