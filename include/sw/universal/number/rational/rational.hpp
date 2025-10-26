// arbitrary one-param number arithmetic type standard header
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
#if !defined(RATIONAL_ENABLE_LITERALS)
// default is to enable them
#define RATIONAL_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(RATIONAL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define RATIONAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/rational/exceptions.hpp>
#include <universal/number/rational/rational_fwd.hpp>
#include <universal/number/rational/rational_impl.hpp>
#include <universal/traits/rational_traits.hpp>
#include <universal/number/rational/numeric_limits.hpp>

// useful functions to work with rational numbers
#include <universal/number/rational/manipulators.hpp>
#include <universal/number/rational/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/rational/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for industry standard floating point configurations
namespace sw { namespace universal {

	// rational binary of 8bits
	using rb8 = rational<8, std::uint8_t>;
	// rational binary of 16bits
	using rb16 = rational<16, std::uint16_t>;
	// rational binary of 32bits
	using rb32 = rational<32, std::uint32_t>;
	// rational binary of 64bits
	using rb64 = rational<64, std::uint64_t>;
	// rational binary of 128bits
	using rb128 = rational<128, std::uint32_t>;

}}
