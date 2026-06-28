// pow.hpp: power and exponential functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/efloat/math/exponent.hpp>
#include <universal/number/efloat/math/logarithm.hpp>
#include <universal/number/efloat/math/classify.hpp>
#include <universal/number/efloat/math/truncate.hpp>

namespace sw { namespace universal {

// Fast integer power using binary exponentiation
template<unsigned nlimbs>
constexpr efloat<nlimbs> integer_power(efloat<nlimbs> base, int exponent) {
	if (exponent < 0) {
		base = efloat<nlimbs>(1.0) / base;
		exponent = -exponent;
	}
	if (exponent == 0) return efloat<nlimbs>(1.0);
	efloat<nlimbs> power(1.0);
	while (exponent > 1) {
		if (exponent & 0x1) {
			power = base * power;
			base *= base;
			exponent = (exponent - 1) / 2;
		} else {
			base *= base;
			exponent /= 2;
		}
	}
	return base * power;
}

// pow(x, int_y): fast integer power overload
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, int y) {
	return integer_power(x, y);
}

// pow(x, y): arbitrary precision power function
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	if (y.iszero()) {
		return efloat<nlimbs>(1.0);
	}
	if (x.iszero()) {
		if (y.sign() == -1) {
			efloat<nlimbs> inf; inf.setinf(false);
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
			}
			return inf;
		}
		return efloat<nlimbs>(0.0);
	}
	if (x == 1.0) {
		return efloat<nlimbs>(1.0);
	}

	// If x is negative, the exponent y must be an integer, otherwise the result is complex (NaN in real space)
	if (x.sign() == -1) {
		efloat<nlimbs> y_floor = floor(y);
		if (y != y_floor) {
			efloat<nlimbs> nan; nan.setnan();
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
			}
			return nan;
		}
		// y is an integer! Convert to long long to check odd/even
		long long y_int = llrint(y);
		bool is_odd = (y_int % 2 != 0);

		// Compute using abs(x)
		efloat<nlimbs> abs_x(x);
		abs_x.setsign(false);
		efloat<nlimbs> res = exp(y * log(abs_x));
		if (is_odd) {
			res.setsign(true);
		}
		return res;
	}

	// Normal positive base: x^y = exp(y * log(x))
	return exp(y * log(x));
}

// exp2(x): exponential base 2
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp2(const efloat<nlimbs>& x) {
	if (x.isnan() || x.iszero()) return x;
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
	if (x.isnan() || x.iszero()) return x;
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
		sum = sum + term;

		if (term.scale() < sum.scale() - sum.get_precision()) {
			break;
		}
	}
	return sum;
}

}} // namespace sw::universal
