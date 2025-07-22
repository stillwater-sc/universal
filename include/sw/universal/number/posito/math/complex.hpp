#pragma once
// complex.hpp: functions for complex positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posito standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Real component of a complex posito
template<unsigned nbits, unsigned es>
posito<nbits,es> real(std::complex< posito<nbits,es> > x) {
  return posito<nbits,es>(x.real());
}

// Imaginary component of a complex posito
template<unsigned nbits, unsigned es>
posito<nbits,es> imag(std::complex< posito<nbits,es> > x) {
  return posito<nbits,es>(x.imag());
}

// Conjucate of a complex posito
template<unsigned nbits, unsigned es>
std::complex< posito<nbits,es> > conj(std::complex< posito<nbits,es> > x) {
  return std::complex< posito<nbits,es> >(x.real(), -x.imag());
}

}} // namespace sw::universal
