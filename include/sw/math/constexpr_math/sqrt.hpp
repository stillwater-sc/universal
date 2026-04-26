#pragma once
// sqrt.hpp: constexpr square root for IEEE-754 float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Algorithm
// -----------------------------------------------------------------------------
// Decompose x = 2^E * M with M in [1, 2). If E is odd, absorb a factor of 2
// into M (giving M in [2, 4)) and decrement E so it becomes even. Then
//
//     sqrt(x) = sqrt(M) * 2^(E/2)
//
// where 2^(E/2) is constructed exactly via detail::pow2 and sqrt(M) is
// computed via Newton's iteration
//
//     y_{n+1} = 0.5 * (y_n + M / y_n)
//
// which has quadratic convergence. From a constant initial guess of 1.5
// (worst-case ~2 bits accurate over M in [1, 4)), six iterations reach
// ~96 bits of precision -- well past double's 53-bit significand. Two extra
// over the strict minimum keeps us safe against the rounding mode of any
// individual multiply/add.
//
// Self-contained: only depends on detail::pow2 (already in the facility) and
// std::bit_cast.

#include <bit>
#include <cstdint>
#include <limits>

#include <math/constexpr_math/detail.hpp>

namespace sw { namespace math { namespace constexpr_math {

// constexpr sqrt(double): returns the square root of a.
// Special values follow IEEE-754 conventions:
//   sqrt(NaN)   -> NaN
//   sqrt(x<0)   -> NaN  (including -inf)
//   sqrt(+/-0)  -> +/-0  (sign preserved)
//   sqrt(+inf)  -> +inf
constexpr double sqrt(double a) {
	if (a != a) return a;                                                // NaN propagation
	if (a < 0.0) return std::numeric_limits<double>::quiet_NaN();
	if (a == 0.0) return a;                                              // +/-0 preserved
	if (a == std::numeric_limits<double>::infinity()) return a;

	// Decompose a = 2^E * M with M in [1, 2). Mirrors log2/exp2's bit handling.
	std::uint64_t bits = std::bit_cast<std::uint64_t>(a);
	int exp_field = static_cast<int>((bits >> 52) & 0x7FFu);

	int subnormal_shift = 0;
	if (exp_field == 0) {
		std::uint64_t mant = bits & ((std::uint64_t{1} << 52) - 1);
		while ((mant & (std::uint64_t{1} << 52)) == 0) {
			mant <<= 1;
			++subnormal_shift;
		}
		bits = mant;
		exp_field = 1;
	}

	int E = exp_field - 1023 - subnormal_shift;
	std::uint64_t m_bits = (bits & ((std::uint64_t{1} << 52) - 1))
	                       | (std::uint64_t{1023} << 52);
	double M = std::bit_cast<double>(m_bits);                            // M in [1, 2)

	// If E is odd, absorb a factor of 2 into M so 2^(E/2) is an integer power.
	// Bitwise & 1 correctly identifies odd magnitudes for both signs in two's
	// complement; E -= 1 brings odd-positive down to even and odd-negative
	// further down to even.
	if (E & 1) {
		M *= 2.0;
		E -= 1;
	}
	// Now E is even and M is in [1, 4).

	// Newton's iteration for sqrt(M). Quadratic convergence: starting from
	// 1.5 (worst-case error ~2 bits over [1, 4)), each iteration roughly
	// doubles the bit-precision: 2 -> 4 -> 8 -> 16 -> 32 -> 64 -> 128.
	// Six iterations reach well past double's 53-bit significand.
	double y = 1.5;
	y = 0.5 * (y + M / y);
	y = 0.5 * (y + M / y);
	y = 0.5 * (y + M / y);
	y = 0.5 * (y + M / y);
	y = 0.5 * (y + M / y);
	y = 0.5 * (y + M / y);

	return y * detail::pow2(E / 2);
}

// constexpr sqrt(float): same algorithm at lower precision.
// 4 Newton iterations from 1.5 reach ~32 bits, well past float's 24-bit
// significand.
constexpr float sqrt(float a) {
	if (a != a) return a;
	if (a < 0.0f) return std::numeric_limits<float>::quiet_NaN();
	if (a == 0.0f) return a;
	if (a == std::numeric_limits<float>::infinity()) return a;

	std::uint32_t bits = std::bit_cast<std::uint32_t>(a);
	int exp_field = static_cast<int>((bits >> 23) & 0xFFu);

	int subnormal_shift = 0;
	if (exp_field == 0) {
		std::uint32_t mant = bits & ((std::uint32_t{1} << 23) - 1);
		while ((mant & (std::uint32_t{1} << 23)) == 0) {
			mant <<= 1;
			++subnormal_shift;
		}
		bits = mant;
		exp_field = 1;
	}

	int E = exp_field - 127 - subnormal_shift;
	std::uint32_t m_bits = (bits & ((std::uint32_t{1} << 23) - 1))
	                       | (std::uint32_t{127} << 23);
	float M = std::bit_cast<float>(m_bits);

	if (E & 1) {
		M *= 2.0f;
		E -= 1;
	}

	float y = 1.5f;
	y = 0.5f * (y + M / y);
	y = 0.5f * (y + M / y);
	y = 0.5f * (y + M / y);
	y = 0.5f * (y + M / y);

	return y * detail::pow2f(E / 2);
}

}}}  // namespace sw::math::constexpr_math
