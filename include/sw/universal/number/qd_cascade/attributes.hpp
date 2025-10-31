#pragma once
// attributes.hpp: attribute functions for qd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

	// functions to provide details about the properties of a qd_cascade configuration

	// categorization functions

	inline bool iszero(qd_cascade const& a) {
		return a.iszero();
	}

	inline bool isone(qd_cascade const& a) {
		return a.isone();
	}

	inline bool isnan(qd_cascade const& a) {
		return a.isnan();
	}

	inline bool isinf(qd_cascade const& a) {
		return a.isinf();
	}

	inline bool isfinite(qd_cascade const& a) {
		return a.isfinite();
	}

	inline bool isnormal(qd_cascade const& a) {
		return !iszero(a) && !isinf(a) && !isnan(a);
	}

	inline bool isdenorm(qd_cascade const& a) {
		// qd_cascade doesn't have a special denorm representation
		// but we can check if it's smaller than the normal minimum
		return std::abs(a[0]) < std::numeric_limits<double>::min() && !iszero(a);
	}

	// sign functions
	// Note: signbit is defined in qd_cascade_impl.hpp

	inline bool sign(qd_cascade const& a) {
		return a.sign() < 0;
	}

	inline bool isneg(qd_cascade const& a) {
		return a.isneg();
	}

	inline bool ispos(qd_cascade const& a) {
		return a.ispos();
	}

	// value functions

	inline qd_cascade abs(qd_cascade const& a) {
		return (a.isneg() ? -a : a);
	}

	inline qd_cascade fabs(qd_cascade const& a) {
		return abs(a);
	}

}} // namespace sw::universal
