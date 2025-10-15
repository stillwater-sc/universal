#pragma once
// decimal_converter.hpp: Unified decimal conversion facility for all Universal floating-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This header provides a unified interface for converting Universal's internal floating-point representations
// (value<> and blocktriple<>) to decimal strings using the Dragon algorithm.
//
// Usage:
//   #include <universal/number/support/decimal_converter.hpp>
//
//   value<52> v = ...;
//   std::string s = sw::universal::to_decimal_string(v, std::ios::scientific, 15);
//
//   blocktriple<52, BlockTripleOperator::REP, uint32_t> bt = ...;
//   std::string s = sw::universal::to_decimal_string(bt, std::ios::fixed, 6);

#include <string>
#include <iostream>
#include <iomanip>
#include <universal/number/support/decimal.hpp>
#include <universal/number/support/dragon.hpp>

namespace sw { namespace universal {

// Forward declarations (to avoid circular dependencies)
namespace internal {
	template<unsigned fbits> class value;
}

///////////////////////////////////////////////////////////////////////////////
// Extraction functions: convert Universal triples to decimal mantissa
///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// extract_mantissa_from_value: convert value<fbits> fraction to decimal
/// value<> stores: (sign, scale, fraction_without_hidden_bit)
/// The value is: (-1)^sign × 1.fraction × 2^scale
/// We need to create a decimal representing the significand: 1.fraction = 1 + fraction
/// as an integer: (2^fbits + fraction_bits)
/// </summary>
template<unsigned fbits>
inline support::decimal extract_mantissa_from_value(const internal::value<fbits>& v) {
	support::decimal mantissa;

	if (v.iszero() || v.isinf() || v.isnan()) {
		mantissa.setzero();
		return mantissa;
	}

	// The value stores: fraction bits (without hidden bit)
	// The actual significand is: 1.ffff = (2^fbits + fraction_bits)
	// We convert the fraction bits to a decimal integer

	mantissa.setzero();
	support::decimal bit_value;
	bit_value.setdigit(1);

	// Add each fraction bit: if bit i is set, add 2^i (counting from LSB=0)
	for (unsigned i = 0; i < fbits; ++i) {
		if (v.fraction().test(i)) {
			support::add(mantissa, bit_value);
		}
		dragon::multiply_by_power_of_2(bit_value, 1); // bit_value *= 2 for next position
	}

	// Now add the hidden bit: 2^fbits
	support::add(mantissa, bit_value);  // bit_value is now 2^fbits

	return mantissa;
}

// Note: blocktriple support removed to avoid circular dependencies
// Add blocktriple conversion in a separate header if needed

///////////////////////////////////////////////////////////////////////////////
// Unified conversion API
///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// to_decimal_string: convert value<fbits> to decimal string representation
/// </summary>
template<unsigned fbits>
inline std::string to_decimal_string(const internal::value<fbits>& v,
                                      std::ios_base::fmtflags flags = std::ios_base::dec,
                                      std::streamsize precision = 6) {
	// Handle special cases
	if (v.isnan()) {
		return "nan";
	}
	if (v.isinf()) {
		return v.sign() ? "-inf" : (flags & std::ios_base::showpos ? "+inf" : "inf");
	}
	if (v.iszero()) {
		std::stringstream ss;
		if (v.sign()) ss << '-';
		else if (flags & std::ios_base::showpos) ss << '+';
		ss << '0';
		if (precision > 0) {
			ss << '.' << std::string(static_cast<size_t>(precision), '0');
		}
		return ss.str();
	}

	// Extract mantissa (represents the significand as an integer: 2^fbits + fraction_bits)
	support::decimal mantissa = extract_mantissa_from_value(v);

	// The value is: mantissa × 2^(scale - fbits)
	// Since mantissa represents (2^fbits + fraction), we need to account for that
	int adjusted_scale = v.scale() - static_cast<int>(fbits);

	// Use Dragon algorithm
	return dragon::to_decimal_string(v.sign(), adjusted_scale, mantissa, flags, precision);
}

// Note: blocktriple support removed to avoid circular dependencies
// Add blocktriple conversion in a separate header if needed

///////////////////////////////////////////////////////////////////////////////
// Stream insertion helpers
///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// decimal_format_inserter: helper to format and insert decimal strings into streams
/// Respects stream width, fill, and alignment
/// </summary>
inline std::ostream& decimal_format_inserter(std::ostream& ostr, const std::string& decimal_str) {
	std::streamsize width = ostr.width();

	if (width > static_cast<std::streamsize>(decimal_str.length())) {
		char fill = ostr.fill();
		std::streamsize padding = width - static_cast<std::streamsize>(decimal_str.length());

		if ((ostr.flags() & std::ios_base::left) == std::ios_base::left) {
			// Left-align
			ostr << decimal_str;
			for (std::streamsize i = 0; i < padding; ++i) ostr << fill;
		} else {
			// Right-align (default)
			for (std::streamsize i = 0; i < padding; ++i) ostr << fill;
			ostr << decimal_str;
		}
	} else {
		ostr << decimal_str;
	}

	// Reset width (per standard behavior)
	ostr.width(0);

	return ostr;
}

///////////////////////////////////////////////////////////////////////////////
// Note: Stream insertion operators are implemented in each type's own header
// to avoid conflicts. Use to_decimal_string() directly for conversion.
///////////////////////////////////////////////////////////////////////////////

}} // namespace sw::universal
