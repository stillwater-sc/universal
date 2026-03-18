#pragma once
// trigonometric.hpp: trigonometric functions for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <math/constants/float_constants.hpp>

namespace sw { namespace universal {

namespace detail {

inline bfloat16 bfloat16_cos_float_path(bfloat16 x) {
	return bfloat16(std::cos(float(x)));
}

inline bfloat16 bfloat16_cos_double_path(bfloat16 x) {
	return bfloat16(std::cos(double(x)));
}

inline bool bfloat16_cos_float_path_is_unreliable() {
	static const bool needs_workaround = [] {
		// Known problematic small input from the bfloat16_mathlib unit test.
		constexpr float probe_x = 0x1p-12f;  // 2^-12 = 1.0f / 4096.0f = 0.000244140625f

		// The estimated sine magnitude must stay within this factor of |probe_x|.
		constexpr double error_factor = 8.0;

		const bfloat16 cos_bf16 = bfloat16_cos_float_path(bfloat16(probe_x));
		const double cos_value = static_cast<double>(float(cos_bf16));
		const double estimated_sin = std::sqrt(std::max(0.0, 1.0 - cos_value * cos_value));
		const double expected_sin = std::abs(static_cast<double>(probe_x));

		// For very small first-quadrant x, sin(x) is approximately x.
		// If the float cosine result yields an estimated sine magnitude that is
		// far too large or far too small, use the double cosine path instead.
		return (estimated_sin > error_factor * expected_sin) ||
		       (estimated_sin < expected_sin / error_factor);
	}();
	return needs_workaround;
}


}  // namespace detail

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
inline bfloat16 sin(bfloat16 x) {
	return bfloat16(std::sin(float(x)));
}

// cosine of an angle of x radians
inline bfloat16 cos(bfloat16 x) {
	if (detail::bfloat16_cos_float_path_is_unreliable()) {
		return detail::bfloat16_cos_double_path(x);
	}
	return detail::bfloat16_cos_float_path(x);
}

// tangent of an angle of x radians
inline bfloat16 tan(bfloat16 x) {
	return bfloat16(std::tan(float(x)));
}

// arc tangent which gives the angle whose tangent is x radians
inline bfloat16 atan(bfloat16 x) {
	return bfloat16(std::atan(float(x)));
}
		
// Arc tangent with two parameters
// Computes the arc tangent of y / x using the signs of arguments to determine the correct quadrant.
inline bfloat16 atan2(bfloat16 y, bfloat16 x) {
	return bfloat16(std::atan2(float(y), float(x)));
}

// arc cosine which gives the angle whose cosine is x radians
inline bfloat16 acos(bfloat16 x) {
	return bfloat16(std::acos(float(x)));
}

// arc sine which gives the angle whose sine is x radians
inline bfloat16 asin(bfloat16 x) {
	return bfloat16(std::asin(float(x)));
}

// cotangent an angle of x radians
inline bfloat16 cot(bfloat16 x) {
	return bfloat16(std::tan(f_pi_2 - float(x)));
}

// secant of an angle of x radians
inline bfloat16 sec(bfloat16 x) {
	return bfloat16(1.0f / std::cos(float(x)));
}

// cosecant of an angle of x radians
inline bfloat16 csc(bfloat16 x) {
	return bfloat16(1.0f / std::sin(float(x)));
}

}} // namespace sw::universal
