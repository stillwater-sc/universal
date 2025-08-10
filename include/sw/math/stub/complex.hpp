#pragma once
// complex.hpp: templated complex functions stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Real component of a complex posit
template<typename Scalar,
    typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
Scalar real(std::complex<Scalar> c) {
	return std::real(c);
}

// Imaginary component of a complex posit
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
Scalar imag(std::complex<Scalar> c) {
	return std::imag(c);
}

// Conjucate of a complex posit
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
std::complex< Scalar > conj(std::complex< Scalar > x) {
	return std::conj(x);
}

}} // namespace sw::universal
