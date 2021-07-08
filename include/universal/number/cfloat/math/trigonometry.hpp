#pragma once
// trigonometric.hpp: trigonometric functions for classic floating-point cfloat
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/math/math_constants.hpp>

namespace sw::universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> sin(cfloat<nbits,es,bt> x) {
	//std::cerr << "sw::universal::sin(cfloat<" << nbits << "," << es << ")";
	return cfloat<nbits,es,bt>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> cos(cfloat<nbits,es,bt> x) {
	//std::cerr << "sw::universal::cos(cfloat<" << nbits << "," << es << ")";
	return cfloat<nbits,es,bt>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> tan(cfloat<nbits,es,bt> x) {
	//std::cerr << "sw::universal::tan(cfloat<" << nbits << "," << es << ")";
	return cfloat<nbits,es,bt>(std::tan(double(x)));
}

// cotangent of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> atan(cfloat<nbits,es,bt> x) {
	//std::cerr << "sw::universal::atan(cfloat<" << nbits << "," << es << ")";
	return cfloat<nbits,es,bt>(std::atan(double(x)));
}
		
// Arc tangent with two parameters
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> atan2(cfloat<nbits,es,bt> y, cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> acos(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::acos(double(x)));
}

// secant of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> asin(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::asin(double(x)));
}

// cotangent an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> cot(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::tan(sw::universal::m_pi_2-double(x)));
}

// secant of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> sec(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> csc(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(1.0/std::sin(double(x)));
}

}  // namespace sw::universal
