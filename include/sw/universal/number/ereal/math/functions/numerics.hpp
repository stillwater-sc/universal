#pragma once
// numerics.hpp: numeric support functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>    // std::logb, std::ilogb, FP_ILOGB0, FP_ILOGBNAN
#include <climits>  // INT_MAX
#include <limits>   // std::numeric_limits

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

	// scalbn(x, n) = x * 2^n -- identical to ldexp for a radix-2 type.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> scalbn(const ereal<maxlimbs>& x, int n) {
		return ldexp(x, n);
	}

	// ilogb(x): unbiased radix-2 exponent floor(log2(|x|)) as an int, with the
	// <cmath> special values. The leading component fixes the exponent to within
	// one, so start from its ilogb and correct against the full magnitude (the
	// lower components can pull |x| across a power-of-two boundary).
	template<unsigned maxlimbs>
	inline int ilogb(const ereal<maxlimbs>& x) {
		if (x.isnan()) return FP_ILOGBNAN;
		if (x.isinf()) return INT_MAX;
		if (x.iszero()) return FP_ILOGB0;

		int e = std::ilogb(x.limbs()[0]);
		ereal<maxlimbs> ax = x.isneg() ? -x : x;              // |x|
		if (ax < ldexp(ereal<maxlimbs>(1.0), e)) {
			--e;
		}
		else if (ax >= ldexp(ereal<maxlimbs>(1.0), e + 1)) {
			++e;
		}
		return e;
	}

	// logb(x): ilogb as a floating value. logb(0) = -inf, logb(+/-inf) = +inf,
	// logb(nan) = nan.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> logb(const ereal<maxlimbs>& x) {
		if (x.isnan()) return x;
		if (x.isinf()) return ereal<maxlimbs>(std::numeric_limits<double>::infinity());
		if (x.iszero()) return ereal<maxlimbs>(-std::numeric_limits<double>::infinity());
		return ereal<maxlimbs>(static_cast<double>(ilogb(x)));
	}

	// fma(x, y, z) = x*y + z. ereal multiplies in exact expansion arithmetic, so
	// no intermediate rounding is introduced. 0*inf yields NaN, matching IEEE.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> fma(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y, const ereal<maxlimbs>& z) {
		return x * y + z;
	}

}} // namespace sw::universal
