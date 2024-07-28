#pragma once
// numeric_limits.hpp: definition of numeric_limits for doubledouble types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dd/dd_fwd.hpp>
namespace std {

template<>
class numeric_limits< sw::universal::dd > {
public:
	using DoubleDouble = sw::universal::dd;
	static constexpr bool is_specialized = true;
	static constexpr DoubleDouble min() { // return minimum value
		return DoubleDouble(sw::universal::SpecificValue::minpos);
	} 
	static constexpr DoubleDouble max() { // return maximum value
		return DoubleDouble(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr DoubleDouble lowest() { // return most negative value
		return DoubleDouble(sw::universal::SpecificValue::maxneg);
	} 
	static DoubleDouble epsilon() { // return smallest effective increment from 1.0
		DoubleDouble one{ 1.0 }, oneplus{ 1.0 };
		++oneplus;
		return oneplus - one;
	}
	static DoubleDouble round_error() { // return largest rounding error
		return DoubleDouble(0.5);
	}
	static constexpr DoubleDouble denorm_min() {  // return minimum denormalized value
		return DoubleDouble(sw::universal::SpecificValue::minpos);;
	}
	static constexpr DoubleDouble infinity() { // return positive infinity
		return DoubleDouble(sw::universal::SpecificValue::infpos);
	}
	static constexpr DoubleDouble quiet_NaN() { // return non-signaling NaN
		return DoubleDouble(sw::universal::SpecificValue::qnan);
	}
	static constexpr DoubleDouble signaling_NaN() { // return signaling NaN
		return DoubleDouble(sw::universal::SpecificValue::snan);
	}

	static constexpr int digits       = 108;
	static constexpr int digits10     = static_cast<int>(digits / 3.3f);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = -int(1 << 11);
	static constexpr int min_exponent10 = static_cast<int>(min_exponent / 3.3f);
	static constexpr int max_exponent   = int(1 << 11);
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
