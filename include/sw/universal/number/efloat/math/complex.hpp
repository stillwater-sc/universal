#pragma once
// complex.hpp: complex number support for efloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// efloat works with sw::universal::complex<T> -- the portable std::complex<T>
// replacement for user-defined types -- through the type-agnostic library in
// <universal/math/complex.hpp>. That library already provides the full surface
// (arithmetic, real/imag/conj/norm/arg/abs, and exp/log/sqrt/pow/sin/cos/sinh/...)
// generically, so this header only registers efloat as a Universal number type
// and re-exposes real/imag/conj for symmetry with the other number systems.
//
// PRECISION: efloat is registered as high precision (is_high_precision), so complex
// arithmetic, real/imag/conj/norm, abs, arg and the transcendentals (exp/log/sqrt/pow/
// sin/cos/sinh/asin/...) all compute in efloat's own arithmetic and carry its precision
// (#2026). The transcendentals follow the C99 branch cuts and Annex G special values.
//
// std::complex<efloat> is intentionally NOT supported: std::complex<T> is only
// defined for float, double, and long double (ISO C++ 26.2/2), so instantiating
// it on a user-defined type is undefined behavior. Use sw::universal::complex<efloat>.

#include <universal/number/efloat/efloat_fwd.hpp>
#include <universal/math/complex.hpp>

namespace sw {
namespace universal {

// Mark efloat as a Universal number type for complex compatibility
template<unsigned nlimbs>
struct is_universal_number<efloat<nlimbs>> : std::true_type {};

// compute the complex functions in efloat, not through double (#2026)
template<unsigned nlimbs>
struct is_high_precision<efloat<nlimbs>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<efloat> (portable)

// Real component of a complex efloat
template<unsigned nlimbs>
efloat<nlimbs> real(complex<efloat<nlimbs>> x) {
	return x.real();
}

// Imaginary component of a complex efloat
template<unsigned nlimbs>
efloat<nlimbs> imag(complex<efloat<nlimbs>> x) {
	return x.imag();
}

// Conjugate of a complex efloat
template<unsigned nlimbs>
complex<efloat<nlimbs>> conj(complex<efloat<nlimbs>> x) {
	return complex<efloat<nlimbs>>(x.real(), -x.imag());
}

}  // namespace universal
}  // namespace sw
