#pragma once
// complex.hpp: functions for complex fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw { namespace universal {

// Real component of a complex fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> real(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
  return fixpnt<nbits, rbits, arithmetic, bt>(x.real());
}

// Imaginary component of a complex fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> imag(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
  return fixpnt<nbits, rbits, arithmetic, bt>(x.imag());
}

// Conjucate of a complex fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
std::complex< fixpnt<nbits, rbits, arithmetic, bt> > conj(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
  return std::complex< fixpnt<nbits, rbits, arithmetic, bt> >(x.real(), -x.imag());
}

// modifies the classify functions as well
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
bool isnan(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return (isnan(x.real()) || isnan(x.imag()));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
bool isinf(std::complex< fixpnt<nbits, rbits, arithmetic, bt> > x) {
	return (isinf(x.real()) || isinf(x.imag()));
}

}} // namespace sw::universal
