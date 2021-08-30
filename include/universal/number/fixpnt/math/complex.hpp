#pragma once
// complex.hpp: functions for complex fixed-points
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw::universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Real component of a complex fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> real(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::real(x));
}

// Imaginary component of a complex fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> imag(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::imag(x));
}

// Conjucate of a complex fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
std::complex< fixpnt<nbits, rbits, arithmetic, bt> > conj(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::conj(x));
}

}  // namespace sw::universal
