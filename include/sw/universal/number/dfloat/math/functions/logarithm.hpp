#pragma once
// logarithm.hpp: logarithm functions for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Natural logarithm of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> log(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::log(double(x)));
}

// Binary logarithm of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> log2(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::log2(double(x)));
}

// Decimal logarithm of x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> log10(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::log10(double(x)));
}

// Natural logarithm of 1+x
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> log1p(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::log1p(double(x)));
}

}} // namespace sw::universal
