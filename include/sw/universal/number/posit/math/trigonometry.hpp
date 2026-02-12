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
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> sin(posit<nbits,es,bt> x) {
	//std::cerr << "sw::universal::sin(posit<" << nbits << "," << es << ")";
	return posit<nbits,es,bt>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> cos(posit<nbits,es,bt> x) {
	//std::cerr << "sw::universal::cos(posit<" << nbits << "," << es << ")";
	return posit<nbits,es,bt>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> tan(posit<nbits,es,bt> x) {
	//std::cerr << "sw::universal::tan(posit<" << nbits << "," << es << ")";
	return posit<nbits,es,bt>(std::tan(double(x)));
}

// cotangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> atan(posit<nbits,es,bt> x) {
	//std::cerr << "sw::universal::atan(posit<" << nbits << "," << es << ")";
	return posit<nbits,es,bt>(std::atan(double(x)));
}
		
// Arc tangent with two parameters
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> atan2(posit<nbits,es,bt> y, posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> acos(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::acos(double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> asin(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::asin(double(x)));
}

// cotangent an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> cot(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::tan(sw::universal::d_pi_2-double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> sec(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> csc(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
