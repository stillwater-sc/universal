#pragma once
// ieee754_components.hpp: sign/exponent/fraction extraction from native float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// THE PATTERN (#1334): ieee754_float.hpp and ieee754_double.hpp are named like
// core headers and are included by everything, but their contents are almost
// entirely TEXT -- to_hex, to_binary, to_triple, to_base2_scientific,
// color_print. Carrying those meant <sstream> and <iomanip> entered the include
// graph of every translation unit that merely decodes a native float.
//
// The one genuinely core function in each was ieee_components(), so that is what
// moved here. This header is 17k preprocessed lines with ZERO stream headers,
// where the two it came from cost ~62k with four.
#include <cstdint>
#include <tuple>
#include <limits>
#include <universal/native/ieee754_decoder.hpp>

namespace sw { namespace universal {

// ieee_components returns a tuple of sign, exponent, and fraction
inline std::tuple<bool, int, uint32_t> ieee_components(float fp)
{
	static_assert(std::numeric_limits<float>::is_iec559,
		"This function only works when float complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");

	float_decoder fd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int, uint32_t>(
		static_cast<bool>(fd.parts.sign), 
		static_cast<int>(fd.parts.exponent),
		static_cast<uint32_t>(fd.parts.fraction) 
	);
}

// ieee_components returns a tuple of sign, exponent, and fraction
inline std::tuple<bool, int, std::uint64_t> ieee_components(double fp)
{
	static_assert(std::numeric_limits<double>::is_iec559,
		"This function only works when double complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");

	double_decoder dd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int, std::uint64_t>(
		static_cast<bool>(dd.parts.sign), 
		static_cast<int>(dd.parts.exponent),
		static_cast<std::uint64_t>(dd.parts.fraction) 
	);
}

}} // namespace sw::universal
