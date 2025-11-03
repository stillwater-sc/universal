#pragma once
// sqrt.hpp: sqrt function for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// sqrt: square root function
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using adaptive-precision Newton-Raphson iteration
	//   Strategy: Use Newton-Raphson: x' = (x + a/x) / 2
	//   Starting with x = sqrt(high component), iterate to requested precision
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> sqrt(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::sqrt(double(x)));
	}

}} // namespace sw::universal
