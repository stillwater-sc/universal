// sqrt.hpp: square root and cube root functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

namespace efloat_detail {

	// sqrt(a) for a finite a > 0 at `bits` bits (#1756): a double seed, accurate to ~52 bits,
	// then Newton's iteration x <- (x + a/x)/2, each step at twice the precision of the last,
	// since each doubles the correct bits, and one final step at the full precision.  a is
	// scaled to [1, 4) first, so the seed holds for any exponent.
	template<unsigned nlimbs>
	inline efloat<nlimbs> sqrt_newton(const efloat<nlimbs>& a, unsigned bits) {
		const int64_t e    = a.scale();
		const int64_t half = (e >= 0) ? e / 2 : (e - 1) / 2;   // floor(e / 2)
		efloat<nlimbs> m(a);
		m.setexponent(e - 2 * half);                            // m in [1, 4)
		efloat<nlimbs> x(std::sqrt(double(m)));
		efloat<nlimbs> mw(m);
		unsigned p = 48;
		while (true) {
			p = std::min(2 * p, bits);
			x.set_precision(p);
			mw = m;
			mw.set_precision(p);
			x += mw / x;
			x.setexponent(x.scale() - 1);
			if (p >= bits) break;
		}
		x += mw / x;   // the final step at the full precision
		x.setexponent(x.scale() - 1 + half);
		return x;
	}

}  // namespace efloat_detail

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

	// the result's precision is a's own (#1757); Newton runs with guard bits past it and the
	// result rounds once, or the last bits are the iteration's rounding noise.  The iteration
	// count follows the precision (#1140), by doubling it from a double seed (#1756).
	const unsigned result_prec = a.get_precision();
	efloat<nlimbs> x = efloat_detail::sqrt_newton(a, result_prec + 64u);
	x.set_precision(result_prec);
	if (!a.carries_precision()) x.clear_precision();   // as its arithmetic would (#1757)
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
	if (!a.carries_precision()) y.clear_precision();
	if (is_neg) {
		y.setsign(true);
	}
	return y;
}

}} // namespace sw::universal
