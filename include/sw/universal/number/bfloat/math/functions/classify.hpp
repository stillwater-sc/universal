#pragma once
// classify.hpp: classification functions for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
inline int fpclassify(bfloat16 x) {
	return std::fpclassify(float(x));
}
	
// Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
inline bool isfinite(bfloat16 x) {
	return !x.isinf() && !x.isnan();
}

// Determines if the given floating point number arg is a cfloative or negative infinity.
inline bool isinf(bfloat16 x) {
	return x.isinf();
}

// Determines if the given floating point number arg is a not-a-number (NaN) value.
inline bool isnan(bfloat16 x) {
	return x.isnan();
}

// Determines if the given floating point number arg is normal, i.e. is zero, or not subnormal, infinite, or NaN.
inline bool isnormal(bfloat16 x) {
	int fpClass = fpclassify(float(x));
	return (fpClass == FP_NORMAL) || (fpClass == FP_ZERO);
}

// Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
inline bool isdenorm(bfloat16 x) {
	return (fpclassify(float(x)) == FP_SUBNORMAL);
}

inline bool isinteger(bfloat16 x) {
	return x.isinteger();
}

}} // namespace sw::universal
