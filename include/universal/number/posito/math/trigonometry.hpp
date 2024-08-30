#pragma once
// trigonometric.hpp: trigonometric functions for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/math/constants/double_constants.hpp>  // for m_pi_2

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posito standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> sin(posito<nbits,es> x) {
	//std::cerr << "sw::universal::sin(posito<" << nbits << "," << es << ")";
	return posito<nbits,es>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> cos(posito<nbits,es> x) {
	//std::cerr << "sw::universal::cos(posito<" << nbits << "," << es << ")";
	return posito<nbits,es>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> tan(posito<nbits,es> x) {
	//std::cerr << "sw::universal::tan(posito<" << nbits << "," << es << ")";
	return posito<nbits,es>(std::tan(double(x)));
}

// cotangent of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> atan(posito<nbits,es> x) {
	//std::cerr << "sw::universal::atan(posito<" << nbits << "," << es << ")";
	return posito<nbits,es>(std::atan(double(x)));
}
		
// Arc tangent with two parameters
template<unsigned nbits, unsigned es>
posito<nbits,es> atan2(posito<nbits,es> y, posito<nbits,es> x) {
	return posito<nbits,es>(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> acos(posito<nbits,es> x) {
	return posito<nbits,es>(std::acos(double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> asin(posito<nbits,es> x) {
	return posito<nbits,es>(std::asin(double(x)));
}

// cotangent an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> cot(posito<nbits,es> x) {
	return posito<nbits,es>(std::tan(sw::universal::d_pi_2-double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> sec(posito<nbits,es> x) {
	return posito<nbits,es>(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> csc(posito<nbits,es> x) {
	return posito<nbits,es>(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
