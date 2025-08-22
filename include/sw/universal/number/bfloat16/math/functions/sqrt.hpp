#pragma once
// sqrt.hpp: square root function for Google's Brain floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

inline bfloat16 sqrt(bfloat16 x) {
	using std::sqrt;
	return bfloat16(std::sqrt(float(x)));
}
		

}} // namespace sw::universal
