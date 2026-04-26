#pragma once
// detail.hpp: shared internal helpers for sw::math::constexpr_math
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Single source of truth for the IEEE-754 layout guards and the transcendental
// constants used by the constexpr_math facility (log2, exp2, log, exp, pow,
// sqrt, ...). Promoting these to a shared header avoids drift across functions.

#include <bit>
#include <cstdint>
#include <limits>

namespace sw { namespace math { namespace constexpr_math { namespace detail {

// IEEE-754 layout guards: this facility decodes fixed exponent/mantissa bit
// positions via std::bit_cast. On platforms whose floating-point format is not
// IEC 559 (e.g., IBM hex float on z/Architecture, VAX), the bit pattern is
// different and the algorithms would silently produce wrong results. Fail at
// compile time rather than at runtime.
static_assert(std::numeric_limits<float>::is_iec559,
              "sw::math::constexpr_math requires IEEE-754 single-precision (IEC 559) float");
static_assert(std::numeric_limits<float>::digits == 24,
              "sw::math::constexpr_math requires float with 24-bit significand (1 implicit + 23 stored)");
static_assert(std::numeric_limits<double>::is_iec559,
              "sw::math::constexpr_math requires IEEE-754 double-precision (IEC 559) double");
static_assert(std::numeric_limits<double>::digits == 53,
              "sw::math::constexpr_math requires double with 53-bit significand (1 implicit + 52 stored)");

// Transcendental constants. All hardcoded to full double precision. Each one
// is independently verifiable (e.g., LOG2E is the well-known C99 M_LOG2E
// constant; LN2 is its reciprocal; SQRT2 is the well-known sqrt(2)).
inline constexpr double LOG2E = 1.4426950408889634;   // log2(e) = 1 / ln(2)
inline constexpr double LN2   = 0.6931471805599453;   // ln(2)
inline constexpr double SQRT2 = 1.4142135623730951;   // sqrt(2)

// Construct 2^n directly from the IEEE-754 representation. Used by exp2 (and
// any future function that needs a fast power-of-two scale factor).
//
// Saturates at the format limits:
//   pow2(n) for n > 1023 (double) / 127 (float) -> +infinity
//   pow2(n) for n < -1074 (double) / -149 (float) -> 0
//
// In the normal range the result is bit-exact (a single bit set in the
// exponent field). In the subnormal range a single bit is set in the
// trailing significand. Both forms are constexpr-safe because std::bit_cast
// is constexpr for trivially-copyable types.

constexpr double pow2(int n) {
	if (n > 1023) return std::numeric_limits<double>::infinity();
	if (n >= -1022) {
		// Normal range: exponent field = n + bias, mantissa = 0
		std::uint64_t bits = static_cast<std::uint64_t>(n + 1023) << 52;
		return std::bit_cast<double>(bits);
	}
	// Subnormal range [-1074, -1023]: single bit at position (n + 1074)
	if (n < -1074) return 0.0;
	std::uint64_t bits = std::uint64_t{1} << (n + 1074);
	return std::bit_cast<double>(bits);
}

constexpr float pow2f(int n) {
	if (n > 127) return std::numeric_limits<float>::infinity();
	if (n >= -126) {
		std::uint32_t bits = static_cast<std::uint32_t>(n + 127) << 23;
		return std::bit_cast<float>(bits);
	}
	// Subnormal range [-149, -127]
	if (n < -149) return 0.0f;
	std::uint32_t bits = std::uint32_t{1} << (n + 149);
	return std::bit_cast<float>(bits);
}

// Constexpr floor for floats: equivalent to std::floor(x), implemented as
// truncate-toward-zero plus a correction for negative non-integer values.
// Used by exp2 to split x into integer and fractional parts.
constexpr int floor_to_int(double x) {
	int n = static_cast<int>(x);
	return (static_cast<double>(n) > x) ? n - 1 : n;
}
constexpr int floor_to_int(float x) {
	int n = static_cast<int>(x);
	return (static_cast<float>(n) > x) ? n - 1 : n;
}

// Test whether y is exactly an integer-valued floating-point number. Used by
// pow to dispatch the negative-base sign rule and the integer-exponent fast
// path.
//
// For |y| >= 2^53 (double) / 2^24 (float), the format's spacing exceeds 1 so
// every representable value is necessarily an integer; the cast-and-compare
// step below would also overflow the long-long range, so we short-circuit.
constexpr bool is_integer(double y) {
	if (y != y) return false;                                            // NaN is not an integer
	if (y >=  9007199254740992.0) return true;                           //  2^53
	if (y <= -9007199254740992.0) return true;                           // -2^53
	long long n = static_cast<long long>(y);
	return static_cast<double>(n) == y;
}
constexpr bool is_integer(float y) {
	if (y != y) return false;
	if (y >=  16777216.0f) return true;                                  //  2^24
	if (y <= -16777216.0f) return true;
	long long n = static_cast<long long>(y);
	return static_cast<float>(n) == y;
}

// Test whether y is an odd integer. Used by pow to determine the sign of the
// result when the base is negative or signed-zero.
//
// Above 2^53 (double) / 2^24 (float) every representable value is a multiple
// of at least 2, so by definition not odd.
constexpr bool is_odd_integer(double y) {
	if (y != y) return false;
	if (y >=  9007199254740992.0) return false;
	if (y <= -9007199254740992.0) return false;
	long long n = static_cast<long long>(y);
	if (static_cast<double>(n) != y) return false;
	return (n & 1) != 0;
}
constexpr bool is_odd_integer(float y) {
	if (y != y) return false;
	if (y >=  16777216.0f) return false;
	if (y <= -16777216.0f) return false;
	long long n = static_cast<long long>(y);
	if (static_cast<float>(n) != y) return false;
	return (n & 1) != 0;
}

// Fast exponentiation by squaring for non-negative integer exponent. Used by
// pow's integer-exponent overload (and as the fast path inside the general
// overload when the floating-point exponent happens to be an integer).
template<typename T>
constexpr T pow_by_squaring(T base, unsigned long long exponent) {
	T result = static_cast<T>(1);
	while (exponent > 0) {
		if (exponent & 1ULL) result *= base;
		base *= base;
		exponent >>= 1;
	}
	return result;
}

}}}}  // namespace sw::math::constexpr_math::detail
