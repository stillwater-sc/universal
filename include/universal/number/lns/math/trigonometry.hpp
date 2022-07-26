#pragma once
// trigonometric.hpp: trigonometric functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/math/math_constants.hpp>

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> sin(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> cos(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> tan(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::tan(double(x)));
}

// cotangent of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> atan(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::atan(double(x)));
}
		
// Arc tangent with two parameters
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> atan2(lns<nbits, rbits, bt> y, lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> acos(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::acos(double(x)));
}

// secant of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> asin(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::asin(double(x)));
}

// cotangent an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> cot(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::tan(sw::universal::m_pi_2-double(x)));
}

// secant of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> sec(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> csc(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
