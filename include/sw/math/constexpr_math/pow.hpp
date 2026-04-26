#pragma once
// pow.hpp: constexpr power function for IEEE-754 float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Two overload families
// -----------------------------------------------------------------------------
//
// 1. Integer-exponent fast path: pow(T, int)
//      - Fast exponentiation by squaring; pure integer logic.
//      - O(log |n|) multiplications, no transcendentals.
//      - Selected when the user knows the exponent is an integer.
//
// 2. General overload: pow(T, T)
//      - First tries the integer fast path when the floating-point exponent
//        is exactly an integer (typical for compile-time constants).
//      - Otherwise computes pow(x, y) = exp2(y * log2(x)) for x > 0.
//      - Negative base with integer exponent: pow(|x|, y) with sign from y.
//      - Negative base with non-integer exponent: NaN.
//      - Full IEEE-754 special-case table per C99 7.12.7.4.
//
// -----------------------------------------------------------------------------
// IEEE-754 special-value handling (C99 7.12.7.4)
// -----------------------------------------------------------------------------
//   pow(x,    0)           -> 1                            (any x including NaN, inf)
//   pow(1,    y)           -> 1                            (any y including NaN, inf)
//   pow(NaN,  y)           -> NaN                          (y != 0)
//   pow(x,    NaN)         -> NaN                          (x != 1)
//   pow(+/-0, y<0, odd int)-> +/-inf  (signed)
//   pow(+/-0, y<0, else)   -> +inf
//   pow(+/-0, y>0, odd int)-> +/-0    (signed)
//   pow(+/-0, y>0, else)   -> +0
//   pow(-1,   +/-inf)      -> 1
//   pow(|x|<1,+inf)        -> +0
//   pow(|x|<1,-inf)        -> +inf
//   pow(|x|>1,+inf)        -> +inf
//   pow(|x|>1,-inf)        -> +0
//   pow(+inf, y<0)         -> +0
//   pow(+inf, y>0)         -> +inf
//   pow(-inf, y<0, odd int)-> -0
//   pow(-inf, y<0, else)   -> +0
//   pow(-inf, y>0, odd int)-> -inf
//   pow(-inf, y>0, else)   -> +inf
//   pow(x<0,  finite non-integer y) -> NaN

#include <limits>

#include <math/constexpr_math/detail.hpp>
#include <math/constexpr_math/exp2.hpp>
#include <math/constexpr_math/log2.hpp>

