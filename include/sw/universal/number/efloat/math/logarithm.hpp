// logarithm.hpp: logarithmic functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/efloat/math/exponent.hpp>

namespace sw { namespace universal {

// Natural logarithm function for efloat using Newton's method on exp(y) - x = 0
template<unsigned nlimbs>
constexpr efloat<nlimbs> log(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision()); // maintain dynamic precision contract
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	// Signbit check captures both negative normal values and negative infinity
	if (x.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.isinf()) return x;

	// High-precision ln(2)
	efloat<nlimbs> ln2;
	parse("0.69314718055994530941723212145817656807550013436025525412068", ln2);

	// Initial guess: y0 = 0.3 + scale * ln2 (accurate and completely avoids double overflows!)
	efloat<nlimbs> y = efloat<nlimbs>(0.3) + efloat<nlimbs>(x.scale()) * ln2;
	efloat<nlimbs> one(1.0);

	// Newton iterations (7 iterations converge up to 2048 bits!)
	for (int i = 0; i < 7; ++i) {
		y = y - one + x / exp(y);
	}
	return y;
}

// Logarithm base 2 function for efloat
template<unsigned nlimbs>
constexpr efloat<nlimbs> log2(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision());
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	if (x.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.isinf()) return x;

	efloat<nlimbs> ln2;
	parse("0.69314718055994530941723212145817656807550013436025525412068", ln2);
	return log(x) / ln2;
}

// Logarithm base 10 function for efloat
template<unsigned nlimbs>
constexpr efloat<nlimbs> log10(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision());
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	if (x.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.isinf()) return x;

	efloat<nlimbs> ln10;
	parse("2.3025850929940456840179914546843642076011014886287729760333", ln10);
	return log(x) / ln10;
}

// Natural logarithm of 1 + x function for efloat
template<unsigned nlimbs>
constexpr efloat<nlimbs> log1p(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;
	if (x < efloat<nlimbs>(-1.0)) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x == efloat<nlimbs>(-1.0)) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision());
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	if (x.isinf()) {
		if (x.sign() == -1) {
			efloat<nlimbs> nan;
			nan.setnan();
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
			}
			return nan;
		}
		return x;
	}

	// Precision-safe range check completely in efloat space (avoids double underflows)
	if (abs(x) < efloat<nlimbs>(0.375)) {
		// Taylor series for log(1 + x) prevents cancellation as x -> 0
		efloat<nlimbs> sum(0.0);
		efloat<nlimbs> term(x);
		efloat<nlimbs> x_pow(x);

		sum += term;
		for (unsigned n = 2; n < 200; ++n) {
			x_pow = x_pow * x;
			term = x_pow / efloat<nlimbs>(n);
			
			// Dynamic precision-safe termination: stops when next term cannot affect the sum
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(sum.get_precision())) {
				break;
			}
			if (n % 2 == 0) {
				sum -= term;
			} else {
				sum += term;
			}
		}
		return sum;
	} else {
		return log(efloat<nlimbs>(1.0) + x);
	}
}

}} // namespace sw::universal
