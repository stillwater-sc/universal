// mxfloat arithmetic type standard header
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
// enable throwing specific exceptions for mxfloat arithmetic errors
// left to application to enable
#if !defined(MXFLOAT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define MXFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// Prerequisites: microfloat and e8m0 must be included first
#include <universal/number/e8m0/e8m0.hpp>
#include <universal/number/microfloat/microfloat.hpp>

#include <universal/number/mxfloat/exceptions.hpp>
#include <universal/number/mxfloat/mxblock_impl.hpp>
#include <universal/traits/mxfloat_traits.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with mxblocks
#include <universal/number/mxfloat/manipulators.hpp>
#include <universal/number/mxfloat/attributes.hpp>
