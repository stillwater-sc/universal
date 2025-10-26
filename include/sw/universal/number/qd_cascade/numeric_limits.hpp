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
	static constexpr QD min() { // return minimum value
		return QD(4.9406564584124654e-324, 0.0, 0.0, 0.0); // std::numeric_limits<double>::min()
	}
	static constexpr QD max() { // return maximum value
		return QD(1.79769313486231570814527423731704357e+308,
		          9.97920154767359795037289025843547926e+291,
		          5.53956966280111259858119742279688267e+275,
		          3.07507899888268538886654502482441665e+259);
	}
	static constexpr QD lowest() { // return most negative value
		return QD(-1.79769313486231570814527423731704357e+308,
		          -9.97920154767359795037289025843547926e+291,
		          -5.53956966280111259858119742279688267e+275,
		          -3.07507899888268538886654502482441665e+259);
	}
	static constexpr QD epsilon() { // return smallest effective increment from 1.0
		return QD(1.21543267145725712652978599954436861e-63, 0.0, 0.0, 0.0);  // 2^-212 for quad-double precision
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

	static constexpr int min_exponent   = std::numeric_limits<double>::min_exponent;
	static constexpr int min_exponent10 = std::numeric_limits<double>::min_exponent10;
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
