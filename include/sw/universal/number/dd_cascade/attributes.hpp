#pragma once
// attributes.hpp: attribute functions for dd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

	// functions to provide details about the properties of a dd_cascade configuration

	// return the Unit in the Last Place
	inline dd_cascade ulp(const dd_cascade& a) {
		// ULP for dd_cascade is 2^-106
		return dd_cascade(4.93038065763132e-32, 0.0);
	}

	// categorization functions

	inline bool iszero(dd_cascade const& a) {
		return a.iszero();
	}

	inline bool isone(dd_cascade const& a) {
		return a.isone();
	}

	inline bool isnan(dd_cascade const& a) {
		return a.isnan();
	}

	inline bool isinf(dd_cascade const& a) {
		return a.isinf();
	}

	inline bool isfinite(dd_cascade const& a) {
		return a.isfinite();
	}

	inline bool isnormal(dd_cascade const& a) {
		return !iszero(a) && !isinf(a) && !isnan(a);
	}

	inline bool isdenorm(dd_cascade const& a) {
		// dd_cascade doesn't have a special denorm representation
		// but we can check if it's smaller than the normal minimum
		return std::abs(a.high()) < std::numeric_limits<double>::min() && !iszero(a);
	}

	// sign functions
	// Note: signbit is defined in dd_cascade_impl.hpp

	inline bool sign(dd_cascade const& a) {
		return a.sign();
	}

	inline bool isneg(dd_cascade const& a) {
		return a.isneg();
	}

	inline bool ispos(dd_cascade const& a) {
		return a.ispos();
	}

	// value functions

	inline dd_cascade abs(dd_cascade const& a) {
		return (a.isneg() ? -a : a);
	}

	inline dd_cascade fabs(dd_cascade const& a) {
		return abs(a);
	}

}} // namespace sw::universal
