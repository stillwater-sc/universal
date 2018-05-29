#pragma once
// math_hypot.hpp: hypot functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {

		// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
		// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

		template<size_t nbits, size_t es>
		posit<nbits,es> hypot(posit<nbits,es> x, posit<nbits,es> y) {
			return posit<nbits,es>(std::hypot(double(x),double(y)));
		}
		
	}  // namespace unum

}  // namespace sw
