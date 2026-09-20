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
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> ldexp(const ereal<maxlimbs, FpType>& x, int exp) {
		if (x.iszero() || exp == 0) return x;

		// Scale all components by 2^exp
		const auto& limbs = x.limbs();
		ereal<maxlimbs, FpType> result;

		result = std::ldexp(limbs[0], exp);
		for (size_t i = 1; i < limbs.size(); ++i) {
			result += ereal<maxlimbs, FpType>(std::ldexp(limbs[i], exp));
		}

		return result;
	}

	// frexp: break into normalized fraction and exponent
	// Phase 2: extracts exponent from high component, scales entire expansion
	// Returns mantissa in range [0.5, 1.0) and sets exponent
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> frexp(const ereal<maxlimbs, FpType>& x, int* exp) {
		if (x.iszero()) {
			*exp = 0;
			return x;
		}

		// Use high component to determine exponent
		const auto& limbs = x.limbs();
		FpType high = limbs[0];

		// Get exponent of high component
		std::frexp(high, exp);

		// Scale entire expansion by 2^(-exponent) using ldexp
		return ldexp(x, -(*exp));
	}

	// series_precision_bits: the bits a configuration carries, plus a guard limb, which is
	// how far a series has to run.
	template<unsigned maxlimbs, typename FpType>
	inline constexpr int series_precision_bits() {
		return static_cast<int>(maxlimbs + 1) * std::numeric_limits<FpType>::digits;
	}

	// series_converged: a term can no longer move the running sum once it sits `bits`
	// below it.
	//
	// The test is on EXPONENTS. These series used to convert each term to a double and
	// compare it against 10^-digits, which reads ANY term below ~1e-308 as exactly zero:
	// on a limb type whose range goes further -- x87 and binary128 reach 1e-4932 -- every
	// series stopped at ~308 digits no matter how many limbs it was given (#1567).
	template<unsigned maxlimbs, typename FpType>
	inline bool series_converged(const ereal<maxlimbs, FpType>& term, const ereal<maxlimbs, FpType>& sum, int bits) {
		if (term.iszero()) return true;
		if (sum.iszero()) return false;                  // nothing to be small against yet
		return ilogb(term) < ilogb(sum) - bits;
	}

	// copysign: copy sign from one value to another
	// Phase 1: uses ereal's sign() method and unary minus operator
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> copysign(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		if (x.sign() == y.sign()) {
			return x;
		} else {
			return -x;
		}
	}

	// scalbn(x, n) = x * 2^n -- identical to ldexp for a radix-2 type.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> scalbn(const ereal<maxlimbs, FpType>& x, int n) {
		return ldexp(x, n);
	}

	// ilogb(x): unbiased radix-2 exponent floor(log2(|x|)) as an int, with the
	// <cmath> special values. The leading component fixes the exponent to within
	// one, so start from its ilogb and correct against the full magnitude (the
	// lower components can pull |x| across a power-of-two boundary).
	template<unsigned maxlimbs, typename FpType>
	inline int ilogb(const ereal<maxlimbs, FpType>& x) {
		if (x.isnan()) return FP_ILOGBNAN;
		if (x.isinf()) return INT_MAX;
		if (x.iszero()) return FP_ILOGB0;

		int e = std::ilogb(x.limbs()[0]);
		ereal<maxlimbs, FpType> ax = x.isneg() ? -x : x;              // |x|
		if (ax < ldexp(ereal<maxlimbs, FpType>(1.0), e)) {
			--e;
		}
		else if (ax >= ldexp(ereal<maxlimbs, FpType>(1.0), e + 1)) {
			++e;
		}
		return e;
	}

	// logb(x): ilogb as a floating value. logb(0) = -inf, logb(+/-inf) = +inf,
	// logb(nan) = nan.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> logb(const ereal<maxlimbs, FpType>& x) {
		if (x.isnan()) return x;
		if (x.isinf()) return ereal<maxlimbs, FpType>(std::numeric_limits<FpType>::infinity());
		if (x.iszero()) return ereal<maxlimbs, FpType>(-std::numeric_limits<FpType>::infinity());
		return ereal<maxlimbs, FpType>(static_cast<double>(ilogb(x)));
	}

	// fma(x, y, z) = x*y + z. ereal multiplies in exact expansion arithmetic, so
	// no intermediate rounding is introduced. 0*inf yields NaN, matching IEEE.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> fma(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y, const ereal<maxlimbs, FpType>& z) {
		return x * y + z;
	}

}} // namespace sw::universal
