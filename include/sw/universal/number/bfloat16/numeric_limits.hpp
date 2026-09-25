#pragma once
// numeric_limits.hpp: definition of numeric_limits for bfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/bfloat16/bfloat16_fwd.hpp>
#include <universal/utility/decimal_digits.hpp>   // exact digit/exponent counts (#1597, #1601, #1602, #1603, #1604)
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

	static constexpr int digits       = 8;   // 7 fraction bits + the implicit leading bit (#1603)
	static constexpr int digits10     = sw::universal::decimal_digits10(digits);
	static constexpr int max_digits10 = sw::universal::decimal_max_digits10(digits);
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	// numeric_limits::min_exponent is the minimum n with radix^(n-1) normalized, so it is
	// one MORE than the scale of the smallest normal, not equal to it (#1608).
	// bfloat16's smallest normal is 2^-126, the same as float's, so this is -125 as
	// numeric_limits<float> reports. It was -128, the negated exponent bias, which is a
	// different quantity.
	static constexpr int min_exponent   = -125;
	static constexpr int min_exponent10 = sw::universal::decimal_min_exponent10(min_exponent);
	static constexpr int max_exponent   = int(1 << 7);
	static constexpr int max_exponent10 = sw::universal::decimal_max_exponent10(max_exponent);
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
	static constexpr float_round_style round_style = round_to_nearest;
};

}
