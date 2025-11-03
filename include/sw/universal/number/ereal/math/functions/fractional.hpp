#pragma once
// fractional.hpp: fractional support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fmod: floating-point remainder of x/y
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> fmod(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return ereal<maxlimbs>(std::fmod(double(x), double(y)));
	}

	// remainder: IEEE remainder of x/y
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> remainder(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return ereal<maxlimbs>(std::remainder(double(x), double(y)));
	}

}} // namespace sw::universal
