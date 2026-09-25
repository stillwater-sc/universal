#pragma once
// numeric_limits.hpp: definition of numeric_limits for generalized quire types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/decimal_digits.hpp>   // exact digit/exponent counts (#1597, #1601, #1602, #1603, #1604)

namespace std {

template<typename NumberType, unsigned capacity, typename LimbType>
class numeric_limits< sw::universal::quire<NumberType, capacity, LimbType> > {
public:
	using QuireType = sw::universal::quire<NumberType, capacity, LimbType>;
	using Traits    = sw::universal::quire_traits<NumberType>;
	static constexpr bool is_specialized = true;
	static constexpr QuireType  min() {  // return minimum value
		QuireType minpos(sw::universal::SpecificValue::minpos);
		return minpos;
	}
	static constexpr QuireType  max() {  // return maximum value
		QuireType maxpos(sw::universal::SpecificValue::maxpos);
		return maxpos;
	}
	static constexpr QuireType  lowest() { // return most negative value
		QuireType maxneg(sw::universal::SpecificValue::maxneg);
		return maxneg;
	}
	static constexpr QuireType  epsilon() { // return smallest effective increment from 1.0
		QuireType eps;
		eps.setbit(0);
		return eps;
	}
	static constexpr QuireType  round_error() { // return largest rounding error
		QuireType eps;
		eps.setbit(0);
		return eps;
	}
	static constexpr QuireType  denorm_min() {  // return minimum denormalized value
		QuireType eps;
		eps.setbit(0);
		return eps;
	}
	static constexpr QuireType  infinity() { // return positive infinity
		return max();
	}
	static constexpr QuireType  quiet_NaN() { // return non-signaling NaN
		return QuireType(0);
	}
	static constexpr QuireType  signaling_NaN() { // return signaling NaN
		return QuireType(0);
	}

	static constexpr int digits       = Traits::qbits;
	// is_exact is true for a quire -- holding an exact sum is its entire purpose -- so it
	// takes the integer pair, and max_digits10 is 0 as it is for the native integers
	// (#1601). Note Traits::qbits resolves to 0 for some instantiations, which is a
	// separate defect; the helpers return 0 for a 0 width rather than a negative count.
	static constexpr int digits10     = sw::universal::decimal_digits10_integer(digits);
	static constexpr int max_digits10 = 0;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int max_exponent   = 0;
	static constexpr int max_exponent10 = 0;
	static constexpr bool has_infinity  = false;
	static constexpr bool has_quiet_NaN = false;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = true;  // or should we saturate
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
