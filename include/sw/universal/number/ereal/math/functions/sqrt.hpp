#pragma once
// sqrt.hpp: sqrt function for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// sqrt: square root function
	// Phase 3: Full adaptive-precision Newton-Raphson iteration
	//   Strategy: Use Newton-Raphson: x' = (x + a/x) / 2
	//   Starting with x = sqrt(high component), iterate to requested precision
	//   For ereal<maxlimbs>: iterations = 3 + log2(maxlimbs + 1)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> sqrt(const ereal<maxlimbs>& a) {
		// Handle special cases
		if (a.iszero()) return ereal<maxlimbs>(0.0);
		if (a.isneg()) {
			// TODO: Return NaN when ereal supports it
			// For now, return input (error case)
			return a;
		}

		// Initial approximation from high component
		// This gives ~53 bits of precision to start
		const auto& limbs = a.limbs();
		ereal<maxlimbs> x = std::sqrt(limbs[0]);

		// Determine number of iterations based on desired precision
		// Each iteration doubles correct digits (quadratic convergence)
		// For adaptive precision with maxlimbs components, need log2(maxlimbs) + margin
		int iterations = 3 + static_cast<int>(std::log2(maxlimbs + 1));

		// Newton-Raphson: x' = (x + a/x) / 2
		// This converges to sqrt(a) with quadratic rate
		for (int i = 0; i < iterations; ++i) {
			x = (x + a / x) * 0.5;
		}

		return x;
	}

}} // namespace sw::universal
