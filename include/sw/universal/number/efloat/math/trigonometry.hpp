// trigonometry.hpp: trigonometric and inverse trigonometric functions for efloat
//                   (sin, cos, tan, asin, acos, atan, atan2)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <algorithm>
#include <universal/number/efloat/math/constants/efloat_constants.hpp>
#include <universal/number/efloat/math/exponent.hpp>   // result_precision, exp_log_guard_bits
#include <universal/number/efloat/math/truncate.hpp>   // round, floor
#include <universal/number/efloat/math/sqrt.hpp>       // sqrt

namespace sw { namespace universal {

// ----------------------------------------------------------------------------
// Derived constants
//
// efloat_pi<nlimbs>() comes from efloat_constants.hpp (a ~1000-digit literal,
// converted exactly to the type's precision). pi/2 and pi/4 are derived by
// exact multiplication by 0.5 / 0.25 (both exactly representable).
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
inline efloat<nlimbs> efloat_pi_2() {
	return efloat_pi<nlimbs>() * efloat<nlimbs>(0.5);
}
template<unsigned nlimbs>
inline efloat<nlimbs> efloat_pi_4() {
	return efloat_pi<nlimbs>() * efloat<nlimbs>(0.25);
}

namespace efloat_detail {

	// atan(1/m) = sum_k (-1)^k / ((2k + 1) m^(2k + 1)) for an integer m >= 2, at `bits` bits
	template<unsigned nlimbs>
	inline efloat<nlimbs> atan_reciprocal(unsigned m, unsigned bits) {
		efloat<nlimbs> one(1.0);
		one.set_precision(bits);
		const efloat<nlimbs> m2(static_cast<double>(m) * static_cast<double>(m));
		efloat<nlimbs> power = one / efloat<nlimbs>(static_cast<double>(m));   // 1 / m^(2k + 1)
		efloat<nlimbs> sum = power;
		for (unsigned k = 1; ; ++k) {
			power = power / m2;
			const efloat<nlimbs> term = power / efloat<nlimbs>(static_cast<double>(2 * k + 1));
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 2) break;
			if (k % 2) sum -= term; else sum += term;
		}
		return sum;
	}

