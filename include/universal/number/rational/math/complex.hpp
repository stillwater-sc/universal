#pragma once
// complex.hpp: functions for complex rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw::universal {

// Real component of a complex rational
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
rational real(std::complex< rational > x) {
  return rational(x.real());
}

// Imaginary component of a complex rational
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
rational imag(std::complex< rational > x) {
  return rational(x.imag());
}

// Conjucate of a complex rational
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
std::complex< rational > conj(std::complex< rational > x) {
  return std::complex< rational >(x.real(), -x.imag());
}

bool isnan(std::complex< rational > x) {
	return (isnan(x.real()) || isnan(x.imag()));
}

bool isinf(std::complex< rational > x) {
	return (isinf(x.real()) || isinf(x.imag()));
}

}  // namespace sw::universal
