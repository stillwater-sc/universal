#pragma once
// numerics.hpp: numerics functions for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { namespace universal {

	// copysign returns a value with the magnitude of a, and the sign of b
inline dd_cascade copysign(const dd_cascade& a, const dd_cascade& b) {
		auto signA = std::copysign(1.0, a.high());
		auto signB = std::copysign(1.0, b.high());

		return signA != signB ? -a : a;
	}

	// decompose doubledouble into a fraction and an exponent
    inline dd_cascade frexp(const dd_cascade& a, int* pexp) {
		double hi = std::frexp(a.high(), pexp);
		double lo = std::ldexp(a.low(), -*pexp);
	    return dd_cascade(hi, lo);
	}

	// recompose doubledouble from a fraction and an exponent
    inline dd_cascade ldexp(const dd_cascade& a, int exp) {
	    static_assert(std::numeric_limits<dd_cascade>::radix == 2, "CONFIGURATION: doubledouble radix must be 2!");
		static_assert(std::numeric_limits< double >::radix == 2, "CONFIGURATION: double radix must be 2!");

		return dd_cascade(std::ldexp(a.high(), exp), std::ldexp(a.low(), exp));
	}

}} // namespace sw::universal
