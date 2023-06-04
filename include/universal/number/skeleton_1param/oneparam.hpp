// arbitrary one-parameter arithmetic type standard header
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _ONEPARAM_STANDARD_HEADER_
#define _ONEPARAM_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(ONEPARAM_ENABLE_LITERALS)
// default is to enable them
#define ONEPARAM_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(ONEPARAM_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define ONEPARAM_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/oneparam/oneparam_impl.hpp>
#include <universal/number/oneparam/numeric_limits.hpp>
#include <universal/number/oneparam/exceptions.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/oneparam/mathlib.hpp>

#endif
