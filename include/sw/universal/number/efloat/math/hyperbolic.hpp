// hyperbolic.hpp: hyperbolic and inverse hyperbolic functions for efloat
//                 (sinh, cosh, tanh, asinh, acosh, atanh)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <type_traits>                                 // std::is_constant_evaluated
#include <universal/behavior/status_flags.hpp>         // ExceptionFlag, efloat_exception_flags
#include <universal/number/efloat/math/exponent.hpp>   // exp, expm1
#include <universal/number/efloat/math/logarithm.hpp>  // log1p
#include <universal/number/efloat/math/sqrt.hpp>       // sqrt

namespace sw {
namespace universal {

// ----------------------------------------------------------------------------
// The forward functions are built on the exact identities
//     sinh(x) = (e^x - e^-x)/2,  cosh(x) = (e^x + e^-x)/2,  tanh(x) = (e^2x-1)/(e^2x+1)
// and the inverses on
//     asinh(x) = log(x + sqrt(x^2+1)),  acosh(x) = log(x + sqrt(x^2-1)),
//     atanh(x) = 0.5*log((1+x)/(1-x)).
// Where the naive identity would cancel near zero, the expm1/log1p forms are
// used instead so that accuracy tracks the operand's full working precision
// (not just double). All special-value results match <cmath>.
//
// Each function is correctly rounded (#1763): it evaluates at its result's precision plus guard
// bits, unrounded, and rounds by Ziv's strategy. The results are transcendental at every
// argument that reaches it -- acosh(1) = 0 is the one exact value, and is returned as such -- and
// a tiny argument, or a large one for tanh, rounds as the leading term nudged toward the next.
// ----------------------------------------------------------------------------

namespace efloat_detail {

	// the hyperbolic functions combine up to three exp/log evaluations, a root and a quotient
	inline constexpr unsigned hyperbolic_error_bits = 24u;

	// sinh, cosh or tanh of a finite x != 0, |x| < 2^17, unrounded, at `target` bits plus guard bits
	template<unsigned nlimbs>
	inline efloat<nlimbs> sinh_working(const efloat<nlimbs>& x, unsigned target) {
		efloat<nlimbs> a(x);
		a.setsign(false);
		efloat<nlimbs> s;
		if (a.scale() < -1) {
			// sinh(a) = u (u + 2) / (2 (u + 1)), u = expm1(a): no cancellation as a -> 0
			const efloat<nlimbs> u = expm1_working(a, target + 4);
			s = u * (u + 2) / ((u + 1) * 2);
		}
		else {
			// (e^a - e^-a) / 2 cancels at most a bit for a >= 1/2
			const efloat<nlimbs> e = exp_working(a, target + 4);
			s = (e - 1 / e) / 2;
		}
		if (x.sign() == -1) s.setsign(true);
		return s;
	}

	template<unsigned nlimbs>
	inline efloat<nlimbs> cosh_working(const efloat<nlimbs>& x, unsigned target) {
		efloat<nlimbs> a(x);
		a.setsign(false);
		const efloat<nlimbs> e = exp_working(a, target + 4);
		return (e + 1 / e) / 2;   // a sum of positives
	}

	template<unsigned nlimbs>
	inline efloat<nlimbs> tanh_working(const efloat<nlimbs>& x, unsigned target) {
		efloat<nlimbs> a(x);
		a.setsign(false);
		a.setexponent(a.scale() + 1);   // 2 |x|, exact
		// tanh |x| = w / (w + 2), w = expm1(2 |x|): no cancellation as x -> 0
		const efloat<nlimbs> w = expm1_working(a, target + 4);
		efloat<nlimbs> t = w / (w + 2);
		if (x.sign() == -1) t.setsign(true);
		return t;
	}

	// asinh, acosh or atanh of a finite x in its domain, x != 0 (x != 1 for acosh), unrounded, at
	// `target` bits plus guard bits, as log1p of an argument that does not cancel; log1p's relative
	// error is at most its argument's
	template<unsigned nlimbs>
	inline efloat<nlimbs> asinh_working(const efloat<nlimbs>& x, unsigned target) {
		efloat<nlimbs> a(x);
		a.setsign(false);
		a.set_precision(target + exp_log_guard_bits + 8);
		// asinh(a) = log1p(a + a^2 / (1 + sqrt(1 + a^2)))
		const efloat<nlimbs> a2 = a * a;
		const efloat<nlimbs> s  = sqrt_to(a2 + 1, a.get_precision());
		efloat<nlimbs> r = log1p_working(a + a2 / (s + 1), target + 4);
		if (x.sign() == -1) r.setsign(true);
		return r;
	}

