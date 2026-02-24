// positional integer standard header
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
#if !defined(POSITIONAL_ENABLE_LITERALS)
// default is to enable them
#define POSITIONAL_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for positional integer arithmetic errors
// left to application to enable
#if !defined(POSITIONAL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define POSITIONAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/positional/exceptions.hpp>
#include <universal/number/positional/positional_fwd.hpp>
#include <universal/number/positional/positional_impl.hpp>
#include <universal/traits/positional_traits.hpp>
#include <universal/number/positional/numeric_limits.hpp>

// useful functions to work with positional integers
#include <universal/number/positional/manipulators.hpp>
#include <universal/number/positional/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/positional/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for positional integer configurations
namespace sw { namespace universal {

	// Octal positional integers
	using oi4  = positional<4, 8>;
	using oi8  = positional<8, 8>;
	using oi16 = positional<16, 8>;
	using oi32 = positional<32, 8>;

	// Decimal positional integers
	using di4  = positional<4, 10>;
	using di8  = positional<8, 10>;
	using di16 = positional<16, 10>;
	using di32 = positional<32, 10>;
	using di64 = positional<64, 10>;

	// Hexadecimal positional integers
	using hi4  = positional<4, 16>;
	using hi8  = positional<8, 16>;
	using hi16 = positional<16, 16>;
	using hi32 = positional<32, 16>;

}}
