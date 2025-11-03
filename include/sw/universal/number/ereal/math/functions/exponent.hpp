#pragma once
// exponent.hpp: exponential functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// exp: exponential function e^x
	// Phase 4a: Implementation using Taylor series with range reduction
	// Strategy:
	//   1. Range reduction: reduce |x| to small value for fast convergence
	//   2. Taylor series: exp(x) = 1 + x + x²/2! + x³/3! + x⁴/4! + ...
	//   3. Reconstruction: use exp properties to recover full result
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Handle special cases
		if (x.iszero()) return Real(1.0);
		if (x.isnan()) return x;
		if (x.isinf()) {
			return x.isneg() ? Real(0.0) : x;
		}

		// Range reduction: reduce |x| to [-0.5, 0.5] for fast Taylor convergence
		// Use exp(x) = exp(x/2^n)^(2^n)
		Real reduced_x = x;
		int reduction_count = 0;

		// Reduce until |x| <= 0.5
		Real half(0.5);
		Real neg_half(-0.5);
		while (reduced_x > half || reduced_x < neg_half) {
			reduced_x = reduced_x * half;
			++reduction_count;
		}

		// Taylor series for exp(reduced_x) where |reduced_x| <= 0.5
		// exp(x) = 1 + x + x²/2! + x³/3! + x⁴/4! + ...
		Real result(1.0);
		Real term = reduced_x;  // First term: x/1!

		// Adaptive termination: stop when term magnitude is negligible
		// Use double precision threshold for now (can be made adaptive later)
		double epsilon = 1.0e-17;  // slightly tighter than double precision

		for (int n = 1; n < 100; ++n) {  // 100 iterations should be plenty for |x| <= 0.5
			result = result + term;

			// Check convergence
			double term_mag = std::abs(double(term));
			if (term_mag < epsilon) break;

			// Compute next term: term_{n+1} = term_n * x / (n+1)
			// NOTE: Use double literal to avoid ereal(int) constructor bug
			term = term * reduced_x / Real(double(n + 1));
		}

		// Reconstruction: exp(x) = exp(reduced_x)^(2^reduction_count)
		// Use repeated squaring
		for (int i = 0; i < reduction_count; ++i) {
			result = result * result;
		}

		return result;
	}

	// exp2: base-2 exponential function 2^x
	// Phase 4a: implement using exp2(x) = exp(x * ln(2))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp2(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;
		// ln(2) ≈ 0.693147180559945309417232121458176568075500134360255254120680009
		Real ln2(0.6931471805599453);
		return exp(x * ln2);
	}

	// exp10: base-10 exponential function 10^x
	// Phase 4a: implement using exp10(x) = exp(x * ln(10))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp10(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;
		// ln(10) ≈ 2.302585092994045684017991454684364207601101488628772976033327900
		Real ln10(2.302585092994045684);
		return exp(x * ln10);
	}

	// expm1: compute e^x - 1 accurately for small x
	// Phase 4a: implement using Taylor series (avoids cancellation for small x)
	// For small x: expm1(x) = x + x²/2! + x³/3! + x⁴/4! + ...
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> expm1(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// For small x, use Taylor series directly
		// This avoids catastrophic cancellation in exp(x) - 1
		Real threshold(0.1);
		Real neg_threshold(-0.1);

		if (x < threshold && x > neg_threshold) {
			// Taylor series: expm1(x) = x + x²/2! + x³/3! + ...
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
