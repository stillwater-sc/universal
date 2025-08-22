#pragma once
// complex.hpp: functions for complex cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Real component of a complex cfloat
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> 
real(std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::real(x));
}

// Imaginary component of a complex cfloat
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> 
imag(std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::imag(x));
}

// Conjucate of a complex cfloat
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > 
conj(std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return std::complex<cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>>(x.real(), -x.imag());
}

}} // namespace sw::universal
