#pragma once
// truncate.hpp: truncation functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {

		// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
		// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

		// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
		template<size_t nbits, size_t es>
		posit<nbits,es> trunc(posit<nbits,es> x) {
			return posit<nbits,es>(std::trunc(double(x)));
		}

		// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
		template<size_t nbits, size_t es>
		posit<nbits,es> round(posit<nbits,es> x) {
			return posit<nbits,es>(std::round(double(x)));
		}

		// Round x downward, returning the largest integral value that is not greater than x
		template<size_t nbits, size_t es>
		posit<nbits,es> floor(posit<nbits,es> x) {
			return posit<nbits,es>(std::floor(double(x)));
		}

		// Round x upward, returning the smallest integral value that is greater than x
		template<size_t nbits, size_t es>
		posit<nbits,es> ceil(posit<nbits,es> x) {
			return posit<nbits,es>(std::ceil(double(x)));
		}

	}  // namespace unum

}  // namespace sw
