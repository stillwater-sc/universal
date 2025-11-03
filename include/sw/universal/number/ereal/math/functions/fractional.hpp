#pragma once
// fractional.hpp: fractional support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fmod: floating-point remainder of x/y
	// Phase 2: uses expansion quotient and trunc
	// fmod(x, y) = x - n*y where n = trunc(x/y)
	// Result has same sign as x, |result| < |y|
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> fmod(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		if (y.iszero()) {
			// fmod(x, 0) is undefined - return x for now
			// TODO: proper NaN handling when ereal supports it
			return x;
		}

		// n = trunc(x / y) - truncate toward zero
		ereal<maxlimbs> quotient = x / y;  // Uses expansion_quotient
		ereal<maxlimbs> n = trunc(quotient);  // Uses Phase 2 trunc

		// result = x - n * y
		return x - (n * y);
	}

	// remainder: IEEE remainder of x/y
	// Phase 2: uses expansion quotient and round
	// remainder(x, y) = x - n*y where n = round(x/y)
	// Result in range [-|y|/2, |y|/2], chooses closest n
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> remainder(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		if (y.iszero()) {
			// remainder(x, 0) is undefined - return x for now
			// TODO: proper NaN handling when ereal supports it
			return x;
		}

		// n = round(x / y) - round to nearest
		ereal<maxlimbs> quotient = x / y;  // Uses expansion_quotient
		ereal<maxlimbs> n = round(quotient);  // Uses Phase 2 round

		// result = x - n * y
		return x - (n * y);
	}

}} // namespace sw::universal
