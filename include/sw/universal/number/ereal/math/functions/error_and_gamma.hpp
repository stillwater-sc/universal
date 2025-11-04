#pragma once
// error_and_gamma.hpp: error and gamma functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// erf: error function
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series or continued fractions
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> erf(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::erf(double(x)));
	}

	// erfc: complementary error function (1 - erf(x))
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series or continued fractions
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> erfc(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::erfc(double(x)));
	}

	// tgamma: gamma function
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Lanczos approximation or Stirling series
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> tgamma(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::tgamma(double(x)));
	}

	// lgamma: natural logarithm of absolute value of gamma function
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Lanczos approximation or Stirling series
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> lgamma(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::lgamma(double(x)));
	}

}} // namespace sw::universal
