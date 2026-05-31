#pragma once
// exponent.hpp: exponential functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// exp: exponential function e^x - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision exponential:
	// 1. Aggressive range reduction to [-0.5, 0.5] for rapid Taylor convergence
	// 2. Pure ereal arithmetic (no double contamination)
	// 3. Adaptive convergence based on working precision
	// 4. Efficient reconstruction via repeated squaring
	//
	// ALGORITHM OVERVIEW:
	// ------------------
	// The Taylor series for exp converges as:
	//   exp(x) = 1 + x + x^2/2! + x^3/3! + x^4/4! + ... = Sum x^n/n!
	//
	// Convergence rate is O(x^n/n!), so we need |x| << 1 for efficiency.
	//
	// RANGE REDUCTION STRATEGY:
	// -------------------------
	// 1. Reduce x -> x/2^k until |x/2^k| <= 0.5
	// 2. Compute exp(x/2^k) using Taylor series (converges in ~20 terms)
	// 3. Reconstruct: exp(x) = [exp(x/2^k)]^(2^k) using repeated squaring
	//
	// For |x| <= 0.5: Taylor series gives ~53 bits per 20 terms
	// Total cost: ~20 terms + k squarings, where k ~= log2(|x|+1)
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	//     - Comprehensive treatment of argument reduction for exp
	// [2] Kahan, W. (1987). "Branch Cuts for Complex Elementary Functions"
	//     - Numerical stability considerations
	// [3] MPFR library: https://www.mpfr.org/algorithms.pdf
	//     - Production implementation details
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to remove double contamination and add adaptive convergence
	//
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// ============================================================================
		// STEP 1: Handle special cases
		// ============================================================================
		if (x.iszero()) return Real(1.0);
		if (x.isnan()) return x;
		if (x.isinf()) {
			return x.isneg() ? Real(0.0) : x;
		}

		// ============================================================================
		// STEP 2: Range reduction - reduce |x| to [-0.5, 0.5]
		// ============================================================================
		// Use identity: exp(x) = [exp(x/2^k)]^(2^k)
		// This reduces the argument exponentially, ensuring fast Taylor convergence
		//
		Real reduced_x = x;
		int reduction_count = 0;

		Real half(0.5);
		Real neg_half(-0.5);

		while (reduced_x > half || reduced_x < neg_half) {
			reduced_x = reduced_x * half;
			++reduction_count;
		}
		// Now |reduced_x| <= 0.5, Taylor series converges rapidly

		// ============================================================================
		// STEP 3: Taylor series for exp(reduced_x)
		// ============================================================================
		// Taylor series: exp(x) = 1 + x + x^2/2! + x^3/3! + x^4/4! + ...
		//              = Sum(n=0 to inf) x^n/n!
		//
		// For |x| <= 0.5: After n terms, error ~= |x|^(n+1)/(n+1)!
		// Example: |x| = 0.5, n = 20 gives error ~= 2^(-69), sufficient for 20+ decimal digits
		//
		Real result(1.0);
		Real term = reduced_x;  // First term: x^1/1!

		// Adaptive convergence threshold
		int precision_digits = static_cast<int>(53.0 * maxlimbs / 3.322);
		int max_iterations = precision_digits * 2;  // Generous safety margin

		double threshold = 1.0;
		for (int i = 0; i < precision_digits; ++i) {
			threshold *= 0.1;
		}

		for (int n = 1; n < max_iterations; ++n) {
			result = result + term;

			// Check convergence
			double term_mag = std::abs(double(term));
			if (term_mag < threshold) break;

			// Compute next term: term_{n+1} = term_n * x / (n+1)
			// Critical: Must avoid double conversion of (n+1)
			Real n_plus_1 = Real(static_cast<double>(n + 1));
			term = term * reduced_x / n_plus_1;
		}

		// Don't forget to add the final term
		result = result + term;

		// ============================================================================
		// STEP 4: Reconstruction via repeated squaring
		// ============================================================================
		// Compute exp(x) = [exp(x/2^k)]^(2^k) by squaring k times
		// This is numerically stable and efficient: k squarings vs. computing exp(x) directly
		//
		for (int i = 0; i < reduction_count; ++i) {
			result = result * result;
		}

		return result;
	}

	// exp2: base-2 exponential function 2^x
	// Implements: 2^x = exp(x * ln(2))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp2(const ereal<maxlimbs>& x) {
		// 2^x = exp(x * ln 2), with ln 2 at full ereal precision (#1002).
		return exp(x * ereal_ln2<maxlimbs>());
	}

	// exp10: base-10 exponential function 10^x
	// Implements: 10^x = exp(x * ln(10))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp10(const ereal<maxlimbs>& x) {
		// 10^x = exp(x * ln 10), with ln 10 at full ereal precision (#1002).
		return exp(x * ereal_ln10<maxlimbs>());
	}

	// expm1: compute e^x - 1 accurately for small x
	// Phase 4a: implement using Taylor series (avoids cancellation for small x)
	// For small x: expm1(x) = x + x^2/2! + x^3/3! + x^4/4! + ...
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> expm1(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// For small x, use Taylor series directly
		// This avoids catastrophic cancellation in exp(x) - 1
		Real threshold(0.1);
		Real neg_threshold(-0.1);

		if (x < threshold && x > neg_threshold) {
			// Taylor series: expm1(x) = x + x^2/2! + x^3/3! + ...
			Real result = x;
			Real term = x;

			double epsilon = 1.0e-17;

			for (int n = 2; n < 100; ++n) {
				// Compute next term: term_n = term_{n-1} * x / n
				// NOTE: Use double literal to avoid ereal(int) constructor bug
				term = term * x / Real(double(n));
				result = result + term;

				double term_mag = std::abs(double(term));
				if (term_mag < epsilon) break;
			}

			return result;
		}
		else {
			// For larger x, use exp(x) - 1
			return exp(x) - Real(1.0);
		}
	}

}} // namespace sw::universal
