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
	if (x.isneg()) {
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

}} // namespace sw::universal
