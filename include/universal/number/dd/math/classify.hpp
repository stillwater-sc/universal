#pragma once
// classify.hpp: classification functions for doubledouble (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
int fpclassify(const dd& a) {
	return std::fpclassify(a.high());
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg is a cfloative or negative infinity.
// specialized for doubledouble (dd)
inline bool isinf(const dd& a) {
	return a.isinf();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for doubledouble (dd)
inline bool isnan(const dd& a) {
	return a.isnan();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for doubledouble (dd)
inline bool isfinite(const dd& a) {
	return !a.isinf() && !a.isnan();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for doubledouble (dd)
inline bool isnormal(const dd& a) {
	return true;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
// specialized for doubledouble (dd)
inline bool isdenorm(const dd& a) {
	return false;
}

}} // namespace sw::universal
