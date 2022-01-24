#pragma once
// logarithm.hpp: logarithm functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Natural logarithm of x
rational log(rational x) {
	return rational(std::log(double(x)));
}

// Binary logarithm of x
rational log2(rational x) {
	return rational(std::log2(double(x)));
}

// Decimal logarithm of x
rational log10(rational x) {
	return rational(std::log10(double(x)));
}
		
// Natural logarithm of 1+x
rational log1p(rational x) {
	return rational(std::log1p(double(x)));
}

}} // namespace sw::universal
