#pragma once
// trigonometry.hpp: trigonometry functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// sin: sine function
	// Phase 0: stub using double conversion
	// TODO Phase 3: implement using Taylor series with argument reduction
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> sin(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::sin(double(x)));
	}

	// cos: cosine function
	// Phase 0: stub using double conversion
	// TODO Phase 3: implement using Taylor series with argument reduction
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> cos(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::cos(double(x)));
	}

	// tan: tangent function
	// Phase 0: stub using double conversion
	// TODO Phase 3: implement using sin/cos
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> tan(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::tan(double(x)));
	}

	// asin: arcsine function
	// Phase 0: stub using double conversion
	// TODO Phase 3: implement using Taylor series or Newton iteration
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> asin(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::asin(double(x)));
	}

	// acos: arccosine function
	// Phase 0: stub using double conversion
	// TODO Phase 3: implement using pi/2 - asin(x)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> acos(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::acos(double(x)));
	}

	// atan: arctangent function
	// Phase 0: stub using double conversion
	// TODO Phase 3: implement using Taylor series with argument reduction
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> atan(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::atan(double(x)));
	}

	// atan2: arctangent of y/x using signs to determine quadrant
	// Phase 0: stub using double conversion
	// TODO Phase 3: implement using atan with quadrant logic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> atan2(const ereal<maxlimbs>& y, const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::atan2(double(y), double(x)));
	}

}} // namespace sw::universal