	// atan(1/2) at `bits` bits, computed rather than parsed (the 64-digit literal it replaces
	// was right to ~210 bits, #1712): atan(1/2) = atan(1/3) + atan(1/7), since
	// (1/3 + 1/7) / (1 - 1/21) = 1/2.  Cached per thread like ln 2.
	template<unsigned nlimbs>
	inline efloat<nlimbs> atan_half(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			cached = atan_reciprocal<nlimbs>(3, bits + 32) + atan_reciprocal<nlimbs>(7, bits + 32);
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

	// pi at `bits` bits, computed rather than taken from efloat_pi<nlimbs>(), which carries the
	// type's own width (nlimbs * 32 bits, at most ~3300): a value widened by set_precision()
	// reduced its argument with a pi of that narrower width (#1712).  Machin's formula,
	// pi = 16 atan(1/5) - 4 atan(1/239), summed with 32 guard bits and cached like ln 2.
	template<unsigned nlimbs>
	inline efloat<nlimbs> pi(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			cached = atan_reciprocal<nlimbs>(5, bits + 32) * efloat<nlimbs>(16.0) - atan_reciprocal<nlimbs>(239, bits + 32) * efloat<nlimbs>(4.0);
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

	// pi * 2^e at `bits` bits: pi/2 and pi/4 are pi with its exponent lowered
	template<unsigned nlimbs>
	inline efloat<nlimbs> pi_scaled(unsigned bits, int e) {
		efloat<nlimbs> value = pi<nlimbs>(bits);
		value.setexponent(value.scale() + e);
		return value;
	}

	// x = k pi/2 + r with |r| <= pi/4, r accurate to `bits` bits relative to itself, and
	// quadrant = k mod 4 (#1712).  x - k pi/2 cancels the scale(x) - scale(r) leading bits, and
	// x can lie as close to a multiple of pi/2 as its own precision allows, so the reduction is
	// retried with x and pi/2 widened by the cancellation until r keeps `bits` + 32 correct bits.
	// A P-bit x cannot lie much closer to k pi/2 than 2^-2(P + scale(x)), so the retries stop at
	// three times that width.
	template<unsigned nlimbs>
	inline efloat<nlimbs> reduce_half_pi(const efloat<nlimbs>& x, unsigned bits, unsigned& quadrant) {
		const unsigned magnitude = static_cast<unsigned>(std::max<int64_t>(0, x.scale()));
		const unsigned limit     = 3 * (bits + magnitude) + 2 * exp_log_guard_bits;
		unsigned working         = bits + exp_log_guard_bits + magnitude;
		while (true) {
			efloat<nlimbs> xw(x);
			xw.set_precision(working);
			const efloat<nlimbs> half_pi = pi_scaled<nlimbs>(working, -1);
			const efloat<nlimbs> k = round(xw / half_pi);
			const efloat<nlimbs> r = xw - k * half_pi;
			const int64_t lost = r.iszero() ? static_cast<int64_t>(working) : std::max<int64_t>(0, x.scale() - r.scale());
			const unsigned needed = bits + 32 + static_cast<unsigned>(lost);
			if (k.iszero() || working >= needed || working >= limit) {
				const efloat<nlimbs> four(4.0);
				const efloat<nlimbs> k_mod_4 = k - floor(k / four) * four;   // k is an integer: exact
				quadrant = static_cast<unsigned>(double(k_mod_4)) & 3u;
				return r;
			}
			working = std::min(limit, needed + exp_log_guard_bits);
		}
	}

	// The versine v = 1 - cos(r) for |r| <= pi/4, at r's precision (#1756).  sin and cos both
	// follow from it: cos r = 1 - v, |sin r| = sqrt(v (2 - v)), neither of which cancels, since
	// v <= 1 - cos(pi/4) < 0.3 and v keeps its relative precision as r -> 0.
	//
	// r is halved h times first, and the series for v(r / 2^h) doubled back h times by
	// v(2y) = 2 v(y) (2 - v(y)): each doubling costs one multiplication and saves the series
	// terms, and it keeps the relative error of v, so the working precision carries h guard
	// bits.  h ~ 0.4 sqrt(precision) + 5 balances the two; an r already below 2^-h is not halved
	// that far.  The series v(y) = y^2/2! - y^4/4! + ... runs its term and y^2 at a precision
	// that falls with the terms, as in exp, with exact integer divisors (#1757).
	template<unsigned nlimbs>
	inline efloat<nlimbs> versine(const efloat<nlimbs>& r) {
		const unsigned precision = r.get_precision();
		const int64_t  target    = static_cast<int64_t>(0.4 * std::sqrt(static_cast<double>(precision))) + 5;
		const int64_t  h_signed  = target + r.scale();   // halvings that bring y below 2^-target
		const unsigned h         = (h_signed > 0) ? static_cast<unsigned>(h_signed) : 0u;
		const unsigned working   = precision + h;

		efloat<nlimbs> y(r);
		y.setsign(false);
		y.set_precision(working);
		y.setexponent(y.scale() - static_cast<int64_t>(h));

		efloat<nlimbs> neg_y2 = y * y;
		efloat<nlimbs> term(neg_y2);
		term.setexponent(term.scale() - 1);              // y^2 / 2!
		neg_y2.setsign(true);
		efloat<nlimbs> v(term);
		efloat<nlimbs> y2_taper = neg_y2;
		unsigned taper = working;
		for (unsigned n = 2; ; ++n) {
			const int64_t below = v.scale() - term.scale();   // bits the term sits below the sum
			const unsigned need = (below + 64 < static_cast<int64_t>(working)) ? static_cast<unsigned>(static_cast<int64_t>(working) - below) : 64u;
			if (need + 32u <= taper) {
				taper = need;
				term.set_precision(taper);
				y2_taper = neg_y2;
				y2_taper.set_precision(taper);
			}
			term *= y2_taper;
			term /= uint64_t(2 * n - 1) * uint64_t(2 * n);
			if (term.iszero() || term.scale() < v.scale() - static_cast<int64_t>(working)) break;
			v += term;
		}
		for (unsigned i = 0; i < h; ++i) {
			v *= 2 - v;
			v.setexponent(v.scale() + 1);
		}
		return v;
	}

	// sin(r) and cos(r) for |r| <= pi/4 from the versine
	template<unsigned nlimbs>
	inline efloat<nlimbs> sin_from_versine(const efloat<nlimbs>& v, const efloat<nlimbs>& r) {
		if (v.iszero()) return r;   // r below the precision's resolution of v: sin r = r
		efloat<nlimbs> s = sqrt_newton(v * (2 - v), v.get_precision());
		if (r.sign() == -1) s.setsign(true);
		return s;
	}

	template<unsigned nlimbs>
	inline efloat<nlimbs> cos_from_versine(const efloat<nlimbs>& v) {
		return 1 - v;
	}

}  // namespace efloat_detail

// The trigonometric functions compute at their result's precision plus guard bits, with pi at
// that precision, and round once at the end (#1712).  sin, cos and tan reduce by pi/2 to
// |r| <= pi/4, widening the reduction by its cancellation, so a result near a zero of the
// function keeps its full relative precision.

// ----------------------------------------------------------------------------
// sin: sine, computed by argument reduction to [-pi, pi] + Taylor series
//   sin(x) = x - x^3/3! + x^5/5! - ...   term_n = -term_{n-1} * x^2 / ((2n)(2n+1))
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> sin(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;               // sin(+/-0) = +/-0
	if (x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}

	const unsigned precision = efloat_detail::result_precision(x);
	unsigned quadrant = 0;
	const efloat<nlimbs> r = efloat_detail::reduce_half_pi(x, precision, quadrant);
	const efloat<nlimbs> v = efloat_detail::versine(r);
	efloat<nlimbs> result = (quadrant & 1u) ? efloat_detail::cos_from_versine(v) : efloat_detail::sin_from_versine(v, r);
	if (quadrant >= 2) result = efloat<nlimbs>(0.0) - result;
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// ----------------------------------------------------------------------------
// cos: cosine, argument reduction to [-pi, pi] + Taylor series
//   cos(x) = 1 - x^2/2! + x^4/4! - ...   term_n = -term_{n-1} * x^2 / ((2n-1)(2n))
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> cos(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}

	const unsigned precision = efloat_detail::result_precision(x);
	unsigned quadrant = 0;
	const efloat<nlimbs> r = efloat_detail::reduce_half_pi(x, precision, quadrant);
	const efloat<nlimbs> v = efloat_detail::versine(r);
	efloat<nlimbs> result = (quadrant & 1u) ? efloat_detail::sin_from_versine(v, r) : efloat_detail::cos_from_versine(v);
	if (quadrant == 1 || quadrant == 2) result = efloat<nlimbs>(0.0) - result;
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// ----------------------------------------------------------------------------
// tan: tan(x) = sin(x) / cos(x)
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> tan(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;
	if (x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	// tan(k pi/2 + r) = tan(r) for even k, -1 / tan(r) for odd k: one reduction, one rounding
	const unsigned precision = efloat_detail::result_precision(x);
	unsigned quadrant = 0;
	const efloat<nlimbs> r = efloat_detail::reduce_half_pi(x, precision, quadrant);
	const efloat<nlimbs> v = efloat_detail::versine(r);
	const efloat<nlimbs> s = efloat_detail::sin_from_versine(v, r);
	const efloat<nlimbs> c = efloat_detail::cos_from_versine(v);
	efloat<nlimbs> result;
	if (quadrant & 1u) {
		if (s.iszero()) {                    // r too small to resolve at the widest reduction: a pole,
			efloat<nlimbs> inf; inf.setinf(quadrant == 3u);   // +inf at pi/2, -inf at -pi/2 (quadrant 3)
			if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
			return inf;
		}
		result = efloat<nlimbs>(0.0) - c / s;
	}
	else {
		result = s / c;
	}
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// ----------------------------------------------------------------------------
// atan: inverse tangent
//   |x| == 1        -> +/- pi/4
//   |x| >  1        -> sign * (pi/2 - atan(1/|x|))
//   |x| >  1/2      -> atan(1/2) + atan((|x|-1/2)/(1+|x|/2))   (addition formula)
//   else Taylor:  atan(x) = x - x^3/3 + x^5/5 - ...
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> atan(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;
	const unsigned precision = efloat_detail::result_precision(x);
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	if (x.isinf()) {                         // atan(+/-inf) = +/- pi/2
		efloat<nlimbs> h = efloat_detail::pi_scaled<nlimbs>(precision, -1);
		h.setsign(x.sign() == -1);           // isneg() is false for inf (state != Normal)
		return h;
	}

	bool negative = x.isneg();
	efloat<nlimbs> ax(x); ax.setsign(false);  // |x|  (abs() free fn is a stub -- do it manually)
	ax.set_precision(working);
	efloat<nlimbs> one(1.0);

	efloat<nlimbs> result;
	if (ax == one) {
		result = efloat_detail::pi_scaled<nlimbs>(working, -2);
	}
	else if (ax > one) {
		result = efloat_detail::pi_scaled<nlimbs>(working, -1) - atan(one / ax);
	}
	else {
		efloat<nlimbs> half(0.5);
		efloat<nlimbs> reduced = ax;
		bool used_addition = false;
		if (ax > half) {
			// atan(x) = atan(1/2) + atan((x - 1/2)/(1 + x/2)),  reduced arg < ~0.32
			efloat<nlimbs> num = ax - half;
			efloat<nlimbs> den = one + ax * half;
			reduced = num / den;
			used_addition = true;
		}
		// Taylor series for |reduced| < ~0.5
		efloat<nlimbs> neg_r2 = reduced * reduced; neg_r2.setsign(true);  // -reduced^2
		efloat<nlimbs> power = reduced;    // reduced^(2n+1)
		efloat<nlimbs> series = reduced;
		for (unsigned n = 1; ; ++n) {
			power = power * neg_r2;                                        // alternates sign
			efloat<nlimbs> t = power / efloat<nlimbs>(static_cast<double>(2 * n + 1));
			if (t.iszero()) break;
			series += t;
			if (!series.iszero() && t.scale() < series.scale() - static_cast<int64_t>(series.get_precision())) break;
		}
		if (used_addition) {
			result = efloat_detail::atan_half<nlimbs>(series.get_precision()) + series;
		}
		else {
			result = series;
		}
	}
	if (negative) result = efloat<nlimbs>(0.0) - result;
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// ----------------------------------------------------------------------------
// asin: inverse sine
//   |x| > 1 -> NaN;  x == +/-1 -> +/- pi/2
//   |x| > 0.8 -> sign * (pi/2 - asin(sqrt(1 - x^2)))   (reduce toward 0)
//   else Taylor:  asin(x) = x + (1/2)x^3/3 + (1*3/2*4)x^5/5 + ...
//                 term_n = term_{n-1} * x^2 * (2n-1)^2 / ((2n)(2n+1))
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> asin(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;

	const unsigned precision = efloat_detail::result_precision(x);
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	bool negative = x.isneg();
	efloat<nlimbs> ax(x); ax.setsign(false);  // |x|
	ax.set_precision(working);
	efloat<nlimbs> one(1.0);

	if (ax > one) {                           // domain error
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	efloat<nlimbs> result;
	if (ax == one) {
		result = efloat_detail::pi_scaled<nlimbs>(working, -1);
	}
	else if (ax > efloat<nlimbs>(0.8)) {
		// asin(x) = pi/2 - asin(sqrt(1 - x^2))   (on |x|)
		efloat<nlimbs> t = sqrt((one - ax) * (one + ax));   // 1 - ax is exact; ax * ax would round first
		result = efloat_detail::pi_scaled<nlimbs>(working, -1) - asin(t);
	}
	else {
		efloat<nlimbs> x2 = ax * ax;
		efloat<nlimbs> term = ax;             // (2n-1)!!/(2n)!! * x^(2n+1) accumulator, term_0 = x
		efloat<nlimbs> sum  = ax;
		for (unsigned n = 1; ; ++n) {
			efloat<nlimbs> odd = efloat<nlimbs>(static_cast<double>(2 * n - 1));
			efloat<nlimbs> num = odd * odd;   // (2n-1)^2
			efloat<nlimbs> den = efloat<nlimbs>(static_cast<double>(2 * n)) * efloat<nlimbs>(static_cast<double>(2 * n + 1));
			term = term * x2 * num / den;
			if (term.iszero()) break;
			sum += term;
			if (!sum.iszero() && term.scale() < sum.scale() - static_cast<int64_t>(sum.get_precision())) break;
		}
		result = sum;
	}
	if (negative) result = efloat<nlimbs>(0.0) - result;
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// ----------------------------------------------------------------------------
// acos: acos(x) = pi/2 - asin(x);  |x| > 1 -> NaN
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> acos(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	efloat<nlimbs> ax(x); ax.setsign(false);
	if (ax > efloat<nlimbs>(1.0)) {           // domain error
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x);
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	efloat<nlimbs> xw(x);
	xw.set_precision(working);
	// pi/2 - asin(x) cancels as x -> 1, where acos(x) -> 0: there acos(x) = 2 asin(sqrt((1 - x) / 2)),
	// and near -1 acos(x) = pi - 2 asin(sqrt((1 + x) / 2)); 1 -/+ x is exact for |x| >= 1/2
	const efloat<nlimbs> one(1.0), half(0.5), two(2.0);
	efloat<nlimbs> result;
	if (xw > half) {
		result = asin(sqrt((one - xw) * half)) * two;
	}
	else if (xw < -half) {
		result = efloat_detail::pi<nlimbs>(working) - asin(sqrt((one + xw) * half)) * two;
	}
	else {
		result = efloat_detail::pi_scaled<nlimbs>(working, -1) - asin(xw);
	}
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// ----------------------------------------------------------------------------
// atan2: full-quadrant arctangent of y/x
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> atan2(const efloat<nlimbs>& y, const efloat<nlimbs>& x) {
	if (y.isnan() || x.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	const unsigned precision = std::max(efloat_detail::result_precision(x), y.get_precision());
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	efloat<nlimbs> pi   = efloat_detail::pi<nlimbs>(precision);
	efloat<nlimbs> pi_2 = efloat_detail::pi_scaled<nlimbs>(precision, -1);

	// Sign checks below use sign() == -1, not isneg(): isneg() is false for an
	// infinity (its state is Infinite, not Normal), and atan2 arguments may be
	// infinite in every branch. sign() reads the stored sign bit directly, so
	// it is correct for both normal and infinite operands (and equivalent to
	// isneg() for finite normals).
	const bool y_neg = (y.sign() == -1);
	const bool x_neg = (x.sign() == -1);

	// both infinite: y/x is indeterminate, but atan2 is defined on the
	// diagonal directions -> +/- pi/4 (x > 0) or +/- 3pi/4 (x < 0).
	if (y.isinf() && x.isinf()) {
		const efloat<nlimbs> pi_4 = efloat_detail::pi_scaled<nlimbs>(precision, -2);
		efloat<nlimbs> angle = x_neg ? (pi - pi_4) : pi_4;

		angle.setsign(y_neg);
		return angle;
	}

	if (x.iszero()) {
		if (y.iszero()) return efloat<nlimbs>(0.0);    // atan2(0,0): convention 0
		efloat<nlimbs> h(pi_2); h.setsign(y_neg);      // +/- pi/2
		return h;
	}
	if (y.iszero()) {
		if (x_neg) return pi;                          // atan2(+0, -x) = pi
		return efloat<nlimbs>(0.0);                     // atan2(+0, +x) = 0
	}

	// the quotient, its arctangent and the quadrant correction at the working precision
	efloat<nlimbs> yw(y), xw(x);
	yw.set_precision(working);
	xw.set_precision(working);
	efloat<nlimbs> angle = atan(yw / xw);
	if (x_neg) {
		// quadrant II / III correction
		const efloat<nlimbs> piw = efloat_detail::pi<nlimbs>(working);
		if (y_neg) angle = angle - piw;                 // QIII: result in (-pi, -pi/2)
		else       angle = angle + piw;                 // QII:  result in ( pi/2, pi)
	}
	efloat_detail::round_result(angle, precision, x.carries_precision() || y.carries_precision());
	return angle;
}

}} // namespace sw::universal
