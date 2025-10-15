#pragma once
// dragon.hpp: Dragon algorithm for floating-point to decimal string conversion
//
// Copyright (C) 2017-2025 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The Dragon algorithm (Steele & White, 1990) provides exact conversion of binary floating-point
// numbers to their shortest decimal representation that rounds back to the original value.
//
// Core Innovation: Uses interval arithmetic to maintain the valid range of decimal values
// that round to the original binary value, generating digits while staying within bounds.
//
// Algorithm:
//   Given: v = f × 2^e (where f is the significand, e is the exponent)
//   Find: The shortest decimal d₁d₂...dₙ × 10^k that rounds back to v
//
//   Method: Maintain three values (r, s, m⁺, m⁻) representing:
//     - r/s: The scaled value we're converting
//     - m⁺/s, m⁻/s: The upper and lower bounds of the rounding interval
//   Generate digits while r/s is within the interval (r-m⁻)/s to (r+m⁺)/s

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
/// Dragon context: configuration for decimal conversion
/// </summary>
struct dragon_context {
	std::ios_base::fmtflags flags;
	std::streamsize precision;
	bool use_scientific;
	bool use_fixed;
	bool show_pos;
	bool uppercase;
	bool shortest;  // Generate shortest representation

	dragon_context(std::ios_base::fmtflags f = std::ios_base::dec, std::streamsize prec = 6)
		: flags(f), precision(prec), shortest(false)
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
/// multiply_by_power_of_2: multiply decimal by 2^exp using repeated doubling
/// </summary>
inline void multiply_by_power_of_2(support::decimal& d, int exp) {
	if (exp <= 0) return;
	for (int i = 0; i < exp; ++i) {
		support::add(d, d); // d *= 2
	}
}

/// <summary>
/// multiply_by_power_of_5: multiply decimal by 5^exp
/// </summary>
inline void multiply_by_power_of_5(support::decimal& d, int exp) {
	if (exp <= 0) return;
	support::decimal five;
	five.setdigit(5);
	for (int i = 0; i < exp; ++i) {
		support::mul(d, five);
	}
}

/// <summary>
/// divide_by_power_of_10: divide decimal by 10^exp
/// </summary>
inline void divide_by_power_of_10(support::decimal& d, int exp) {
	if (exp <= 0) return;
	support::decimal ten;
	ten.setdigit(1);
	support::add(ten, ten);  // ten = 2
	support::decimal five;
	five.setdigit(5);
	support::mul(ten, five);  // ten = 10

	for (int i = 0; i < exp; ++i) {
		d = support::div(d, ten);
	}
}

/// <summary>
/// Dragon4 algorithm: Generate digits using interval arithmetic
///
/// The algorithm maintains:
///   r = numerator of the scaled value
///   s = denominator of the scaled value
///   mp = upper bound margin (m⁺)
///   mm = lower bound margin (m⁻)
///
/// The value v = r/s, and valid decimal representations lie in the interval:
///   (r - mm)/s  to  (r + mp)/s
///
/// We generate digits by computing d = floor(10r/s) and checking if we're
/// still within the valid interval after each digit.
///
/// Parameters:
///   f: mantissa as decimal integer
///   e: binary exponent (value = f × 2^e)
///   fbits: number of fraction bits in original binary representation
///   is_even: IEEE round-to-even flag
///   ctx: formatting context
///   decimal_exponent: output decimal exponent
/// </summary>
inline std::string dragon4(const support::decimal& f, int e, int fbits, bool is_even,
                           const dragon_context& ctx, int& decimal_exponent) {
	// f × 2^e is the value to convert
	// We need to scale it to the form r/s where we can generate decimal digits

	//std::cerr << "dragon4: f=" << f << " e=" << e << " fbits=" << fbits << "\n";

	support::decimal r = f;  // Numerator (will be scaled)
	support::decimal s;      // Denominator (will be scaled)
	s.setdigit(1);

	support::decimal mp;     // Upper margin m⁺
	mp.setdigit(1);

	support::decimal mm;     // Lower margin m⁻
	mm.setdigit(1);

	// Scale r, s, mp, mm based on the exponent e
	// Goal: Get r/s into a range where we can extract decimal digits

	if (e >= 0) {
		// Value = f × 2^e = (f × 2^e) / 1
		multiply_by_power_of_2(r, e);
		// mp and mm are already 1 (representing the ULP at this scale)
	} else {
		// Value = f × 2^e = f / 2^(-e)
		// Scale s by 2^(-e)
		multiply_by_power_of_2(s, -e);
	}

	// Estimate the decimal exponent k such that 10^k <= v < 10^(k+1)
	// Using log10(2) ≈ 0.30103
	// The value is f × 2^e, where f is approximately 2^fbits (hidden bit + fraction)
	constexpr double LOG10_2 = 0.301029995663981;
	// log10(f × 2^e) ≈ log10(2^(fbits+e)) = (fbits+e) × log10(2)
	int k = static_cast<int>(std::floor((e + fbits) * LOG10_2));

	// Adjust k to ensure we're in the right range
	// After scaling, we want 1 <= (r/s) / 10^k < 10
	// Loop to find correct k
	while (true) {
		// Compute r / (s × 10^k) by comparing r with s×10^k
		support::decimal s_times_10k = s;
		if (k > 0) {
			multiply_by_power_of_5(s_times_10k, k);
			multiply_by_power_of_2(s_times_10k, k);
		}

		support::decimal s_times_10k1 = s;
		int k1 = k + 1;
		if (k1 > 0) {
			multiply_by_power_of_5(s_times_10k1, k1);
			multiply_by_power_of_2(s_times_10k1, k1);
		}

		// Check if r/s / 10^k >= 10 (i.e., r >= s × 10^(k+1))
		if (k1 > 0 && support::lessOrEqual(s_times_10k1, r)) {
			k++;
			continue;
		}

		// Check if r/s / 10^k < 1 (i.e., r < s × 10^k)
		if (k > 0 && support::less(r, s_times_10k)) {
			k--;
			continue;
		}

		// For negative k, we need different comparison
		if (k < 0) {
			// r/(s×10^k) = r×10^(-k)/s
			support::decimal r_scaled = r;
			multiply_by_power_of_5(r_scaled, -k);
			multiply_by_power_of_2(r_scaled, -k);

			// Check if r_scaled/s >= 10
			support::decimal s_times_10 = s;
			multiply_by_power_of_5(s_times_10, 1);
			multiply_by_power_of_2(s_times_10, 1);
			if (support::lessOrEqual(s_times_10, r_scaled)) {
				k++;
				continue;
			}

			// Check if r_scaled/s < 1
			if (support::less(r_scaled, s)) {
				k--;
				continue;
			}
		}

		// k is in the correct range
		break;
	}

	// Scale r, s, mp, mm by 10^(-k) to normalize
	// This puts the value in the range [1, 10)
	if (k >= 0) {
		// Multiply s, mp, mm by 10^k
		multiply_by_power_of_5(s, k);
		multiply_by_power_of_2(s, k);
		multiply_by_power_of_5(mp, k);
		multiply_by_power_of_2(mp, k);
		multiply_by_power_of_5(mm, k);
		multiply_by_power_of_2(mm, k);
	} else {
		// Multiply r by 10^(-k)
		multiply_by_power_of_5(r, -k);
		multiply_by_power_of_2(r, -k);
	}

	decimal_exponent = k;

	// Now generate digits using the interval test
	std::string digits;
	int nrDigits = static_cast<int>(ctx.precision) + 3;  // Generate extra for rounding

	support::decimal ten;
	ten.setdigit(1);
	support::add(ten, ten);  // ten = 2
	support::decimal five;
	five.setdigit(5);
	support::mul(ten, five);  // ten = 10

	for (int i = 0; i < nrDigits; ++i) {
		//std::cerr << "  iter " << i << ": r=" << r << " s=" << s << "\n";

		// Compute digit d = floor(r/s)
		int digit = 0;
		support::decimal digit_times_s;
		digit_times_s.setzero();

		// Find the largest digit d such that d×s <= r
		for (int d = 9; d >= 0; --d) {
			digit_times_s.setzero();
			for (int mult = 0; mult < d; ++mult) {
				support::add(digit_times_s, s);
			}
			if (d == 0 || support::lessOrEqual(digit_times_s, r)) {
				digit = d;
				break;
			}
		}

		// r = r - d×s (the remainder)
		support::sub(r, digit_times_s);

		// Check if we're within bounds
		// Low test: r < mm (we're too close to the lower bound)
		// High test: r + mp > s (we're past the upper bound)
		bool low = support::less(r, mm);

		support::decimal r_plus_mp = r;
		support::add(r_plus_mp, mp);
		bool high = support::less(s, r_plus_mp);

		// Rounding decision
		if (low || high) {
			// We need to round. Compare 2r vs s to decide direction
			support::decimal two_r = r;
			support::add(two_r, r);

			if (support::less(two_r, s)) {
				// Round down
				digits += static_cast<char>('0' + digit);
			} else if (support::less(s, two_r)) {
				// Round up
				digits += static_cast<char>('0' + digit + 1);
			} else {
				// Exactly halfway - use IEEE round-to-even
				if (is_even) {
					// Round to even digit
					if (digit % 2 == 0) {
						digits += static_cast<char>('0' + digit);
					} else {
						digits += static_cast<char>('0' + digit + 1);
					}
				} else {
					// Round up
					digits += static_cast<char>('0' + digit + 1);
				}
			}
			break;  // Done generating digits
		}

		digits += static_cast<char>('0' + digit);

		// Prepare for next digit: multiply r, mp, mm by 10
		multiply_by_power_of_2(r, 1);      // r *= 2
		multiply_by_power_of_5(r, 1);      // r *= 5  (together: r *= 10)
		multiply_by_power_of_2(mp, 1);     // mp *= 2
		multiply_by_power_of_5(mp, 1);     // mp *= 5 (together: mp *= 10)
		multiply_by_power_of_2(mm, 1);     // mm *= 2
		multiply_by_power_of_5(mm, 1);     // mm *= 5 (together: mm *= 10)
	}

	// Handle carry if last digit rounded to 10
	for (size_t i = digits.length(); i > 0; --i) {
		if (digits[i-1] > '9') {
			digits[i-1] = '0';
			if (i == 1) {
				digits = "1" + digits;
				decimal_exponent++;
			} else {
				digits[i-2]++;
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
/// significant_digits: estimate the number of significant bits in the mantissa
/// This is a helper for the Dragon algorithm's initial k estimate
/// </summary>
inline int estimate_significant_bits(const support::decimal& d) {
	// Rough estimate: log2 of the decimal value
	// For now, use the string length as a proxy
	std::stringstream ss;
	ss << d;
	std::string str = ss.str();
	return static_cast<int>(str.length()) * 3 + 1;  // ~3.32 bits per decimal digit
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
/// </summary>
inline std::string to_decimal_string(bool sign, int scale, const support::decimal& mantissa, int fbits,
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

	// Run Dragon4 algorithm
	// The mantissa represents the significand, scale is the binary exponent
	// IEEE round-to-even: last bit of mantissa determines tie-breaking
	bool is_even = true;  // Assume even for now (would need to check last bit of mantissa)

	int decimal_exp = 0;
	std::string digits = dragon4(mantissa, scale, fbits, is_even, ctx, decimal_exp);

	// Format according to ioflags
	return format_decimal_string(sign, digits, decimal_exp, ctx);
}

} // namespace dragon

}} // namespace sw::universal
