#pragma once
// numerics.hpp: numerics functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { namespace universal {

	// clang <complex> implementation is calling these functions so we need implementations for doubledouble (qd)


	// copysign returns a value with the magnitude of a, and the sign of b
	qd copysign(qd const& a, qd const& b) {
		auto signA = std::copysign(1.0, a[0]);
		auto signB = std::copysign(1.0, b[0]);

		return signA != signB ? -a : a;
	}

	// decompose quad-double into a fraction and an exponent
	qd frexp(qd const& a, int* pexp) {
		double hi = std::frexp(a[0], pexp);
		double lo = std::ldexp(a[1], -*pexp);
		return qd(hi, lo);
	}

	// recompose quad-double from a fraction and an exponent
	qd ldexp(qd const& a, int exp) {
		static_assert(std::numeric_limits< qd >::radix == 2, "CONFIGURATION: qd radix must be 2!");
		static_assert(std::numeric_limits< double >::radix == 2, "CONFIGURATION: double radix must be 2!");

		return qd(std::ldexp(a[0], exp), std::ldexp(a[1], exp));
	}

}} // namespace sw::universal
