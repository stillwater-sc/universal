#pragma once
// complex.hpp: complex number support for lns types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This file provides complex number support for lns types using sw::universal::complex<T>,
// which is a portable replacement for std::complex<T> that works with user-defined types.
// The std::complex<lns> overloads are retained for backward compatibility on compilers
// that do not strictly enforce ISO C++ 26.2/2.

#include <complex>
#include <universal/math/complex.hpp>

namespace sw { namespace universal {

// Mark lns as a Universal number type for complex compatibility
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
struct is_universal_number<lns<nbits, rbits, bt, xtra...>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<lns> (preferred, portable)

// Real component of a complex lns
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
real(complex< lns<nbits, rbits, bt, xtra...> > x) {
	return x.real();
}

// Imaginary component of a complex lns
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
imag(complex< lns<nbits, rbits, bt, xtra...> > x) {
	return x.imag();
}

// Conjugate of a complex lns
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
complex< lns<nbits, rbits, bt, xtra...> >
conj(complex< lns<nbits, rbits, bt, xtra...> > x) {
	return complex<lns<nbits, rbits, bt, xtra...>>(x.real(), -x.imag());
}

////////////////////////////////////////////////////////////////////
// Functions for std::complex<lns> (backward compatibility)
//
// NOTE: These functions are provided for backward compatibility with
// existing code that uses std::complex<lns>. However, std::complex<T>
// is only defined for float, double, and long double per ISO C++ 26.2/2.
// Apple Clang and other strict compilers may reject std::complex<lns>.
// For portable code, use sw::universal::complex<lns> instead.

// Real component of a complex lns (std::complex version)
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
real(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return lns<nbits, rbits, bt, xtra...>(std::real(x));
}

// Imaginary component of a complex lns (std::complex version)
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
imag(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return lns<nbits, rbits, bt, xtra...>(std::imag(x));
}

// Conjugate of a complex lns (std::complex version)
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
std::complex< lns<nbits, rbits, bt, xtra...> >
conj(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return std::complex<lns<nbits, rbits, bt, xtra...>>(x.real(), -x.imag());
}

}} // namespace sw::universal
