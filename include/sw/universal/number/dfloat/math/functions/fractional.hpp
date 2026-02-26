#pragma once
// fractional.hpp: fractional functions for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// floating-point remainder of the division operation x/y
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> fmod(dfloat<ndigits, es, Encoding, bt> x, dfloat<ndigits, es, Encoding, bt> y) {
	return dfloat<ndigits, es, Encoding, bt>(std::fmod(double(x), double(y)));
}

// IEEE remainder of the division operation x/y
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> remainder(dfloat<ndigits, es, Encoding, bt> x, dfloat<ndigits, es, Encoding, bt> y) {
	return dfloat<ndigits, es, Encoding, bt>(std::remainder(double(x), double(y)));
}

}} // namespace sw::universal
