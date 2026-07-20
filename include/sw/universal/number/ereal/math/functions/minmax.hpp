#pragma once
// minmax.hpp: minmax support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// min: return minimum of two values
	// Phase 1: uses adaptive-precision comparison operators
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> min(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return (x < y) ? x : y;
	}

	// max: return maximum of two values
	// Phase 1: uses adaptive-precision comparison operators
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> max(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return (x > y) ? x : y;
	}

	// fdim: positive difference -- x - y if x > y, otherwise +0 (NaN if either is NaN)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> fdim(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		if (x.isnan() || y.isnan()) return ereal<maxlimbs>(std::numeric_limits<double>::quiet_NaN());
		return (x > y) ? (x - y) : ereal<maxlimbs>(0.0);
	}

}} // namespace sw::universal
