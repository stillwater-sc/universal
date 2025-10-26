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
	static constexpr TD min() { return TD(std::numeric_limits<double>::min(), 0.0, 0.0); }
	static constexpr TD max() { return TD(1.7976931348623157e+308, 1.9958403095347196e+292, 1.9958403095347196e+292); }
	static constexpr TD lowest() { return TD(-1.7976931348623157e+308, -1.9958403095347196e+292, -1.9958403095347196e+292); }

	static constexpr int digits = 159;        // 3 * 53 bits of fraction
	static constexpr int digits10 = 47;       // floor((159 * log10(2)))
	static constexpr int max_digits10 = 49;   // ceiling((159 * log10(2)) + 1)
	static constexpr bool is_signed = true;
	static constexpr bool is_integer = false;
	static constexpr bool is_exact = false;
	static constexpr int radix = 2;

	// epsilon is 2^-159
	static constexpr TD epsilon() { return TD(1.38777878078144567553615370835363e-48, 0.0, 0.0); }
	static constexpr TD round_error() { return TD(0.5, 0.0, 0.0); }

	static constexpr int min_exponent = std::numeric_limits<double>::min_exponent;
	static constexpr int min_exponent10 = std::numeric_limits<double>::min_exponent10;
	static constexpr int max_exponent = std::numeric_limits<double>::max_exponent;
	static constexpr int max_exponent10 = std::numeric_limits<double>::max_exponent10;

	static constexpr bool has_infinity = true;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm = denorm_present;
	static constexpr bool has_denorm_loss = true;

	static constexpr TD infinity() { return TD(std::numeric_limits<double>::infinity(), 0.0, 0.0); }
	static constexpr TD quiet_NaN() { return TD(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0); }
	static constexpr TD signaling_NaN() { return TD(std::numeric_limits<double>::signaling_NaN(), 0.0, 0.0); }
	static constexpr TD denorm_min() { return TD(std::numeric_limits<double>::denorm_min(), 0.0, 0.0); }

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;

	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
