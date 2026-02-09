// nvblock arithmetic type standard header
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
// enable throwing specific exceptions for nvblock arithmetic errors
// left to application to enable
#if !defined(NVBLOCK_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define NVBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// Prerequisites: microfloat must be included first (provides both element and scale types)
#include <universal/number/microfloat/microfloat.hpp>

#include <universal/number/nvblock/exceptions.hpp>
#include <universal/number/nvblock/nvblock_impl.hpp>
#include <universal/traits/nvblock_traits.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with nvblocks
#include <universal/number/nvblock/manipulators.hpp>
#include <universal/number/nvblock/attributes.hpp>
