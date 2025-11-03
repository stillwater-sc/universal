#pragma once
// hypot.hpp: hypot support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// hypot: compute sqrt(x^2 + y^2) without overflow
	// Phase 3: Full adaptive-precision implementation using Phase 3 sqrt
	//   Strategy: Simple wrapper - ereal's expansion arithmetic prevents overflow naturally
	//   No need for complex scaling (unlike fixed-precision types)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> hypot(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		// Compute using expansion arithmetic (naturally overflow-safe)
		ereal<maxlimbs> x2 = x * x;
		ereal<maxlimbs> y2 = y * y;
		return sqrt(x2 + y2);
	}

	// hypot: compute sqrt(x^2 + y^2 + z^2) without overflow (3-argument version)
	// Phase 3: Full adaptive-precision implementation using Phase 3 sqrt
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> hypot(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y, const ereal<maxlimbs>& z) {
		// Compute using expansion arithmetic (naturally overflow-safe)
		ereal<maxlimbs> x2 = x * x;
		ereal<maxlimbs> y2 = y * y;
		ereal<maxlimbs> z2 = z * z;
		return sqrt(x2 + y2 + z2);
	}

}} // namespace sw::universal
