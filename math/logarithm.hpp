#pragma once
// logarithm.hpp: logarithm functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {

		// Natural logarithm
		template<size_t nbits, size_t es>
		posit<nbits,es> log(posit<nbits,es> x) {
			return posit<nbits,es>(std::log(double(x)));
		}

		// Binary logarithm
		template<size_t nbits, size_t es>
		posit<nbits,es> log2(posit<nbits,es> x) {
			return posit<nbits,es>(std::log2(double(x)));
		}

		// Decimal logarithm
		template<size_t nbits, size_t es>
		posit<nbits,es> log10(posit<nbits,es> x) {
			return posit<nbits,es>(std::log10(double(x)));
		}

	}  // namespace unum

}  // namespace sw
