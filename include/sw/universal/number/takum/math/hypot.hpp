// hypot.hpp: hypotenuse functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> hypot(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return takum<nbits, rbits, bt>(std::hypot(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> hypotf(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return takum<nbits, rbits, bt>(std::hypotf(float(x), float(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> hypotl(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return takum<nbits, rbits, bt>(std::hypotl((long double)(x), (long double)(y)));
}

}} // namespace sw::universal
