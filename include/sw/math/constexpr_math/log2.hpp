#pragma once
// log2.hpp: constexpr base-2 logarithm for IEEE-754 float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Algorithm
// -----------------------------------------------------------------------------
// Given x > 0, IEEE-754 representation gives x = 2^E * M with M in [1, 2).
// We further range-reduce so that M is in [1/sqrt(2), sqrt(2)] by halving M
// and incrementing E when M > sqrt(2). Then with
//
//     u = (M - 1) / (M + 1),   |u| <= (sqrt(2) - 1)/(sqrt(2) + 1) ~ 0.1716
//
// the identity
//
//     ln(M) = 2 * artanh(u) = 2 * (u + u^3/3 + u^5/5 + u^7/7 + ...)
//
// converges very quickly because |u^2| <= 0.0294. The series uses only odd
// reciprocals (1, 1/3, 1/5, ...) so the coefficients are derivable from first
// principles without any external tooling (no Sollya, no Remez, no minimax).
//
// Finally  log2(x) = E + ln(M) / ln(2).
//
// Polynomial degree is chosen so the truncation error is below 1 ulp:
//   - float : degree 11 (terms up to u^11 / 11) gives ~1e-9 error
//   - double: degree 21 (terms up to u^21 / 21) gives ~5e-17 error
//
// -----------------------------------------------------------------------------
// Why is_constant_evaluated dispatch is NOT needed
// -----------------------------------------------------------------------------
// std::bit_cast (C++20) is constexpr for trivially-copyable types. Floating-
// point arithmetic in constexpr is allowed since C++14 on gcc/clang. So the
// same code path works at compile time and at runtime.

#include <bit>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace sw { namespace math { namespace constexpr_math {

namespace detail {

// IEEE-754 layout guards: this implementation decodes fixed exponent/mantissa
// bit positions via std::bit_cast. On platforms whose floating-point format is
// not IEC 559 (e.g., IBM hex float on z/Architecture, VAX), the bit pattern is
// different and the algorithm would silently produce wrong results. Fail at
// compile time rather than at runtime.
static_assert(std::numeric_limits<float>::is_iec559,
              "sw::math::constexpr_math requires IEEE-754 single-precision (IEC 559) float");
static_assert(std::numeric_limits<float>::digits == 24,
              "sw::math::constexpr_math requires float with 24-bit significand (1 implicit + 23 stored)");
static_assert(std::numeric_limits<double>::is_iec559,
              "sw::math::constexpr_math requires IEEE-754 double-precision (IEC 559) double");
static_assert(std::numeric_limits<double>::digits == 53,
              "sw::math::constexpr_math requires double with 53-bit significand (1 implicit + 52 stored)");

// 1 / ln(2), to full double precision. Hardcoded as the well-known value
// log2(e) = 1.4426950408889634073599246810018921... (cf. C99 M_LOG2E).
// Verifiable independently by computing ln(2) via the same atanh series and
// taking the reciprocal.
inline constexpr double LOG2E = 1.4426950408889634;
inline constexpr double SQRT2 = 1.4142135623730951;  // sqrt(2)

}  // namespace detail

// constexpr log2(double): returns the base-2 logarithm of x.
// Special values follow IEEE-754 conventions:
//   log2(NaN)  -> NaN
//   log2(x<0)  -> NaN
//   log2(0)    -> -infinity
//   log2(+inf) -> +infinity
constexpr double log2(double x) {
	if (x != x) return x;                                                // NaN propagation
	if (x < 0.0) return std::numeric_limits<double>::quiet_NaN();
	if (x == 0.0) return -std::numeric_limits<double>::infinity();
	if (x == std::numeric_limits<double>::infinity()) return x;

	// Decompose x into 2^E * M with M in [1, 2).
	std::uint64_t bits = std::bit_cast<std::uint64_t>(x);
	int exp_field = static_cast<int>((bits >> 52) & 0x7FFu);

	// Subnormal: normalize by counting how many bits we need to shift the
	// mantissa left so its MSB sits at bit 52 (the implicit-1 position).
	int subnormal_shift = 0;
	if (exp_field == 0) {
		std::uint64_t mant = bits & ((std::uint64_t{1} << 52) - 1);
		// mant != 0 because x != 0 (handled above)
		while ((mant & (std::uint64_t{1} << 52)) == 0) {
			mant <<= 1;
			++subnormal_shift;
		}
		bits = mant;
		exp_field = 1;  // base exponent for the renormalized mantissa
	}

	int E = exp_field - 1023 - subnormal_shift;

	// Reconstruct M in [1, 2) by clearing the exponent field and setting it
	// to bias (1023). Sign bit is necessarily zero (we already returned for x<0).
	std::uint64_t m_bits = (bits & ((std::uint64_t{1} << 52) - 1))
	                       | (std::uint64_t{1023} << 52);
	double M = std::bit_cast<double>(m_bits);

	// Range reduction: shift M into [1/sqrt(2), sqrt(2)] for fast convergence.
	if (M > detail::SQRT2) {
		M *= 0.5;
		E += 1;
	}

	// u = (M - 1) / (M + 1), |u| <= 0.1716
	double u = (M - 1.0) / (M + 1.0);
	double u2 = u * u;

	// Horner evaluation of the artanh series
	//   artanh(u) = u * (1 + u^2/3 + u^4/5 + ... + u^20/21)
	// from the smallest term down. Degree 21 -> ~5e-17 error.
	double s = 1.0 / 21.0;
	s = u2 * s + 1.0 / 19.0;
	s = u2 * s + 1.0 / 17.0;
	s = u2 * s + 1.0 / 15.0;
	s = u2 * s + 1.0 / 13.0;
	s = u2 * s + 1.0 / 11.0;
	s = u2 * s + 1.0 / 9.0;
	s = u2 * s + 1.0 / 7.0;
	s = u2 * s + 1.0 / 5.0;
	s = u2 * s + 1.0 / 3.0;
	s = u2 * s + 1.0;
	double artanh = u * s;

	// log2(M) = (2 * artanh(u)) * (1 / ln(2))
	double log2M = 2.0 * artanh * detail::LOG2E;

	return static_cast<double>(E) + log2M;
}

// constexpr log2(float): same algorithm at lower precision (degree 11 series).
constexpr float log2(float x) {
	if (x != x) return x;
	if (x < 0.0f) return std::numeric_limits<float>::quiet_NaN();
	if (x == 0.0f) return -std::numeric_limits<float>::infinity();
	if (x == std::numeric_limits<float>::infinity()) return x;

	std::uint32_t bits = std::bit_cast<std::uint32_t>(x);
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

	if (M > static_cast<float>(detail::SQRT2)) {
		M *= 0.5f;
		E += 1;
	}

	float u = (M - 1.0f) / (M + 1.0f);
	float u2 = u * u;

	// Degree 11: ~1e-9 error, well below float epsilon (~1.2e-7).
	float s = 1.0f / 11.0f;
	s = u2 * s + 1.0f / 9.0f;
	s = u2 * s + 1.0f / 7.0f;
	s = u2 * s + 1.0f / 5.0f;
	s = u2 * s + 1.0f / 3.0f;
	s = u2 * s + 1.0f;
	float artanh = u * s;

	float log2M = 2.0f * artanh * static_cast<float>(detail::LOG2E);

	return static_cast<float>(E) + log2M;
}

}}}  // namespace sw::math::constexpr_math
