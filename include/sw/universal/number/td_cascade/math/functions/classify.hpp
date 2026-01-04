#pragma once
// classify.hpp: classification functions for triple-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
inline int fpclassify(const td_cascade& a) {
	return (std::fpclassify(a.high()));
}
	
// Determines if the given floating point number arg is a cfloative or negative infinity.
// specialized for triple-double (dd)
inline bool isinf(const td_cascade& a) {
	return (std::fpclassify(a.high()) == FP_INFINITE);
}

// Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for triple-double (dd)
inline bool isnan(const td_cascade& a) {
	return (std::fpclassify(a.high()) == FP_NAN);
}

// Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for triple-double (dd)
inline bool isfinite(const td_cascade& a) {
	return (std::fpclassify(a.high()) != FP_INFINITE) && (std::fpclassify(a.high()) != FP_NAN);
}

// Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for triple-double (dd)
inline bool isnormal(const td_cascade& a) {
	return (std::fpclassify(a.high()) == FP_NORMAL);
}
//inline bool isnormal(td_cascade const& a) {
//	return !iszero(a) && !isinf(a) && !isnan(a);
//}

// Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
// specialized for triple-double (dd)
inline bool isdenorm(const td_cascade& a) {
	return (std::fpclassify(a.high()) == FP_SUBNORMAL);
}
//inline bool isdenorm(td_cascade const& a) {
//	// td_cascade doesn't have a special denorm representation
//	// but we can check if it's smaller than the normal minimum
//	return std::abs(a.high()) < std::numeric_limits<double>::min() && !iszero(a);
//}

inline bool iszero(const td_cascade& a) {
	return (a.high() == 0.0 && a.low() == 0.0);
}

/*
inline td_cascade copysign(const dd& a, const dd& b)
{
	auto signA = std::copysign(1.0, a.high());
	auto signB = std::copysign(1.0, b.high());

	return signA != signB ? -a : a;
}
*/
}} // namespace sw::universal
