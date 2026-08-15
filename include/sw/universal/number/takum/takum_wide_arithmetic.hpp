#pragma once
// takum_wide_arithmetic.hpp: exact integer evaluation path for the linear takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// WHY THIS EXISTS
//
// takum<>'s arithmetic operators evaluated in a double.  That is a single rounding
// -- and therefore correct -- only while the operands themselves are exactly
// representable as doubles, which stops being true at the wide end of the format:
// a takum<nbits, rbits> significand is 1 + p bits with p reaching
// nbits - 2 - rbits, so takum<64,3> carries 60 bits against a double's 53 and both
// operands were quantized BEFORE the operation.  Adding zero was not a no-op
// there.  Issue #1300.
//
// WHAT IT DOES INSTEAD
//
// Everything a linear takum denotes is a dyadic rational:
//
//     |value| = (1 + M/2^p) * 2^c = S * 2^e     with  S = 2^p + M,  e = c - p
//
// so S is an integer below 2^62 and the arithmetic is integer arithmetic.  The
// four operations produce an exact (or exactly-tracked) 128-bit result:
//
//     multiply   the 124-bit integer product of the two significands, exact
//     divide     128 quotient bits developed by restoring division, plus a
//                sticky remainder
//     add / sub  both terms aligned into a 128-bit window, with a sticky bit for
//                whatever falls below it
//
// and all three then take the SAME tail: normalize, round to odd at qbits, and
// hand the result to takum_codec::encode_fraction(), which performs the one
// rounding the target layout actually calls for.
//
// WHY ROUND TO ODD
//
// The tail narrows twice -- 128 bits to qbits here, qbits to p in the codec -- and
// two roundings to nearest disagree with one whenever the exact value sits near a
// tie at the p-bit boundary.  That is not a corner case; the same double rounding
// left 1.8% of cross-conversions off by an ulp before takum_cross_conversion.hpp
// adopted round-to-odd.  Truncating and forcing the low bit instead makes the
// subsequent round-to-nearest-even agree with rounding the exact value directly,
// provided qbits >= p + 2 (see qbits below).
//
// WHY NOT A DOUBLE-DOUBLE
//
// #1300 first proposed evaluating in dd_cascade.  Its ~106 bits restore + - /, but
// the exact product of two 60-bit significands is 120 bits, and while an exact TIE
// is safe there (a tie needs exactly p+1 significant bits, which dd holds exactly)
// a NEAR-tie is not: the exact product can sit 2^-120 from a tie point, below
// dd_cascade's own 2^-104 error.  Integers have no such hole, cost no dependency
// in the core header, and stay usable in a constant-evaluated context.  This is
// the same argument that made takum_log's multiply and divide exact in #1312.
//
// NOT A GENERAL-PURPOSE 128-BIT LIBRARY
//
// u128 below carries exactly the operations this path needs, with the
// preconditions this path guarantees.  It is not a substitute for
// internal/uint128.hpp or for blockbinary<>.

#include <cassert>
#include <cstdint>

