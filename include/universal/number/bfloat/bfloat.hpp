// bfloat arithmetic type standard header
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _BFLOAT_STANDARD_HEADER_
#define _BFLOAT_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(BFLOAT_ENABLE_LITERALS)
// default is to enable them
#define BFLOAT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(BFLOAT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/bfloat/exceptions.hpp>
#include <universal/number/bfloat/bfloat16_impl.hpp>
#include <universal/number/bfloat/numeric_limits.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with bfloats
#include <universal/number/bfloat/manipulators.hpp>
#include <universal/number/bfloat/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
//#include <universal/number/bfloat/math_functions.hpp>

#endif
