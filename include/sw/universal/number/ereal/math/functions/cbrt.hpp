#pragma once
// cbrt.hpp: cbrt function for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// cbrt: cube root function
	// Phase 3: Full adaptive-precision implementation with range reduction
	//   Strategy: Use frexp/ldexp for range reduction, then Newton-Raphson
	//   Algorithm: (1) Extract sign, (2) Use frexp to get r × 2^e,
	//             (3) Adjust exponent divisible by 3, (4) Newton iteration on r,
	//             (5) Scale result by 2^(e/3), (6) Restore sign
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> cbrt(const ereal<maxlimbs>& a) {
		// Handle special cases
		if (a.iszero()) return ereal<maxlimbs>(0.0);
		if (a.isnan() || a.isinf()) return a;

		// Extract and save sign (cbrt preserves sign)
		bool negative = a.isneg();
		ereal<maxlimbs> abs_a = negative ? -a : a;

		// Use frexp to get: abs_a = r × 2^e where 0.5 ≤ r < 1
		int e;
		ereal<maxlimbs> r = frexp(abs_a, &e);

		// Adjust exponent to be divisible by 3
		// This keeps r in range [0.125, 1.0) and ensures 2^(e/3) is exact
		while (e % 3 != 0) {
			++e;
			r = ldexp(r, -1);  // r = r / 2
		}

		// At this point: 0.125 ≤ r < 1.0 and e is divisible by 3
		// Initial approximation for cbrt(r) from high component
		const auto& r_limbs = r.limbs();
		ereal<maxlimbs> x = std::cbrt(r_limbs[0]);

		// Determine iterations for adaptive precision
		int iterations = 3 + static_cast<int>(std::log2(maxlimbs + 1));

		// Newton-Raphson for cbrt: x' = (2x + r/x²) / 3
		// This converges to cbrt(r)
		for (int i = 0; i < iterations; ++i) {
			ereal<maxlimbs> x_squared = x * x;
			x = (ereal<maxlimbs>(2.0) * x + r / x_squared) / ereal<maxlimbs>(3.0);
		}

		// Scale by 2^(e/3) to get cbrt(abs_a)
		x = ldexp(x, e / 3);

		// Restore sign
		if (negative) x = -x;

		return x;
	}

}} // namespace sw::universal
