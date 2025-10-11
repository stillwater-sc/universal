// <universal/integer/integer>: arbitrary integer arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/compiler.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

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

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/integer/exceptions.hpp>
#include <universal/number/integer/integer_fwd.hpp>
#include <universal/number/integer/integer_impl.hpp>
#include <universal/traits/integer_traits.hpp>
#include <universal/number/integer/numeric_limits.hpp>

/// useful functions to work with integers
#include <universal/number/integer/primes.hpp>
#include <universal/number/integer/sieves.hpp>
#include <universal/number/integer/manipulators.hpp>
#include <universal/number/integer/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math library specialized for integer<>
#include <universal/number/integer/mathlib.hpp>
