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
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> fmod(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		if (y.iszero()) {
			// fmod(x, 0) is undefined - raise domain error
			throw ereal_divide_by_zero();
		}

		using Real = ereal<maxlimbs, FpType>;
		// n = trunc(x / y) - truncate toward zero
		Real quotient = x / y;  // Uses expansion_quotient
		Real n = trunc(quotient);  // Uses Phase 2 trunc

		// The quotient is rounded, and at an exact multiple it can land just below the integer,
		// which trunc then takes one short: fmod(0.41943359375, 5.1200389862060547e-05), 8192 y
		// exactly, returned y (#1668).  The remainder is exact, so correct n until it is in range:
		// the sign of x, or zero, and below |y| in magnitude.
		const Real ay = abs(y);
		const Real step((x.isneg() == y.isneg()) ? 1.0 : -1.0);    // moves n away from zero
		Real r = x - (n * y);
		for (int i = 0; i < 4; ++i) {
			if (!r.iszero() && r.isneg() != x.isneg()) n = n - step;
			else if (abs(r) >= ay)                     n = n + step;
			else break;
			r = x - (n * y);
		}
		return r;
	}

	// remainder: IEEE remainder of x/y
	// Phase 2: uses expansion quotient with IEEE round-to-nearest-even
	// remainder(x, y) = x - n*y where n = round_to_nearest_even(x/y)
	// Result in range [-|y|/2, |y|/2], chooses closest n
	// On exact halfway cases (frac = 0.5), rounds to even integer
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> remainder(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		using Real = ereal<maxlimbs, FpType>;

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

		// The quotient is rounded, so floor and the half comparison can misjudge an exact multiple
		// or an exact half (#1668).  The remainder is exact: correct n until |r| <= |y| / 2, with
		// an even n when |r| is exactly |y| / 2.
		const Real ay = abs(y);
		Real r = x - (n * y);
		for (int i = 0; i < 4; ++i) {
			const Real twice = abs(r) * Real(2.0);
			const bool odd = floor(n / Real(2.0)) * Real(2.0) != n;
			if (twice < ay || (twice == ay && !odd)) break;
			// r and y of the same sign: a larger n reduces r
			n = n + Real((r.isneg() == y.isneg()) ? 1.0 : -1.0);
			r = x - (n * y);
		}
		return r;
	}

	// modf: split x into integer and fractional parts. Returns the fractional
	// part (same sign as x) and stores the integer part (toward zero) in *iptr.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> modf(const ereal<maxlimbs, FpType>& x, ereal<maxlimbs, FpType>* iptr) {
		if (x.isnan()) {
			if (iptr != nullptr) *iptr = x;
			return x;
		}
		if (x.isinf()) {
			if (iptr != nullptr) *iptr = x;
			return x.isneg() ? -ereal<maxlimbs, FpType>(0.0) : ereal<maxlimbs, FpType>(0.0);
		}
		ereal<maxlimbs, FpType> ipart = trunc(x);
		if (iptr != nullptr) *iptr = ipart;
		return x - ipart;   // fractional part; subtraction carries x's sign
	}

}} // namespace sw::universal
