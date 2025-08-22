#pragma once
// exponent.hpp: exponent functions for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Base-e exponential function
inline bfloat16 exp(bfloat16 x) {
	return bfloat16(std::exp(double(x)));
}

// Base-2 exponential function
inline bfloat16 exp2(bfloat16 x) {
	return bfloat16(std::exp2(double(x)));
}

// Base-10 exponential function
inline bfloat16 exp10(bfloat16 x) {
	return bfloat16(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
inline bfloat16 expm1(bfloat16 x) {
	return bfloat16(std::expm1(double(x)));
}

}} // namespace sw::universal
