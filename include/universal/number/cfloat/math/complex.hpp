#pragma once
// complex.hpp: functions for complex cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw::universal {

// the current shims are NON-COMPLIANT with the cfloat standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Real component of a complex cfloat
template<size_t nbits, size_t es, typename bt>
cfloat<nbits, es, bt> real(std::complex< cfloat<nbits, es, bt> > x) {
	return cfloat<nbits, es, bt>(std::real(x));
}

// Imaginary component of a complex cfloat
template<size_t nbits, size_t es, typename bt>
cfloat<nbits, es, bt> imag(std::complex< cfloat<nbits, es, bt> > x) {
	return cfloat<nbits, es, bt>(std::imag(x));
}

// Conjucate of a complex cfloat
template<size_t nbits, size_t es, typename bt>
std::complex< cfloat<nbits, es, bt> > conj(std::complex< cfloat<nbits, es, bt> > x) {
	return std::conj(x);
}

}  // namespace sw::universal
