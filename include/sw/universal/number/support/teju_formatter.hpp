#pragma once
// teju_formatter.hpp: Format blocktriple values as decimal strings using the Teju Jagua algorithm
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Provides free functions to convert blocktriple values to formatted decimal
// strings using the Teju Jagua shortest-representation algorithm.
// These functions honor std::ostream formatting flags: fixed, scientific,
// precision, width, fill, showpos, uppercase, alignment.
// Note: defaultfloat (neither fixed nor scientific) is rendered as scientific.
// Hexfloat (both fixed and scientific) is not supported and falls back to scientific.
//
// Usage:
//   #include <universal/internal/blocktriple/blocktriple.hpp>
//   #include <universal/number/support/teju_formatter.hpp>
//
//   blocktriple<23, BlockTripleOperator::REP, uint32_t> bt(3.14f);
//   std::string s = sw::universal::teju_to_string(bt);
//   // or with formatting:
//   std::string s = sw::universal::teju_to_string(bt, 10, 0, false, true, false, false, false, false, ' ');
//
//   // or insert into an ostream (honors its formatting flags):
//   sw::universal::teju_insert(std::cout, bt);
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <universal/number/support/teju.hpp>

namespace sw { namespace universal {

namespace teju_detail {

// Format a decimal exponent as ±NN or ±NNN
inline void append_exponent(std::string& str, int e, bool uppercase) {
	str += uppercase ? 'E' : 'e';
	str += (e < 0) ? '-' : '+';
	int ae = (e < 0) ? -e : e;
	std::string digits;
	if (ae == 0) {
		digits = "00";
	}
	else {
		while (ae > 0) {
			digits += static_cast<char>('0' + ae % 10);
			ae /= 10;
		}
	}
	while (digits.length() < 2) digits += '0';
	std::reverse(digits.begin(), digits.end());
	str += digits;
}

// Extract decimal digits from an integer mantissa
inline std::vector<int> extract_digits(uint64_t m) {
	if (m == 0) return { 0 };
	std::vector<int> digits;
	while (m > 0) {
		digits.push_back(static_cast<int>(m % 10));
		m /= 10;
	}
	std::reverse(digits.begin(), digits.end());
	return digits;
}

// Format decimal digits with precision, applying rounding if needed
inline std::string format_scientific(const std::vector<int>& digits, int sci_exponent,
	std::streamsize precision, bool uppercase) {
	std::string s;

	// digits[0] is the MSD; sci_exponent is its power of 10
	// scientific format: d.ddddddde±EE
	auto needed = static_cast<size_t>(1 + precision); // 1 integer digit + precision fraction digits

	// Make a mutable copy and extend/round as needed
	std::vector<int> d(digits);

	if (d.size() > needed) {
		// Round at position 'needed'
		int round_digit = d[needed];
		if (round_digit >= 5) {
			// Propagate carry backwards
			for (int k = static_cast<int>(needed) - 1; k >= 0; --k) {
				d[static_cast<size_t>(k)]++;
				if (d[static_cast<size_t>(k)] < 10) break;
				d[static_cast<size_t>(k)] = 0;
				if (k == 0) {
					d.insert(d.begin(), 1);
					++sci_exponent;
				}
			}
		}
		d.resize(needed);
	}
	else {
		d.resize(needed, 0); // pad with trailing zeros
	}

	s += static_cast<char>('0' + d[0]);
	if (precision > 0) {
		s += '.';
		for (size_t i = 1; i < needed; ++i) {
			s += static_cast<char>('0' + d[i]);
		}
	}
	append_exponent(s, sci_exponent, uppercase);
	return s;
}

// Format decimal digits in fixed notation
inline std::string format_fixed(const std::vector<int>& digits, int sci_exponent,
	std::streamsize precision) {
	std::string s;

	auto needed_int = static_cast<long long>(sci_exponent + 1);
	auto needed_total = needed_int + precision;

	// Make a mutable copy
	std::vector<int> d(digits);

	if (needed_total < 1) needed_total = 1;
	if (static_cast<long long>(d.size()) > needed_total) {
		// Round at position needed_total
		int round_digit = d[static_cast<size_t>(needed_total)];
		if (round_digit >= 5) {
			for (auto k = needed_total - 1; k >= 0; --k) {
				d[static_cast<size_t>(k)]++;
				if (d[static_cast<size_t>(k)] < 10) break;
				d[static_cast<size_t>(k)] = 0;
				if (k == 0) {
					d.insert(d.begin(), 1);
					needed_int++;
					needed_total++;
					sci_exponent++;
				}
			}
		}
		d.resize(static_cast<size_t>(needed_total));
	}
	else {
		d.resize(static_cast<size_t>(needed_total), 0);
	}

	if (needed_int <= 0) {
		s += '0';
		if (precision > 0) {
			s += '.';
			for (long long z = 0; z < -needed_int && z < precision; ++z) s += '0';
			long long remaining = precision - (-needed_int);
			if (remaining < 0) remaining = 0;
			for (long long i = 0; i < remaining && i < static_cast<long long>(d.size()); ++i) {
				s += static_cast<char>('0' + d[static_cast<size_t>(i)]);
			}
			auto written = (-needed_int) + std::min(remaining, static_cast<long long>(d.size()));
			for (long long i = written; i < precision; ++i) s += '0';
		}
	}
	else {
		for (long long i = 0; i < needed_int; ++i) {
			if (i < static_cast<long long>(d.size()))
				s += static_cast<char>('0' + d[static_cast<size_t>(i)]);
			else
				s += '0';
		}
		if (precision > 0) {
			s += '.';
			for (long long i = needed_int; i < needed_int + precision; ++i) {
				if (i < static_cast<long long>(d.size()))
					s += static_cast<char>('0' + d[static_cast<size_t>(i)]);
				else
					s += '0';
			}
		}
	}
	return s;
}

// Apply width/fill/alignment to a formatted string
inline void apply_width(std::string& s, std::streamsize width, bool internal_align, bool left, char fill) {
	if (width > 0 && s.length() < static_cast<size_t>(width)) {
		size_t padding = static_cast<size_t>(width) - s.length();
		if (internal_align) {
			bool has_sign = !s.empty() && (s[0] == '-' || s[0] == '+');
			s.insert(has_sign ? 1u : 0u, padding, fill);
		}
		else if (left) {
			s.append(padding, fill);
		}
		else {
			s.insert(0u, padding, fill);
		}
	}
}

} // namespace teju_detail

/// Convert a blocktriple to a formatted decimal string using the Teju Jagua algorithm.
///
/// This is a free function that does NOT replace the existing blocktriple::to_string().
/// It provides an alternative implementation based on the Teju Jagua shortest-representation
/// algorithm. For types with mantissa width > 53 bits, it falls back to the existing
/// blocktriple::to_string() method.
///
/// @param v            The blocktriple value to format
/// @param precision    Number of digits after decimal point (default 6)
/// @param width        Minimum field width (default 0)
/// @param fixed        Use fixed-point notation
/// @param scientific   Use scientific notation
/// @param internal_align  Internal alignment (between sign and digits)
/// @param left         Left-align within field
/// @param showpos      Show '+' for positive values
/// @param uppercase    Use uppercase 'E' and 'INF'/'NAN'
/// @param fill         Fill character for width padding
/// @return             Formatted decimal string
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string teju_to_string(
	const blocktriple<fbits, op, bt>& v,
	std::streamsize precision = 6,
	std::streamsize width = 0,
	bool fixed = false,
	bool scientific = false,
	bool internal_align = false,
	bool left = false,
	bool showpos = false,
	bool uppercase = false,
	char fill = ' ')
{
	// Resolve floatfield: defaultfloat (neither set) and hexfloat (both set) are
	// not directly supported; both fall back to scientific notation.
	if (!fixed && !scientific) scientific = true;  // defaultfloat -> scientific
	if (fixed && scientific) fixed = false;        // hexfloat -> scientific

	std::string s;

	// Handle special values
	if (v.isnan()) {
		s = v.sign() ? (uppercase ? "SNAN" : "snan") : (uppercase ? "QNAN" : "qnan");
		teju_detail::apply_width(s, width, internal_align, left, fill);
		return s;
	}

	// Sign prefix
	if (v.sign()) {
		s += '-';
	}
	else if (showpos) {
		s += '+';
	}

	if (v.isinf()) {
		s += uppercase ? "INF" : "inf";
		teju_detail::apply_width(s, width, internal_align, left, fill);
		return s;
	}

	if (v.iszero()) {
		s += '0';
		if (precision > 0) {
			s += '.';
			s.append(static_cast<size_t>(precision), '0');
		}
		if (scientific) {
			teju_detail::append_exponent(s, 0, uppercase);
		}
		teju_detail::apply_width(s, width, internal_align, left, fill);
		return s;
	}

	// For types wider than 53 significand bits, fall back to exact conversion.
	// Use bfbits (the actual significand storage width) which varies by operator:
	// REP: fbits+2, ADD: fbits+6, MUL/DIV/SQRT: 2*fbits+2
	constexpr unsigned effective_mantissa_width = blocktriple<fbits, op, bt>::bfbits;
	if constexpr (effective_mantissa_width > 53) {
		// Fall back to blocktriple's built-in to_string (exact decimal arithmetic)
		return v.to_string(precision, width, fixed, scientific,
			internal_align, left, showpos, uppercase, fill);
	}
	else {
		// Extract binary mantissa and exponent from blocktriple
		uint64_t mantissa = v.significand_ull();
		int exponent = v.scale() - static_cast<int>(blocktriple<fbits, op, bt>::radix);

		// Call Teju Jagua core algorithm
		teju::decimal_fp dec = teju::to_decimal(mantissa, exponent, effective_mantissa_width);

		// Convert decimal mantissa to digit array
		auto digits = teju_detail::extract_digits(dec.mantissa);

		// sci_exponent: the power-of-10 of the MSD
		// dec.mantissa * 10^dec.exponent = d[0].d[1]d[2]... * 10^sci_exponent
		int sci_exponent = dec.exponent + static_cast<int>(digits.size()) - 1;

		// Format according to mode
		if (scientific) {
			s += teju_detail::format_scientific(digits, sci_exponent, precision, uppercase);
		}
		else {
			s += teju_detail::format_fixed(digits, sci_exponent, precision);
		}

		teju_detail::apply_width(s, width, internal_align, left, fill);
		return s;
	}
}

/// Stream inserter using Teju Jagua for blocktriple values.
/// Extracts formatting flags from the ostream and delegates to teju_to_string.
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::ostream& teju_insert(std::ostream& ostr, const blocktriple<fbits, op, bt>& v) {
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize prec = ostr.precision();
	std::streamsize w = ostr.width();
	char fillChar = ostr.fill();
	bool bShowpos    = (fmt & std::ios_base::showpos) != 0;
	bool bUppercase  = (fmt & std::ios_base::uppercase) != 0;
	bool bFixed      = (fmt & std::ios_base::fixed) != 0;
	bool bScientific = (fmt & std::ios_base::scientific) != 0;
	bool bInternal   = (fmt & std::ios_base::internal) != 0;
	bool bLeft       = (fmt & std::ios_base::left) != 0;
	ostr << teju_to_string(v, prec, w, bFixed, bScientific,
		bInternal, bLeft, bShowpos, bUppercase, fillChar);
	return ostr;
}

}} // namespace sw::universal
