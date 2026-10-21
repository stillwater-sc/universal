// pow.hpp: power functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <bit>
#include <cmath>
#include <vector>
#include <universal/number/efloat/math/exponent.hpp>
#include <universal/number/efloat/math/logarithm.hpp>
#include <universal/number/efloat/math/classify.hpp>
#include <universal/number/efloat/math/truncate.hpp>

namespace sw { namespace universal {

namespace efloat_detail {

	// x^y = exp(y log x) carries log x's error through the product, and exp's
	inline constexpr unsigned pow_error_bits = exp_error_bits + 8u;

	// x^n by binary powering at `target` bits plus guard bits, unrounded: each squaring doubles the
	// relative error, so the n-th power carries bit_width(n) more bits; a negative exponent adds a
	// reciprocal
	inline constexpr unsigned integer_power_error_bits = 8u;

	template<unsigned nlimbs>
	inline efloat<nlimbs> integer_power_working(const efloat<nlimbs>& x, uint64_t n, bool reciprocal, unsigned target) {
		const unsigned working = target + exp_log_guard_bits + static_cast<unsigned>(std::bit_width(n)) + 2u;
		efloat<nlimbs> base(x);
		base.set_precision(working);
		efloat<nlimbs> power(1.0);
		power.set_precision(working);
		for (;;) {
			if (n & 1u) power *= base;
			n >>= 1;
			if (n == 0) break;
			base *= base;
		}
		if (reciprocal) power = 1 / power;
		return power;
	}

	// The largest exact power computed outright: x^n has at most n significant_bits(x) bits, and
	// one wider than this is left to Ziv's strategy, which separates it from the rounding
	// boundaries once its target passes the power's width
	inline unsigned pow_exact_bits(unsigned precision) { return 16u * precision + 1024u; }

	// |x|^y for a finite x != 0 and a finite y != 0 when it may lie on a rounding boundary of
	// `precision` bits, exactly: x = 2^e and an integer e y; an integer y = n > 0 with
	// n (significant_bits(x) - 1) <= precision, below which x^n has more than precision + 1
	// significant bits; or y = m / 2^k, m > 0 odd, with |x| a perfect 2^k-th power. Every other
	// |x|^y is irrational, wider than a boundary or, at a y < 0, not a binary fraction.
	template<unsigned nlimbs>
	inline bool exact_pow(const efloat<nlimbs>& x, const efloat<nlimbs>& y, unsigned precision, efloat<nlimbs>& result) {
		efloat<nlimbs> ax(x);
		ax.setsign(false);
		const unsigned ybits = significant_bits(y);
		if (significant_bits(ax) == 1) {   // 2^(e y)
			if (y.scale() >= 62) return false;
			efloat<nlimbs> ey;
			ey.set_precision(ybits + 64);
			ey = ax.scale();
			ey *= y;
			if (!is_integer(ey)) return false;
			result = efloat<nlimbs>(1.0);
			result.setexponent(static_cast<int64_t>(double(ey)));   // |e y| < 2^53: within exp's range
			return true;
		}
		if (y.sign() == -1) return false;
		// y = m / 2^k: |x|^(1/2^k) by k exact roots, each checked by squaring back
		const int64_t k = std::max<int64_t>(0, static_cast<int64_t>(ybits) - 1 - y.scale());
		if (k == 0) {
			if (y.scale() >= 62) return false;
			const uint64_t n = static_cast<uint64_t>(double(y));
			if (static_cast<double>(n) * (significant_bits(ax) - 1) > precision) return false;
			result = exact_power(ax, n);
			return true;
		}
		efloat<nlimbs> root(ax);
		for (int64_t i = 0; i < k; ++i) {
			const unsigned bits = significant_bits(root);
			// a square r^2 = m 2^2e, m odd, has m = 1 mod 8: its last three significant bits are 001, at an even exponent
			const std::vector<uint32_t> limbs = root.bits();
			const unsigned last = bits - 1;   // the position of the last 1 bit
			auto bit = [&limbs](unsigned p) -> uint32_t { return (limbs[p / 32] >> (31 - p % 32)) & 1u; };
			if (bits < 4 || bit(last - 1) || bit(last - 2) || ((root.scale() - static_cast<int64_t>(last)) & 1)) return false;
			const efloat<nlimbs> r = sqrt_to(root, bits + 32);
			efloat<nlimbs> square(r);
			square.set_precision(2 * (bits + 32));
			square *= r;
			if (square != root) return false;
			root = r;
		}
		efloat<nlimbs> m(y);
		m.setexponent(y.scale() + k);   // the odd integer m
		if (m.scale() >= 62) return false;
		const uint64_t n = static_cast<uint64_t>(double(m));
		if (static_cast<double>(n) * significant_bits(root) > pow_exact_bits(precision)) return false;
		result = exact_power(root, n);
		return true;
	}

