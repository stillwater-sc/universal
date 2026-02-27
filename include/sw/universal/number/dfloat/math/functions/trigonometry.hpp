#pragma once
// trigonometry.hpp: trigonometric functions for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> sin(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> cos(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> tan(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::tan(double(x)));
}

// arc sine
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> asin(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::asin(double(x)));
}

// arc cosine
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> acos(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::acos(double(x)));
}

// arc tangent
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> atan(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::atan(double(x)));
}

// arc tangent with two parameters
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> atan2(dfloat<ndigits, es, Encoding, bt> y, dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::atan2(double(y), double(x)));
}

}} // namespace sw::universal
