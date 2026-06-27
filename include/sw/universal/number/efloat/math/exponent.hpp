// exponent.hpp: exponential functions for efloat
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

}} // namespace sw::universal