	// |x|^y = exp(y log |x|) for a finite x != 0, |x| != 1, and a finite y != 0, unrounded, at
	// `target` bits plus guard bits, or, with |y log x| >= 2^17, inf or 0, with `range` set to
	// +1 or -1. log x carries the scale(y log x) bits the product moves above its absolute error.
	template<unsigned nlimbs>
	inline efloat<nlimbs> pow_working(const efloat<nlimbs>& x, const efloat<nlimbs>& y, unsigned target, int& range) {
		efloat<nlimbs> ax(x);
		ax.setsign(false);
		const int64_t magnitude = y.scale() + log_magnitude(ax) + 4;   // |y log x| < 2^magnitude
		const unsigned working  = target + exp_log_guard_bits + log_error_bits + static_cast<unsigned>(std::max<int64_t>(0, magnitude));
		efloat<nlimbs> t(y);
		t.set_precision(working);
		t *= log_working(ax, working);
		range = 0;
		if (t.scale() >= 17) {
			range = t.sign();
			efloat<nlimbs> limit(0.0);
			if (range > 0) limit.setinf(false);
			return limit;
		}
		return exp_working(t, target);
	}

}  // namespace efloat_detail

// Fast integer power using binary exponentiation, fully safe from INT_MIN overflow
template<unsigned nlimbs>
constexpr efloat<nlimbs> integer_power(efloat<nlimbs> base, int exponent) {
	if (exponent == 0) return efloat<nlimbs>(1.0);
	uint64_t exp_mag = 0;
	if (exponent < 0) {
		base = efloat<nlimbs>(1.0) / base;
		// Safe negation to prevent INT_MIN overflow undefined behavior
		exp_mag = 0ULL - static_cast<uint64_t>(exponent);
	} else {
		exp_mag = static_cast<uint64_t>(exponent);
	}

	efloat<nlimbs> power(1.0);
	while (exp_mag > 1) {
		if (exp_mag & 0x1) {
			power = base * power;
			base *= base;
			exp_mag = (exp_mag - 1) / 2;
		} else {
			base *= base;
			exp_mag /= 2;
		}
	}
	return base * power;
}

// pow(x, y), correctly rounded (#1763), with an integer y converted exactly
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, int y) {
	return pow(x, efloat<nlimbs>(y));
}

// pow(x, y): correctly rounded (#1763). An exact power -- a binary fraction, which a rounding
// boundary may be -- is computed outright and rounded once; every other one is irrational or
// not a binary fraction, and rounds by Ziv's strategy. The special values follow IEEE-754.
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	// pow(x, +/-0) is 1 for any x (including NaN)
	if (y.iszero()) {
		return efloat<nlimbs>(1.0);
	}
	// pow(1, y) is 1 for any y (including NaN)
	if (x == 1.0) {
		return efloat<nlimbs>(1.0);
	}
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x, y);
	const bool     carries   = x.carries_precision() || y.carries_precision();
	const bool     y_neg     = (y.sign() == -1);
	const bool     y_integer = !y.isinf() && efloat_detail::is_integer(y);
	// whether y is an odd integer: its last significant bit is its units bit
	const bool     y_odd     = y_integer && static_cast<int64_t>(efloat_detail::significant_bits(y)) == y.scale() + 1;
	efloat<nlimbs> ax(x);
	ax.setsign(false);

	if (y.isinf()) {   // pow(-1, +/-inf) = 1; |x| > 1 grows to +inf at +inf, |x| < 1 at -inf
		if (ax == 1.0) return efloat<nlimbs>(1.0);
		efloat<nlimbs> limit(0.0);
		if ((ax > efloat<nlimbs>(1.0)) != y_neg) limit.setinf(false);
		return limit;
	}
	// a zero or infinite x: +/-0 or +/-inf, signed as x for an odd integer y
	if (x.iszero() || x.isinf()) {
		const bool negative = (x.sign() == -1) && y_odd;
		efloat<nlimbs> limit(0.0);
		if (x.iszero() == y_neg) limit.setinf(negative);
		else limit.setsign(negative);
		if (x.iszero() && y_neg && !std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return limit;
	}
	// a negative x has a real power only at an integer y, negative at an odd one
	if (x.sign() == -1 && !y_integer) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	const bool negative = (x.sign() == -1) && y_odd;
	if (ax == 1.0) {   // (-1)^y for an integer y
		return efloat<nlimbs>(negative ? -1.0 : 1.0);
	}

	// out of exp's range where |y log x| >= 2^17, as exp is: from the scales, and closer to the
	// limit by the evaluation's range check
	const int64_t magnitude = y.scale() + efloat_detail::log_magnitude(ax);   // |y log x| >= 2^magnitude
	if (magnitude >= 17) {
		efloat<nlimbs> limit = efloat_detail::exp_out_of_range<nlimbs>(y_neg != (ax < efloat<nlimbs>(1.0)));
		limit.setsign(negative);
		return limit;
	}
	efloat<nlimbs> result;
	if (efloat_detail::exact_pow(ax, y, precision, result)) {
		if (negative) result.setsign(true);
		efloat_detail::round_result(result, precision, carries);
		return result;
	}
	// x^y = 1 + y log x + ..., within 2^-(P + 4) of 1 for |y log x| < 2^-(P + 5), on its side of it
	if (magnitude + 4 < -static_cast<int64_t>(precision) - 5) {
		const int direction = (y_neg == (ax < efloat<nlimbs>(1.0))) ? 1 : -1;
		return efloat_detail::round_near(efloat<nlimbs>(negative ? -1.0 : 1.0), negative ? -direction : direction, precision, carries);
	}
	// an integer y by binary powering
	if (y_integer && y.scale() < 53) {
		const uint64_t n = static_cast<uint64_t>(std::fabs(double(y)));
		return efloat_detail::round_ziv<nlimbs>([&](unsigned target) {
			efloat<nlimbs> v = efloat_detail::integer_power_working(ax, n, y_neg, target);
			if (negative) v.setsign(true);
			return v;
		}, precision, efloat_detail::integer_power_error_bits, carries);
	}
	int range = 0;
	result = efloat_detail::round_ziv<nlimbs>([&](unsigned target) {
		efloat<nlimbs> v = efloat_detail::pow_working(ax, y, target, range);
		if (negative) v.setsign(true);
		return v;
	}, precision, efloat_detail::pow_error_bits, carries);
	if (range != 0 && !std::is_constant_evaluated()) {
		efloat_exception_flags.set((range > 0 ? ExceptionFlag::Overflow : ExceptionFlag::Underflow) | ExceptionFlag::Inexact);
	}
	return result;
}

}} // namespace sw::universal
