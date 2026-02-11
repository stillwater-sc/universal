#pragma once
// numeric_limits.hpp: definition of numeric_limits for e8m0 type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/e8m0/e8m0_fwd.hpp>
namespace std {

template<>
class numeric_limits< sw::universal::e8m0 > {
public:
	using E8m0 = sw::universal::e8m0;
	static constexpr bool is_specialized = true;
	static constexpr E8m0 min() {
		E8m0 v;
		v.minpos();
		return v;
	}
	static constexpr E8m0 max() {
		return E8m0(sw::universal::SpecificValue::maxpos);
	}
	static constexpr E8m0 lowest() {
		// e8m0 is always positive, so lowest == min
		E8m0 v;
		v.minpos();
		return v;
	}
	static E8m0 epsilon() {
		// smallest difference is a factor of 2 (one encoding step)
		return E8m0(2.0f);
	}
	static E8m0 round_error() {
		return E8m0(1.0f);
	}
	static constexpr E8m0 denorm_min() {
		return E8m0(sw::universal::SpecificValue::minpos);
	}
	static constexpr E8m0 infinity() {
		return E8m0(sw::universal::SpecificValue::maxpos);
	}
	static constexpr E8m0 quiet_NaN() {
		return E8m0(sw::universal::SpecificValue::qnan);
	}
	static constexpr E8m0 signaling_NaN() {
		return E8m0(sw::universal::SpecificValue::snan);
	}

	static constexpr int digits       = 1;
	static constexpr int digits10     = 0;
	static constexpr int max_digits10 = 0;
	static constexpr bool is_signed   = false;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = -127;
	static constexpr int min_exponent10 = -38;
	static constexpr int max_exponent   = 127;
	static constexpr int max_exponent10 = 38;
	static constexpr bool has_infinity  = false;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = false;
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

}
