#pragma once
// complex.hpp: functions for complex posits
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
template<unsigned nbits, unsigned es>
posit<nbits,es> real(std::complex< posit<nbits,es> > x) {
  return posit<nbits,es>(x.real());
}

// Imaginary component of a complex posit
template<unsigned nbits, unsigned es>
posit<nbits,es> imag(std::complex< posit<nbits,es> > x) {
  return posit<nbits,es>(x.imag());
}

// Conjucate of a complex posit
template<unsigned nbits, unsigned es>
std::complex< posit<nbits,es> > conj(std::complex< posit<nbits,es> > x) {
  return std::complex< posit<nbits,es> >(x.real(), -x.imag());
}

}} // namespace sw::universal
