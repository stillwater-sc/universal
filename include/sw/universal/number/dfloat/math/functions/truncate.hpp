#pragma once
// truncate.hpp: truncation functions for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// truncate towards zero: trunc(x)
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> trunc(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::trunc(double(x)));
}

// round to nearest: round(x)
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> round(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::round(double(x)));
}

// round towards negative infinity: floor(x)
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> floor(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::floor(double(x)));
}

// round towards positive infinity: ceil(x)
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> ceil(dfloat<ndigits, es, Encoding, bt> x) {
	return dfloat<ndigits, es, Encoding, bt>(std::ceil(double(x)));
}

}} // namespace sw::universal
