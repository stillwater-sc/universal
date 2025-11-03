#pragma once
// pow.hpp: power functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// pow: power function x^y
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using exp(y * log(x)) with special cases
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> pow(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return ereal<maxlimbs>(std::pow(double(x), double(y)));
	}

	// pow: power function x^y (mixed type: ereal^double)
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using exp(y * log(x)) with special cases
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> pow(const ereal<maxlimbs>& x, double y) {
		return ereal<maxlimbs>(std::pow(double(x), y));
	}

	// pow: power function x^y (mixed type: double^ereal)
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using exp(y * log(x)) with special cases
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> pow(double x, const ereal<maxlimbs>& y) {
		return ereal<maxlimbs>(std::pow(x, double(y)));
	}

}} // namespace sw::universal
