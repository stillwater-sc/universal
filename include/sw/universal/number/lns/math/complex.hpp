#pragma once
// complex.hpp: functions for complex lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw { namespace universal {

// Real component of a complex lns
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
real(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return lns<nbits, rbits, bt, xtra...>(std::real(x));
}

// Imaginary component of a complex lns
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
imag(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return lns<nbits, rbits, bt, xtra...>(std::imag(x));
}

// Conjucate of a complex lns
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
std::complex< lns<nbits, rbits, bt, xtra...> >
conj(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return std::complex<lns<nbits, rbits, bt, xtra...>>(x.real(), -x.imag());
}

}} // namespace sw::universal
