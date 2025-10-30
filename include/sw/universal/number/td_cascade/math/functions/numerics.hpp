#pragma once
// numerics.hpp: numerics functions for triple-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { namespace universal {

	// clang <complex> implementation is calling these functions so we need implementations for doubledouble (td)


	// copysign returns a value with the magnitude of a, and the sign of b
	inline td_cascade copysign(const td_cascade& a, const td_cascade& b) {
		auto signA = std::copysign(1.0, a[0]);
		auto signB = std::copysign(1.0, b[0]);

		return signA != signB ? -a : a;
	}

	// decompose quad-double into a fraction and an exponent
	inline td_cascade frexp(const td_cascade& a, int* pexp) {
		double a0 = std::frexp(a[0], pexp);
		double a1 = std::ldexp(a[1], -*pexp);
		double a2 = std::ldexp(a[2], -*pexp);
		return td_cascade(a0, a1, a2);
	}

	// recompose quad-double from a fraction and an exponent
	inline td_cascade ldexp(const td_cascade& a, int exponent) {
		static_assert(std::numeric_limits< td_cascade >::radix == 2, "CONFIGURATION: td_cascade radix must be 2!");
		static_assert(std::numeric_limits< double >::radix == 2, "CONFIGURATION: double radix must be 2!");

		return td_cascade(std::ldexp(a[0], exponent), std::ldexp(a[1], exponent), std::ldexp(a[2], exponent));
	}

}} // namespace sw::universal
