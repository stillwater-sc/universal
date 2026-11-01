#pragma once
// numeric_limits.hpp: definition of numeric_limits for quad-double types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <universal/number/qd/qd_fwd.hpp>
namespace std {

template<>
class numeric_limits< sw::universal::qd > {
public:
	using QuadDouble = sw::universal::qd;
	static constexpr bool is_specialized = true;
	// min() is the smallest value at full precision: below it the last limb is subnormal and the
	// expansion holds fewer than digits bits, so epsilon() no longer bounds the relative rounding
	// error. It is 2^(-1022 + 53 * (limbs - 1)), and min_exponent follows it (#1882).
	static constexpr QuadDouble min() { // return minimum value
		return QuadDouble(0x1p-863);
	} 
	static constexpr QuadDouble max() { // return maximum value
		return QuadDouble(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr QuadDouble lowest() { // return most negative value
		return QuadDouble(sw::universal::SpecificValue::maxneg);
	} 
	// epsilon is eps(double)^4 / 2 = 2^-209, the same in qd_cascade
	static constexpr QuadDouble epsilon() { // return smallest effective increment from 1.0
		return QuadDouble(0x1p-209);
	}
	static constexpr QuadDouble round_error() { // return largest rounding error
		return QuadDouble(1.0 / radix);
	}
	static constexpr QuadDouble denorm_min() {  // return minimum denormalized value
		return QuadDouble(std::numeric_limits<double>::denorm_min());
	}
	static constexpr QuadDouble infinity() { // return positive infinity
		return QuadDouble(sw::universal::SpecificValue::infpos);
	}
	static constexpr QuadDouble quiet_NaN() { // return non-signaling NaN
		return QuadDouble(sw::universal::SpecificValue::qnan);
	}
	static constexpr QuadDouble signaling_NaN() { // return signaling NaN
		return QuadDouble(sw::universal::SpecificValue::snan);
	}

	static constexpr int  digits                   = 4 * std::numeric_limits<double>::digits;
	static constexpr int  digits10                 = static_cast<int>(digits * 0.30103);
	static constexpr int  max_digits10             = digits10;
	static constexpr bool is_signed                = true;
	static constexpr bool is_integer               = false;
	static constexpr bool is_exact                 = false;
	static constexpr int  radix                    = 2;

	// C++ specification: min_exponent is one more than the smallest negative power
	// of the radix that is a valid normalized number: min() == 2^(min_exponent - 1)
	static constexpr int  min_exponent             = std::numeric_limits<double>::min_exponent + 3 * std::numeric_limits<double>::digits;
	static constexpr int  min_exponent10           = static_cast<int>(min_exponent * 0.30103);
	// C++ specification: max_exponent is one more than the largest integer power 
	// of the radix that is a valid finite floating-point number
	static constexpr int  max_exponent             = QuadDouble::MAX_EXP;
	static constexpr int  max_exponent10           = static_cast<int>(max_exponent * 0.30103);
	static constexpr bool has_infinity             = true;
	static constexpr bool has_quiet_NaN            = true;
	static constexpr bool has_signaling_NaN        = true;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss          = false;

	static constexpr bool is_iec559                = false;
	static constexpr bool is_bounded               = false;
	static constexpr bool is_modulo                = false;
	static constexpr bool traps                    = false;
	static constexpr bool tinyness_before          = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
