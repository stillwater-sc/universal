#pragma once
// dragon.hpp: Dragon algorithm for floating-point to decimal string conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The Dragon algorithm (named after "Printing Floating-Point Numbers Quickly and Accurately" by Steele & White, 1990)
// provides exact conversion of binary floating-point numbers to decimal strings using arbitrary-precision arithmetic.
//
// This implementation works with Universal's internal triple representations:
//   - value<fbits>: (sign, scale, fraction without hidden bit) using bitblock
//   - blocktriple<fbits, op, bt>: (sign, scale, significant) using multi-limb blocks
//
// The algorithm avoids floating-point arithmetic entirely, using only integer operations to guarantee correctness.

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

namespace dragon {

/// <summary>
/// dragon_context holds the configuration and state for Dragon algorithm decimal conversion
/// </summary>
struct dragon_context {
	std::ios_base::fmtflags flags;
	std::streamsize precision;
	bool use_scientific;
	bool use_fixed;
	bool show_pos;
	bool uppercase;

	dragon_context(std::ios_base::fmtflags f = std::ios_base::dec, std::streamsize prec = 6)
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
/// dragon_fp represents a floating-point number in (sign, exponent, mantissa) form
/// The mantissa is an arbitrary-precision unsigned integer
/// The value represented is: (-1)^sign × mantissa × 2^exponent
/// </summary>
struct dragon_fp {
	bool sign;                      // true for negative
	int exponent;                   // binary exponent
	support::decimal mantissa;      // arbitrary-precision mantissa (as decimal digits)

	dragon_fp() : sign(false), exponent(0) {}

	dragon_fp(bool s, int e, const support::decimal& m)
		: sign(s), exponent(e), mantissa(m) {}

	// Normalize: ensure mantissa has no trailing zeros in its decimal representation
	void normalize() {
		mantissa.unpad();
	}

	bool iszero() const {
		return mantissa.iszero();
	}
};

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
	} else {
		// Division by 2^(-exp) is not supported here;
		// caller must handle negative exponents differently
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
/// divide_by_power_of_2: divide decimal by 2^exp using repeated halving
/// Note: This is integer division; any remainder is discarded
/// </summary>
inline void divide_by_power_of_2(support::decimal& d, int exp) {
	if (exp == 0) return;

	support::decimal two;
	two.setdigit(2);

	for (int i = 0; i < exp; ++i) {
		d = support::div(d, two);
	}
}

/// <summary>
/// extract_decimal_digits: extract digits from mantissa scaled appropriately
/// This is the core of the Dragon algorithm
///
/// Given a floating-point number f = mantissa × 2^exponent, generate decimal digits
/// Algorithm: We maintain r = mantissa × 2^e2 and compute v = r / 10^k iteratively
/// where k is chosen so that 1 <= v < 10
/// </summary>
inline std::string extract_decimal_digits(const dragon_fp& fp, const dragon_context& ctx, int& decimal_exponent) {
	if (fp.iszero()) {
		decimal_exponent = 0;
		return std::string(static_cast<size_t>(ctx.precision), '0');
	}

	// The value is: mantissa × 2^exponent
	// We want to express this as: d.ddd... × 10^k
	//
	// Key insight: mantissa × 2^e = mantissa × 5^e × 2^e / 5^e = mantissa × 5^e × 10^e / (2×5)^e
	//            = (mantissa × 5^e) / 10^(-e)  when e < 0
	//            = (mantissa × 2^e) × 10^0      when e >= 0

	support::decimal r = fp.mantissa;  // Start with the mantissa
	int e2 = fp.exponent;              // Binary exponent
	int k = 0;                         // Decimal exponent

	// Transform based on sign of e2:
	// If e2 >= 0: r = mantissa × 2^e2, and the value is r × 10^0
	// If e2 < 0:  r = mantissa × 5^(-e2), and the value is r × 10^e2
	if (e2 >= 0) {
		// Positive exponent: multiply by 2^e2
		multiply_by_power_of_2(r, e2);
		k = 0;
	} else {
		// Negative exponent: multiply by 5^(-e2)
		// This gives us: r = mantissa × 5^(-e2)
		// And the value is: r × 10^e2 = r / 10^(-e2)
		multiply_by_power_of_5(r, -e2);
		k = e2;  // Start with decimal exponent = binary exponent
	}

	// Create constants
	support::decimal ten;
	ten.setdigit(1);
	support::add(ten, ten);  // ten = 2
	support::decimal five;
	five.setdigit(5);
	support::mul(ten, five);  // ten = 10

	support::decimal one;
	one.setdigit(1);

	// Extract ALL digits from r (as an integer), then we'll place the decimal point
	std::stringstream ss;
	ss << r;
	std::string all_digits_str = ss.str();  // Get the string representation of r as an integer

	// Adjust k based on the number of digits in r
	k += static_cast<int>(all_digits_str.length()) - 1;

	decimal_exponent = k;

	// Now extract the requested precision worth of digits
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
inline std::string format_decimal_string(bool sign, const std::string& digits, int decimal_exp, const dragon_context& ctx) {
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
/// to_decimal_string: main entry point for Dragon algorithm conversion
/// Converts an arbitrary-precision floating-point triple to decimal string
/// </summary>
inline std::string to_decimal_string(bool sign, int scale, const support::decimal& mantissa,
                                      std::ios_base::fmtflags flags = std::ios_base::dec,
                                      std::streamsize precision = 6) {
	dragon_context ctx(flags, precision);

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

	// Create dragon_fp representation
	dragon_fp fp(sign, scale, mantissa);
	fp.normalize();

	// Extract decimal digits
	int decimal_exp = 0;
	std::string digits = extract_decimal_digits(fp, ctx, decimal_exp);

	// Format according to ioflags
	return format_decimal_string(sign, digits, decimal_exp, ctx);
}

} // namespace dragon

}} // namespace sw::universal
