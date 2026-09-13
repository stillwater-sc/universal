// positional integer standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// POSITIONAL_ENABLE_LITERALS and POSITIONAL_THROW_ARITHMETIC_EXCEPTION now default in
/// positional_impl.hpp, so that a translation unit which includes core.hpp directly gets
/// the same defaults this umbrella used to supply (#1334, #1436). Defining either before
/// this header still wins, as before.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/positional/core.hpp>

// useful functions to work with positional integers
// layer 2a: the string producers -- type_tag, to_binary, color_print
#include <universal/number/positional/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<<
#include <universal/number/positional/iostream.hpp>
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