	template<unsigned nlimbs>
	inline efloat<nlimbs> acosh_working(const efloat<nlimbs>& x, unsigned target) {
		efloat<nlimbs> xw(x);
		xw.set_precision(target + exp_log_guard_bits + 8);
		// acosh(x) = log1p((x - 1) + sqrt((x - 1)(x + 1))); x - 1 is exact for x <= 2
		const efloat<nlimbs> xm1 = xw - 1;
		return log1p_working(xm1 + sqrt_to(xm1 * (xw + 1), xw.get_precision()), target + 4);
	}

	template<unsigned nlimbs>
	inline efloat<nlimbs> atanh_working(const efloat<nlimbs>& x, unsigned target) {
		efloat<nlimbs> a(x);
		a.setsign(false);
		a.set_precision(target + exp_log_guard_bits + 8);
		// atanh(a) = log1p(2a / (1 - a)) / 2 on |x|, where 1 + 2a / (1 - a) does not cancel; 1 - a is
		// exact for a >= 1/2
		efloat<nlimbs> twice_a(a);
		twice_a.setexponent(twice_a.scale() + 1);
		efloat<nlimbs> r = log1p_working(twice_a / (1 - a), target + 4);
		r.setexponent(r.scale() - 1);
		if (x.sign() == -1) r.setsign(true);
		return r;
	}

	// a hyperbolic function of x, rounded to x's precision by Ziv's strategy
	template<unsigned nlimbs, typename Evaluate>
	inline efloat<nlimbs> round_hyperbolic(const efloat<nlimbs>& x, Evaluate evaluate) {
		return round_ziv<nlimbs>([&x, &evaluate](unsigned target) { return evaluate(x, target); },
		                         result_precision(x), hyperbolic_error_bits, x.carries_precision());
	}

	// a tiny x, 2 scale(x) < -(P + 6): f(x) = x + c x^3 + ... lies within 2^-(P + 4) |x| of x
	template<unsigned nlimbs>
	inline bool hyperbolic_tiny(const efloat<nlimbs>& x) {
		return 2 * x.scale() < -static_cast<int64_t>(result_precision(x)) - 6;
	}

	// sinh or cosh overflow at |x| >= 2^17, where exp does
	template<unsigned nlimbs>
	inline efloat<nlimbs> hyperbolic_overflow(const efloat<nlimbs>& x, bool negative) {
		efloat<nlimbs> inf = exp_out_of_range<nlimbs>(false, result_precision(x), x.carries_precision());
		inf.setinf(negative);
		return inf;
	}
}

// sinh: hyperbolic sine (odd). sinh(+/-0)=+/-0, sinh(+/-inf)=+/-inf.
template<unsigned nlimbs>
constexpr efloat<nlimbs> sinh(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.iszero())
		return x;  // preserve signed zero
	if (x.isinf()) {
		efloat<nlimbs> inf;
		inf.setinf(x.sign() == -1);
		return inf;
	}
	if (x.scale() >= 17) return efloat_detail::hyperbolic_overflow<nlimbs>(x, x.sign() == -1);
	// sinh x = x + x^3/6 + ..., away from 0
	if (efloat_detail::hyperbolic_tiny(x)) {
		return efloat_detail::round_near(x, x.sign(), efloat_detail::result_precision(x), x.carries_precision());
	}
	return efloat_detail::round_hyperbolic(x, [](const efloat<nlimbs>& v, unsigned t) { return efloat_detail::sinh_working(v, t); });
}

// cosh: hyperbolic cosine (even). cosh(+/-0)=1, cosh(+/-inf)=+inf.
template<unsigned nlimbs>
constexpr efloat<nlimbs> cosh(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.iszero())
		return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		efloat<nlimbs> inf;
		inf.setinf(false);
		return inf;
	}
	if (x.scale() >= 17) return efloat_detail::hyperbolic_overflow<nlimbs>(x, false);
	// cosh x = 1 + x^2/2 + ..., above 1
	if (efloat_detail::hyperbolic_tiny(x)) {
		return efloat_detail::round_near(efloat<nlimbs>(1.0), 1, efloat_detail::result_precision(x), x.carries_precision());
	}
	return efloat_detail::round_hyperbolic(x, [](const efloat<nlimbs>& v, unsigned t) { return efloat_detail::cosh_working(v, t); });
}

