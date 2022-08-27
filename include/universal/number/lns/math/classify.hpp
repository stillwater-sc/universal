#pragma once
// classify.hpp: classification functions for logarithmic floating point 
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
int fpclassify(const lns<nbits, rbits, bt, xtra...>& a) {
	if constexpr (nbits < 33) {
		return std::fpclassify(float(a));
	}
	else if constexpr (nbits < 65) {
		return std::fpclassify(double(a));
	}
	else {
#if LONG_DOUBLE_SUPPORT
	return std::fpclassify((long double)(a));
#else
	return std::fpclassify(double(a));
#endif
	}
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
inline bool isfinite(const lns<nbits, rbits, bt, xtra...>& a) {
	return !a.isinf() && !a.isnan();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a posative or negative infinity.
// specialized for lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
inline bool isinf(const lns<nbits, rbits, bt, xtra...>& a) {
	return a.isinf();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
inline bool isnan(const lns<nbits, rbits, bt, xtra...>& a) {
	return a.isnan();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
inline bool isnormal(const lns<nbits, rbits, bt, xtra...>& a) {
	return std::isnormal(double(a));
}

#ifdef NOW
// STD LIB function for IEEE floats: Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
// specialized for lns
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
inline bool isdenorm(const lns<nbits, rbits, bt, xtra...>& a) {
	return std::isdenormal(double(a));
}
#endif // NOW

}} // namespace sw::universal
