// exponent.hpp: exponential functions (exp, exp2, exp10, expm1) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// Natural exponentiation function for efloat using range-reduced Taylor series
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp(const efloat<nlimbs>& x) {
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isnan()) return x;
	if (x.isinf()) {
		return (x.sign() == -1 ? efloat<nlimbs>(0.0) : x);
	}

	// Protect against extremely large inputs that would overflow double/int64_t during range reduction
	if (x.scale() >= 17) {
		if (x.isneg()) {
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::Underflow | ExceptionFlag::Inexact);
			}
			return efloat<nlimbs>(0.0);
		} else {
			efloat<nlimbs> inf;
			inf.setinf(false);
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::Overflow | ExceptionFlag::Inexact);
			}
			return inf;
		}
	}

	// Range reduction: x = k * ln2 + r, where |r| <= 0.5 * ln2
	// High-precision ln(2)
	efloat<nlimbs> ln2;
	parse("0.69314718055994530941723212145817656807550013436025525412068", ln2);

	double x_dbl = double(x);
	int64_t k = static_cast<int64_t>(std::round(x_dbl / 0.6931471805599453));
	efloat<nlimbs> r = x - efloat<nlimbs>(k) * ln2;

	// Taylor series for exp(r)
	efloat<nlimbs> sum(1.0);
	efloat<nlimbs> term(1.0);

	for (unsigned n = 1; n < 100; ++n) {
		term = term * r / efloat<nlimbs>(n);
		if (term.iszero()) break;
		sum += term;
	}

	// Multiply by 2^k using exponent scaling
	if (k != 0) {
		sum.setexponent(sum.scale() + k);
	}

	return sum;
}

// exp2(x): exponential base 2
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp2(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(0.0);
		return x;
	}

	efloat<nlimbs> ln2;
	parse("0.69314718055994530941723212145817656807550013436025525412068", ln2);
	return exp(x * ln2);
}

// exp10(x): exponential base 10
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp10(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(0.0);
		return x;
	}

	efloat<nlimbs> ln10;
	parse("2.3025850929940456840179914546843642076011014886287729760333", ln10);
	return exp(x * ln10);
}

// expm1(x): exp(x) - 1 with Taylor series for small inputs to avoid cancellation
template<unsigned nlimbs>
constexpr efloat<nlimbs> expm1(const efloat<nlimbs>& x) {
	if (x.isnan() || x.iszero()) return x;
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(-1.0);
		return x;
	}

	// For inputs with magnitude >= 0.375, use std formula exp(x) - 1.0
	efloat<nlimbs> abs_x(x);
	abs_x.setsign(false);
	if (abs_x >= efloat<nlimbs>(0.375)) {
		return exp(x) - efloat<nlimbs>(1.0);
	}

	// Taylor series for small x: x + x^2/2! + x^3/3! + ...
	efloat<nlimbs> sum = x;
	efloat<nlimbs> term = x;
	efloat<nlimbs> k(1.0);
	efloat<nlimbs> one(1.0);

	while (true) {
		k = k + one;
		term = (term * x) / k;
		if (term.iszero()) break;
		sum = sum + term;

		if (term.scale() < sum.scale() - sum.get_precision()) {
			break;
		}
	}
	return sum;
}

}} // namespace sw::universal