// tanh: hyperbolic tangent (odd). tanh(+/-0)=+/-0, tanh(+/-inf)=+/-1.
template<unsigned nlimbs>
constexpr efloat<nlimbs> tanh(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.iszero())
		return x;  // preserve signed zero
	if (x.isinf())
		return efloat<nlimbs>(x.sign() == -1 ? -1.0 : 1.0);

	const unsigned precision = efloat_detail::result_precision(x);
	// tanh x = x - x^3/3 + ..., toward 0
	if (efloat_detail::hyperbolic_tiny(x)) {
		return efloat_detail::round_near(x, -x.sign(), precision, x.carries_precision());
	}
	// tanh |x| = 1 - 2 e^-2|x| + ..., within 2^-(P + 6) of 1 for |x| > (P + 7) ln 2 / 2, below it
	if (x.scale() >= 17 || std::fabs(double(x)) > static_cast<double>(precision + 7) * 0.34657359027997264) {
		return efloat_detail::round_near(efloat<nlimbs>(x.sign() == -1 ? -1.0 : 1.0), -x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_hyperbolic(x, [](const efloat<nlimbs>& v, unsigned t) { return efloat_detail::tanh_working(v, t); });
}

// asinh: inverse hyperbolic sine (odd). asinh(+/-0)=+/-0, asinh(+/-inf)=+/-inf.
template<unsigned nlimbs>
constexpr efloat<nlimbs> asinh(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.iszero())
		return x;  // preserve signed zero
	if (x.isinf()) {
		efloat<nlimbs> inf;
		inf.setinf(x.sign() == -1);
		return inf;
	}
	// asinh x = x - x^3/6 + ..., toward 0
	if (efloat_detail::hyperbolic_tiny(x)) {
		return efloat_detail::round_near(x, -x.sign(), efloat_detail::result_precision(x), x.carries_precision());
	}
	return efloat_detail::round_hyperbolic(x, [](const efloat<nlimbs>& v, unsigned t) { return efloat_detail::asinh_working(v, t); });
}

// acosh: inverse hyperbolic cosine. Domain x >= 1. acosh(1)=+0, acosh(+inf)=+inf.
//        acosh(x<1) is a domain error -> NaN.
template<unsigned nlimbs>
constexpr efloat<nlimbs> acosh(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.isinf()) {
		if (x.sign() == -1) {  // acosh(-inf) is undefined
			efloat<nlimbs> nan;
			nan.setnan();
			if (!std::is_constant_evaluated())
				efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
			return nan;
		}
		return x;  // acosh(+inf) = +inf
	}

	const efloat<nlimbs> one(1.0);
	if (x < one) {  // includes all negatives; domain error
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated())
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	if (x == one) {  // +0 in every rounding mode
		efloat<nlimbs> zero(0.0);
		efloat_detail::round_result(zero, efloat_detail::result_precision(x), x.carries_precision());
		return zero;
	}
	return efloat_detail::round_hyperbolic(x, [](const efloat<nlimbs>& v, unsigned t) { return efloat_detail::acosh_working(v, t); });
}

// atanh: inverse hyperbolic tangent. Domain |x| < 1. atanh(+/-0)=+/-0,
//        atanh(+/-1)=+/-inf (pole), atanh(|x|>1) and atanh(+/-inf) -> NaN.
template<unsigned nlimbs>
constexpr efloat<nlimbs> atanh(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.iszero())
		return x;  // preserve signed zero
	if (x.isinf()) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated())
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}

	const efloat<nlimbs> one(1.0);
	efloat<nlimbs>       abs_x(x);
	abs_x.setsign(false);
	if (abs_x == one) {  // pole at +/-1
		efloat<nlimbs> inf;
		inf.setinf(x.sign() == -1);
		if (!std::is_constant_evaluated())
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		return inf;
	}
	if (abs_x > one) {  // domain error
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated())
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	// atanh x = x + x^3/3 + ..., away from 0
	if (efloat_detail::hyperbolic_tiny(x)) {
		return efloat_detail::round_near(x, x.sign(), efloat_detail::result_precision(x), x.carries_precision());
	}
	return efloat_detail::round_hyperbolic(x, [](const efloat<nlimbs>& v, unsigned t) { return efloat_detail::atanh_working(v, t); });
}

}  // namespace universal
}  // namespace sw
