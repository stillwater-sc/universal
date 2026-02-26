#pragma once
// hypot.hpp: hypotenuse function for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// hypotenuse function: sqrt(x*x + y*y) without undue overflow or underflow
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> hypot(dfloat<ndigits, es, Encoding, bt> x, dfloat<ndigits, es, Encoding, bt> y) {
	return dfloat<ndigits, es, Encoding, bt>(std::hypot(double(x), double(y)));
}

}} // namespace sw::universal
