#pragma once
// numeric_limits.hpp: definition of numeric_limits for dd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace std {

template <>
class numeric_limits< sw::universal::dd_cascade > {
public:
	using DD = sw::universal::dd_cascade;
	static constexpr bool is_specialized = true;
	static constexpr DD min() { // return minimum value
		return DD(4.9406564584124654e-324, 0.0); // std::numeric_limits<double>::min()
	}
	static constexpr DD max() { // return maximum value
		return DD(1.7976931348623157e+308, 9.9792015476735972e+291);
	}
	static constexpr DD lowest() { // return most negative value
		return DD(-1.7976931348623157e+308, -9.9792015476735972e+291);
	}
	static constexpr DD epsilon() { // return smallest effective increment from 1.0
		return DD(4.93038065763132e-32, 0.0);  // 2^-106 for double-double precision
	}
	static constexpr DD round_error() { // return largest rounding error
		return DD(0.5, 0.0);
	}
	static constexpr DD denorm_min() {  // return minimum denormalized value
		return DD(std::numeric_limits<double>::denorm_min(), 0.0);
	}
	static constexpr DD infinity() { // return positive infinity
		return DD(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
	}
	static constexpr DD quiet_NaN() { // return non-signaling NaN
		return DD(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
	}
	static constexpr DD signaling_NaN() { // return signaling NaN
		return DD(std::numeric_limits<double>::signaling_NaN(), std::numeric_limits<double>::signaling_NaN());
	}

	static constexpr int digits       = 106; // 2 * 53 bits of precision
	static constexpr int digits10     = 31;  // floor(106 * log10(2))
	static constexpr int max_digits10 = 33;  // ceil(106 * log10(2)) + 1
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
