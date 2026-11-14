#pragma once
// math_classify.hpp: classification functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// A posit is zero, NaR, or normal: it has no infinity and no subnormals. Each function decides on
// the encoding; fpclassify and isnormal went through long double, which made posit<1024,5> maxpos
// FP_INFINITE and minpos FP_ZERO (#2071). NaR is the Posit Standard's single exception value: it
// is a NaN to isnan and fpclassify, and never an infinity (#2071, user decision: NaR propagates).

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<unsigned nbits, unsigned es, typename bt>
int fpclassify(const posit<nbits,es,bt>& p) {
	if (p.iszero()) return FP_ZERO;
	if (p.isnar())  return FP_NAN;
	return FP_NORMAL;
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for posits
template<unsigned nbits, unsigned es, typename bt>
inline bool isfinite(const posit<nbits,es,bt>& p) {
	return !p.isnar();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a positive or negative infinity.
// specialized for posits
template<unsigned nbits, unsigned es, typename bt>
inline bool isinf(const posit<nbits, es, bt>&) {
	return false;   // NaR is not an infinity (#2071); it was reported as both
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for posits
template<unsigned nbits, unsigned es, typename bt>
inline bool isnan(const posit<nbits, es, bt>& p) {
	return p.isnar();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for posits
template<unsigned nbits, unsigned es, typename bt>
inline bool isnormal(const posit<nbits, es, bt>& p) {
	return !p.iszero() && !p.isnar();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for posits
template<unsigned nbits, unsigned es, typename bt>
inline bool isdenorm(const posit<nbits, es, bt>&) {
	return false;   // posits are never denormalized
}

}} // namespace sw::universal
