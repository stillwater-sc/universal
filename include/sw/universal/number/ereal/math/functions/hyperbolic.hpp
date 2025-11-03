#pragma once
// hyperbolic.hpp: hyperbolic functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// sinh: hyperbolic sine
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series or (e^x - e^-x)/2
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> sinh(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::sinh(double(x)));
	}

	// cosh: hyperbolic cosine
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series or (e^x + e^-x)/2
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> cosh(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::cosh(double(x)));
	}

	// tanh: hyperbolic tangent
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using sinh/cosh or (e^2x - 1)/(e^2x + 1)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> tanh(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::tanh(double(x)));
	}

	// asinh: inverse hyperbolic sine
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using log(x + sqrt(x^2 + 1))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> asinh(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::asinh(double(x)));
	}

	// acosh: inverse hyperbolic cosine
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using log(x + sqrt(x^2 - 1))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> acosh(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::acosh(double(x)));
	}

	// atanh: inverse hyperbolic tangent
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using 0.5 * log((1+x)/(1-x))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> atanh(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::atanh(double(x)));
	}

}} // namespace sw::universal
