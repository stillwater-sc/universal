// fractional.hpp: fractional, remainder, and sign-copy functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <universal/number/efloat/math/exponent.hpp>   // exact_product, exact_sum, lowest_bit, round_result
#include <universal/number/efloat/math/truncate.hpp>
#include <universal/number/efloat/math/classify.hpp>

namespace sw { namespace universal {

namespace efloat_detail {

	// x 2^k, exactly, for a finite x
	template<unsigned nlimbs>
	inline efloat<nlimbs> times_power_of_2(efloat<nlimbs> x, int64_t k) {
		if (!x.iszero()) x.setexponent(x.scale() + k);
		return x;
	}

	// a mod m = a - floor(a / m) m for integers a >= 0 and m > 0, exactly. a / m = n + f with f = 0
	// or 1/m <= f <= 1 - 1/m, and the quotient is carried at scale(a) + sig(m) + 66 bits, so its
	// rounding error, in any mode, lies below 2^-(scale(m) + 65), far inside 1/m > 2^-(scale(m) + 1):
	// its truncation is n, and the remainder a - n m is exact at the same width
	template<unsigned nlimbs>
	inline efloat<nlimbs> integer_mod(const efloat<nlimbs>& a, const efloat<nlimbs>& m) {
		if (a.iszero() || a < m) return a;
		const unsigned bits = static_cast<unsigned>(a.scale() + 2) + significant_bits(m) + 64u;
		const efloat<nlimbs> A = widened(a, bits), M = widened(m, bits);
		const efloat<nlimbs> q = trunc(A / M);
		return A - q * M;
	}

	// 2^d mod m for d >= 0 and an integer m > 0, by binary powering modulo m
	template<unsigned nlimbs>
	inline efloat<nlimbs> power_of_2_mod(int64_t d, const efloat<nlimbs>& m) {
		const int64_t direct = 2 * static_cast<int64_t>(significant_bits(m)) + 64;
		if (d <= direct) return integer_mod(times_power_of_2(efloat<nlimbs>(1), d), m);
		efloat<nlimbs> t(1);
		for (int b = 62; b >= 0; --b) {
			if (!t.iszero()) t = integer_mod(exact_product(t, t), m);
			if ((d >> b) & 1) t = integer_mod(times_power_of_2(t, 1), m);
		}
		return t;
	}

	// x mod y = x - trunc(x / y) y for finite x >= 0 and y > 0, exactly (#1783). With 2^lo the
	// smaller of their last bits, X = x 2^-lo and Y = y 2^-lo are integers, and X = mx 2^d with mx
	// x's own significand: X mod Y = (mx mod Y) (2^d mod Y) mod Y, where 2^d mod Y takes about
	// log2(d) squarings, so an exponent gap of up to 2^62 costs no more than its bit count.
	template<unsigned nlimbs>
	inline efloat<nlimbs> exact_mod(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
		if (x.iszero() || x < y) return x;
		const int64_t lx = lowest_bit(x), ly = lowest_bit(y);
		const int64_t lo = std::min(lx, ly);
		const efloat<nlimbs> mx = times_power_of_2(x, -lx);
		const efloat<nlimbs> Y  = times_power_of_2(y, -lo);
		efloat<nlimbs> r = integer_mod(mx, Y);
		if (lx > lo && !r.iszero()) r = integer_mod(exact_product(r, power_of_2_mod(lx - lo, Y)), Y);
		return times_power_of_2(r, lo);
	}

}  // namespace efloat_detail

// fmod(x, y) = x - trunc(x / y) y, exactly (#1783): the remainder is always representable at the
// wider of x's and y's precisions, so no rounding mode changes it. Before, x / y was rounded,
// truncated and multiplied back, which lost the remainder once the quotient passed the precision.
// Special values as IEEE 754: NaN for an infinite x or a zero y, x for an infinite y.
template<unsigned nlimbs>
inline efloat<nlimbs> fmod(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	if (x.isinf() || y.iszero()) {
		efloat<nlimbs> nan; nan.setnan();
		efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x, y);
	const bool carries = x.carries_precision() || y.carries_precision();
	efloat<nlimbs> result(x);
	if (!x.iszero() && !y.isinf()) {
		efloat<nlimbs> ax(x), ay(y);
		ax.setsign(false);
		ay.setsign(false);
		result = efloat_detail::exact_mod(ax, ay);
		result.setsign(x.sign() == -1);   // the remainder, zero included, has x's sign
	}
	efloat_detail::round_result(result, precision, carries);
	return result;
}

// remainder(x, y) = x - n y with n the integer nearest x / y, ties to even, exactly (#1783):
// x mod 2|y| tells n's parity, and the remainder is moved into [-|y|/2, |y|/2]. Special values as
// IEEE 754: NaN for an infinite x or a zero y, x for an infinite y.
template<unsigned nlimbs>
inline efloat<nlimbs> remainder(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	if (y.iszero() || x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x, y);
	const bool carries = x.carries_precision() || y.carries_precision();
	efloat<nlimbs> result(x);
	if (!x.iszero() && !y.isinf()) {
		efloat<nlimbs> ax(x), ay(y);
		ax.setsign(false);
		ay.setsign(false);
		efloat<nlimbs> r = efloat_detail::exact_mod(ax, efloat_detail::times_power_of_2(ay, 1));   // in [0, 2|y|)
		const bool odd = (r >= ay);
		if (odd) r = efloat_detail::exact_sum(r, -ay, 0);
		const efloat<nlimbs> twice = efloat_detail::times_power_of_2(r, 1);
		if (twice > ay || (twice == ay && odd)) r = efloat_detail::exact_sum(r, -ay, 0);
		const bool flip = (x.sign() == -1);
		if (r.iszero()) r.setsign(flip);   // a zero remainder has x's sign
		else if (flip) r = -r;
		result = r;
	}
	efloat_detail::round_result(result, precision, carries);
	return result;
}

template<unsigned nlimbs>
constexpr efloat<nlimbs> drem(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	return remainder(x, y);
}

// frac: return the fractional part of x (x - trunc(x))
template<unsigned nlimbs>
constexpr efloat<nlimbs> frac(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	return x - trunc(x);
}

// modf: decomposes x into integer and fractional parts
template<unsigned nlimbs>
constexpr efloat<nlimbs> modf(const efloat<nlimbs>& x, efloat<nlimbs>* iptr) {
	if (x.isnan()) {
		if (iptr) iptr->setnan();
		return x;
	}
	if (x.isinf()) {
		if (iptr) *iptr = x;
		efloat<nlimbs> zero(0.0);
		zero.setsign(x.sign() == -1);
		return zero;
	}
	efloat<nlimbs> integer_part = trunc(x);
	if (iptr) {
		*iptr = integer_part;
	}
	efloat<nlimbs> frac_part = x - integer_part;
	frac_part.setsign(x.sign() == -1);
	return frac_part;
}

// copysign: returns x with its sign set to match y
template<unsigned nlimbs>
constexpr efloat<nlimbs> copysign(const efloat<nlimbs>& x, const efloat<nlimbs>& y) noexcept {
	efloat<nlimbs> res(x);
	res.setsign(y.sign() == -1);
	return res;
}

}} // namespace sw::universal
