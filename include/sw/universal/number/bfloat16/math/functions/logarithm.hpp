#pragma once
// logarithm.hpp: logarithm functions for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Natural logarithm of x
inline bfloat16 log(bfloat16 x) {
	return bfloat16(std::log(double(x)));
}

// Binary logarithm of x
inline bfloat16 log2(bfloat16 x) {
	return bfloat16(std::log2(double(x)));
}

// Decimal logarithm of x
inline bfloat16 log10(bfloat16 x) {
	return bfloat16(std::log10(double(x)));
}
		
// Natural logarithm of 1+x
inline bfloat16 log1p(bfloat16 x) {
	return bfloat16(std::log1p(double(x)));
}

}} // namespace sw::universal
