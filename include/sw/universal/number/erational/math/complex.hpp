#pragma once
// complex.hpp: functions for complex adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw { namespace universal {

	// Real component of a complex erational
	template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
	erational real(std::complex< erational > x) {
	  return erational(x.real());
	}

	// Imaginary component of a complex erational
	template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
	erational imag(std::complex< erational > x) {
	  return erational(x.imag());
	}

	// Conjucate of a complex erational
	template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
	std::complex< erational > conj(std::complex< erational > x) {
	  return std::complex< erational >(x.real(), -x.imag());
	}

	bool isnan(std::complex< erational > x) {
		return (isnan(x.real()) || isnan(x.imag()));
	}

	bool isinf(std::complex< erational > x) {
		return (isinf(x.real()) || isinf(x.imag()));
	}

}} // namespace sw::universal
