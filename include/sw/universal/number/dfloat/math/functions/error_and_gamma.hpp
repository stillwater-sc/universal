#pragma once
// error_and_gamma.hpp: error and gamma functions for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// error function
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> erf(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::erf(double(x)));
}

// complementary error function
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> erfc(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::erfc(double(x)));
}

// true gamma function
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> tgamma(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::tgamma(double(x)));
}

// log of the gamma function
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> lgamma(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::lgamma(double(x)));
}

}} // namespace sw::universal
