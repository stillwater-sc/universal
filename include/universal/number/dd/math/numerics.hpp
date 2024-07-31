#pragma once
// numerics.hpp: numerics functions for doubledouble (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { namespace universal {

	// clang <complex> implementation is calling these functions so we need implementations for doubledouble (dd)


	// copysign returns a value with the magnitude of a, and the sign of b
	dd copysign(dd const& a, dd const& b) {
		auto signA = std::copysign(1.0, a.high());
		auto signB = std::copysign(1.0, b.high());

		return signA != signB ? -a : a;
	}

	// decompose doubledouble into a fraction and an exponent
	dd frexp(dd const& a, int* pexp) {
		double hi = std::frexp(a.high(), pexp);
		double lo = std::ldexp(a.low(), -*pexp);
		return dd(hi, lo);
	}

	// recompose doubledouble from a fraction and an exponent
	dd ldexp(dd const& a, int exp) {
		static_assert(std::numeric_limits< dd >::radix == 2, "CONFIGURATION: dd radix must be 2!");
		static_assert(std::numeric_limits< double >::radix == 2, "CONFIGURATION: double radix must be 2!");

		return dd(std::ldexp(a.high(), exp), std::ldexp(a.low(), exp));
	}

}} // namespace sw::universal
