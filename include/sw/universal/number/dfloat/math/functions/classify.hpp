#pragma once
// classify.hpp: classification functions for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories:
// zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
int fpclassify(const dfloat<ndigits, es, Encoding, bt>& a) {
	return std::fpclassify(double(a));
}

// Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero,
// but not infinite or NaN.
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline bool isfinite(const dfloat<ndigits, es, Encoding, bt>& a) {
	return !a.isinf() && !a.isnan();
}

// Determines if the given floating point number arg is a positive or negative infinity.
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline bool isinf(const dfloat<ndigits, es, Encoding, bt>& a) {
	return a.isinf();
}

// Determines if the given floating point number arg is a not-a-number (NaN) value.
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline bool isnan(const dfloat<ndigits, es, Encoding, bt>& a) {
	return a.isnan();
}

// Determines if the given floating point number arg is the zero value.
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline bool iszero(const dfloat<ndigits, es, Encoding, bt>& a) {
	return a.iszero();
}

}} // namespace sw::universal
