#pragma once
// trigonometric.hpp: trigonometric functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <math/constants/double_constants.hpp>  // for m_pi_2

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> sin(posit<nbits,es> x) {
	//std::cerr << "sw::universal::sin(posit<" << nbits << "," << es << ")";
	return posit<nbits,es>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> cos(posit<nbits,es> x) {
	//std::cerr << "sw::universal::cos(posit<" << nbits << "," << es << ")";
	return posit<nbits,es>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> tan(posit<nbits,es> x) {
	//std::cerr << "sw::universal::tan(posit<" << nbits << "," << es << ")";
	return posit<nbits,es>(std::tan(double(x)));
}

// cotangent of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> atan(posit<nbits,es> x) {
	//std::cerr << "sw::universal::atan(posit<" << nbits << "," << es << ")";
	return posit<nbits,es>(std::atan(double(x)));
}
		
// Arc tangent with two parameters
template<unsigned nbits, unsigned es>
posit<nbits,es> atan2(posit<nbits,es> y, posit<nbits,es> x) {
	return posit<nbits,es>(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> acos(posit<nbits,es> x) {
	return posit<nbits,es>(std::acos(double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> asin(posit<nbits,es> x) {
	return posit<nbits,es>(std::asin(double(x)));
}

// cotangent an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> cot(posit<nbits,es> x) {
	return posit<nbits,es>(std::tan(sw::universal::d_pi_2-double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> sec(posit<nbits,es> x) {
	return posit<nbits,es>(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es>
posit<nbits,es> csc(posit<nbits,es> x) {
	return posit<nbits,es>(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
