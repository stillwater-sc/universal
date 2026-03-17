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
		constexpr float probe = 0x1p-12f;
		constexpr double threshold_scale = 8.0;

		const bfloat16 c = bfloat16_cos_float_path(bfloat16(probe));
		const double cd = static_cast<double>(float(c));
		const double s_est = std::sqrt(std::max(0.0, 1.0 - cd * cd));

		// For this tiny first-quadrant probe, cos(x) should stay close enough to 1
		// that the implied sine magnitude remains on the order of x, not orders of
		// magnitude larger because of a bad float-path rounding step.
		return std::abs(s_est - probe) > threshold_scale * probe;
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
