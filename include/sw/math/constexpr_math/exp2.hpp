#pragma once
// exp2.hpp: constexpr base-2 exponential for IEEE-754 float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Algorithm
// -----------------------------------------------------------------------------
// Given x = N + F with N = floor(x) integer and F in [0, 1), we have
//
//     exp2(x) = 2^N * 2^F
//
// 2^N is computed directly from the IEEE-754 representation in O(1) via
// detail::pow2 (set the exponent field, no mantissa).
//
// For 2^F we range-reduce: when F >= 1/2, factor out sqrt(2) and shift F by
// -1/2 so the residual fraction lives in [0, 1/2). With F in [0, 1/2),
//
//     2^F = e^(F * ln(2)),  z = F * ln(2) in [0, ln(2)/2] ~ [0, 0.347]
//
// and the Taylor series
//
//     e^z = sum_{k>=0} z^k / k!
//
// converges very rapidly because |z| <= 0.347. Coefficients are factorial
// reciprocals (1, 1, 1/2, 1/6, 1/24, ...) -- self-derivable, no external
// tooling required.
//
// Polynomial degree:
//   - double: 14 terms gives truncation error ~5e-19 (sub-ulp).
//   - float : 8 terms gives truncation error ~5e-9 (well below float eps).

#include <limits>

#include <math/constexpr_math/detail.hpp>

namespace sw { namespace math { namespace constexpr_math {

// constexpr exp2(double): returns 2 raised to the power x.
// Special values follow IEEE-754 conventions:
//   exp2(NaN)  -> NaN
//   exp2(+inf) -> +inf
//   exp2(-inf) -> 0
//   overflow (x > 1024) -> +inf
//   underflow (x < -1075) -> 0
constexpr double exp2(double x) {
	if (x != x) return x;                                                // NaN propagation
	if (x == std::numeric_limits<double>::infinity()) return x;
	if (x == -std::numeric_limits<double>::infinity()) return 0.0;

	// Saturate against the format's representable range.
	if (x >= 1024.0) return std::numeric_limits<double>::infinity();
	if (x < -1075.0) return 0.0;

	// Decompose x = N + F with F in [0, 1).
	int N = detail::floor_to_int(x);
	double F = x - static_cast<double>(N);

	// Range reduce F into [0, 1/2). When F >= 1/2 we factor out a sqrt(2)
	// scale: 2^F = sqrt(2) * 2^(F - 1/2).
	double scale = 1.0;
	if (F >= 0.5) {
		scale = detail::SQRT2;
		F -= 0.5;
	}

	// 2^F = e^z where z = F * ln(2).  |z| <= ln(2)/2 ~ 0.347.
	double z = F * detail::LN2;

	// Horner Taylor series of e^z, evaluated from highest degree down.
	// Coefficients are 1/k! for k = 0..14.
	double s = 1.0 / 87178291200.0;        // 1/14!
	s = z * s + 1.0 / 6227020800.0;        // 1/13!
	s = z * s + 1.0 / 479001600.0;         // 1/12!
	s = z * s + 1.0 / 39916800.0;          // 1/11!
	s = z * s + 1.0 / 3628800.0;           // 1/10!
	s = z * s + 1.0 / 362880.0;            // 1/9!
	s = z * s + 1.0 / 40320.0;             // 1/8!
	s = z * s + 1.0 / 5040.0;              // 1/7!
	s = z * s + 1.0 / 720.0;               // 1/6!
	s = z * s + 1.0 / 120.0;               // 1/5!
	s = z * s + 1.0 / 24.0;                // 1/4!
	s = z * s + 1.0 / 6.0;                 // 1/3!
	s = z * s + 1.0 / 2.0;                 // 1/2!
	s = z * s + 1.0;                       // 1/1!
	s = z * s + 1.0;                       // 1/0!
	double pow2_F = scale * s;

	// 2^x = 2^N * 2^F. detail::pow2 produces 2^N exactly.
	return detail::pow2(N) * pow2_F;
}

// constexpr exp2(float): same algorithm at lower precision (degree-8 series).
constexpr float exp2(float x) {
	if (x != x) return x;
	if (x == std::numeric_limits<float>::infinity()) return x;
	if (x == -std::numeric_limits<float>::infinity()) return 0.0f;

	if (x >= 128.0f) return std::numeric_limits<float>::infinity();
	if (x < -150.0f) return 0.0f;

	int N = detail::floor_to_int(x);
	float F = x - static_cast<float>(N);

	float scale = 1.0f;
	if (F >= 0.5f) {
		scale = static_cast<float>(detail::SQRT2);
		F -= 0.5f;
	}

	float z = F * static_cast<float>(detail::LN2);

	// Horner Taylor series, degree 8: ~5e-9 error, well below float eps.
	float s = 1.0f / 40320.0f;             // 1/8!
	s = z * s + 1.0f / 5040.0f;            // 1/7!
	s = z * s + 1.0f / 720.0f;             // 1/6!
	s = z * s + 1.0f / 120.0f;             // 1/5!
	s = z * s + 1.0f / 24.0f;              // 1/4!
	s = z * s + 1.0f / 6.0f;               // 1/3!
	s = z * s + 1.0f / 2.0f;               // 1/2!
	s = z * s + 1.0f;                      // 1/1!
	s = z * s + 1.0f;                      // 1/0!
	float pow2_F = scale * s;

	return detail::pow2f(N) * pow2_F;
}

}}}  // namespace sw::math::constexpr_math
