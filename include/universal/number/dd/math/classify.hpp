#pragma once
// classify.hpp: classification functions for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
int fpclassify(const dd& a) {
	return (std::fpclassify(a.high()));
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg is a cfloative or negative infinity.
// specialized for double-double (dd)
inline bool isinf(const dd& a) {
	return (std::fpclassify(a.high()) == FP_INFINITE);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for double-double (dd)
inline bool isnan(const dd& a) {
	return (std::fpclassify(a.high()) == FP_NAN);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for double-double (dd)
inline bool isfinite(const dd& a) {
	return (std::fpclassify(a.high()) != FP_INFINITE) && (std::fpclassify(a.high()) != FP_NAN);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for double-double (dd)
inline bool isnormal(const dd& a) {
	return (std::fpclassify(a.high()) == FP_NORMAL);
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
// specialized for double-double (dd)
inline bool isdenorm(const dd& a) {
	return (std::fpclassify(a.high()) == FP_SUBNORMAL);
}

inline bool iszero(const dd& a) {
	return (std::fpclassify(a.high()) == FP_ZERO);
}

inline bool signbit(const dd& a) {
	auto signA = std::copysign(1.0, a.high());
	return signA < 0.0;
}
/*
inline dd copysign(const dd& a, const dd& b)
{
	auto signA = std::copysign(1.0, a.high());
	auto signB = std::copysign(1.0, b.high());

	return signA != signB ? -a : a;
}
*/
}} // namespace sw::universal
