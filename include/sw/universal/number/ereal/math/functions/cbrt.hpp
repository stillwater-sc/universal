#pragma once
// cbrt.hpp: cbrt function for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// cbrt: cube root function
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using adaptive-precision Newton iteration
	//   Strategy: Use specialized Newton iteration with range reduction
	//   Similar to dd_cascade/td_cascade/qd_cascade implementations
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> cbrt(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::cbrt(double(x)));
	}

}} // namespace sw::universal
