// arbitrary configuration rational arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <cstdint>      // std::uint8_t, in the type aliases below
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries

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
/// INCLUDE FILES that make up the library
///
/// Layered per #1334:
///     core.hpp          arithmetic, no streams
///     manipulators.hpp  type_tag / to_binary / to_native
///     iostream.hpp      operator<< / operator>>
///
/// A translation unit that only computes can include core.hpp directly and skip the
/// stream headers entirely.
#include <universal/number/rational/core.hpp>
#include <universal/number/rational/manipulators.hpp>
#include <universal/number/rational/iostream.hpp>
#include <universal/number/rational/attributes.hpp>

/// the report builders stay at the umbrella: they pull <sstream>/<iomanip> by design
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/rational/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for rational configurations
namespace sw { namespace universal {

	// Binary rationals (base2) -- existing standard aliases
	using rb8   = rational<8, base2, std::uint8_t>;
	using rb16  = rational<16, base2, std::uint16_t>;
	using rb32  = rational<32, base2, std::uint32_t>;
	using rb64  = rational<64, base2, std::uint64_t>;
	using rb128 = rational<128, base2, std::uint32_t>;

	// Octal rationals (base8), use default digit size of 8 bits per digit
	using ro8  = rational<8, base8>;   
	using ro16 = rational<16, base8>;
    using ro32 = rational<32, base8>;
    using ro64 = rational<64, base8>;

	// Decimal rationals (base10), use default digit size of 8 bits per digit
	using rd8  = rational<8, base10>;
	using rd16 = rational<16, base10>;
	using rd32 = rational<32, base10>;
	using rd64 = rational<64, base10>;

	// Hexadecimal rationals (base16), use default digit size of 8 bits per digit
	using rh8  = rational<8, base16>;
	using rh16 = rational<16, base16>;
    using rh32 = rational<32, base16>;
    using rh64 = rational<64, base16>;

}}
