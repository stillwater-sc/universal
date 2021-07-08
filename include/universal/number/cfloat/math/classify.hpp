#pragma once
// math_classify.hpp: classification functions for classic floating-point cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<size_t nbits, size_t es, typename BlockType>
int fpclassify(const cfloat<nbits, es, BlockType>& a) {
	return std::fpclassify((long double)(a));
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for cfloats
template<size_t nbits, size_t es, typename BlockType>
inline bool isfinite(const cfloat<nbits, es, BlockType>& a) {
	return !a.isinf() && !a.isnan();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a cfloative or negative infinity.
// specialized for cfloats
template<size_t nbits, size_t es, typename BlockType>
inline bool isinf(const cfloat<nbits, es, BlockType>& a) {
	return a.isinf();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for cfloats
template<size_t nbits, size_t es, typename BlockType>
inline bool isnan(const cfloat<nbits, es, BlockType>& a) {
	return a.isnan();
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for cfloats
template<size_t nbits, size_t es, typename BlockType>
inline bool isnormal(const cfloat<nbits, es, BlockType>& a) {
	return std::isnormal((long double)(a));
}

}  // namespace sw::universal
