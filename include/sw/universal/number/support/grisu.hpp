#pragma once
// grisu.hpp: Grisu algorithm for floating-point to decimal string conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The Grisu algorithm (named after "Printing Floating-Point Numbers Quickly and Accurately with Integers" by Loitsch, 2010)
// provides fast conversion of binary floating-point numbers to decimal strings using cached powers of 10.
//
// Grisu is typically faster than Dragon but may fall back to slower methods in rare cases.
// This implementation works with Universal's internal triple representations.

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <universal/number/support/decimal.hpp>

namespace sw { namespace universal {

// Forward declarations
namespace internal {
	template<unsigned fbits> class value;
}

namespace grisu {

/// <summary>
/// grisu_context holds the configuration and state for Grisu algorithm decimal conversion
/// </summary>
struct grisu_context {
	std::ios_base::fmtflags flags;
	std::streamsize precision;
	bool use_scientific;
	bool use_fixed;
	bool show_pos;
	bool uppercase;

	grisu_context(std::ios_base::fmtflags f = std::ios_base::dec, std::streamsize prec = 6)
		: flags(f), precision(prec)
	{
		use_scientific = (flags & std::ios_base::scientific) != 0;
		use_fixed = (flags & std::ios_base::fixed) != 0;
		show_pos = (flags & std::ios_base::showpos) != 0;
		uppercase = (flags & std::ios_base::uppercase) != 0;

		// If both fixed and scientific are set, scientific takes precedence
		if (use_scientific && use_fixed) use_fixed = false;

		// Default precision
		if (precision <= 0) precision = 6;
	}
};

/// <summary>
/// grisu_fp represents a floating-point number in (sign, exponent, mantissa) form
/// The mantissa is an arbitrary-precision unsigned integer
/// The value represented is: (-1)^sign × mantissa × 2^exponent
/// </summary>
struct grisu_fp {
	bool sign;                      // true for negative
	int exponent;                   // binary exponent
	support::decimal mantissa;      // arbitrary-precision mantissa (as decimal digits)

	grisu_fp() : sign(false), exponent(0) {}

	grisu_fp(bool s, int e, const support::decimal& m)
		: sign(s), exponent(e), mantissa(m) {}

	void normalize() {
		mantissa.unpad();
	}

