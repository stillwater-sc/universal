// sqrt.hpp: square root, cube root, and hypotenuse functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// Standard square root function for efloat using Newton's method
template<unsigned nlimbs>
constexpr efloat<nlimbs> sqrt(const efloat<nlimbs>& a) {
	if (a.iszero()) return a;
	if (a.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (a.isinf() || a.isnan()) return a;

	// Initial guess: x0 = 1.0 * 2^(scale / 2) to support infinite exponent ranges
	efloat<nlimbs> x;
	x.clear();
	x.setexponent(a.scale() / 2);
	x.setlimb(0, 0x80000000); // set implicit leading bit

	efloat<nlimbs> half(0.5);

	// Newton iterations (7 iterations converge up to 2048 bits!)
	for (int i = 0; i < 7; ++i) {
		x = half * (x + a / x);
	}
	return x;
}

// cbrt: cube root function for efloat using Newton's method
template<unsigned nlimbs>
constexpr efloat<nlimbs> cbrt(const efloat<nlimbs>& a) {
	if (a.iszero() || a.isinf() || a.isnan()) return a;

	// standard cbrt(-x) == -cbrt(x)
	bool is_neg = (a.sign() == -1);
	efloat<nlimbs> abs_a(a);
	abs_a.setsign(false);

	// Initial guess: y0 = 1.0 * 2^(scale / 3)
	efloat<nlimbs> y;
	y.clear();
	y.setexponent(abs_a.scale() / 3);
	y.setlimb(0, 0x80000000);

	efloat<nlimbs> one_third = efloat<nlimbs>(1.0) / efloat<nlimbs>(3.0);
	efloat<nlimbs> two(2.0);

	// Newton iterations for y^3 - abs_a = 0: y_next = 1/3 * (2 * y + abs_a / y^2)
	for (int i = 0; i < 7; ++i) {
		y = one_third * (two * y + abs_a / (y * y));
	}

	if (is_neg) {
		y.setsign(true);
	}
	return y;
}

// hypot: hypotenuse function sqrt(x^2 + y^2) with scale-based overflow prevention
template<unsigned nlimbs>
constexpr efloat<nlimbs> hypot(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	// IEEE-754: Infinity suppresses NaN inside hypot! Infinity check must run first.
	if (x.isinf() || y.isinf()) {
		efloat<nlimbs> inf; inf.setinf(false);
		return inf;
	}
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}

	efloat<nlimbs> abs_x(x); abs_x.setsign(false);
	efloat<nlimbs> abs_y(y); abs_y.setsign(false);

	// Scale by max to prevent intermediate underflow/overflow
	efloat<nlimbs> max_val = (abs_x < abs_y) ? abs_y : abs_x;
	efloat<nlimbs> min_val = (abs_x < abs_y) ? abs_x : abs_y;

	if (max_val.iszero()) {
		return efloat<nlimbs>(0.0);
	}

	efloat<nlimbs> r = min_val / max_val;
	return max_val * sqrt(efloat<nlimbs>(1.0) + r * r);
}

}} // namespace sw::universal
