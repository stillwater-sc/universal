// eposit arithmetic type standard header
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _EPOSIT_STANDARD_HEADER_
#define _EPOSIT_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(EPOSIT_ENABLE_LITERALS)
// default is to enable them
#define EPOSIT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(EPOSIT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define EPOSIT_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/eposit/eposit_impl.hpp>
//#include <universal/number/eposit/numeric_limits.hpp>
#include <universal/number/eposit/exceptions.hpp>
//#include <universal/number/eposit/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
//#include <universal/number/eposit/math_functions.hpp>

#endif
