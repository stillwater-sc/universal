// arbitrary configuration classic floating-point arithmetic standard header
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _CFLOAT_STANDARD_HEADER_
#define _CFLOAT_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/bit_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(CFLOAT_ENABLE_LITERALS)
// default is to enable them
#define CFLOAT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(CFLOAT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable native sqrt implementation
// 
#if !defined(CFLOAT_NATIVE_SQRT)
#define CFLOAT_NATIVE_SQRT 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/numeric_limits.hpp>
#include <universal/number/cfloat/exceptions.hpp>
#include <universal/number/cfloat/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/cfloat/math_functions.hpp>

#endif
