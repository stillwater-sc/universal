#pragma once
// classify.hpp: classification functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
int fpclassify(const qd& a) {
	return (std::fpclassify(a[0]));
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg is a cfloative or negative infinity.
// specialized for quad-double (qd)
inline bool isinf(const qd& a) {
	return (std::fpclassify(a[0]) == FP_INFINITE);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for quad-double (qd)
inline bool isnan(const qd& a) {
	return (std::fpclassify(a[0]) == FP_NAN);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for quad-double (qd)
inline bool isfinite(const qd& a) {
	return (std::fpclassify(a[0]) != FP_INFINITE) && (std::fpclassify(a[0]) != FP_NAN);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for quad-double (qd)
inline bool isnormal(const qd& a) {
	return (std::fpclassify(a[0]) == FP_NORMAL);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
// specialized for quad-double (qd)
inline bool isdenorm(const qd& a) {
	return (std::fpclassify(a[0]) == FP_SUBNORMAL);
}

inline bool iszero(const qd& a) {
	return (std::fpclassify(a[0]) == FP_ZERO);
}

bool signbit(qd const& a) {
	auto signA = std::copysign(1.0, a[0]);
	return signA < 0.0;
}

}} // namespace sw::universal
