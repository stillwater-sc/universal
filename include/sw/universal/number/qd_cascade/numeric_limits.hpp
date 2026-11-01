#pragma once
// numeric_limits.hpp: definition of numeric_limits for qd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace std {

template <>
class numeric_limits< sw::universal::qd_cascade > {
public:
	using QD = sw::universal::qd_cascade;
	static constexpr bool is_specialized = true;
	// min() is the smallest value at full precision: below it the last limb is subnormal and the
	// expansion holds fewer than digits bits, so epsilon() no longer bounds the relative rounding
	// error. It is 2^(-1022 + 53 * (limbs - 1)), and min_exponent follows it (#1882).
	static constexpr QD min() { // return minimum value
		return QD(0x1p-863, 0.0, 0.0, 0.0);
	}
	// the largest normalized expansion: DBL_MAX, then each limb DBL_MAX * 2^-54k, below half an ulp of the one before
	static constexpr QD max() { // return maximum value
		return QD(0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+969, 0x1.fffffffffffffp+915, 0x1.fffffffffffffp+861);
	}
	static constexpr QD lowest() { // return most negative value
		return QD(-0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+969, -0x1.fffffffffffffp+915, -0x1.fffffffffffffp+861);
	}
	// epsilon is eps(double)^4 / 2 = 2^-209, the same in qd
	static constexpr QD epsilon() { // return smallest effective increment from 1.0
		return QD(0x1p-209, 0.0, 0.0, 0.0);
	}
	static constexpr QD round_error() { // return largest rounding error
		return QD(0.5, 0.0, 0.0, 0.0);
	}
	static constexpr QD denorm_min() {  // return minimum denormalized value
		return QD(std::numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0);
	}
	static constexpr QD infinity() { // return positive infinity
		return QD(std::numeric_limits<double>::infinity(),
		          std::numeric_limits<double>::infinity(),
		          std::numeric_limits<double>::infinity(),
		          std::numeric_limits<double>::infinity());
	}
	static constexpr QD quiet_NaN() { // return non-signaling NaN
		return QD(std::numeric_limits<double>::quiet_NaN(),
		          std::numeric_limits<double>::quiet_NaN(),
		          std::numeric_limits<double>::quiet_NaN(),
		          std::numeric_limits<double>::quiet_NaN());
	}
	static constexpr QD signaling_NaN() { // return signaling NaN
		return QD(std::numeric_limits<double>::signaling_NaN(),
		          std::numeric_limits<double>::signaling_NaN(),
		          std::numeric_limits<double>::signaling_NaN(),
		          std::numeric_limits<double>::signaling_NaN());
	}

	static constexpr int digits       = 212; // 4 * 53 bits of precision
	static constexpr int digits10     = 63;  // floor(212 * log10(2))
	static constexpr int max_digits10 = 65;  // ceil(212 * log10(2)) + 1
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	// min() == 2^(min_exponent - 1)
	static constexpr int min_exponent   = std::numeric_limits<double>::min_exponent + 3 * std::numeric_limits<double>::digits;
	static constexpr int min_exponent10 = static_cast<int>(min_exponent * 0.30103);
	static constexpr int max_exponent   = std::numeric_limits<double>::max_exponent;
	static constexpr int max_exponent10 = std::numeric_limits<double>::max_exponent10;
	static constexpr bool has_infinity  = true;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm = denorm_present;
	static constexpr bool has_denorm_loss = true;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = false;
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
