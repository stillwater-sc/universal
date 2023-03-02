// arbitrary decimal arithmetic type standard header
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _DECIMAL_STANDARD_HEADER_
#define _DECIMAL_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS

// compiler specific configuration for long double support
#include <universal/utility/long_double.hpp>
// compiler specific configuration for C++20 bit_cast
#include <universal/utility/bit_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(DECIMAL_ENABLE_LITERALS)
// default is to enable them
#define DECIMAL_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(DECIMAL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr as a signalling error
#define DECIMAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/decimal/exceptions.hpp>
#include <universal/number/decimal/decimal_impl.hpp>
#include <universal/number/decimal/numeric_limits.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/decimal/mathlib.hpp>


// no enable_if_decimal
// #include <universal/number/decimal/cmath_integration.hpp>

#endif
