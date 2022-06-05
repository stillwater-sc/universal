// arbitrary logarithmic number arithmetic type standard header
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _LNS_STANDARD_HEADER_
#define _LNS_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS

// compiler specific configuration for long double support
#include <universal/utility/long_double.hpp>
// compiler specific configuration for C++20 bit_cast
#include <universal/utility/bit_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(LNS_ENABLE_LITERALS)
// default is to enable them
#define LNS_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(LNS_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/lns/exceptions.hpp>
#include <universal/number/lns/lns_impl.hpp>
#include <universal/number/lns/lns_traits.hpp>
#include <universal/number/lns/numeric_limits.hpp>

// useful functions to work with logarithmic numbers
#include <universal/number/lns/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/lns/math_functions.hpp>

#endif
