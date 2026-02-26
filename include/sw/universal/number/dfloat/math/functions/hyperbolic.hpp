#pragma once
// hyperbolic.hpp: hyperbolic functions for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// hyperbolic sine of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> sinh(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::sinh(double(x)));
}

// hyperbolic cosine of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> cosh(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::cosh(double(x)));
}

// hyperbolic tangent of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> tanh(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::tanh(double(x)));
}

// inverse hyperbolic sine of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> asinh(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::asinh(double(x)));
}

// inverse hyperbolic cosine of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> acosh(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::acosh(double(x)));
}

// inverse hyperbolic tangent of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> atanh(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::atanh(double(x)));
}

}} // namespace sw::universal
