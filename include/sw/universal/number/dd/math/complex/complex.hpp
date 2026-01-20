#pragma once
// complex.hpp: complex number support for double-double (dd) types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This file provides complex number support for dd types using sw::universal::complex<T>.
// Native implementations of transcendental functions are provided to preserve
// the full ~32 decimal digit precision of dd arithmetic.

#include <universal/math/complex.hpp>
#include <universal/math/complex/complex_functions_dd.hpp>

namespace sw { namespace universal {

// Mark dd as a Universal number type for complex compatibility
template<>
struct is_universal_number<dd> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Functions for sw::universal::complex<dd>

// Real component of a complex dd
inline dd real(complex<dd> x) {
	return x.real();
}

// Imaginary component of a complex dd
inline dd imag(complex<dd> x) {
	return x.imag();
}

// Conjugate of a complex dd
inline complex<dd> conj(complex<dd> x) {
	return complex<dd>(x.real(), -x.imag());
}

// Classification functions for complex<dd>
inline bool isnan(complex<dd> x) {
	return (isnan(x.real()) || isnan(x.imag()));
}

inline bool isinf(complex<dd> x) {
	return (isinf(x.real()) || isinf(x.imag()));
}

}} // namespace sw::universal
