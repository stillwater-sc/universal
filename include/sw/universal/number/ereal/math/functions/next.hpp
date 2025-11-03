#pragma once
// next.hpp: nextafter/nexttoward functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// nextafter: return next representable value after x in direction of y
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic component manipulation
	//   Note: For adaptive precision, "next" may involve adding a small limb
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> nextafter(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return ereal<maxlimbs>(std::nextafter(double(x), double(y)));
	}

	// nexttoward: return next representable value after x in direction of y (long double)
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic component manipulation
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> nexttoward(const ereal<maxlimbs>& x, long double y) {
		return ereal<maxlimbs>(std::nexttoward(double(x), y));
	}

}} // namespace sw::universal
