#pragma once
// pow.hpp: pow functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// power function: x raised to the power of y
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> pow(hfloat<ndigits, es, bt> x, hfloat<ndigits, es, bt> y) {
	return hfloat<ndigits, es, bt>(std::pow(double(x), double(y)));
}

// power function: x raised to the integer power of y
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> pow(hfloat<ndigits, es, bt> x, int y) {
	return hfloat<ndigits, es, bt>(std::pow(double(x), double(y)));
}

// power function: x raised to the double power of y
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> pow(hfloat<ndigits, es, bt> x, double y) {
	return hfloat<ndigits, es, bt>(std::pow(double(x), y));
}

}} // namespace sw::universal
