#pragma once
// logarithm.hpp: logarithm functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// log: natural logarithm (base e) - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision logarithm:
	// 1. Efficient range reduction using frexp (extracts exponent exactly)
	// 2. Artanh-based series for mantissa (fast convergence)
	// 3. Pure ereal arithmetic (no double contamination)
	// 4. Adaptive convergence based on working precision
	//
	// ALGORITHM OVERVIEW:
	// ------------------
	// The logarithm uses range reduction to avoid slow convergence:
	//   log(x) = log(m * 2^e) = log(m) + e*ln(2)
	//
	// Where frexp() gives: x = m * 2^e with 0.5 <= m < 1
	//
	// SERIES SELECTION:
	// -----------------
	// For the mantissa m in [0.5, 1), we use the artanh-based series:
	//   u = (m-1)/(m+1)  ->  m = (1+u)/(1-u)
	//   log(m) = log((1+u)/(1-u)) = 2*artanh(u) = 2(u + u^3/3 + u^5/5 + u^7/7 + ...)
	//
	// This converges much faster than the naive log(1+z) series because:
	//   - For m in [0.5, 1), we have u in [-1/3, 0)
	//   - Series error ~= u^(2n+1) drops rapidly since |u| <= 1/3
	//   - Achieves ~53 bits in ~10 terms (vs. ~50 terms for log(1+z))
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	//     - Analysis of artanh-based log series
	// [2] Borwein, J. M. & Borwein, P. B. (1987). "Pi and the AGM"
	//     - AGM-based methods for ultimate performance
	// [3] MPFR library: https://www.mpfr.org/algorithms.pdf
	//     - Production implementation strategies
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to remove double contamination and add adaptive convergence
	//
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> log(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// ============================================================================
		// STEP 1: Handle special cases
		// ============================================================================
		if (x.iszero()) {
			// log(0) = -inf (return large negative value)
			return Real(-1.0e308);
		}
		if (x.isneg()) {
			// log(negative) = NaN
			return Real(std::numeric_limits<double>::quiet_NaN());
		}
		if (x.isone()) return Real(0.0);

		// ============================================================================
		// STEP 2: Range reduction using frexp
		// ============================================================================
		// Extract: x = mantissa * 2^exponent where 0.5 <= mantissa < 1
		// Then: log(x) = log(mantissa) + exponent*ln(2)
		//
		int exponent;
		Real mantissa = frexp(x, &exponent);

		// ln(2) at full ereal precision (#1002)
		Real ln2 = ereal_ln2<maxlimbs, FpType>();

		// ============================================================================
		// STEP 3: Compute log(mantissa) using artanh-based series
		// ============================================================================
		// Transform: m = (1+u)/(1-u), solve for u: u = (m-1)/(m+1)
		// Then: log(m) = log((1+u)/(1-u)) = 2*artanh(u)
		//              = 2(u + u^3/3 + u^5/5 + u^7/7 + ...)
		//
		// For m in [0.5, 1): u in [-1/3, 0), so |u| <= 1/3
		// Convergence: error after n terms ~= (1/3)^(2n+1)/(2n+1)
		// Example: n=10 gives error ~= 10^(-6), n=20 gives error ~= 10^(-11)
		//
		Real one(1.0);
		Real u = (mantissa - one) / (mantissa + one);

		// Series: log(m) = 2*Sum(n=0 to inf) u^(2n+1)/(2n+1)
		Real u_squared = u * u;
		Real term = u;
		Real result = term;

		// Adaptive convergence threshold
		// how far the series must run for this configuration, and the convergence test:
		// both in the limb type's own terms (#1567)
		const int precision_bits = series_precision_bits<maxlimbs, FpType>();
		int max_iterations = precision_bits;

		for (int n = 1; n < max_iterations; ++n) {
			// Next term: u^(2n+1) / (2n+1)
			term = term * u_squared;

			// Denominator: 2n+1 (must avoid double contamination)
			Real denominator = Real(static_cast<double>(2 * n + 1));
			Real series_term = term / denominator;
			result = result + series_term;

			// Check convergence
			if (series_converged(series_term, result, precision_bits)) break;
		}

		// Multiply by 2 for the artanh series
		Real two(2.0);
		result = result * two;

		// ============================================================================
		// STEP 4: Add exponent contribution
		// ============================================================================
		// log(x) = log(m) + e*ln(2)
		if (exponent != 0) {
			Real exp_term = Real(static_cast<double>(exponent)) * ln2;
			result = result + exp_term;
		}

		return result;
	}

	// log2: binary logarithm (base 2)
	// Phase 4a: implement using log(x) / log(2)
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> log2(const ereal<maxlimbs, FpType>& x) {
		// log2(x) = ln(x) / ln(2), ln 2 at full ereal precision (#1002)
		return log(x) / ereal_ln2<maxlimbs, FpType>();
	}

	// log10: common logarithm (base 10)
	// Phase 4a: implement using log(x) / log(10)
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> log10(const ereal<maxlimbs, FpType>& x) {
		// log10(x) = ln(x) / ln(10), ln 10 at full ereal precision (#1002)
		return log(x) / ereal_ln10<maxlimbs, FpType>();
	}

	// log1p: compute log(1 + x) accurately for small x
	// Phase 4a: implement using Taylor series (avoids cancellation for small x)
	// For small x: log(1+x) = x - x^2/2 + x^3/3 - x^4/4 + ...
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> log1p(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// For small x, use Taylor series directly
		// This avoids catastrophic cancellation in log(1+x)
		Real threshold(0.1);
		Real neg_threshold(-0.1);

		if (x < threshold && x > neg_threshold) {
			// Taylor series: log(1+x) = x - x^2/2 + x^3/3 - x^4/4 + ...
			Real result = x;
			Real term = x;
			Real neg_x = -x;

			// the series runs to this configuration's precision, and the 100-term bound
			// that follows is generous for |x| < 0.1 at any width
			const int precision_bits = series_precision_bits<maxlimbs, FpType>();

			for (int n = 2; n < 100; ++n) {
				// Alternating series: term_n = -term_{n-1} * x / n
				// NOTE: Use double literal to avoid ereal(int) constructor bug
				term = term * neg_x / Real(double(n));
				result = result + term;

				if (series_converged(term, result, precision_bits)) break;
			}

			return result;
		}
		else {
			// For larger x, use log(1+x)
			Real one(1.0);
			return log(one + x);
		}
	}

}} // namespace sw::universal
