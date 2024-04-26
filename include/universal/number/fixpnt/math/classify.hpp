#pragma once
// math_classify.hpp: classification functions for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int fpclassify(const fixpnt<nbits, rbits, arithmetic, BlockType>& a) {
	return std::fpclassify((long double)(a));
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for fixpnts
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline bool isfinite(const fixpnt<nbits, rbits, arithmetic, BlockType>& a) {
	return true;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a fixpntive or negative infinity.
// specialized for fixpnts
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline bool isinf(const fixpnt<nbits, rbits, arithmetic, BlockType>& a) {
	return false;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for fixpnts
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline bool isnan(const fixpnt<nbits, rbits, arithmetic, BlockType>& a) {
	return false;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for fixpnts
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline bool isnormal(const fixpnt<nbits, rbits, arithmetic, BlockType>& a) {
	return true;
}

}} // namespace sw::universal
