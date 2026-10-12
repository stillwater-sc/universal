#pragma once
// complex.hpp: complex number support for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sw::universal::complex<bposit> is the portable complex type: it carries the arithmetic,
// abs, arg, exp, log, sqrt, pow and the trigonometric functions for any Universal type.
// Those functions are evaluated through std::complex<double> and rounded once per
// component, so, as for the real functions here, they are not correctly rounded.
// std::complex<bposit> is outside the ISO C++ guarantee (std::complex is only defined for
// float, double and long double) and strict compilers reject it; use the Universal one.
#include <type_traits>   // std::true_type
#include <universal/math/complex.hpp>

namespace sw { namespace universal {

// mark the b-posit as a Universal number type, which sw::universal::complex accepts
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
struct is_universal_number<bposit<nbits, rs, es, bt>> : std::true_type {};

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> real(const complex<bposit<nbits, rs, es, bt>>& z) { return z.real(); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> imag(const complex<bposit<nbits, rs, es, bt>>& z) { return z.imag(); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
complex<bposit<nbits, rs, es, bt>> conj(const complex<bposit<nbits, rs, es, bt>>& z) {
	return complex<bposit<nbits, rs, es, bt>>(z.real(), -z.imag());
}

}} // namespace sw::universal
