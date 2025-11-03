#pragma once
// exponent.hpp: exponential functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// exp: exponential function e^x
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series with argument reduction
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::exp(double(x)));
	}

	// exp2: base-2 exponential function 2^x
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series with argument reduction
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp2(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::exp2(double(x)));
	}

	// exp10: base-10 exponential function 10^x
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using exp(x * ln(10))
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> exp10(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::pow(10.0, double(x)));
	}

	// expm1: compute e^x - 1 accurately for small x
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series (avoids cancellation)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> expm1(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::expm1(double(x)));
	}

}} // namespace sw::universal
