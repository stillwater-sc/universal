// erational arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _ERATIONAL_STANDARD_HEADER_
#define _ERATIONAL_STANDARD_HEADER_

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
#if !defined(ERATIONAL_ENABLE_LITERALS)
// default is to enable them
#define ERATIONAL_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(ERATIONAL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define ERATIONAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/erational/exceptions.hpp>
#include <universal/number/erational/erational_fwd.hpp>
#include <universal/number/erational/erational_impl.hpp>
#include <universal/number/erational/numeric_limits.hpp>
#include <universal/traits/erational_traits.hpp>

// useful functions to work with rationals
#include <universal/number/erational/manipulators.hpp>
#include <universal/number/erational/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/erational/mathlib.hpp>

#endif
