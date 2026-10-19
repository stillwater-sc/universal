// sqrt.hpp: square root and cube root functions for efloat
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
	// the result's precision is a's own (#1757); Newton runs with guard bits past it and the
	// result rounds once, or the last bits are the iteration's rounding noise
	const unsigned result_prec = a.get_precision();
	const unsigned prec = result_prec + 64u;
	x.set_precision(prec);
	efloat<nlimbs> aw(a);
	aw.set_precision(prec);

	efloat<nlimbs> half(0.5);

	// Newton's iteration x <- (x + a/x)/2 doubles the number of correct bits
	// each step. The exponent-only seed above is accurate to ~1 bit (it keeps
	// efloat's unbounded exponent range but carries no mantissa information), so
	// the iteration count must scale with the working precision: ~ceil(log2(P))
	// doublings plus a few guard steps. A fixed 7 iterations previously capped
	// accuracy at ~97 digits regardless of get_precision() (issue #1140). The
	// convergence break stops as soon as the update falls below the working
	// precision, so high-precision operands do not pay for the guard steps.
	int max_iters = 6;
	for (unsigned p = prec; p > 1u; p >>= 1) ++max_iters;   // ceil(log2(prec)) + 6
	for (int i = 0; i < max_iters; ++i) {
		efloat<nlimbs> prev(x);
		x = half * (x + aw / x);
		efloat<nlimbs> d(x - prev); d.setsign(false);
		if (d.iszero() || (!x.iszero() && d.scale() < x.scale() - static_cast<int64_t>(prec))) break;
	}
	x.set_precision(result_prec);
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
	const unsigned result_prec = abs_a.get_precision();   // the result's precision: a's own (#1757)
	const unsigned prec = result_prec + 64u;             // guard bits, as sqrt
	y.set_precision(prec);
	abs_a.set_precision(prec);

	// 1/3 at that precision: a 1/3 at fewer bits moves the fixed point of the iteration
	efloat<nlimbs> one_third(1.0);
	one_third.set_precision(prec);
	one_third /= 3;
	efloat<nlimbs> two(2.0);

	// Newton iterations for y^3 - abs_a = 0: y_next = 1/3 * (2 * y + abs_a / y^2).
	// Precision-adaptive iteration count with a convergence break, for the same
	// reason as sqrt above (issue #1140): the ~1-bit exponent-only seed needs
	// ~ceil(log2(precision)) doublings to reach full precision.
	int max_iters = 6;
	for (unsigned p = prec; p > 1u; p >>= 1) ++max_iters;
	for (int i = 0; i < max_iters; ++i) {
		efloat<nlimbs> prev(y);
		y = one_third * (two * y + abs_a / (y * y));
		efloat<nlimbs> d(y - prev); d.setsign(false);
		if (d.iszero() || (!y.iszero() && d.scale() < y.scale() - static_cast<int64_t>(prec))) break;
	}

	y.set_precision(result_prec);
	if (is_neg) {
		y.setsign(true);
	}
	return y;
}

}} // namespace sw::universal
