#pragma once
// classify.hpp: classification functions for fixpnts
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Categorizes fixed-point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
// subnormal, infinite, and NaN are not applicable to fixed-point numbers, so we map it to normal
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int fpclassify(const fixpnt<nbits, rbits, arithmetic, BlockType>& a) {
	return std::fpclassify((double)(a));
}
	
// Determines if the given fixed-point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline constexpr bool isfinite(const fixpnt<nbits, rbits, arithmetic, BlockType>&) {
	return true;
}

// Determines if the given fixed-point number arg is a positive or negative infinity.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline constexpr bool isinf(const fixpnt<nbits, rbits, arithmetic, BlockType>&) {
	return false;
}

// Determines if the given fixed-point number arg is a not-a-number (NaN) value.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline constexpr bool isnan(const fixpnt<nbits, rbits, arithmetic, BlockType>&) {
	return false;
}

// Determines if the given fixed-point number arg is the 0 value.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline constexpr bool iszero(const fixpnt<nbits, rbits, arithmetic, BlockType>& a) {
	return a.iszero();
}

// Determines if the given fixed-point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline constexpr bool isnormal(const fixpnt<nbits, rbits, arithmetic, BlockType>&) {
	return true;
}

// Determines if the given fixed-point number arg is subnormal
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
inline constexpr bool isdenorm(const fixpnt<nbits, rbits, arithmetic, BlockType>&) {
	return false;
}

}} // namespace sw::universal
