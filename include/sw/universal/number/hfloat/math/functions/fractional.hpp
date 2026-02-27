#pragma once
// fractional.hpp: fractional functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// fmod returns x - n*y where n = x/y with the fractional part truncated
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> fmod(hfloat<ndigits, es, bt> x, hfloat<ndigits, es, bt> y) {
	return hfloat<ndigits, es, bt>(std::fmod(double(x), double(y)));
}

// remainder of the floating point division operation x/y
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> remainder(hfloat<ndigits, es, bt> x, hfloat<ndigits, es, bt> y) {
	return hfloat<ndigits, es, bt>(std::remainder(double(x), double(y)));
}

}} // namespace sw::universal
