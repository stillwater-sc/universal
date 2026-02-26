#pragma once
// hyperbolic.hpp: hyperbolic functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// hyperbolic sine of an angle of x radians
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> sinh(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> cosh(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> tanh(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::tanh(double(x)));
}

// inverse hyperbolic sine
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> asinh(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::asinh(double(x)));
}

// inverse hyperbolic cosine
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> acosh(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::acosh(double(x)));
}

// inverse hyperbolic tangent
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> atanh(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::atanh(double(x)));
}

}} // namespace sw::universal
