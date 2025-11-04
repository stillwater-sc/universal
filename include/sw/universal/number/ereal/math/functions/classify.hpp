#pragma once
// classify.hpp: classification functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fpclassify: categorize floating-point value
	// Phase 1: uses ereal's native classification methods
	// Note: ereal has no subnormal representation (expansion arithmetic)
	template<unsigned maxlimbs>
	inline int fpclassify(const ereal<maxlimbs>& x) {
		if (x.isnan()) return FP_NAN;
		if (x.isinf()) return FP_INFINITE;
		if (x.iszero()) return FP_ZERO;
		// For ereal, everything else is normal (no subnormal representation)
		return FP_NORMAL;
	}

	// isnan: test for NaN
	// Phase 1: uses ereal's native isnan() method
	template<unsigned maxlimbs>
	inline bool isnan(const ereal<maxlimbs>& x) {
		return x.isnan();
	}

	// isinf: test for infinity
	// Phase 1: uses ereal's native isinf() method
	template<unsigned maxlimbs>
	inline bool isinf(const ereal<maxlimbs>& x) {
		return x.isinf();
	}

	// isfinite: test for finite value
	// Phase 1: finite = not infinite and not NaN
	template<unsigned maxlimbs>
	inline bool isfinite(const ereal<maxlimbs>& x) {
		return !x.isinf() && !x.isnan();
	}

	// isnormal: test for normal value
	// Phase 1: for ereal, any non-zero finite value is "normal"
	// Note: expansion arithmetic has no subnormal representation
	template<unsigned maxlimbs>
	inline bool isnormal(const ereal<maxlimbs>& x) {
		return !x.iszero() && !x.isinf() && !x.isnan();
	}

	// signbit: test sign bit
	// Phase 1: uses ereal's native isneg() method
	template<unsigned maxlimbs>
	inline bool signbit(const ereal<maxlimbs>& x) {
		return x.isneg();
	}

}} // namespace sw::universal
