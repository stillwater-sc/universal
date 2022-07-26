#pragma once
// logarithm.hpp: logarithm functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Natural logarithm of x
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> log(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::log(double(x)));
}

// Binary logarithm of x
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> log2(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::log2(double(x)));
}

// Decimal logarithm of x
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> log10(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::log10(double(x)));
}
		
// Natural logarithm of 1+x
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> log1p(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::log1p(double(x)));
}

}} // namespace sw::universal
