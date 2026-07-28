#pragma once
// complex.hpp: complex number support for posit types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This file provides complex number support for posit types using sw::universal::complex<T>,
// which is a portable replacement for std::complex<T> that works with user-defined types.
// The std::complex<posit> overloads are retained for backward compatibility on compilers
// that do not strictly enforce ISO C++ 26.2/2.

#include <complex>
#include <universal/math/complex.hpp>

namespace sw { namespace universal {

// Mark posit as a Universal number type for complex compatibility
template<unsigned nbits, unsigned es>
struct is_universal_number<posit<nbits, es>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<posit> (preferred, portable)

// Real component of a complex posit
template<unsigned nbits, unsigned es>
posit<nbits,es> real(complex< posit<nbits,es> > x) {
	return x.real();
}

// Imaginary component of a complex posit
template<unsigned nbits, unsigned es>
posit<nbits,es> imag(complex< posit<nbits,es> > x) {
	return x.imag();
}

// Conjugate of a complex posit
template<unsigned nbits, unsigned es>
complex< posit<nbits,es> > conj(complex< posit<nbits,es> > x) {
	return complex< posit<nbits,es> >(x.real(), -x.imag());
}

////////////////////////////////////////////////////////////////////
// Functions for std::complex<posit> (backward compatibility)
//
// NOTE: These functions are provided for backward compatibility with
// existing code that uses std::complex<posit>. However, std::complex<T>
// is only defined for float, double, and long double per ISO C++ 26.2/2.
// Apple Clang and other strict compilers may reject std::complex<posit>.
// For portable code, use sw::universal::complex<posit> instead.

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Real component of a complex posit (std::complex version)
template<unsigned nbits, unsigned es>
posit<nbits,es> real(std::complex< posit<nbits,es> > x) {
	return posit<nbits,es>(x.real());
}

// Imaginary component of a complex posit (std::complex version)
template<unsigned nbits, unsigned es>
posit<nbits,es> imag(std::complex< posit<nbits,es> > x) {
	return posit<nbits,es>(x.imag());
}

// Conjugate of a complex posit (std::complex version)
template<unsigned nbits, unsigned es>
std::complex< posit<nbits,es> > conj(std::complex< posit<nbits,es> > x) {
	return std::complex< posit<nbits,es> >(x.real(), -x.imag());
}

}} // namespace sw::universal
