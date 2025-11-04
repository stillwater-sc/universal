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
	// Phase 4b: Implementation using exp(y * log(x)) with special case handling
	// Special cases:
	//   - x^0 = 1 for any x (including 0)
	//   - 0^y = 0 for y > 0
	//   - 1^y = 1 for any y
	//   - x^1 = x for any x
	//   - x^2, x^3, ... use direct multiplication for small integer powers
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> pow(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		using Real = ereal<maxlimbs>;

		// Special case: y = 0 => x^0 = 1 (for any x, including 0)
		if (y.iszero()) return Real(1.0);

		// Special case: x = 0
		if (x.iszero()) {
			if (y.isneg()) {
				// 0^(-y) = 1/0^y = undefined, return NaN
				return Real(std::numeric_limits<double>::quiet_NaN());
			}
			// 0^y = 0 for y > 0
			return Real(0.0);
		}

		// Special case: x = 1 => 1^y = 1
		if (x.isone()) return Real(1.0);

		// Special case: y = 1 => x^1 = x
		if (y.isone()) return x;

		// Check if y is an integer that fits in int range for optimized calculation
		// This handles all integer exponents (including large ones and negative bases)
		double y_val = double(y);
		double y_int;
		if (std::modf(y_val, &y_int) == 0.0 && y_int >= INT_MIN && y_int <= INT_MAX) {
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

		// General case: x^y = exp(y * log(x))
		// Only valid for x > 0
		if (x.isneg()) {
			// Negative base with non-integer exponent is complex
			return Real(std::numeric_limits<double>::quiet_NaN());
		}

		return exp(y * log(x));
	}

	// pow: power function x^y (mixed type: ereal^double)
	// Phase 4b: Forward to main pow implementation
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> pow(const ereal<maxlimbs>& x, double y) {
		return pow(x, ereal<maxlimbs>(y));
	}

	// pow: power function x^y (mixed type: double^ereal)
	// Phase 4b: Forward to main pow implementation
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> pow(double x, const ereal<maxlimbs>& y) {
		return pow(ereal<maxlimbs>(x), y);
	}

}} // namespace sw::universal
