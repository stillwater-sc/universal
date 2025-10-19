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

	// return the Unit in the Last Place
	inline td_cascade ulp(const td_cascade& a) {
		// ULP for td_cascade is 2^-159
		return td_cascade(1.38777878078144567553615370835363e-48, 0.0, 0.0);
	}

	// categorization functions

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

	// sign functions
	// Note: signbit is defined in td_cascade_impl.hpp

	inline bool sign(td_cascade const& a) {
		return a.sign();
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
