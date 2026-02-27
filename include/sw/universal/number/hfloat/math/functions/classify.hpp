#pragma once
// classify.hpp: classification functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the hfloat number system has NO NaN and NO infinity encodings

// Categorizes floating point value arg into the following categories: zero, subnormal, normal, or implementation-defined category.
// Since hfloat has no NaN or Inf, fpclassify can only return FP_ZERO or FP_NORMAL.
template<unsigned ndigits, unsigned es, typename bt>
int fpclassify(const hfloat<ndigits, es, bt>& a) {
	return (a.iszero() ? FP_ZERO : FP_NORMAL);
}

// Determines if the given floating point number arg has finite value.
// hfloat has no infinity or NaN, so all values are finite.
template<unsigned ndigits, unsigned es, typename bt>
inline bool isfinite(const hfloat<ndigits, es, bt>&) {
	return true;
}

// Determines if the given floating point number arg is a positive or negative infinity.
// hfloat has no infinity encoding, so this always returns false.
template<unsigned ndigits, unsigned es, typename bt>
inline bool isinf(const hfloat<ndigits, es, bt>&) {
	return false;
}

// Determines if the given floating point number arg is a not-a-number (NaN) value.
// hfloat has no NaN encoding, so this always returns false.
template<unsigned ndigits, unsigned es, typename bt>
inline bool isnan(const hfloat<ndigits, es, bt>&) {
	return false;
}

// Determines if the given floating point number arg is the zero value.
template<unsigned ndigits, unsigned es, typename bt>
inline bool iszero(const hfloat<ndigits, es, bt>& a) {
	return a.iszero();
}

}} // namespace sw::universal
