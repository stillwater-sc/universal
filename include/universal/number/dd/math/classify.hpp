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

	switch (::_fpclass(a.high())) {
	case _FPCLASS_SNAN:
	case _FPCLASS_QNAN:
		return FP_NAN;

	case _FPCLASS_NINF:
	case _FPCLASS_PINF:
		return FP_INFINITE;

	case _FPCLASS_ND:
	case _FPCLASS_PD:
		return FP_SUBNORMAL;

	case _FPCLASS_NZ:
	case _FPCLASS_PZ:
		return FP_ZERO;

	case _FPCLASS_NN:
	case _FPCLASS_PN:
	default:
		return FP_NORMAL;
	}
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg is a cfloative or negative infinity.
// specialized for doubledouble (dd)
inline bool isinf(const dd& a) {
	return (::_fpclass(a.high()) & (_FPCLASS_NINF | _FPCLASS_PINF)) != 0;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for doubledouble (dd)
inline bool isnan(const dd& a) {
	return (::_fpclass(a.high()) & (_FPCLASS_SNAN | _FPCLASS_QNAN)) != 0;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for doubledouble (dd)
inline bool isfinite(const dd& a) {
	return (::_fpclass(a.high()) & (_FPCLASS_SNAN | _FPCLASS_QNAN | _FPCLASS_NINF | _FPCLASS_PINF)) == 0;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for doubledouble (dd)
inline bool isnormal(const dd& a) {
	return (::_fpclass(a.high()) & (_FPCLASS_NN | _FPCLASS_PN)) != 0;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
// specialized for doubledouble (dd)
inline bool isdenorm(const dd& a) {
	bool bIsDenormal = false;
	switch (std::fpclassify(a.high())) {
	case FP_SUBNORMAL:
		bIsDenormal = true;
	case FP_NAN:
	case FP_INFINITE:
	case FP_ZERO:
	case FP_NORMAL:
	default:
		break;
	}
	return bIsDenormal;
}

bool signbit(dd const& a) {
	auto signA = std::copysign(1.0, a.high());
	return signA < 0.0;
}

}} // namespace sw::universal
