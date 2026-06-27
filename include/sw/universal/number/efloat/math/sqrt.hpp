// sqrt.hpp: square root function for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// Standard square root function for efloat using Newton's method
template<unsigned nlimbs>
constexpr efloat<nlimbs> sqrt(const efloat<nlimbs>& a) {
	if (a.iszero()) return a;
	if (a.isneg()) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (a.isinf() || a.isnan()) return a;

	// Initial guess using double precision sqrt
	efloat<nlimbs> x(std::sqrt(double(a)));
	efloat<nlimbs> half(0.5);

	// Newton iterations (5 iterations double precision 5 times, reaching >1000 bits)
	for (int i = 0; i < 5; ++i) {
		x = half * (x + a / x);
	}
	return x;
}

}} // namespace sw::universal
