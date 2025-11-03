#pragma once
// hypot.hpp: hypot support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// hypot: compute sqrt(x^2 + y^2) without overflow
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic for accuracy
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> hypot(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return ereal<maxlimbs>(std::hypot(double(x), double(y)));
	}

	// hypot: compute sqrt(x^2 + y^2 + z^2) without overflow (3-argument version)
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic for accuracy
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> hypot(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y, const ereal<maxlimbs>& z) {
		return ereal<maxlimbs>(std::hypot(double(x), double(y), double(z)));
	}

}} // namespace sw::universal
