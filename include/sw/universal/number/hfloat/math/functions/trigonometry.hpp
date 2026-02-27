#pragma once
// trigonometry.hpp: trigonometric functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> sin(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> cos(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> tan(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::tan(double(x)));
}

// arc sine of x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> asin(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::asin(double(x)));
}

// arc cosine of x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> acos(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::acos(double(x)));
}

// arc tangent of x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> atan(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::atan(double(x)));
}

// arc tangent of y/x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> atan2(hfloat<ndigits, es, bt> y, hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::atan2(double(y), double(x)));
}

}} // namespace sw::universal
