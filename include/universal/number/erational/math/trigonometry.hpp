#pragma once
// trigonometric.hpp: trigonometric functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/math/constants/double_constants.hpp>

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
erational sin(erational x) {
	return erational(std::sin(double(x)));
}

// cosine of an angle of x radians
erational cos(erational x) {
	return erational(std::cos(double(x)));
}

// tangent of an angle of x radians
erational tan(erational x) {
	return erational(std::tan(double(x)));
}

// cotangent of an angle of x radians
erational atan(erational x) {
	return erational(std::atan(double(x)));
}
		
// Arc tangent with two parameters
erational atan2(erational y, erational x) {
	return erational(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
erational acos(erational x) {
	return erational(std::acos(double(x)));
}

// secant of an angle of x radians
erational asin(erational x) {
	return erational(std::asin(double(x)));
}

// cotangent an angle of x radians
erational cot(erational x) {
	return erational(std::tan(sw::universal::d_pi_2-double(x)));
}

// secant of an angle of x radians
erational sec(erational x) {
	return erational(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
erational csc(erational x) {
	return erational(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
