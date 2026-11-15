#pragma once
// math_classify.hpp: classification functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
int fpclassify(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	// classified on the encoding: going through double made cfloat<128,15>'s minpos FP_ZERO and
	// its maxpos FP_INFINITE (#2088).  Without subnormals, a zero exponent field is a zero.
	if (a.isnan()) return FP_NAN;
	if (a.isinf()) return FP_INFINITE;
	if (a.iszero()) return FP_ZERO;
	if (a.isnormal() || a.ismaxexpvalue()) return FP_NORMAL;   // max-exponent values are normals of the layout
	return FP_SUBNORMAL;
}
	
// Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline bool isfinite(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	return !a.isinf() && !a.isnan();
}

// Determines if the given floating point number arg is a cfloative or negative infinity.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline bool isinf(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	return a.isinf();
}

// Determines if the given floating point number arg is a not-a-number (NaN) value.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline bool isnan(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	return a.isnan();
}

// Determines if the given floating point number arg is the zero value.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline bool iszero(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	return a.iszero();
}

// Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline bool isnormal(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	return a.isnormal();
}

// Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline bool isdenorm(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	return a.isdenormal();
}

}} // namespace sw::universal
