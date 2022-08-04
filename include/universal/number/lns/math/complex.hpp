#pragma once
// complex.hpp: functions for complex lns
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw { namespace universal {

// Real component of a complex lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
real(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return lns<nbits, rbits, bt, xtra...>(std::real(x));
}

// Imaginary component of a complex lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
imag(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return lns<nbits, rbits, bt, xtra...>(std::imag(x));
}

// Conjucate of a complex lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
std::complex< lns<nbits, rbits, bt, xtra...> >
conj(std::complex< lns<nbits, rbits, bt, xtra...> > x) {
	return lns<nbits, rbits, bt, xtra...>(std::conj(x));
}

}} // namespace sw::universal
