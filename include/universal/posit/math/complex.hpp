#pragma once
// complex.hpp: functions for complex posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <complex>

namespace sw {
	namespace unum {

		// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
		// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

		// Real component of a complex posit
		template<size_t nbits, size_t es>
		posit<nbits,es> real(std::complex< posit<nbits,es> > x) {
			return posit<nbits,es>(std::real(x));
		}

		// Imaginary component of a complex posit
		template<size_t nbits, size_t es>
		posit<nbits,es> imag(std::complex< posit<nbits,es> > x) {
			return posit<nbits,es>(std::imag(x));
		}

		// Conjucate of a complex posit
		template<size_t nbits, size_t es>
		std::complex< posit<nbits,es> > conj(std::complex< posit<nbits,es> > x) {
			return std::conj(x);
		}

	}  // namespace unum

}  // namespace sw
