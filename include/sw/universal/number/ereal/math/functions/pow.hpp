#pragma once
// pow.hpp: power functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <climits>

namespace sw { namespace universal {

	// pow: power function x^y
	// Small integer exponents use binary powering; the rest exp(y * log(|x|)), with the sign of
	// an odd integer exponent for a negative base.  Special values follow C99 F.10.4.4:
	//   - pow(x, +-0) = 1 and pow(+1, y) = 1, even for a NaN operand
	//   - pow(-1, +-inf) = 1; pow(x, -inf) is +inf for |x| < 1 and +0 for |x| > 1, and
	//     pow(x, +inf) the reverse
	//   - pow(+-0, y): +-0 or +-inf for an odd integer y > 0 or < 0, else +0 or +inf
	//   - pow(-inf, y): -0 or -inf for an odd integer y < 0 or > 0, else +0 or +inf;
	//     pow(+inf, y) is +0 for y < 0 and +inf for y > 0
	//   - NaN for a negative finite base and a finite non-integer exponent
	// pow(0, -1) used to be NaN, and pow(-2, 1e200), an even integer beyond int, NaN too (#2006).
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> pow(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		using Real = ereal<maxlimbs, FpType>;

		if (y.iszero()) return Real(1.0);
		if (x.isone()) return Real(1.0);
		if (x.isnan() || y.isnan()) return ereal_detail::quiet_nan<maxlimbs, FpType>();

		if (y.isinf()) {
			const Real ax = abs(x);
			const Real one(1.0);
			if (ax == one) return one;                             // pow(-1, +-inf)
			const bool grows = (ax > one) == !y.isneg();
			return grows ? ereal_detail::signed_inf<maxlimbs, FpType>(false) : Real(0.0);
		}

		const bool y_odd = ereal_detail::is_odd_integer(y);
		if (x.iszero()) {
			const bool negative = y_odd && x.signbit();
			return y.isneg() ? ereal_detail::signed_inf<maxlimbs, FpType>(negative)
			                 : ereal_detail::signed_zero<maxlimbs, FpType>(negative);
		}
		if (x.isinf()) {
			const bool negative = y_odd && x.isneg();
			return y.isneg() ? ereal_detail::signed_zero<maxlimbs, FpType>(negative)
			                 : ereal_detail::signed_inf<maxlimbs, FpType>(negative);
		}

		// finite, nonzero x and y from here on

		// Special case: y = 1 => x^1 = x
		if (y.isone()) return x;

		// Check if y is an integer that fits in int range for optimized calculation
		// This handles all integer exponents (including large ones and negative bases)
		// y itself must be the integer, not only its leading limb: 3 + 1e-30 is not (#1668)
		double y_val = double(y);
		double y_int;
		if (std::modf(y_val, &y_int) == 0.0 && y_int >= INT_MIN && y_int <= INT_MAX && y == Real(y_int)) {
			// y is an integer that fits in int range, use repeated squaring
			// This correctly handles negative bases with integer exponents
			int n = static_cast<int>(y_int);

			// Fast paths for very small exponents
			if (n == 2) return x * x;
			if (n == 3) return x * x * x;
			if (n == -1) return Real(1.0) / x;
			if (n == -2) {
				Real x_sq = x * x;
				return Real(1.0) / x_sq;
			}

			// General integer power using repeated squaring
			// Works for any integer n (positive or negative, large or small)
			// Correctly handles negative bases: (-2)^3 = -8, (-2)^4 = 16
			if (n > 0) {
				Real result(1.0);
				Real base = x;
				int exp = n;

				while (exp > 0) {
					if (exp & 1) result = result * base;
					base = base * base;
					exp >>= 1;
				}
				return result;
			}
			else if (n < 0) {
				// Negative integer power: x^(-n) = 1 / x^n
				Real result(1.0);
				Real base = x;
				int exp = -n;

				while (exp > 0) {
					if (exp & 1) result = result * base;
					base = base * base;
					exp >>= 1;
				}
				return Real(1.0) / result;
			}
			// n == 0 is already handled at the top of the function
		}

		// General case: x^y = exp(y * log(|x|)).  A negative base needs an integer exponent, one
		// too large for the loop above, and takes its sign from the exponent's parity.
		if (x.isneg()) {
			if (!ereal_detail::is_integer(y)) return ereal_detail::quiet_nan<maxlimbs, FpType>();
			const Real r = exp(y * log(-x));
			return y_odd ? -r : r;
		}

		return exp(y * log(x));
	}

	// pow: power function x^y (mixed type: ereal^double)
	// Phase 4b: Forward to main pow implementation
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> pow(const ereal<maxlimbs, FpType>& x, double y) {
		return pow(x, ereal<maxlimbs, FpType>(y));
	}

	// pow: power function x^y (mixed type: double^ereal)
	// Phase 4b: Forward to main pow implementation
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> pow(double x, const ereal<maxlimbs, FpType>& y) {
		return pow(ereal<maxlimbs, FpType>(x), y);
	}

}} // namespace sw::universal
