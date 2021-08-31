#pragma once
// logarithm.hpp: logarithm functions for fixed-points
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// Natural logarithm of x
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> log(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::log(double(x)));
}

// Binary logarithm of x
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> log2(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::log2(double(x)));
}

// Decimal logarithm of x
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> log10(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::log10(double(x)));
}
		
// Natural logarithm of 1+x
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> log1p(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::log1p(double(x)));
}

}  // namespace sw::universal
