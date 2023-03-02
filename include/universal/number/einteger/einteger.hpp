// einteger arithmetic type standard header
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _EINTEGER_STANDARD_HEADER_
#define _EINTEGER_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(EINTEGER_ENABLE_LITERALS)
// default is to enable them
#define EINTEGER_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(EINTEGER_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define EINTEGER_THROW_ARITHMETIC_EXCEPTION 0
#endif

#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/einteger/einteger_impl.hpp>
#include <universal/number/einteger/numeric_limits.hpp>
#include <universal/number/einteger/exceptions.hpp>
#include <universal/number/einteger/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
//#include <universal/number/einteger/math_functions.hpp>

// no enable_if_einteger
// #include <universal/number/einteger/cmath_integration.hpp>

#endif
