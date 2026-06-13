// sqrt.hpp: square root function for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// Square root. Negative input or NaR produces NaR (Not-a-Real).
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> sqrt(const takum<nbits, rbits, bt>& a) {
	if (a.isneg() || a.isnar()) {
		takum<nbits, rbits, bt> p;
		p.setnar();
		return p;
	}
	return takum<nbits, rbits, bt>(std::sqrt(double(a)));
}

// Reciprocal square root.
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> rsqrt(const takum<nbits, rbits, bt>& a) {
	takum<nbits, rbits, bt> v = sqrt(a);
	return takum<nbits, rbits, bt>(1.0) / v;
}

}} // namespace sw::universal
