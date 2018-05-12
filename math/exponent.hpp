#pragma once
// exponent.hpp: exponent functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {

		// Base-e exponential function
		template<size_t nbits, size_t es>
		posit<nbits,es> exp(posit<nbits,es> x) {
			return posit<nbits,es>(std::exp(double(x)));
		}

		// Base-2 exponential function
		template<size_t nbits, size_t es>
		posit<nbits,es> exp2(posit<nbits,es> x) {
			return posit<nbits,es>(std::exp2(double(x)));
		}

		// Base-10 exponential function
		template<size_t nbits, size_t es>
		posit<nbits,es> exp10(posit<nbits,es> x) {
			return posit<nbits,es>(std::exp10(double(x)));
		}

	}  // namespace unum

}  // namespace sw
