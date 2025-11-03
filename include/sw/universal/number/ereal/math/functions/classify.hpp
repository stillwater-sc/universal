#pragma once
// classify.hpp: classification functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fpclassify: categorize floating-point value
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion component analysis
	template<unsigned maxlimbs>
	inline int fpclassify(const ereal<maxlimbs>& x) {
		return std::fpclassify(double(x));
	}

	// isnan: test for NaN
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion component analysis
	template<unsigned maxlimbs>
	inline bool isnan(const ereal<maxlimbs>& x) {
		return std::isnan(double(x));
	}

	// isinf: test for infinity
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion component analysis
	template<unsigned maxlimbs>
	inline bool isinf(const ereal<maxlimbs>& x) {
		return std::isinf(double(x));
	}

	// isfinite: test for finite value
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion component analysis
	template<unsigned maxlimbs>
	inline bool isfinite(const ereal<maxlimbs>& x) {
		return std::isfinite(double(x));
	}

	// isnormal: test for normal value
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion component analysis
	template<unsigned maxlimbs>
	inline bool isnormal(const ereal<maxlimbs>& x) {
		return std::isnormal(double(x));
	}

	// signbit: test sign bit
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using high component sign
	template<unsigned maxlimbs>
	inline bool signbit(const ereal<maxlimbs>& x) {
		return std::signbit(double(x));
	}

}} // namespace sw::universal
