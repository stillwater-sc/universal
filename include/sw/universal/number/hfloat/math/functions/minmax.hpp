#pragma once
// minmax.hpp: min/max functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt>
min(hfloat<ndigits, es, bt> x, hfloat<ndigits, es, bt> y) {
	return hfloat<ndigits, es, bt>(std::min(double(x), double(y)));
}

template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt>
max(hfloat<ndigits, es, bt> x, hfloat<ndigits, es, bt> y) {
	return hfloat<ndigits, es, bt>(std::max(double(x), double(y)));
}

}} // namespace sw::universal
