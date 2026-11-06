#pragma once
// sqrt.hpp: the square root of a faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A compensated square root (#1950). Taking std::sqrt of value + error would return only the
// FloatingPointType's precision, half of what the pair holds. Instead:
//   1. scale the argument by an even power of two into [0.5, 2), which is exact, so the square
//      below cannot overflow or reach the subnormals whatever the argument's magnitude;
//   2. s = std::sqrt of the rounded argument, accurate to half an ulp of the FloatingPointType;
//   3. one Newton correction, r = s + (b - s*s) / (2s). s*s is exact through twoProd, so the
//      residual b - s*s is computed in faithful arithmetic without cancellation error, and the
//      correction, a value about an ulp of s, needs only the FloatingPointType's precision;
//   4. scale back.
// Newton doubles the bits of s, so r carries about twice the FloatingPointType's precision. The
// regression test measures the worst error at 1.4 units of eps(Real)^2 / 2, the pair's epsilon(),
// over 100000 arguments for float and for double.
// The same scaling and correction are dd's (#1332, #1883).
//
// Only a native FloatingPointType: the scaling uses std::frexp and std::ldexp.
#include <cerrno>        // errno, EDOM
#include <cmath>         // std::sqrt, std::frexp, std::ldexp
#include <limits>        // std::numeric_limits<>::quiet_NaN
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>

namespace sw { namespace universal {

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> sqrt(const faithful<FloatingPointType>& a) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (a.isnan()) return a;
	// sqrt(+-0) = +-0. A pair whose sum cancels, such as (1, -1), is +0: only a value part that is
	// itself a zero carries its sign.
	if (a.iszero()) return F(a.high() == Real(0) ? a.high() : Real(0), Real(0));
	if (a.sign()) {
#if FAITHFUL_THROW_ARITHMETIC_EXCEPTION
		throw faithful_negative_sqrt_arg();
#else
		errno = EDOM;
		return F(std::numeric_limits<Real>::quiet_NaN(), Real(0));
#endif
	}
	if (a.isinf()) return a;

	// 1. b = a * 2^(-2k) in [0.5, 2): both parts scale by the same power of two, exactly
	int e = 0;
	(void)std::frexp(Real(a), &e);
	const int k = e >> 1;                            // floor(e / 2), also for negative e
	const F b(std::ldexp(a.high(), -2 * k), std::ldexp(a.low(), -2 * k));

	// 2. the FloatingPointType's square root of the rounded argument
	const Real s = std::sqrt(Real(b));

	// 3. one Newton correction from the exact residual
	const F residual = b - F(s) * F(s);
	const F r = F(s) + F(Real(residual) / (Real(2) * s));

	// 4. scale back by 2^k
	return F(std::ldexp(r.high(), k), std::ldexp(r.low(), k));
}

}} // namespace sw::universal
