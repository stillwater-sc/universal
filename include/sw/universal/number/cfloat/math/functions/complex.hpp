#pragma once
// complex.hpp: complex number support for cfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This file provides complex number support for cfloat types using sw::universal::complex<T>,
// which is a portable replacement for std::complex<T> that works with user-defined types.
// The std::complex<cfloat> overloads are retained for backward compatibility on compilers
// that do not strictly enforce ISO C++ 26.2/2.

#include <complex>
#include <universal/math/complex.hpp>

namespace sw { namespace universal {

// Mark cfloat as a Universal number type for complex compatibility
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
struct is_universal_number<cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<cfloat> (preferred, portable)

// Real component of a complex cfloat
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
real(complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return x.real();
}

// Imaginary component of a complex cfloat
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
imag(complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return x.imag();
}

// Conjugate of a complex cfloat
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> >
conj(complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> >(x.real(), -x.imag());
}

////////////////////////////////////////////////////////////////////
// Functions for std::complex<cfloat> (backward compatibility)
//
// NOTE: These functions are provided for backward compatibility with
// existing code that uses std::complex<cfloat>. However, std::complex<T>
// is only defined for float, double, and long double per ISO C++ 26.2/2.
// Apple Clang and other strict compilers may reject std::complex<cfloat>.
// For portable code, use sw::universal::complex<cfloat> instead.

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Real component of a complex cfloat (std::complex version)
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
real(std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::real(x));
}

// Imaginary component of a complex cfloat (std::complex version)
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
imag(std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::imag(x));
}

// Conjugate of a complex cfloat (std::complex version)
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> >
conj(std::complex< cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) {
	return std::complex<cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>>(x.real(), -x.imag());
}

}} // namespace sw::universal
