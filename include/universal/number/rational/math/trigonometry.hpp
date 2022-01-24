#pragma once
// trigonometric.hpp: trigonometric functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/math/math_constants.hpp>

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
rational sin(rational x) {
	return rational(std::sin(double(x)));
}

// cosine of an angle of x radians
rational cos(rational x) {
	return rational(std::cos(double(x)));
}

// tangent of an angle of x radians
rational tan(rational x) {
	return rational(std::tan(double(x)));
}

// cotangent of an angle of x radians
rational atan(rational x) {
	return rational(std::atan(double(x)));
}
		
// Arc tangent with two parameters
rational atan2(rational y, rational x) {
	return rational(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
rational acos(rational x) {
	return rational(std::acos(double(x)));
}

// secant of an angle of x radians
rational asin(rational x) {
	return rational(std::asin(double(x)));
}

// cotangent an angle of x radians
rational cot(rational x) {
	return rational(std::tan(sw::universal::m_pi_2-double(x)));
}

// secant of an angle of x radians
rational sec(rational x) {
	return rational(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
rational csc(rational x) {
	return rational(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
