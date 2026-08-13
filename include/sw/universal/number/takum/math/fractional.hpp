// fractional.hpp: fractional functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> fmod(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return takum<nbits, rbits, bt>(std::fmod(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> remainder(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return takum<nbits, rbits, bt>(std::remainder(double(x), double(y)));
}

// Fractional part: x - trunc(x)
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> frac(const takum<nbits, rbits, bt>& x) {
	return takum<nbits, rbits, bt>(double(x) - std::trunc(double(x)));
}

// ---------------------------------------------------------------------------
// takum_log: remainder and modulus are linear-domain operations.
// ---------------------------------------------------------------------------

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> fmod(const takum_log<nbits, rbits, bt>& x,
                                        const takum_log<nbits, rbits, bt>& y) {
	return takum_log<nbits, rbits, bt>(std::fmod(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> remainder(const takum_log<nbits, rbits, bt>& x,
                                             const takum_log<nbits, rbits, bt>& y) {
	return takum_log<nbits, rbits, bt>(std::remainder(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> frac(const takum_log<nbits, rbits, bt>& x) {
	double v = double(x);
	return takum_log<nbits, rbits, bt>(v - std::trunc(v));
}

}} // namespace sw::universal