	bool iszero() const {
		return mantissa.iszero();
	}
};

/// <summary>
/// Cached power of 10 structure
/// Stores: 10^k ≈ significand × 2^binary_exponent
/// </summary>
struct cached_power {
	uint64_t significand;  // 64-bit approximation of 10^k
	int binary_exponent;   // Binary exponent
	int decimal_exponent;  // The actual k value
};

/// <summary>
/// get_cached_power: Returns a cached power of 10 for Grisu
/// For a given target exponent, returns the closest cached power
/// </summary>
inline cached_power get_cached_power(int target_exponent) {
	// Simplified cache: we'll compute on the fly for now
	// In a production implementation, this would use a precomputed table

	// Estimate decimal exponent
	constexpr double LOG10_2 = 0.301029995663981;
	int k = static_cast<int>(std::ceil((target_exponent + 63) * LOG10_2));

	// For simplicity, return a structure indicating we need to compute 10^k
	// In full Grisu, we'd have a lookup table of precomputed values
	cached_power cp;
	cp.decimal_exponent = k;
	cp.binary_exponent = target_exponent;
	cp.significand = 0;  // Will be computed

	return cp;
}

/// <summary>
/// multiply_by_power_of_2: multiply decimal by 2^exp using repeated doubling
/// </summary>
inline void multiply_by_power_of_2(support::decimal& d, int exp) {
	if (exp == 0) return;

	if (exp > 0) {
		// Multiply by 2^exp
		for (int i = 0; i < exp; ++i) {
			support::add(d, d); // d *= 2
		}
	}
}

/// <summary>
/// multiply_by_power_of_5: multiply decimal by 5^exp
/// </summary>
inline void multiply_by_power_of_5(support::decimal& d, int exp) {
	if (exp == 0) return;

	support::decimal five;
	five.setdigit(5);

	for (int i = 0; i < exp; ++i) {
		support::mul(d, five);
	}
}

/// <summary>
/// extract_decimal_digits_grisu: Grisu-based digit extraction
/// This uses the same approach as Dragon but with potential optimizations
/// </summary>
inline std::string extract_decimal_digits_grisu(const grisu_fp& fp, const grisu_context& ctx, int& decimal_exponent) {
	if (fp.iszero()) {
		decimal_exponent = 0;
		return std::string(static_cast<size_t>(ctx.precision), '0');
	}

	// The value is: mantissa × 2^exponent
	// We want to express this as: d.ddd... × 10^k
	//
	// Grisu approach: use cached powers of 10 to speed up conversion
	// For simplicity, we'll use the same algorithm as Dragon but structured for Grisu

	support::decimal r = fp.mantissa;
	int e2 = fp.exponent;
	int k = 0;

	// Transform based on sign of e2:
	// If e2 >= 0: r = mantissa × 2^e2
	// If e2 < 0:  r = mantissa × 5^(-e2), value is r × 10^e2
	if (e2 >= 0) {
		multiply_by_power_of_2(r, e2);
		k = 0;
	} else {
		// Negative exponent: multiply by 5^(-e2)
		multiply_by_power_of_5(r, -e2);
		k = e2;
	}

	// Extract ALL digits from r as a string
	std::stringstream ss;
	ss << r;
	std::string all_digits_str = ss.str();

	// Adjust k based on the number of digits in r
	k += static_cast<int>(all_digits_str.length()) - 1;

	decimal_exponent = k;

	// Extract the requested precision worth of digits
	std::string digits;
	int nrDigits = static_cast<int>(ctx.precision) + 1;  // +1 for rounding

	// Take the first nrDigits from all_digits_str
	for (int i = 0; i < nrDigits; ++i) {
		if (i < static_cast<int>(all_digits_str.length())) {
			digits += all_digits_str[static_cast<size_t>(i)];
		} else {
			digits += '0';  // Pad with zeros if we run out
		}
	}

	// Pad with zeros if needed
	while (digits.length() < static_cast<size_t>(nrDigits)) {
		digits += '0';
	}

	// Round the last digit
	if (nrDigits > 0 && static_cast<size_t>(nrDigits) < digits.length()) {
		if (digits[static_cast<size_t>(nrDigits)] >= '5') {
			// Round up
			int i = nrDigits - 1;
			while (i >= 0) {
				if (digits[static_cast<size_t>(i)] < '9') {
					digits[static_cast<size_t>(i)]++;
					break;
				} else {
					digits[static_cast<size_t>(i)] = '0';
					i--;
				}
			}
			if (i < 0) {
				// Carry out: insert '1' at front and adjust exponent
				digits = "1" + digits;
				decimal_exponent++;
			}
		}
	}

	// Trim to requested precision
	if (digits.length() > static_cast<size_t>(ctx.precision)) {
		digits = digits.substr(0, static_cast<size_t>(ctx.precision));
	}

	return digits;
}

/// <summary>
/// format_decimal_string: format the digits and exponent according to ioflags
/// </summary>
inline std::string format_decimal_string(bool sign, const std::string& digits, int decimal_exp, const grisu_context& ctx) {
	std::stringstream ss;

	// Sign
	if (sign) {
		ss << '-';
	} else if (ctx.show_pos) {
		ss << '+';
	}

	if (ctx.use_fixed) {
		// Fixed-point notation: ddd.ddd
		int integer_digits = decimal_exp + 1;

		if (integer_digits > 0) {
			// Number >= 1
			if (integer_digits <= static_cast<int>(digits.length())) {
				ss << digits.substr(0, static_cast<size_t>(integer_digits));
				if (ctx.precision > 0) {
					ss << '.';
					std::string frac = digits.substr(static_cast<size_t>(integer_digits));
					if (frac.length() < static_cast<size_t>(ctx.precision)) {
						frac.append(static_cast<size_t>(ctx.precision) - frac.length(), '0');
					}
					ss << frac.substr(0, static_cast<size_t>(ctx.precision));
				}
			} else {
				ss << digits;
				ss << std::string(static_cast<size_t>(integer_digits - digits.length()), '0');
				if (ctx.precision > 0) {
					ss << '.' << std::string(static_cast<size_t>(ctx.precision), '0');
				}
			}
		} else {
			// Number < 1
			ss << "0.";
			ss << std::string(static_cast<size_t>(-integer_digits), '0');
			if (digits.length() + static_cast<size_t>(-integer_digits) < static_cast<size_t>(ctx.precision)) {
				ss << digits;
				ss << std::string(static_cast<size_t>(ctx.precision) - digits.length() + static_cast<size_t>(integer_digits), '0');
			} else {
				ss << digits.substr(0, static_cast<size_t>(ctx.precision + integer_digits));
			}
		}
	} else {
		// Scientific notation: d.ddde±ee
		ss << digits[0];
		if (ctx.precision > 0 && digits.length() > 1) {
			ss << '.';
			std::string frac = digits.substr(1);
			if (frac.length() < static_cast<size_t>(ctx.precision)) {
				frac.append(static_cast<size_t>(ctx.precision) - frac.length(), '0');
			}
			ss << frac.substr(0, static_cast<size_t>(ctx.precision));
		}

		ss << (ctx.uppercase ? 'E' : 'e');
		ss << (decimal_exp >= 0 ? '+' : '-');
		int abs_exp = (decimal_exp >= 0) ? decimal_exp : -decimal_exp;
		if (abs_exp < 10) ss << '0';
		ss << abs_exp;
	}

	return ss.str();
}

/// <summary>
/// to_decimal_string: main entry point for Grisu algorithm conversion
/// Converts an arbitrary-precision floating-point triple to decimal string
/// </summary>
inline std::string to_decimal_string(bool sign, int scale, const support::decimal& mantissa, int fbits,
                                      std::ios_base::fmtflags flags = std::ios_base::dec,
                                      std::streamsize precision = 6) {
	grisu_context ctx(flags, precision);

	// Handle zero
	if (mantissa.iszero()) {
		std::stringstream ss;
		if (sign) ss << '-';
		else if (ctx.show_pos) ss << '+';
		ss << '0';
		if (ctx.precision > 0) {
			ss << '.' << std::string(static_cast<size_t>(ctx.precision), '0');
		}
		return ss.str();
	}

	// Create grisu_fp representation
	grisu_fp fp(sign, scale, mantissa);
	fp.normalize();

	// Extract decimal digits using Grisu approach
	int decimal_exp = 0;
	std::string digits = extract_decimal_digits_grisu(fp, ctx, decimal_exp);

	// Format according to ioflags
	return format_decimal_string(sign, digits, decimal_exp, ctx);
}

} // namespace grisu

}} // namespace sw::universal
