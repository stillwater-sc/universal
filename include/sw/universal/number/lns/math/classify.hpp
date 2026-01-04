#pragma once
// classify.hpp: classification functions for logarithmic floating point 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
int fpclassify(const lns<nbits, rbits, bt, xtra...>& a) {
	return std::fpclassify(double(a));
}
	
// Determines if the given logarithmic number number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline bool isfinite(const lns<nbits, rbits, bt, xtra...>& a) {
	return !a.isinf() && !a.isnan();
}

// Determines if the given logarithmic number number arg is a posative or negative infinity.
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline bool isinf(const lns<nbits, rbits, bt, xtra...>& a) {
	return a.isinf();
}

// Determines if the given logarithmic number number arg is a not-a-number (NaN) value.
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline bool isnan(const lns<nbits, rbits, bt, xtra...>& a) {
	return a.isnan();
}

// Determines if the given logarithmic number number arg is a not-a-number (NaN) value.
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline bool iszero(const lns<nbits, rbits, bt, xtra...>& a) {
	return a.iszero();
}

// Determines if the given logarithmic number number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline bool isnormal(const lns<nbits, rbits, bt, xtra...>& a) {
	return std::isnormal(double(a));
}

#ifdef NOW
// Determines if the given logarithmic number number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline bool isdenorm(const lns<nbits, rbits, bt, xtra...>& a) {
	return std::isdenormal(double(a));
}
#endif // NOW

}} // namespace sw::universal
