#pragma once
// fractional.hpp: fractional functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// fmod retuns x - n*y where n = x/y with the fractional part truncated
inline bfloat16 fmod(bfloat16 x, bfloat16 y) {
	return bfloat16(std::fmod(float(x), float(y)));
}

// remainder returns x - n*y, where n = trunc(x/y)
inline bfloat16 remainder(bfloat16 x, bfloat16 y) {
	return bfloat16(std::remainder(float(x), float(y)));
}

// TODO: validate the rounding of these conversion, versus a method that manipulates the fraction bits directly

// frac returns the fraction of a bfloat value that is > 1
inline bfloat16 frac(bfloat16 x) {
	long long intValue = (long long)(x);
	return abs(x-bfloat16(intValue));  // with the logic that fractions are unsigned quantities
}

}} // namespace sw::universal
