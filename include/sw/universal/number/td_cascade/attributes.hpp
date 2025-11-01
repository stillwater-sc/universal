#pragma once
// attributes.hpp: attribute functions for triple-double cascade (td_cascade)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

	// functions to provide details about the properties of a td_cascade configuration

	// categorization functions

	// fpclassify categorizes floating point value into zero, subnormal, normal, infinite, NAN
	inline int fpclassify(const td_cascade& a) {
		return std::fpclassify(a[0]);
	}

	inline bool iszero(td_cascade const& a) {
		return a.iszero();
	}

	inline bool isone(td_cascade const& a) {
		return a.isone();
	}

	inline bool isnan(td_cascade const& a) {
		return a.isnan();
	}

	inline bool isinf(td_cascade const& a) {
		return a.isinf();
	}

	inline bool isfinite(td_cascade const& a) {
		return a.isfinite();
	}

	inline bool isnormal(td_cascade const& a) {
		return !iszero(a) && !isinf(a) && !isnan(a);
	}

	inline bool isdenorm(td_cascade const& a) {
		// td_cascade doesn't have a special denorm representation
		// but we can check if it's smaller than the normal minimum
		return std::abs(a[0]) < std::numeric_limits<double>::min() && !iszero(a);
	}


	inline bool isneg(td_cascade const& a) {
		return a.isneg();
	}

	inline bool ispos(td_cascade const& a) {
		return a.ispos();
	}

	// value functions

	inline td_cascade abs(td_cascade const& a) {
		return (a.isneg() ? -a : a);
	}

	inline td_cascade fabs(td_cascade const& a) {
		return abs(a);
	}

}} // namespace sw::universal
