#pragma once
// math_classify.hpp: classification functions for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posito standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<unsigned nbits, unsigned es>
int fpclassify(const posito<nbits,es>& p) {
	return std::fpclassify((long double)(p));
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for positos
template<unsigned nbits, unsigned es>
inline bool isfinite(const posito<nbits,es>& p) {
	return !p.isnar();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a positoive or negative infinity.
// specialized for positos
template<unsigned nbits, unsigned es>
inline bool isinf(const posito<nbits, es>& p) {
	return p.isnar();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for positos
template<unsigned nbits, unsigned es>
inline bool isnan(const posito<nbits, es>& p) {
	return p.isnar();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for positos
template<unsigned nbits, unsigned es>
inline bool isnormal(const posito<nbits, es>& p) {
	return std::isnormal((long double)(p));
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for positos
template<unsigned nbits, unsigned es>
inline bool isdenorm(const posito<nbits, es>& p) {
	return (p.isnar() ? false : false); // positos are never denormalized
}

}} // namespace sw::universal
