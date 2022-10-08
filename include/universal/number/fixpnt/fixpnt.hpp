// arbitrary fixed-point arithmetic type standard header
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _FIXPNT_STANDARD_HEADER_
#define _FIXPNT_STANDARD_HEADER_

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
#if !defined(FIXPNT_ENABLE_LITERALS)
// default is to enable them
#define FIXPNT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(FIXPNT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/fixpnt/exceptions.hpp>
#include <universal/number/fixpnt/fixpnt_impl.hpp>
#include <universal/traits/fixpnt_traits.hpp>
#include <universal/number/fixpnt/numeric_limits.hpp>

// useful functions to work with fixpnts
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/fixpnt/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/fixpnt/mathlib.hpp>

#endif