namespace sw { namespace math { namespace constexpr_math {

// -----------------------------------------------------------------------------
// Integer-exponent fast path
// -----------------------------------------------------------------------------

constexpr double pow(double x, int n) {
	if (n == 0) return 1.0;
	if (n < 0) {
		// Promote to long long to safely negate INT_MIN.
		unsigned long long exp = static_cast<unsigned long long>(-static_cast<long long>(n));
		return 1.0 / detail::pow_by_squaring(x, exp);
	}
	return detail::pow_by_squaring(x, static_cast<unsigned long long>(n));
}

constexpr float pow(float x, int n) {
	if (n == 0) return 1.0f;
	if (n < 0) {
		unsigned long long exp = static_cast<unsigned long long>(-static_cast<long long>(n));
		return 1.0f / detail::pow_by_squaring(x, exp);
	}
	return detail::pow_by_squaring(x, static_cast<unsigned long long>(n));
}

// -----------------------------------------------------------------------------
// General overload
// -----------------------------------------------------------------------------

constexpr double pow(double x, double y) {
	// Cases that win regardless of NaN: pow(x, 0) and pow(1, y).
	if (y == 0.0) return 1.0;
	if (x == 1.0) return 1.0;

	// NaN propagation (after the two NaN-overriding cases above).
	if (x != x || y != y) return std::numeric_limits<double>::quiet_NaN();

	const double pinf = std::numeric_limits<double>::infinity();

	// Special case: pow(-1, +/-inf) == 1
	if (x == -1.0 && (y == pinf || y == -pinf)) return 1.0;

	// y is +/- infinity
	if (y == pinf) {
		double ax = (x < 0.0) ? -x : x;
		if (ax < 1.0) return 0.0;
		// |x| > 1 (==1 was handled by pow(1,y) early-out and x==-1 above)
		return pinf;
	}
	if (y == -pinf) {
		double ax = (x < 0.0) ? -x : x;
		if (ax < 1.0) return pinf;
		return 0.0;
	}

	// x is +/- infinity (y is finite, non-zero)
	if (x == pinf) {
		return (y < 0.0) ? 0.0 : pinf;
	}
	if (x == -pinf) {
		bool y_odd = detail::is_odd_integer(y);
		if (y < 0.0) return y_odd ? -0.0 : 0.0;
		return y_odd ? -pinf : pinf;
	}

	// x is +/- 0 (y is finite, non-zero, not infinity)
	if (x == 0.0) {
		bool y_odd = detail::is_odd_integer(y);
		// Distinguish +0 from -0 to honor the signed-zero base rule.
		bool x_is_neg = std::bit_cast<std::uint64_t>(x) >> 63;
		if (y < 0.0) {
			return (y_odd && x_is_neg) ? -pinf : pinf;
		}
		// y > 0: pow(+/-0, y>0) is +/-0 if y is odd integer, else +0
		return (y_odd && x_is_neg) ? -0.0 : 0.0;
	}

	// x < 0: integer exponent uses |x|^y with sign; non-integer is NaN.
	// Use the squaring fast path for exact integer-power semantics when y fits
	// safely in long long; fall back to the transcendental path only when y is
	// outside that range (where y is necessarily an even integer above 2^53,
	// so the sign flip via is_odd_integer correctly returns false).
	if (x < 0.0) {
		if (!detail::is_integer(y)) return std::numeric_limits<double>::quiet_NaN();
		bool y_odd = detail::is_odd_integer(y);
		if (y > -detail::LL_BOUND_DOUBLE && y < detail::LL_BOUND_DOUBLE) {
			long long n = static_cast<long long>(y);
			unsigned long long m = (n >= 0) ? static_cast<unsigned long long>(n)
			                                : static_cast<unsigned long long>(-n);
			double mag = detail::pow_by_squaring(-x, m);
			if (n < 0) mag = 1.0 / mag;
			return y_odd ? -mag : mag;
		}
		double mag = exp2(y * log2(-x));
		return y_odd ? -mag : mag;
	}

	// x > 0, finite y: integer fast path when applicable, else exp2(y*log2(x)).
	// Bounds are exclusive on both sides so the cast and the subsequent unary
	// minus are always defined: 2^63 is representable as double but outside
	// long long; -2^63 cast then negated overflows.
	if (detail::is_integer(y)) {
		if (y > -detail::LL_BOUND_DOUBLE && y < detail::LL_BOUND_DOUBLE) {
			long long n = static_cast<long long>(y);
			if (n >= 0) return detail::pow_by_squaring(x, static_cast<unsigned long long>(n));
			unsigned long long m = static_cast<unsigned long long>(-n);
			return 1.0 / detail::pow_by_squaring(x, m);
		}
	}
	return exp2(y * log2(x));
}

constexpr float pow(float x, float y) {
	if (y == 0.0f) return 1.0f;
	if (x == 1.0f) return 1.0f;
	if (x != x || y != y) return std::numeric_limits<float>::quiet_NaN();

	const float pinf = std::numeric_limits<float>::infinity();

	if (x == -1.0f && (y == pinf || y == -pinf)) return 1.0f;

	if (y == pinf) {
		float ax = (x < 0.0f) ? -x : x;
		return (ax < 1.0f) ? 0.0f : pinf;
	}
	if (y == -pinf) {
		float ax = (x < 0.0f) ? -x : x;
		return (ax < 1.0f) ? pinf : 0.0f;
	}

	if (x == pinf) {
		return (y < 0.0f) ? 0.0f : pinf;
	}
	if (x == -pinf) {
		bool y_odd = detail::is_odd_integer(y);
		if (y < 0.0f) return y_odd ? -0.0f : 0.0f;
		return y_odd ? -pinf : pinf;
	}

	if (x == 0.0f) {
		bool y_odd = detail::is_odd_integer(y);
		bool x_is_neg = std::bit_cast<std::uint32_t>(x) >> 31;
		if (y < 0.0f) {
			return (y_odd && x_is_neg) ? -pinf : pinf;
		}
		return (y_odd && x_is_neg) ? -0.0f : 0.0f;
	}

	if (x < 0.0f) {
		if (!detail::is_integer(y)) return std::numeric_limits<float>::quiet_NaN();
		bool y_odd = detail::is_odd_integer(y);
		if (y > -detail::LL_BOUND_FLOAT && y < detail::LL_BOUND_FLOAT) {
			long long n = static_cast<long long>(y);
			unsigned long long m = (n >= 0) ? static_cast<unsigned long long>(n)
			                                : static_cast<unsigned long long>(-n);
			float mag = detail::pow_by_squaring(-x, m);
			if (n < 0) mag = 1.0f / mag;
			return y_odd ? -mag : mag;
		}
		float mag = exp2(y * log2(-x));
		return y_odd ? -mag : mag;
	}

	if (detail::is_integer(y)) {
		if (y > -detail::LL_BOUND_FLOAT && y < detail::LL_BOUND_FLOAT) {
			long long n = static_cast<long long>(y);
			if (n >= 0) return detail::pow_by_squaring(x, static_cast<unsigned long long>(n));
			unsigned long long m = static_cast<unsigned long long>(-n);
			return 1.0f / detail::pow_by_squaring(x, m);
		}
	}
	return exp2(y * log2(x));
}

}}}  // namespace sw::math::constexpr_math
