#pragma once
// trigonometry.hpp: trigonometry functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// sin: sine function
	// Phase 6: Implementation using Taylor series with angle reduction
	// sin(x) = x - x³/3! + x⁵/5! - x⁷/7! + ...
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> sin(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special case
		if (x.iszero()) return Real(0.0);

		// Angle reduction: reduce to [-π, π]
		Real pi(3.141592653589793238462643383279502884);
		Real two_pi = pi * Real(2.0);

		// Reduce angle modulo 2π
		Real reduced_x = x;
		Real x_abs = abs(x);
		if (x_abs > two_pi) {
			// Use fmod-like reduction
			double periods = std::floor(double(x_abs / two_pi));
			reduced_x = x - two_pi * Real(double(periods));
		}

		// Further reduce to [-π, π]
		if (reduced_x > pi) reduced_x = reduced_x - two_pi;
		if (reduced_x < -pi) reduced_x = reduced_x + two_pi;

		// Taylor series: sin(x) = x - x³/3! + x⁵/5! - x⁷/7! + ...
		Real x_squared = reduced_x * reduced_x;
		Real term = reduced_x;
		Real result = term;
		double epsilon = 1.0e-17;

		for (int n = 1; n < 50; ++n) {
			// term = term * (-x²) / ((2n)(2n+1))
			term = term * (-x_squared) / Real(double(2 * n * (2 * n + 1)));
			result = result + term;

			if (std::abs(double(term)) < epsilon) break;
		}

		return result;
	}

	// cos: cosine function
	// Phase 6: Implementation using Taylor series with angle reduction
	// cos(x) = 1 - x²/2! + x⁴/4! - x⁶/6! + ...
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> cos(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special case
		if (x.iszero()) return Real(1.0);

		// Angle reduction: reduce to [-π, π]
		Real pi(3.141592653589793238462643383279502884);
		Real two_pi = pi * Real(2.0);

		// Reduce angle modulo 2π
		Real reduced_x = x;
		Real x_abs = abs(x);
		if (x_abs > two_pi) {
			double periods = std::floor(double(x_abs / two_pi));
			reduced_x = x - two_pi * Real(double(periods));
		}

		// Further reduce to [-π, π]
		if (reduced_x > pi) reduced_x = reduced_x - two_pi;
		if (reduced_x < -pi) reduced_x = reduced_x + two_pi;

		// Taylor series: cos(x) = 1 - x²/2! + x⁴/4! - x⁶/6! + ...
		Real x_squared = reduced_x * reduced_x;
		Real term(1.0);
		Real result = term;
		double epsilon = 1.0e-17;

		for (int n = 1; n < 50; ++n) {
			// term = term * (-x²) / ((2n-1)(2n))
			term = term * (-x_squared) / Real(double((2 * n - 1) * (2 * n)));
			result = result + term;

			if (std::abs(double(term)) < epsilon) break;
		}

		return result;
	}

	// tan: tangent function
	// Phase 6: Implementation using sin/cos
	// tan(x) = sin(x) / cos(x)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> tan(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special case
		if (x.iszero()) return Real(0.0);

		// tan(x) = sin(x) / cos(x)
		Real sin_x = sin(x);
		Real cos_x = cos(x);

		// Check for division by zero (cos(x) = 0 at π/2, 3π/2, etc.)
		if (cos_x.iszero()) {
			return Real(std::numeric_limits<double>::quiet_NaN());
		}

		return sin_x / cos_x;
	}

	// asin: arcsine function
	// Phase 6: Implementation using Taylor series or atan formula
	// For |x| ≤ 0.5: Taylor series asin(x) = x + x³/6 + 3x⁵/40 + ...
	// For |x| > 0.5: asin(x) = π/2 - asin(sqrt(1-x²)) for x > 0
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> asin(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Domain check: |x| must be ≤ 1
		Real abs_x = abs(x);
		Real one(1.0);
		if (abs_x > one) {
			return Real(std::numeric_limits<double>::quiet_NaN());
		}

		// Special cases
		if (x.iszero()) return Real(0.0);
		if (x == one) return Real(1.5707963267948966);  // π/2
		if (x == -one) return Real(-1.5707963267948966);  // -π/2

		// For |x| > 0.8, use asin(x) = sign(x) * (π/2 - asin(sqrt(1-x²)))
		Real threshold(0.8);
		if (abs_x > threshold) {
			Real pi_2(1.5707963267948966);
			Real sqrt_arg = sqrt(one - abs_x * abs_x);
			Real result = pi_2 - asin(sqrt_arg);
			return x.isneg() ? -result : result;
		}

		// Taylor series for |x| ≤ 0.7
		// asin(x) = x + (1/2)x³/3 + (1·3/2·4)x⁵/5 + (1·3·5/2·4·6)x⁷/7 + ...
		Real x_squared = x * x;
		Real term = x;
		Real result = term;
		double epsilon = 1.0e-17;

		for (int n = 1; n < 50; ++n) {
			// Update coefficients: multiply by (2n-1)/(2n) * x²/(2n+1)
			term = term * x_squared * Real(double(2 * n - 1)) / Real(double(2 * n * (2 * n + 1)));
			result = result + term;

			if (std::abs(double(term)) < epsilon) break;
		}

		return result;
	}

	// acos: arccosine function
	// Phase 6: Implementation using acos(x) = π/2 - asin(x)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> acos(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Domain check: |x| must be ≤ 1
		Real abs_x = abs(x);
		Real one(1.0);
		if (abs_x > one) {
			return Real(std::numeric_limits<double>::quiet_NaN());
		}

		// acos(x) = π/2 - asin(x)
		Real pi_2(1.5707963267948966);
		return pi_2 - asin(x);
	}

	// atan: arctangent function
	// Phase 6: Implementation using Taylor series with argument reduction
	// For |x| ≤ 1: atan(x) = x - x³/3 + x⁵/5 - x⁷/7 + ...
	// For |x| > 1: atan(x) = π/2 - atan(1/x) for x > 0
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> atan(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special case
		if (x.iszero()) return Real(0.0);

		// For |x| > 1, use atan(x) = ±π/2 - atan(1/x)
		Real abs_x = abs(x);
		Real one(1.0);
		if (abs_x > one) {
			Real pi_2(1.5707963267948966);
			Real reciprocal_atan = atan(one / x);
			return x.isneg() ? -pi_2 + reciprocal_atan : pi_2 - reciprocal_atan;
		}

		// Taylor series: atan(x) = x - x³/3 + x⁵/5 - x⁷/7 + ...
		Real x_squared = x * x;
		Real term = x;
		Real result = term;
		double epsilon = 1.0e-17;

		for (int n = 1; n < 100; ++n) {
			// term = term * (-x²) with denominator (2n+1)
			term = term * (-x_squared);
			Real series_term = term / Real(double(2 * n + 1));
			result = result + series_term;

			if (std::abs(double(series_term)) < epsilon) break;
		}

		return result;
	}

	// atan2: arctangent of y/x using signs to determine quadrant
	// Phase 6: Implementation using atan with quadrant logic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> atan2(const ereal<maxlimbs>& y, const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		Real pi(3.141592653589793238462643383279502884);
		Real pi_2(1.5707963267948966);

		// Special cases
		if (x.iszero() && y.iszero()) {
			return Real(std::numeric_limits<double>::quiet_NaN());
		}

		if (x.iszero()) {
			// x = 0, result is ±π/2
			return y.isneg() ? -pi_2 : pi_2;
		}

		if (y.iszero()) {
			// y = 0, result is 0 or ±π
			if (x.isneg()) return pi;
			return Real(0.0);
		}

		// General case: compute atan(y/x) and adjust for quadrant
		Real ratio = y / x;
		Real angle = atan(ratio);

		// Quadrant adjustment
		if (x.isneg()) {
			// Quadrants II or III
			if (y.isneg()) {
				angle = angle - pi;  // Quadrant III
			} else {
				angle = angle + pi;  // Quadrant II
			}
		}
		// Quadrants I and IV are already correct from atan(y/x)

		return angle;
	}

}} // namespace sw::universal
