#pragma once
// numeric_limits.hpp: definition of numeric_limits for generalized quire types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/fixpnt/fixpnt.hpp>

namespace std {

	using namespace sw::universal;

template<typename NumberType, unsigned capacity, typename LimbType>
    class numeric_limits< quire<NumberType, capacity, LimbType> > {
public:
	using QuireType = sw::universal::quire<NumberType, capacity, LimbType>;
    using Traits    = quire_traits<NumberType>;
	static constexpr bool is_specialized = true;
	static constexpr QuireType  min() {  // return minimum value
		QuireType minpos(SpecificValue::minpos);
		return minpos;
	}
	static constexpr QuireType  max() {  // return maximum value
		QuireType maxpos(SpecificValue::maxpos);
		return maxpos;
	}
	static constexpr QuireType  lowest() { // return most negative value
		QuireType maxneg(SpecificValue::maxneg);
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
		return 0; 
	}
	static constexpr QuireType  signaling_NaN() { // return signaling NaN
		return 0;
	}

	static constexpr int digits       = Traits::qbits;
	static constexpr int digits10     = int((digits) / 3.3);
	static constexpr int max_digits10 = int((digits) / 3.3);
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
