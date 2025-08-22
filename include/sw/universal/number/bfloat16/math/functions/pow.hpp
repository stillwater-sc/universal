#pragma once
// pow.hpp: pow functions for Google's Brain floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

inline bfloat16 pow(bfloat16 x, bfloat16 y) {
	using std::pow;
	return bfloat16(std::pow(float(x), float(y)));
}
		
inline bfloat16 pow(bfloat16 x, int y) {
	using std::pow;
	return bfloat16(std::pow(float(x), float(y)));
}
		
inline bfloat16 pow(bfloat16 x, float y) {
	using std::pow;
	return bfloat16(std::pow(float(x), y));
}

}} // namespace sw::universal
