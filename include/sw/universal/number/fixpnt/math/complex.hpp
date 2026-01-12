#pragma once
// complex.hpp: complex number support for fixpnt types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This file provides complex number support for fixpnt types using sw::universal::complex<T>,
// which is a portable replacement for std::complex<T> that works with user-defined types.
// The std::complex<fixpnt> overloads are retained for backward compatibility on compilers
// that do not strictly enforce ISO C++ 26.2/2.

#include <complex>
#include <universal/math/complex.hpp>

namespace sw { namespace universal {

// Mark fixpnt as a Universal number type for complex compatibility
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
struct is_universal_number<fixpnt<nbits, rbits, arithmetic, bt>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<fixpnt> (preferred, portable)

// Real component of a complex fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> real(complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return x.real();
}

// Imaginary component of a complex fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> imag(complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return x.imag();
}

// Conjugate of a complex fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
complex< fixpnt<nbits, rbits, arithmetic, bt> > conj(complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return complex< fixpnt<nbits, rbits, arithmetic, bt> >(x.real(), -x.imag());
}

// Classification functions for sw::universal::complex<fixpnt>
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
bool isnan(complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return (isnan(x.real()) || isnan(x.imag()));
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
bool isinf(complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return (isinf(x.real()) || isinf(x.imag()));
}

////////////////////////////////////////////////////////////////////
// Functions for std::complex<fixpnt> (backward compatibility)
//
// NOTE: These functions are provided for backward compatibility with
// existing code that uses std::complex<fixpnt>. However, std::complex<T>
// is only defined for float, double, and long double per ISO C++ 26.2/2.
// Apple Clang and other strict compilers may reject std::complex<fixpnt>.
// For portable code, use sw::universal::complex<fixpnt> instead.

// Real component of a complex fixpnt (std::complex version)
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> real(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(x.real());
}

// Imaginary component of a complex fixpnt (std::complex version)
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> imag(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(x.imag());
}

// Conjugate of a complex fixpnt (std::complex version)
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
std::complex< fixpnt<nbits, rbits, arithmetic, bt> > conj(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return std::complex< fixpnt<nbits, rbits, arithmetic, bt> >(x.real(), -x.imag());
}

// Classification functions for std::complex<fixpnt>
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
bool isnan(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return (isnan(x.real()) || isnan(x.imag()));
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
bool isinf(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return (isinf(x.real()) || isinf(x.imag()));
}

}} // namespace sw::universal
