#pragma once
// complex.hpp: complex number support for ereal types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ereal works with sw::universal::complex<T> -- the portable std::complex<T>
// replacement for user-defined types -- through the type-agnostic library in
// <universal/math/complex.hpp>. That library already provides the full surface
// (arithmetic, real/imag/conj/norm/arg/abs, and exp/log/sqrt/pow/sin/cos/sinh/...)
// generically, so this header only registers ereal as a Universal number type
// and re-exposes real/imag/conj for symmetry with the other number systems.
//
// PRECISION NOTE: complex ARITHMETIC (+ - * /) and real/imag/conj/norm/arg/abs
// run on ereal's own operators and therefore carry the operand's full working
// precision. The complex TRANSCENDENTALS (exp/log/sqrt/pow/sin/cos/sinh/...) in
// the shared library currently pivot through std::complex<double>, so on ereal
// they are only accurate to ~double. This is a property of the shared library
// (it affects every user-defined type, not just ereal); a full-precision complex
// transcendental layer built on ereal's own sin/cos/exp is a future enhancement.
//
// std::complex<ereal> is intentionally NOT supported: std::complex<T> is only
// defined for float, double, and long double (ISO C++ 26.2/2), so instantiating
// it on a user-defined type is undefined behavior. Use sw::universal::complex<ereal>.

#include <universal/number/ereal/ereal_fwd.hpp>
#include <universal/math/complex.hpp>

namespace sw { namespace universal {

// Mark ereal as a Universal number type for complex compatibility
template<unsigned maxlimbs>
struct is_universal_number<ereal<maxlimbs>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<ereal> (portable)

// Real component of a complex ereal
template<unsigned maxlimbs>
ereal<maxlimbs> real(complex<ereal<maxlimbs>> x) {
	return x.real();
}

// Imaginary component of a complex ereal
template<unsigned maxlimbs>
ereal<maxlimbs> imag(complex<ereal<maxlimbs>> x) {
	return x.imag();
}

// Conjugate of a complex ereal
template<unsigned maxlimbs>
complex<ereal<maxlimbs>> conj(complex<ereal<maxlimbs>> x) {
	return complex<ereal<maxlimbs>>(x.real(), -x.imag());
}

}} // namespace sw::universal
