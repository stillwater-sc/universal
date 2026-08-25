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

// Suppress -Wpedantic warnings for __int128 (a compiler extension supported
// by both GCC and Clang on 64-bit targets, but not part of ISO C++)
#if defined(__SIZEOF_INT128__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
using uint128_t = unsigned __int128;
#pragma GCC diagnostic pop
#endif

/// add two uint64_t limbs with carry-in, producing a sum and carry-out
///
/// CONTRACT: carry_in is a FULL 64-bit addend, not a single carry bit. It computes
/// a + b + carry_in as a 128-bit value, returning the low limb and setting carry_out
/// to the high limb (0, 1 or 2). Multi-limb multiply relies on this: it feeds back
/// the high half of a 64x64 partial product, which is an arbitrary 64-bit number.
inline uint64_t addcarry(uint64_t a, uint64_t b, uint64_t carry_in, uint64_t& carry_out) {
#if defined(_MSC_VER)
	// _addcarry_u64's carry-in is a SINGLE BIT. Passing carry_in through it (the old
	// static_cast<unsigned char>) silently discarded every bit but the lowest, so
	// integer<N,uint64_t> and blockbinary multiply produced wrong products on MSVC
	// while gcc/clang -- whose __int128 branch honours the whole value -- were correct.
	// Add carry_in as a second operand instead of as the carry bit.
	// Use distinct local variables to avoid optimizer issues with reference-derived pointers.
	unsigned long long s1, s2;
	unsigned char c1 = _addcarry_u64(0, a, b, &s1);
	unsigned char c2 = _addcarry_u64(0, s1, carry_in, &s2);
	carry_out = static_cast<uint64_t>(c1) + static_cast<uint64_t>(c2);
	return static_cast<uint64_t>(s2);

#elif defined(__SIZEOF_INT128__)
	// GCC/Clang on 64-bit: use unsigned __int128 for widening add
	uint128_t wide = static_cast<uint128_t>(a) + b + carry_in;
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
///
/// CONTRACT, mirroring addcarry: borrow_in is a FULL 64-bit subtrahend, not a single
/// borrow bit. Returns the low limb of a - b - borrow_in and sets borrow_out to the
/// number of borrows out (0, 1 or 2).
///
/// No caller passes a multi-bit borrow_in today, but the MSVC branch had the same
/// single-bit truncation that broke addcarry, and the __int128 branch capped
/// borrow_out at 1 where the portable branch reported 2 -- three implementations,
/// three different answers. They agree now.
inline uint64_t subborrow(uint64_t a, uint64_t b, uint64_t borrow_in, uint64_t& borrow_out) {
#if defined(_MSC_VER)
	// _subborrow_u64's borrow-in is a SINGLE BIT; subtract borrow_in as an operand.
	// Use distinct local variables to avoid optimizer issues with reference-derived pointers.
	unsigned long long d1, d2;
	unsigned char b1 = _subborrow_u64(0, a, b, &d1);
	unsigned char b2 = _subborrow_u64(0, d1, borrow_in, &d2);
	borrow_out = static_cast<uint64_t>(b1) + static_cast<uint64_t>(b2);
	return static_cast<uint64_t>(d2);

#else
	// Portable: two-step borrow detection. __int128 buys nothing for subtraction.
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
	// Use local variable for hi output to avoid optimizer issues with reference-derived pointers
	unsigned long long h;
	lo = _umul128(a, b, &h);
	hi = static_cast<uint64_t>(h);

#elif defined(__SIZEOF_INT128__)
	// GCC/Clang on 64-bit: use unsigned __int128
	uint128_t product = static_cast<uint128_t>(a) * b;
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
