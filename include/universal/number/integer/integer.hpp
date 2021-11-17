// <universal/integer/integer>: arbitrary integer arithmetic type standard header
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _INTEGER_STANDARD_HEADER_
#define _INTEGER_STANDARD_HEADER_

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
#if !defined(INTEGER_ENABLE_LITERALS)
// default is to enable them
#define INTEGER_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(INTEGER_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr as a signalling error
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/integer/exceptions.hpp>
#include <universal/number/integer/integer_impl.hpp>
#include <universal/number/integer/numeric_limits.hpp>

#include <universal/number/integer/primes.hpp>
#include <universal/number/integer/sieves.hpp>
#include <universal/number/integer/manipulators.hpp>
#include <universal/number/integer/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/integer/math_functions.hpp>

#endif
