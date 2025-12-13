#pragma once
// numeric_limits.hpp: definition of numeric_limits for fixed-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/fixpnt/fixpnt.hpp>

namespace std {

	using namespace sw::universal;

template <unsigned nbits, unsigned rbits, bool arithmetic, typename bt> 
class numeric_limits< fixpnt<nbits,rbits,arithmetic,bt> > {
public:
	using FixedPoint = sw::universal::fixpnt<nbits, rbits, arithmetic, bt>;
	static constexpr bool is_specialized = true;
	static constexpr FixedPoint  min() {  // return minimum value
		FixedPoint minpos(SpecificValue::minpos);
		return minpos;
	}
	static constexpr FixedPoint  max() {  // return maximum value
		FixedPoint maxpos(SpecificValue::maxpos);
		return maxpos;
	}
	static constexpr FixedPoint  lowest() { // return most negative value
		FixedPoint maxneg(SpecificValue::maxneg);
		return maxneg;
	} 
	static constexpr FixedPoint  epsilon() { // return smallest effective increment from 1.0
		FixedPoint eps{};
		eps.setbit(0);
		return eps;
	}
	static constexpr FixedPoint  round_error() { // return largest rounding error
		FixedPoint eps{};
		eps.setbit(0);
		return eps;
	}
	static constexpr FixedPoint  denorm_min() {  // return minimum denormalized value
		FixedPoint eps{};
		eps.setbit(0);
		return eps;
	}
	static constexpr FixedPoint  infinity() { // return positive infinity
		return max(); 
	}
	static constexpr FixedPoint  quiet_NaN() { // return non-signaling NaN
		return 0; 
	}
	static constexpr FixedPoint  signaling_NaN() { // return signaling NaN
		return 0;
	}

	static constexpr int digits       = nbits - 1;
	static constexpr int digits10     = int((digits) / 3.3);
	static constexpr int max_digits10 = int((digits) / 3.3);
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = -int(rbits);
	static constexpr int min_exponent10 = -int((rbits) / 3.3);
	static constexpr int max_exponent   = nbits - 1 - rbits;
	static constexpr int max_exponent10 = int((nbits - 1 - rbits) / 3.3);
	static constexpr bool has_infinity  = false;
	static constexpr bool has_quiet_NaN = false;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = arithmetic;
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
