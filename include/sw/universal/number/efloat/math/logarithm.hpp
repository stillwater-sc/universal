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

	// Initial guess using double precision log
	efloat<nlimbs> y(std::log(double(x)));
	efloat<nlimbs> one(1.0);

	// Newton iterations (5 iterations double precision 5 times, reaching >1000 bits)
	for (int i = 0; i < 5; ++i) {
		y = y - one + x / exp(y);
	}
	return y;
}

}} // namespace sw::universal
