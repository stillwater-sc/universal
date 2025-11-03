#pragma once
// numerics.hpp: numeric support functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// ldexp: multiply by power of 2
	// Phase 2: efficient power-of-2 scaling via component manipulation
	// Multiplying by 2^exp doesn't introduce rounding error (for reasonable exponents)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> ldexp(const ereal<maxlimbs>& x, int exp) {
		if (x.iszero() || exp == 0) return x;

		// Scale all components by 2^exp
		const auto& limbs = x.limbs();
		ereal<maxlimbs> result;

		result = std::ldexp(limbs[0], exp);
		for (size_t i = 1; i < limbs.size(); ++i) {
			result += ereal<maxlimbs>(std::ldexp(limbs[i], exp));
		}

		return result;
	}

	// frexp: break into normalized fraction and exponent
	// Phase 2: extracts exponent from high component, scales entire expansion
	// Returns mantissa in range [0.5, 1.0) and sets exponent
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> frexp(const ereal<maxlimbs>& x, int* exp) {
		if (x.iszero()) {
			*exp = 0;
			return x;
		}

		// Use high component to determine exponent
		const auto& limbs = x.limbs();
		double high = limbs[0];

		// Get exponent of high component
		std::frexp(high, exp);

		// Scale entire expansion by 2^(-exponent) using ldexp
		return ldexp(x, -(*exp));
	}

	// copysign: copy sign from one value to another
	// Phase 1: uses ereal's sign() method and unary minus operator
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> copysign(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		if (x.sign() == y.sign()) {
			return x;
		} else {
			return -x;
		}
	}

}} // namespace sw::universal
