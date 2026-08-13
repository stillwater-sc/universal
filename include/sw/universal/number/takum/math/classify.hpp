// classify.hpp: classification functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// Categorize takum into: zero, normal, infinite, NaN, or implementation-defined.
// Round-trip through long double; takum has no subnormal or infinity encoding.
template<unsigned nbits, unsigned rbits, typename bt>
int fpclassify(const takum<nbits, rbits, bt>& t) {
	return std::fpclassify((long double)(t));
}

// Takum is finite for every value except NaR (Not-a-Real).
template<unsigned nbits, unsigned rbits, typename bt>
inline bool isfinite(const takum<nbits, rbits, bt>& t) {
	return !t.isnar();
}

// Takum has no infinity encoding; the only exceptional value is NaR.
template<unsigned nbits, unsigned rbits, typename bt>
inline bool isinf(const takum<nbits, rbits, bt>& t) {
	(void)t;
	return false;
}

// NaR (Not-a-Real) is the takum analogue of NaN.
template<unsigned nbits, unsigned rbits, typename bt>
inline bool isnan(const takum<nbits, rbits, bt>& t) {
	return t.isnar();
}

// Every non-NaR, non-zero takum is normal; the format has no subnormal encodings.
template<unsigned nbits, unsigned rbits, typename bt>
inline bool isnormal(const takum<nbits, rbits, bt>& t) {
	return !t.isnar() && !t.iszero();
}

// Takum has no subnormal encodings.
template<unsigned nbits, unsigned rbits, typename bt>
inline bool isdenorm(const takum<nbits, rbits, bt>& t) {
	(void)t;
	return false;
}

// ---------------------------------------------------------------------------
// takum_log: classification is a property of the shared encoding, so these
// mirror the linear takum exactly.  The value map does not enter into it.
// ---------------------------------------------------------------------------

template<unsigned nbits, unsigned rbits, typename bt>
int fpclassify(const takum_log<nbits, rbits, bt>& t) {
	if (t.isnar())  return FP_NAN;
	if (t.iszero()) return FP_ZERO;
	return FP_NORMAL;   // no subnormal or infinite encodings; see below
}

template<unsigned nbits, unsigned rbits, typename bt>
inline bool isfinite(const takum_log<nbits, rbits, bt>& t) {
	return !t.isnar();
}

template<unsigned nbits, unsigned rbits, typename bt>
inline bool isinf(const takum_log<nbits, rbits, bt>& t) {
	(void)t;
	return false;
}

template<unsigned nbits, unsigned rbits, typename bt>
inline bool isnan(const takum_log<nbits, rbits, bt>& t) {
	return t.isnar();
}

template<unsigned nbits, unsigned rbits, typename bt>
inline bool isnormal(const takum_log<nbits, rbits, bt>& t) {
	return !t.isnar() && !t.iszero();
}

template<unsigned nbits, unsigned rbits, typename bt>
inline bool isdenorm(const takum_log<nbits, rbits, bt>& t) {
	(void)t;
	return false;
}

}} // namespace sw::universal
