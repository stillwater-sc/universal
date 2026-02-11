// e8m0 arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
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
// enable throwing specific exceptions for e8m0 arithmetic errors
#if !defined(E8M0_THROW_ARITHMETIC_EXCEPTION)
#define E8M0_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/e8m0/exceptions.hpp>
#include <universal/number/e8m0/e8m0_impl.hpp>
#include <universal/traits/e8m0_traits.hpp>
#include <universal/number/e8m0/numeric_limits.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with e8m0
#include <universal/number/e8m0/manipulators.hpp>
#include <universal/number/e8m0/attributes.hpp>
