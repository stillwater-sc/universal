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
// ----------------------------------------------------------------------------

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

	const efloat<nlimbs> one(1.0), two(2.0), half(0.5);
	efloat<nlimbs>       abs_x(x);
	abs_x.setsign(false);
	if (abs_x < half) {
		// sinh(x) = u(u+2) / (2(u+1)),  u = e^x - 1 = expm1(x); accurate as x -> 0
		efloat<nlimbs> u = expm1(x);
		return u * (u + two) / (two * (u + one));
	}
	// |x| >= 0.5: no cancellation, and exp's inf/zero handling gives the right
	// overflow behavior (sinh(+huge)=+inf, sinh(-huge)=-inf).
	return (exp(x) - exp(-x)) * half;
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

	const efloat<nlimbs> one(1.0), half(0.5);
	efloat<nlimbs>       abs_x(x);
	abs_x.setsign(false);            // cosh is even
	efloat<nlimbs> ex = exp(abs_x);  // >= 1 (or +inf for huge x)
	// cosh(x) = (e^|x| + e^-|x|)/2; a sum of positives, so never cancels.
	return (ex + one / ex) * half;
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

	const efloat<nlimbs> one(1.0), two(2.0);
	efloat<nlimbs>       abs_x(x);
	abs_x.setsign(false);
	// tanh(|x|) = w/(w+2),  w = e^(2|x|) - 1 = expm1(2|x|); accurate as x -> 0.
	efloat<nlimbs> w = expm1(two * abs_x);
	efloat<nlimbs> t = w.isinf() ? one : (w / (w + two));  // saturates to 1 for huge |x|
	if (x.sign() == -1)
		t.setsign(true);
	return t;
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

	const efloat<nlimbs> one(1.0);
	efloat<nlimbs>       a(x);
	a.setsign(false);
	// asinh(a) = log(a + sqrt(a^2+1)) = log1p(a + a^2/(1 + sqrt(1+a^2)))
	// (the log1p form avoids cancellation in the argument as a -> 0).
	efloat<nlimbs> a2 = a * a;
	efloat<nlimbs> s  = sqrt(one + a2);
	efloat<nlimbs> r  = log1p(a + a2 / (one + s));
	if (x.sign() == -1)
		r.setsign(true);
	return r;
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
	// acosh(x) = log(x + sqrt(x^2-1)) = log1p((x-1) + sqrt((x-1)(x+1)))
	// (the log1p form keeps precision near x = 1, where acosh -> 0).
	efloat<nlimbs> xm1 = x - one;
	return log1p(xm1 + sqrt(xm1 * (x + one)));
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

	const efloat<nlimbs> one(1.0), two(2.0), half(0.5);
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
	// atanh(x) = 0.5*log((1+x)/(1-x)) = 0.5*log1p(2x/(1-x)); accurate as x -> 0
	// and sign-correct for negative x.
	return half * log1p((two * x) / (one - x));
}

}  // namespace universal
}  // namespace sw
