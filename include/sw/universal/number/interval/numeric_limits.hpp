#pragma once
// numeric_limits.hpp: numeric_limits specialization for the interval number type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace std {

template<typename Scalar>
class numeric_limits<sw::universal::interval<Scalar>> {
public:
	using Interval = sw::universal::interval<Scalar>;
	static constexpr bool is_specialized = true;

	static constexpr Interval min() noexcept {
		return Interval(numeric_limits<Scalar>::min());
	}
	static constexpr Interval max() noexcept {
		return Interval(numeric_limits<Scalar>::max());
	}
	static constexpr Interval lowest() noexcept {
		return Interval(numeric_limits<Scalar>::lowest());
	}

	static constexpr int digits = numeric_limits<Scalar>::digits;
	static constexpr int digits10 = numeric_limits<Scalar>::digits10;
	static constexpr int max_digits10 = numeric_limits<Scalar>::max_digits10;

	static constexpr bool is_signed = numeric_limits<Scalar>::is_signed;
	static constexpr bool is_integer = false;  // intervals are never integers
	static constexpr bool is_exact = false;    // intervals represent uncertainty
	static constexpr int radix = numeric_limits<Scalar>::radix;

	static constexpr Interval epsilon() noexcept {
		return Interval(numeric_limits<Scalar>::epsilon());
	}
	static constexpr Interval round_error() noexcept {
		return Interval(numeric_limits<Scalar>::round_error());
	}

	static constexpr int min_exponent = numeric_limits<Scalar>::min_exponent;
	static constexpr int min_exponent10 = numeric_limits<Scalar>::min_exponent10;
	static constexpr int max_exponent = numeric_limits<Scalar>::max_exponent;
	static constexpr int max_exponent10 = numeric_limits<Scalar>::max_exponent10;

	static constexpr bool has_infinity = numeric_limits<Scalar>::has_infinity;
	static constexpr bool has_quiet_NaN = numeric_limits<Scalar>::has_quiet_NaN;
	static constexpr bool has_signaling_NaN = numeric_limits<Scalar>::has_signaling_NaN;
	static constexpr float_denorm_style has_denorm = numeric_limits<Scalar>::has_denorm;
	static constexpr bool has_denorm_loss = numeric_limits<Scalar>::has_denorm_loss;

	static constexpr Interval infinity() noexcept {
		return Interval(numeric_limits<Scalar>::infinity());
	}
	static constexpr Interval quiet_NaN() noexcept {
		return Interval(numeric_limits<Scalar>::quiet_NaN());
	}
	static constexpr Interval signaling_NaN() noexcept {
		return Interval(numeric_limits<Scalar>::signaling_NaN());
	}
	static constexpr Interval denorm_min() noexcept {
		return Interval(numeric_limits<Scalar>::denorm_min());
	}

	static constexpr bool is_iec559 = false;  // intervals don't conform to IEC 559
	static constexpr bool is_bounded = numeric_limits<Scalar>::is_bounded;
	static constexpr bool is_modulo = false;

	static constexpr bool traps = numeric_limits<Scalar>::traps;
	static constexpr bool tinyness_before = numeric_limits<Scalar>::tinyness_before;
	static constexpr float_round_style round_style = numeric_limits<Scalar>::round_style;
};

} // namespace std
