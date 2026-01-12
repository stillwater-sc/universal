#pragma once
// complex.hpp: complex number support for quad-double (qd) types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This file provides complex number support for qd types using sw::universal::complex<T>.
// Native implementations of transcendental functions are provided to preserve
// the full ~64 decimal digit precision of qd arithmetic.

#include <universal/math/complex.hpp>
#include <universal/math/complex/complex_functions_qd.hpp>

namespace sw { namespace universal {

// Mark qd as a Universal number type for complex compatibility
template<>
struct is_universal_number<qd> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<qd>

// Real component of a complex qd
inline qd real(complex<qd> x) {
	return x.real();
}

// Imaginary component of a complex qd
inline qd imag(complex<qd> x) {
	return x.imag();
}

// Conjugate of a complex qd
inline complex<qd> conj(complex<qd> x) {
	return complex<qd>(x.real(), -x.imag());
}

// Classification functions for complex<qd>
inline bool isnan(complex<qd> x) {
	return (isnan(x.real()) || isnan(x.imag()));
}

inline bool isinf(complex<qd> x) {
	return (isinf(x.real()) || isinf(x.imag()));
}

}} // namespace sw::universal
