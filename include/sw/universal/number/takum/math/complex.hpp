#pragma once
// complex.hpp: complex number support for takum types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This file provides complex number support for takum types using sw::universal::complex<T>,
// which is a portable replacement for std::complex<T> that works with user-defined types.
// The std::complex<takum> overloads are retained for backward compatibility on compilers
// that do not strictly enforce ISO C++ 26.2/2.

#include <complex>
#include <universal/math/complex.hpp>

namespace sw { namespace universal {

// Mark takum as a Universal number type for complex compatibility
template<unsigned nbits, unsigned rbits, typename bt>
struct is_universal_number<takum<nbits, rbits, bt>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<takum> (preferred, portable)

// Real component of a complex takum
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> real(complex< takum<nbits, rbits, bt> > x) {
	return x.real();
}

// Imaginary component of a complex takum
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> imag(complex< takum<nbits, rbits, bt> > x) {
	return x.imag();
}

// Conjugate of a complex takum
template<unsigned nbits, unsigned rbits, typename bt>
complex< takum<nbits, rbits, bt> > conj(complex< takum<nbits, rbits, bt> > x) {
	return complex< takum<nbits, rbits, bt> >(x.real(), -x.imag());
}

////////////////////////////////////////////////////////////////////
// Functions for std::complex<takum> (backward compatibility)
//
// NOTE: These functions are provided for backward compatibility with
// existing code that uses std::complex<takum>. However, std::complex<T>
// is only defined for float, double, and long double per ISO C++ 26.2/2.
// Apple Clang and other strict compilers may reject std::complex<takum>.
// For portable code, use sw::universal::complex<takum> instead.

// Real component of a complex takum (std::complex version)
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> real(std::complex< takum<nbits, rbits, bt> > x) {
	return takum<nbits, rbits, bt>(x.real());
}

// Imaginary component of a complex takum (std::complex version)
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> imag(std::complex< takum<nbits, rbits, bt> > x) {
	return takum<nbits, rbits, bt>(x.imag());
}

// Conjugate of a complex takum (std::complex version)
template<unsigned nbits, unsigned rbits, typename bt>
std::complex< takum<nbits, rbits, bt> > conj(std::complex< takum<nbits, rbits, bt> > x) {
	return std::complex< takum<nbits, rbits, bt> >(x.real(), -x.imag());
}

}} // namespace sw::universal
