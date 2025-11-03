#pragma once
// truncate.hpp: truncate support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// trunc: truncate value by rounding toward zero
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> trunc(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::trunc(double(x)));
	}

	// round: round to nearest integer, halfway cases away from zero
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> round(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::round(double(x)));
	}

	// floor: return largest integer value not greater than x
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> floor(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::floor(double(x)));
	}

	// ceil: return smallest integer value not less than x
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> ceil(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::ceil(double(x)));
	}

}} // namespace sw::universal
