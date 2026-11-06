#pragma once
// numeric_limits.hpp: definition of numeric_limits for triple-double cascade (td_cascade) types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace std {

template<>
class numeric_limits<sw::universal::td_cascade> {
public:
	using TD = sw::universal::td_cascade;

	static constexpr bool is_specialized = true;
	// min() is the smallest value at full precision: below it the last limb is subnormal and the
	// expansion holds fewer than digits bits, so epsilon() no longer bounds the relative rounding
	// error. It is 2^(-1022 + 53 * (limbs - 1)), and min_exponent follows it (#1882).
	static constexpr TD min() { return TD(0x1p-916, 0.0, 0.0); }
	// the largest normalized expansion: DBL_MAX, then each limb DBL_MAX * 2^-54k, below half an ulp of the one before
	static constexpr TD max() { return TD(0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+969, 0x1.fffffffffffffp+915); }
	static constexpr TD lowest() { return TD(-0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+969, -0x1.fffffffffffffp+915); }

	static constexpr int digits = 159;        // 3 * 53 bits of fraction
	static constexpr int digits10 = 47;       // floor((159 * log10(2)))
	static constexpr int max_digits10 = 49;   // ceiling((159 * log10(2)) + 1)
	static constexpr bool is_signed = true;
	static constexpr bool is_integer = false;
	static constexpr bool is_exact = false;
	static constexpr int radix = 2;

	// epsilon is eps(double)^3 / 2 = 2^-157, the convention of dd (2^-105) and qd (2^-209)
	static constexpr TD epsilon() { return TD(0x1p-157, 0.0, 0.0); }
	static constexpr TD round_error() { return TD(0.5, 0.0, 0.0); }

	// min() == 2^(min_exponent - 1)
	static constexpr int min_exponent = std::numeric_limits<double>::min_exponent + 2 * std::numeric_limits<double>::digits;
	static constexpr int min_exponent10 = static_cast<int>(min_exponent * 0.30103);
	static constexpr int max_exponent = std::numeric_limits<double>::max_exponent;
	static constexpr int max_exponent10 = std::numeric_limits<double>::max_exponent10;

	static constexpr bool has_infinity = true;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = true;
	// The same in dd, qd, dd_cascade, td_cascade and qd_cascade (#1888). Below min() the low
	// limbs go subnormal and then underflow, so precision falls off gradually down to
	// denorm_min(), the double subnormal; each limb operation rounds to nearest.
	static constexpr float_denorm_style has_denorm = denorm_present;
	static constexpr bool has_denorm_loss = std::numeric_limits<double>::has_denorm_loss;

	// special values carry the code in the leading limb and zeros in the rest, as setinf() and
	// setnan() build them and isinf()/isnan() test them, the same in dd and qd (#1889)
	static constexpr TD infinity() { // return positive infinity
		return TD(sw::universal::SpecificValue::infpos);
	}
	static constexpr TD quiet_NaN() { // return non-signaling NaN
		return TD(sw::universal::SpecificValue::qnan);
	}
	static constexpr TD signaling_NaN() { // return signaling NaN
		return TD(sw::universal::SpecificValue::snan);
	}
	static constexpr TD denorm_min() { return TD(std::numeric_limits<double>::denorm_min(), 0.0, 0.0); }

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;

	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
