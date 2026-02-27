#pragma once
// logarithm.hpp: logarithm functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Natural logarithm of x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> log(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::log(double(x)));
}

// Binary logarithm of x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> log2(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::log2(double(x)));
}

// Decimal logarithm of x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> log10(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::log10(double(x)));
}

// Natural logarithm of 1+x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> log1p(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::log1p(double(x)));
}

}} // namespace sw::universal
