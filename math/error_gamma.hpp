#pragma once
// error_gamma.hpp: error and gamma functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {

		// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
		// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

		// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
		template<size_t nbits, size_t es>
		posit<nbits,es> erf(posit<nbits,es> x) {
			return posit<nbits,es>(std::erf(double(x)));
		}

		// Compute the complementary error function: 1 - erf(x)
		template<size_t nbits, size_t es>
		posit<nbits,es> erfc(posit<nbits,es> x) {
			return posit<nbits,es>(std::erfc(double(x)));
		}

	}  // namespace unum

}  // namespace sw
