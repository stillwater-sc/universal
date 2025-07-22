#pragma once
// math_classify.hpp: classification functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
	int fpclassify(const erational& a) {
		return std::fpclassify(double(a));
	}
	
	// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
	// specialized for erationals
	inline bool isfinite(const erational& a) {
		return true;
	}

	// STD LIB function for IEEE floats: Determines if the given floating point number arg is a fixpntive or negative infinity.
	// specialized for erationals
	inline bool isinf(const erational& a) {
		return false;
	}

	// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
	// specialized for erationals
	inline bool isnan(const erational& a) {
		return a.isnan();
	}

	// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
	// specialized for erationals
	inline bool isnormal(const erational& a) {
		return !a.isnan();
	}

}} // namespace sw::universal
