#pragma once
// carry.hpp: carry-detection intrinsics for uint64_t limb arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// When using uint64_t as the block type for multi-limb arithmetic, there is no
// larger native type to cast into for carry detection. These helper functions
// provide platform-specific carry propagation using compiler intrinsics or
// unsigned __int128 where available, with a portable fallback.
#include <cstdint>

// Platform detection for intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace sw { namespace universal {

/// add two uint64_t limbs with carry-in, producing a sum and carry-out
inline uint64_t addcarry(uint64_t a, uint64_t b, uint64_t carry_in, uint64_t& carry_out) {
#if defined(_MSC_VER)
	// MSVC: use _addcarry_u64 intrinsic
	unsigned char c;
	uint64_t sum;
	c = _addcarry_u64(static_cast<unsigned char>(carry_in), a, b, reinterpret_cast<unsigned long long*>(&sum));
	carry_out = c;
	return sum;

#elif defined(__SIZEOF_INT128__)
	// GCC/Clang on 64-bit: use unsigned __int128 for widening add
	unsigned __int128 wide = static_cast<unsigned __int128>(a) + b + carry_in;
	carry_out = static_cast<uint64_t>(wide >> 64);
	return static_cast<uint64_t>(wide);

#else
	// Portable fallback: detect carry using comparison
	uint64_t sum = a + b;
	uint64_t carry1 = (sum < a) ? 1u : 0u;
	uint64_t result = sum + carry_in;
	uint64_t carry2 = (result < sum) ? 1u : 0u;
	carry_out = carry1 + carry2;
	return result;
#endif
}

/// subtract two uint64_t limbs with borrow-in, producing a difference and borrow-out
inline uint64_t subborrow(uint64_t a, uint64_t b, uint64_t borrow_in, uint64_t& borrow_out) {
#if defined(_MSC_VER)
	// MSVC: use _subborrow_u64 intrinsic
	unsigned char borrow;
	uint64_t diff;
	borrow = _subborrow_u64(static_cast<unsigned char>(borrow_in), a, b, reinterpret_cast<unsigned long long*>(&diff));
	borrow_out = borrow;
	return diff;

#elif defined(__SIZEOF_INT128__)
	// GCC/Clang on 64-bit: use unsigned __int128
	unsigned __int128 wide_a = static_cast<unsigned __int128>(a);
	unsigned __int128 wide_sub = static_cast<unsigned __int128>(b) + borrow_in;
	borrow_out = (wide_a < wide_sub) ? 1u : 0u;
	return static_cast<uint64_t>(wide_a - wide_sub);

#else
	// Portable fallback
	uint64_t diff = a - b;
	uint64_t borrow1 = (a < b) ? 1u : 0u;
	uint64_t result = diff - borrow_in;
	uint64_t borrow2 = (diff < borrow_in) ? 1u : 0u;
	borrow_out = borrow1 + borrow2;
	return result;
#endif
}

/// multiply two uint64_t values producing a 128-bit result as lo and hi halves
inline void mul128(uint64_t a, uint64_t b, uint64_t& lo, uint64_t& hi) {
#if defined(_MSC_VER)
	// MSVC: use _umul128 intrinsic
	lo = _umul128(a, b, reinterpret_cast<unsigned long long*>(&hi));

#elif defined(__SIZEOF_INT128__)
	// GCC/Clang on 64-bit: use unsigned __int128
	unsigned __int128 product = static_cast<unsigned __int128>(a) * b;
	lo = static_cast<uint64_t>(product);
	hi = static_cast<uint64_t>(product >> 64);

#else
	// Portable fallback: split into 32-bit halves
	uint64_t a_lo = a & 0xFFFFFFFFu;
	uint64_t a_hi = a >> 32;
	uint64_t b_lo = b & 0xFFFFFFFFu;
	uint64_t b_hi = b >> 32;

	uint64_t p0 = a_lo * b_lo;
	uint64_t p1 = a_lo * b_hi;
	uint64_t p2 = a_hi * b_lo;
	uint64_t p3 = a_hi * b_hi;

	uint64_t mid = p1 + (p0 >> 32);
	uint64_t mid_carry = 0;
	uint64_t mid2 = mid + p2;
	if (mid2 < mid) mid_carry = 1;

	lo = (mid2 << 32) | (p0 & 0xFFFFFFFFu);
	hi = p3 + (mid2 >> 32) + (mid_carry << 32);
#endif
}

}} // namespace sw::universal
