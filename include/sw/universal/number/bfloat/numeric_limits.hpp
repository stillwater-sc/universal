#pragma once
// numeric_limits.hpp: definition of numeric_limits for bfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/bfloat/bfloat16_fwd.hpp>
namespace std {

template<>
class numeric_limits< sw::universal::bfloat16 > {
public:
	using Bfloat = sw::universal::bfloat16;
	static constexpr bool is_specialized = true;
	static constexpr Bfloat min() { // return minimum value
		Bfloat bf;
		bf.setbits(0b0000000010000000);
		return bf;
	} 
	static constexpr Bfloat max() { // return maximum value
		return Bfloat(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr Bfloat lowest() { // return most negative value
		return Bfloat(sw::universal::SpecificValue::maxneg);
	} 
	static Bfloat epsilon() { // return smallest effective increment from 1.0
		Bfloat one{ 1.0f }, oneplus{ 1.0f };
		++oneplus;
		return oneplus - one;
	}
	static Bfloat round_error() { // return largest rounding error
		return Bfloat(0.5f);
	}
	static constexpr Bfloat denorm_min() {  // return minimum denormalized value
		return Bfloat(sw::universal::SpecificValue::minpos);;
	}
	static constexpr Bfloat infinity() { // return positive infinity
		return Bfloat(sw::universal::SpecificValue::infpos);
	}
	static constexpr Bfloat quiet_NaN() { // return non-signaling NaN
		return Bfloat(sw::universal::SpecificValue::qnan);
	}
	static constexpr Bfloat signaling_NaN() { // return signaling NaN
		return Bfloat(sw::universal::SpecificValue::snan);
	}

	static constexpr int digits       = 7;
	static constexpr int digits10     = static_cast<int>(digits / 3.3f);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = -int(1 << 7);
	static constexpr int min_exponent10 = static_cast<int>(min_exponent / 3.3f);
	static constexpr int max_exponent   = int(1 << 7);
	static constexpr int max_exponent10 = static_cast<int>(max_exponent / 3.3f);
	static constexpr bool has_infinity  = true;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = false;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