namespace sw { namespace universal { namespace takum_wide {

// ---------------------------------------------------------------------------
// u128: a 128-bit unsigned integer, hi:lo
// ---------------------------------------------------------------------------

struct u128 {
	uint64_t hi;
	uint64_t lo;
};

constexpr u128 make_u128(uint64_t v) noexcept { return u128{ 0ull, v }; }

constexpr bool iszero(const u128& v) noexcept { return (v.hi | v.lo) == 0ull; }

constexpr bool less(const u128& a, const u128& b) noexcept {
	return (a.hi != b.hi) ? (a.hi < b.hi) : (a.lo < b.lo);
}

// Pre: the sum fits 128 bits.  Every caller reserves a bit for the carry.
constexpr u128 add(const u128& a, const u128& b) noexcept {
	u128 r{ a.hi + b.hi, a.lo + b.lo };
	if (r.lo < a.lo) ++r.hi;
	return r;
}

// Pre: a >= b.
constexpr u128 sub(const u128& a, const u128& b) noexcept {
	u128 r{ a.hi - b.hi, a.lo - b.lo };
	if (a.lo < b.lo) --r.hi;
	return r;
}

// Pre: s < 128, and no significant bit is shifted out of the 128-bit value.
//
// The low word's own overflow is intended and is not that: `v.lo << s` drops the
// bits the `v.hi` term simultaneously carries up, which is what makes this a
// 128-bit shift rather than two 64-bit ones.  Unsigned overflow is defined, so
// this is clean under -fsanitize=undefined; -fsanitize=integer reports it, since
// that group flags defined wraparound too.
constexpr u128 shift_left(const u128& v, unsigned s) noexcept {
	if (s == 0u) return v;
	if (s >= 64u) return u128{ v.lo << (s - 64u), 0ull };
	return u128{ (v.hi << s) | (v.lo >> (64u - s)), v.lo << s };
}

// Pre: s < 128.  Bits shifted out are lost; ask any_low_bits() first if they matter.
constexpr u128 shift_right(const u128& v, unsigned s) noexcept {
	if (s == 0u) return v;
	if (s >= 64u) return u128{ 0ull, v.hi >> (s - 64u) };
	return u128{ v.hi >> s, (v.lo >> s) | (v.hi << (64u - s)) };
}

// Is any of the low s bits set?  This is the sticky bit of a right shift by s.
constexpr bool any_low_bits(const u128& v, unsigned s) noexcept {
	if (s == 0u) return false;
	if (s >= 128u) return !iszero(v);
	if (s >= 64u) {
		if (v.lo != 0ull) return true;
		const unsigned t = s - 64u;
		return (t == 0u) ? false : ((v.hi & ((1ull << t) - 1ull)) != 0ull);
	}
	return (v.lo & ((1ull << s) - 1ull)) != 0ull;
}

constexpr bool bit_at(const u128& v, unsigned i) noexcept {
	return (i >= 64u) ? (((v.hi >> (i - 64u)) & 1ull) != 0ull)
	                  : (((v.lo >> i) & 1ull) != 0ull);
}

// Index of the highest set bit.  Pre: v != 0.
constexpr unsigned msb_index(const u128& v) noexcept {
	uint64_t w    = (v.hi != 0ull) ? v.hi : v.lo;
	unsigned base = (v.hi != 0ull) ? 64u : 0u;
	unsigned k    = 0;
	while (w > 1ull) { w >>= 1; ++k; }
	return base + k;
}

// The exact 128-bit product of two 64-bit values, via 32-bit halves.  No
// __int128 and no intrinsic, so this is portable and constant-evaluable.
constexpr u128 mul64(uint64_t a, uint64_t b) noexcept {
	const uint64_t a0 = a & 0xFFFFFFFFull, a1 = a >> 32;
	const uint64_t b0 = b & 0xFFFFFFFFull, b1 = b >> 32;
	const uint64_t p00 = a0 * b0;
	const uint64_t p01 = a0 * b1;
	const uint64_t p10 = a1 * b0;
	const uint64_t p11 = a1 * b1;
	const uint64_t mid = (p00 >> 32) + (p01 & 0xFFFFFFFFull) + (p10 & 0xFFFFFFFFull);
	const uint64_t lo  = (p00 & 0xFFFFFFFFull) | (mid << 32);
	const uint64_t hi  = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
	return u128{ hi, lo };
}

// ---------------------------------------------------------------------------
// Values
// ---------------------------------------------------------------------------

// A decoded takum operand:  value = (-1)^sign * S * 2^e,
// with S = 2^p + M in [2^p, 2^(p+1)) -- the significand with its implicit bit
// restored -- and e = c - p.  S < 2^62 for every supported configuration.
struct operand {
	uint64_t S;
	int64_t  e;
	bool     sign;
};

// A partially evaluated result:  value = (-1)^sign * (V + theta) * 2^e,
// with theta in [0,1) and theta > 0 exactly when inexact is set.
//
// Carrying the residue as a flag rather than a value is all round-to-odd needs:
// it asks only whether anything was discarded, never how much.
struct wide_value {
	u128     V;
	int64_t  e;
	bool     sign;
	bool     inexact;
};

template<typename Codec>
constexpr operand decode_operand(bool sign, uint64_t magnitude) noexcept {
	const auto d = Codec::decode(magnitude);
	return operand{ (1ull << d.p) | d.M_bits,
	                d.c - static_cast<int64_t>(d.p),
	                sign };
}

constexpr wide_value widen(const operand& x) noexcept {
	return wide_value{ make_u128(x.S), x.e, x.sign, false };
}

// ---------------------------------------------------------------------------
// The three producers
// ---------------------------------------------------------------------------

// Exact: the integer product of the significands, at most 124 bits.
constexpr wide_value multiply(const operand& a, const operand& b) noexcept {
	return wide_value{ mul64(a.S, b.S), a.e + b.e, a.sign != b.sign, false };
}

// S_a / S_b developed to 128 quotient bits with a sticky remainder.
// Pre: b.S != 0 (the caller has already turned x/0 into NaR).
constexpr wide_value divide(const operand& a, const operand& b) noexcept {
	// (S_a << 64) / S_b by restoring division.  The running remainder stays below
	// the divisor, which is under 2^62, so doubling it cannot overflow a uint64_t.
	// The quotient is at most (2^62 << 64) / 1 = 2^126 and cannot overflow either.
	const u128 num = shift_left(make_u128(a.S), 64u);
	u128 q{ 0ull, 0ull };
	uint64_t rem = 0ull;
	for (int i = 127; i >= 0; --i) {
		rem = (rem << 1) | (bit_at(num, static_cast<unsigned>(i)) ? 1ull : 0ull);
		q = shift_left(q, 1u);
		if (rem >= b.S) { rem -= b.S; q.lo |= 1ull; }
	}
	return wide_value{ q, a.e - b.e - 64, a.sign != b.sign, rem != 0ull };
}

// Align two exact terms into a 128-bit window and combine them.  Handles addition,
// subtraction (subtract == true negates b), and the accumulate step of fma, whose
// first term is a 124-bit product rather than a bare significand.
//
// Pre: neither V is zero, and both terms are exact.  An inexact input would break
// the "at most one term is truncated" argument the subtraction path rests on.
constexpr wide_value sum(const wide_value& a, const wide_value& b0, bool subtract) noexcept {
	assert(!iszero(a.V) && !iszero(b0.V));
	assert(!a.inexact && !b0.inexact);

	wide_value b = b0;
	if (subtract) b.sign = !b.sign;

	// Anchor the window on whichever term has the higher leading bit and let it
	// land on bit 126, leaving bit 127 free so a magnitude sum cannot carry out.
	const unsigned ka   = msb_index(a.V);
	const unsigned kb   = msb_index(b.V);
	const int64_t  topA = a.e + static_cast<int64_t>(ka);
	const int64_t  topB = b.e + static_cast<int64_t>(kb);
	const int64_t  top  = (topA >= topB) ? topA : topB;
	const int64_t  base = top - 126;

	// Bring both terms onto the common base.  A term shifts left by
	// (its top) - base, which is at most 126 and therefore loses nothing; only a
	// term whose top is strictly below `top` can shift right.  So the truncated
	// term, when there is one, is strictly the smaller of the two in magnitude:
	// its leading bit sits below the other's, and both are positioned exactly.
	auto place = [base](const wide_value& t, bool& lost) -> u128 {
		const int64_t s = t.e - base;
		if (s >= 0) { lost = false; return shift_left(t.V, static_cast<unsigned>(s)); }
		if (-s >= 128) { lost = true; return u128{ 0ull, 0ull }; }
		const unsigned r = static_cast<unsigned>(-s);
		lost = any_low_bits(t.V, r);
		return shift_right(t.V, r);
	};
	bool lostA = false, lostB = false;
	const u128 A = place(a, lostA);
	const u128 B = place(b, lostB);

	if (a.sign == b.sign) {
		return wide_value{ add(A, B), base, a.sign, lostA || lostB };
	}

	// Magnitudes subtract.  Order them first: equal tops mean both were placed
	// exactly and a direct comparison decides, including the exactly-cancelling
	// case; unequal tops decide it outright.
	bool aLarger = true;
	if (topA == topB) {
		if (less(A, B))      aLarger = false;
		else if (less(B, A)) aLarger = true;
		else return wide_value{ u128{ 0ull, 0ull }, base, false, false };  // exact cancellation
	}
	else {
		aLarger = (topA > topB);
	}

	const u128 L       = aLarger ? A : B;
	const u128 S       = aLarger ? B : A;
	const bool inexact = aLarger ? lostB : lostA;   // only the smaller term can be truncated

	// The subtrahend's discarded residue theta lies in (0,1), so the exact
	// difference is (L - S - 1) + (1 - theta): one less than the integer
	// difference, and still inexact.
	u128 V = sub(L, S);
	if (inexact) {
		// L - S - 1 cannot underflow.  L's leading bit is at 126 and the truncated
		// term carries at most 124 significant bits below bit 125, so
		// S <= 2^126 - 4 while L >= 2^126.
		assert(!iszero(V));
		V = sub(V, make_u128(1ull));
	}
	return wide_value{ V, base, aLarger ? a.sign : b.sign, inexact };
}

// ---------------------------------------------------------------------------
// The shared tail
// ---------------------------------------------------------------------------

// Fraction width handed to encode_fraction().  Two bits wider than the widest
// trailing field any supported layout produces -- p reaches maxCharBits, which is
// nbits - 2 - rbits and therefore 61 at nbits = 64, rbits = 1 -- which is exactly
// the headroom round-to-odd needs to be equivalent to a single rounding.
inline constexpr unsigned qbits = 63;

// Normalize, round to odd at qbits, and let the codec perform the one rounding
// the layout calls for.  Pre: r.V != 0 (zero has no (c, m) and is the caller's
// business, since the codec owns no storage).
template<typename Codec>
constexpr typename Codec::encoded encode(const wide_value& r) noexcept {
	assert(!iszero(r.V));

	const unsigned k = msb_index(r.V);
	const int64_t  c = r.e + static_cast<int64_t>(k);

	// The bits below the leading one, as a qbits numerator.
	uint64_t N    = 0ull;
	bool     lost = r.inexact;
	if (k >= qbits) {
		const unsigned drop = k - qbits;
		lost = lost || any_low_bits(r.V, drop);
		N = shift_right(r.V, drop).lo & ((1ull << qbits) - 1ull);  // strip the leading one
	}
	else {
		// k < qbits < 64, so the whole value sits in the low word
		N = (r.V.lo & ((1ull << k) - 1ull)) << (qbits - k);
	}
	if (lost) N |= 1ull;   // round to odd

	return Codec::encode_fraction(c, N, qbits);
}

}}} // namespace sw::universal::takum_wide
