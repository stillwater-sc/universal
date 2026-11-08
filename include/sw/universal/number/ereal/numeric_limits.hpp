#pragma once
// numeric_limits.hpp: definition of numeric_limits for ereal types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>    // std::ldexp, for epsilon()
#include <limits>
// ereal_impl.hpp, not the ereal.hpp umbrella: the umbrella includes core.hpp, which
// includes this header, so naming it here would be a cycle that #pragma once merely
// masks (the trap areal hit, #1452). dd_fwd.hpp was included here and never used.
#include <universal/number/ereal/ereal_impl.hpp>
namespace std {

// Every limit follows the limb type (#1565): for the default double limbs they are the
// values they always were.
template<unsigned maxLimbs, typename FpType>
class numeric_limits< sw::universal::ereal<maxLimbs, FpType> > {
public:
	using ErealType = sw::universal::ereal<maxLimbs, FpType>;
	static constexpr bool is_specialized = true;
	// min() is the smallest value at full precision: the leading limb 2^(min_exponent - 1) of
	// the limb type raised by (maxLimbs - 1) limbs, so that all maxLimbs limbs stay normal.
	// For double limbs it is 2^(-1022 + 53 (maxLimbs - 1)): 2^-651 for ereal<8>, 2^-68 for
	// ereal<19>.  This is the convention of dd, qd and the cascades (#1882), applied to
	// ereal's own limb count (user decision, #2005).  It was the two-limb floor 2^-969 for
	// every maxLimbs, while min_exponent and minpos() said 2^-1022.  Below min(), down to
	// minpos() = denorm_min(), a value carries fewer limbs: the denormal range.
	static constexpr ErealType min() { // return minimum value
		FpType v = numeric_limits< FpType >::min();
		for (int i = 0; i < full_precision_shift; ++i) v *= FpType(radix);
		return ErealType(v);
	} 
	static constexpr ErealType max() { // return maximum value
		return ErealType(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr ErealType lowest() { // return most negative value
		//return ErealType(sw::universal::SpecificValue::maxneg);
		return (-(max)());
	} 
	static ErealType epsilon() { // return smallest effective increment from 1.0
		// 2^(1 - digits), with digits = maxLimbs * the limb type's digits: the whole
		// expansion's precision, so that epsilon() and digits agree. It used to be
		// (eps*eps)/2 of the limb type -- 2^(1-2p), the value for TWO limbs, whatever
		// maxLimbs said: 2^-105 for every ereal<n> (#1574).
		return ErealType(std::ldexp(FpType(1), 1 - digits));
	}
	static constexpr ErealType round_error() { // return largest rounding error
		return ErealType(FpType(1) / FpType(radix));
	}
	static ErealType denorm_min() {  // return minimum denormalized value
		// the smallest positive value, a single limb at the limb type's smallest normal:
		// minpos().  Between it and min() a value keeps fewer limbs, as a subnormal keeps
		// fewer bits (#2005).
		return ErealType(sw::universal::SpecificValue::minpos);
	}
	static constexpr ErealType infinity() { // return positive infinity
		return ErealType(sw::universal::SpecificValue::infpos);
	}
	static constexpr ErealType quiet_NaN() { // return non-signaling NaN
		return ErealType(sw::universal::SpecificValue::qnan);
	}
	static constexpr ErealType signaling_NaN() { // return signaling NaN
		return ErealType(sw::universal::SpecificValue::snan);
	}

	// Each limb provides one limb type's worth of precision
	// digits: number of bits of precision (53 per double limb, 64 per x87 limb, ...)
	// For conservative estimate, use maxLimbs * FpType::digits
	static constexpr int  digits                   = static_cast<int>(maxLimbs) * std::numeric_limits<FpType>::digits;
	// digits10: number of decimal digits that can be represented without change
	// log10(2^digits) = digits * log10(2) ~= digits * 0.30103
	static constexpr int  digits10                 = static_cast<int>(digits * 0.30103);
	// max_digits10: decimal digits needed to differentiate all values
	static constexpr int  max_digits10             = digits10 + 2;
	static constexpr bool is_signed                = true;
	static constexpr bool is_integer               = false;
	static constexpr bool is_exact                 = false;
	static constexpr int  radix                    = 2;

	// C++ specification: min_exponent is one more than the smallest negative power 
	// of the radix that is a valid normalized number
	// min() == 2^(min_exponent - 1): the limb type's min_exponent raised by (maxLimbs - 1) limbs
	static constexpr int  full_precision_shift     = static_cast<int>(maxLimbs - 1) * std::numeric_limits<FpType>::digits;
	static constexpr int  min_exponent             = std::numeric_limits<FpType>::min_exponent + full_precision_shift;
	static constexpr int  min_exponent10           = static_cast<int>(min_exponent * 0.30103);
	// C++ specification: max_exponent is one more than the largest integer power 
	// of the radix that is a valid finite floating-point number
	static constexpr int  max_exponent             = ErealType::MAX_EXP;
	static constexpr int  max_exponent10           = static_cast<int>(max_exponent * 0.30103);
	static constexpr bool has_infinity             = true;
	static constexpr bool has_quiet_NaN            = true;
	static constexpr bool has_signaling_NaN        = true;
	static constexpr float_denorm_style has_denorm = denorm_present;   // fewer limbs below min()
	static constexpr bool has_denorm_loss          = false;

	static constexpr bool is_iec559                = false;
	static constexpr bool is_bounded               = false;
	static constexpr bool is_modulo                = false;
	static constexpr bool traps                    = false;
	static constexpr bool tinyness_before          = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
