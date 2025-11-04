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
			// fmod(x, 0) is undefined - raise domain error
			throw ereal_divide_by_zero();
		}

		// n = trunc(x / y) - truncate toward zero
		ereal<maxlimbs> quotient = x / y;  // Uses expansion_quotient
		ereal<maxlimbs> n = trunc(quotient);  // Uses Phase 2 trunc

		// result = x - n * y
		return x - (n * y);
	}

	// remainder: IEEE remainder of x/y
	// Phase 2: uses expansion quotient with IEEE round-to-nearest-even
	// remainder(x, y) = x - n*y where n = round_to_nearest_even(x/y)
	// Result in range [-|y|/2, |y|/2], chooses closest n
	// On exact halfway cases (frac = 0.5), rounds to even integer
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> remainder(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		using Real = ereal<maxlimbs>;

		if (y.iszero()) {
			// remainder(x, 0) is undefined - raise domain error
			throw ereal_divide_by_zero();
		}

		// Compute quotient = x / y
		Real quotient = x / y;

		// IEEE round-to-nearest-even:
		// Get floor and ceiling of quotient
		Real floor_q = floor(quotient);
		Real ceil_q = ceil(quotient);

		// Compute fractional part
		Real frac = quotient - floor_q;

		// Choose n based on IEEE round-to-nearest-even rule
		Real n;
		Real half(0.5);

		if (frac < half) {
			// Fraction < 0.5: round down
			n = floor_q;
		}
		else if (frac > half) {
			// Fraction > 0.5: round up
			n = ceil_q;
		}
		else {
			// Fraction == 0.5: tie-breaking case
			// Round to even: choose floor_q if even, else ceil_q
			// An integer is even if floor(n/2) * 2 == n
			Real two(2.0);
			Real floor_q_div_2 = floor_q / two;
			Real floor_q_div_2_floor = floor(floor_q_div_2);
			Real twice_floor = floor_q_div_2_floor * two;

			if (twice_floor == floor_q) {
				// floor_q is even
				n = floor_q;
			}
			else {
				// floor_q is odd, so ceil_q is even
				n = ceil_q;
			}
		}

		// Return x - n * y
		return x - (n * y);
	}

}} // namespace sw::universal
