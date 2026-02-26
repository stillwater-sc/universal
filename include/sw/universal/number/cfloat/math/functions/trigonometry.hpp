#pragma once
// trigonometric.hpp: trigonometric functions for classic floating-point cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <math/constants/double_constants.hpp>

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> sin(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> cos(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> tan(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::tan(double(x)));
}

// cotangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> atan(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::atan(double(x)));
}
		
// Arc tangent with two parameters
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> atan2(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y, cfloat<nbits,es,bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> acos(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::acos(double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> asin(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::asin(double(x)));
}

// cotangent an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> cot(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::tan(sw::universal::d_pi_2-double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> sec(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> csc(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt,hasSubnormals,hasMaxExpValues,isSaturating>(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
