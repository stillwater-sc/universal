#pragma once
// logarithm.hpp: logarithm functions for rational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Natural logarithm of x
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt> log(rational<nbits, Base, bt> x) {
	return rational<nbits, Base, bt>(std::log(double(x)));
}

// Binary logarithm of x
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt> log2(rational<nbits, Base, bt> x) {
	return rational<nbits, Base, bt>(std::log2(double(x)));
}

// Decimal logarithm of x
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt> log10(rational<nbits, Base, bt> x) {
	return rational<nbits, Base, bt>(std::log10(double(x)));
}

// Natural logarithm of 1+x
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt> log1p(rational<nbits, Base, bt> x) {
	return rational<nbits, Base, bt>(std::log1p(double(x)));
}

}} // namespace sw::universal
