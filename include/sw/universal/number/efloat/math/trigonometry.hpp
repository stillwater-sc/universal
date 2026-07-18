// trigonometry.hpp: trigonometric and inverse trigonometric functions for efloat
//                   (sin, cos, tan, asin, acos, atan, atan2)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// ----------------------------------------------------------------------------
// High-precision constants
//
// Constants are seeded from a ~62-digit decimal literal via efloat's arbitrary-
// precision decimal-to-binary parse() (the same idiom exp()/log() use for ln2).
// pi/2 and pi/4 are derived by exact multiplication by 0.5 / 0.25 (both exactly
// representable), so only pi and atan(1/2) need a literal.
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> efloat_pi() {
	efloat<nlimbs> pi;
	parse("3.14159265358979323846264338327950288419716939937510582097494459", pi);
	return pi;
}
template<unsigned nlimbs>
constexpr efloat<nlimbs> efloat_pi_2() {
	return efloat_pi<nlimbs>() * efloat<nlimbs>(0.5);
}
template<unsigned nlimbs>
constexpr efloat<nlimbs> efloat_pi_4() {
	return efloat_pi<nlimbs>() * efloat<nlimbs>(0.25);
}

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

	efloat<nlimbs> pi = efloat_pi<nlimbs>();
	efloat<nlimbs> two_pi = pi * efloat<nlimbs>(2.0);

	// reduce to [-pi, pi]:  r = x - round(x / 2pi) * 2pi
	efloat<nlimbs> k = round(x / two_pi);
	efloat<nlimbs> r = x - k * two_pi;

	// Taylor series
	efloat<nlimbs> neg_r2 = r * r; neg_r2.setsign(true);   // -r^2 (r^2 >= 0)
	efloat<nlimbs> term = r;
	efloat<nlimbs> sum  = r;
	for (unsigned n = 1; n < 1000; ++n) {
		efloat<nlimbs> denom = efloat<nlimbs>(static_cast<double>(2 * n)) * efloat<nlimbs>(static_cast<double>(2 * n + 1));
		term = term * neg_r2 / denom;
		if (term.iszero()) break;
		sum += term;
		if (!sum.iszero() && term.scale() < sum.scale() - static_cast<int64_t>(sum.get_precision())) break;
	}
	return sum;
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

	efloat<nlimbs> pi = efloat_pi<nlimbs>();
	efloat<nlimbs> two_pi = pi * efloat<nlimbs>(2.0);

	efloat<nlimbs> k = round(x / two_pi);
	efloat<nlimbs> r = x - k * two_pi;

	efloat<nlimbs> neg_r2 = r * r; neg_r2.setsign(true);   // -r^2
	efloat<nlimbs> term(1.0);
	efloat<nlimbs> sum(1.0);
	for (unsigned n = 1; n < 1000; ++n) {
		efloat<nlimbs> denom = efloat<nlimbs>(static_cast<double>(2 * n - 1)) * efloat<nlimbs>(static_cast<double>(2 * n));
		term = term * neg_r2 / denom;
		if (term.iszero()) break;
		sum += term;
		if (!sum.iszero() && term.scale() < sum.scale() - static_cast<int64_t>(sum.get_precision())) break;
	}
	return sum;
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
	efloat<nlimbs> s = sin(x);
	efloat<nlimbs> c = cos(x);
	if (c.iszero()) {                        // odd multiple of pi/2: pole
		efloat<nlimbs> inf; inf.setinf(s.isneg());
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		return inf;
	}
	return s / c;
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
	if (x.isinf()) {                         // atan(+/-inf) = +/- pi/2
		efloat<nlimbs> h = efloat_pi_2<nlimbs>();
		h.setsign(x.sign() == -1);           // isneg() is false for inf (state != Normal)
		return h;
	}

	bool negative = x.isneg();
	efloat<nlimbs> ax(x); ax.setsign(false);  // |x|  (abs() free fn is a stub -- do it manually)
	efloat<nlimbs> one(1.0);

	efloat<nlimbs> result;
	if (ax == one) {
		result = efloat_pi_4<nlimbs>();
	}
	else if (ax > one) {
		result = efloat_pi_2<nlimbs>() - atan(one / ax);
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
		for (unsigned n = 1; n < 2000; ++n) {
			power = power * neg_r2;                                        // alternates sign
			efloat<nlimbs> t = power / efloat<nlimbs>(static_cast<double>(2 * n + 1));
			if (t.iszero()) break;
			series += t;
			if (!series.iszero() && t.scale() < series.scale() - static_cast<int64_t>(series.get_precision())) break;
		}
		if (used_addition) {
			efloat<nlimbs> atan_half;
			parse("0.46364760900080611621425623146121440202853705428612026381093309", atan_half);
			result = atan_half + series;
		}
		else {
			result = series;
		}
	}
	if (negative) result = efloat<nlimbs>(0.0) - result;
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

	bool negative = x.isneg();
	efloat<nlimbs> ax(x); ax.setsign(false);  // |x|
	efloat<nlimbs> one(1.0);

	if (ax > one) {                           // domain error
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	efloat<nlimbs> result;
	if (ax == one) {
		result = efloat_pi_2<nlimbs>();
	}
	else if (ax > efloat<nlimbs>(0.8)) {
		// asin(x) = pi/2 - asin(sqrt(1 - x^2))   (on |x|)
		efloat<nlimbs> t = sqrt(one - ax * ax);
		result = efloat_pi_2<nlimbs>() - asin(t);
	}
	else {
		efloat<nlimbs> x2 = ax * ax;
		efloat<nlimbs> term = ax;             // (2n-1)!!/(2n)!! * x^(2n+1) accumulator, term_0 = x
		efloat<nlimbs> sum  = ax;
		for (unsigned n = 1; n < 4000; ++n) {
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
	return efloat_pi_2<nlimbs>() - asin(x);
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
	efloat<nlimbs> pi   = efloat_pi<nlimbs>();
	efloat<nlimbs> pi_2 = efloat_pi_2<nlimbs>();

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
		efloat<nlimbs> angle = x_neg ? (pi - efloat_pi_4<nlimbs>()) : efloat_pi_4<nlimbs>();
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

	efloat<nlimbs> angle = atan(y / x);
	if (x_neg) {
		// quadrant II / III correction
		if (y_neg) angle = angle - pi;                  // QIII: result in (-pi, -pi/2)
		else       angle = angle + pi;                  // QII:  result in ( pi/2, pi)
	}
	return angle;
}

}} // namespace sw::universal
