#pragma once
// next.hpp: nextafter function for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// nextafter: returns the next representable value of x in the direction of target
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
dfloat<ndigits, es, Encoding, bt> nextafter(dfloat<ndigits, es, Encoding, bt> x, dfloat<ndigits, es, Encoding, bt> target) {
	return dfloat<ndigits, es, Encoding, bt>(std::nextafter(double(x), double(target)));
}

}} // namespace sw::universal
