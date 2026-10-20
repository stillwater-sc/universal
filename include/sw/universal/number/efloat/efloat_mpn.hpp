#pragma once
// efloat_mpn.hpp: products of natural numbers on 64-bit limbs, the multiplication kernel of efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// efloat stores its significand as 32-bit limbs, most significant first.  Its multiply packs both
// operands into 64-bit limbs, least significant first, and multiplies those here: a 64 x 64 -> 128
// bit partial product does the work of four 32 x 32 -> 64 bit ones, and above a threshold
// Karatsuba replaces the schoolbook product (#1762).  The names follow GMP's mpn layer: every
// operand is a pointer and a limb count, least significant limb first, and nothing allocates;
// the caller provides the scratch space.
#include <cstddef>
#include <cstdint>
#include <universal/internal/blocktype/carry.hpp>   // mul128, uint128_t

namespace sw { namespace universal { namespace efloat_mpn {

using limb_t = std::uint64_t;

// operands of at least this many limbs multiply by Karatsuba, below it by schoolbook
inline constexpr std::size_t karatsuba_threshold = 24;
inline constexpr std::size_t karatsuba_sqr_threshold = 32;

// r[0, n) = a[0, n) * b, returns the high limb
inline limb_t mul_1(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
#if defined(__SIZEOF_INT128__)
		const uint128_t t = static_cast<uint128_t>(a[i]) * b + carry;
		r[i]  = static_cast<limb_t>(t);
		carry = static_cast<limb_t>(t >> 64);
#else
		limb_t lo, hi;
		mul128(a[i], b, lo, hi);
		lo += carry;
		hi += (lo < carry) ? 1u : 0u;
		r[i]  = lo;
		carry = hi;
#endif
	}
	return carry;
}

// r[0, n) += a[0, n) * b, returns the carry out of the top: a * b + r + carry < 2^128
inline limb_t addmul_1(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
#if defined(__SIZEOF_INT128__)
		const uint128_t t = static_cast<uint128_t>(a[i]) * b + r[i] + carry;
		r[i]  = static_cast<limb_t>(t);
		carry = static_cast<limb_t>(t >> 64);
#else
		limb_t lo, hi;
		mul128(a[i], b, lo, hi);
		lo += carry;
		hi += (lo < carry) ? 1u : 0u;
		lo += r[i];
		hi += (lo < r[i]) ? 1u : 0u;
		r[i]  = lo;
		carry = hi;
#endif
	}
	return carry;
}

// r[0, n) = a[0, n) + b[0, n), returns the carry; r may be a or b
inline limb_t add_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
		const limb_t s = a[i] + carry;
		const limb_t c = (s < carry) ? 1u : 0u;
		r[i]  = s + b[i];
		carry = c + ((r[i] < s) ? 1u : 0u);
	}
	return carry;
}

// r[0, n) = a[0, n) - b[0, n), returns the borrow; r may be a or b
inline limb_t sub_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	limb_t borrow = 0;
	for (std::size_t i = 0; i < n; ++i) {
		const limb_t bi = b[i] + borrow;
		const limb_t c  = (bi < borrow) ? 1u : 0u;
		const limb_t ai = a[i];
		r[i]   = ai - bi;
		borrow = c + ((ai < bi) ? 1u : 0u);
	}
	return borrow;
}

// r[0, n) += c, returns the carry out of the top
inline limb_t add_1(limb_t* r, std::size_t n, limb_t c) noexcept {
	for (std::size_t i = 0; i < n && c != 0; ++i) {
		r[i] += c;
		c = (r[i] < c) ? 1u : 0u;
	}
	return c;
}

// r[0, m + n) = a[0, m) * b[0, n), n >= 1, schoolbook; r overlaps neither operand
inline void mul_basecase(limb_t* r, const limb_t* a, std::size_t m, const limb_t* b, std::size_t n) noexcept {
	r[m] = mul_1(r, a, m, b[0]);
	for (std::size_t j = 1; j < n; ++j) r[m + j] = addmul_1(r + j, a, m, b[j]);
}

// r[0, 2n) = a[0, n)^2: each cross product a_i a_j (i < j) once, doubled, plus the squares a_i^2
inline void sqr_basecase(limb_t* r, const limb_t* a, std::size_t n) noexcept {
	for (std::size_t i = 0; i < 2 * n; ++i) r[i] = 0;
	for (std::size_t i = 0; i + 1 < n; ++i) r[i + n] = addmul_1(r + 2 * i + 1, a + i + 1, n - i - 1, a[i]);
	// double the cross products: their sum is below a^2 / 2, so no bit leaves the top
	limb_t top = 0;
	for (std::size_t k = 0; k < 2 * n; ++k) {
		const limb_t next = r[k] >> 63;
		r[k] = (r[k] << 1) | top;
		top  = next;
	}
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
		limb_t lo, hi;
#if defined(__SIZEOF_INT128__)
		const uint128_t sq = static_cast<uint128_t>(a[i]) * a[i];
		lo = static_cast<limb_t>(sq);
		hi = static_cast<limb_t>(sq >> 64);
#else
		mul128(a[i], a[i], lo, hi);
#endif
		limb_t s = r[2 * i] + carry;
		limb_t c = (s < carry) ? 1u : 0u;
		r[2 * i] = s + lo;
		c += (r[2 * i] < s) ? 1u : 0u;
		s = r[2 * i + 1] + c;
		c = (s < c) ? 1u : 0u;
		r[2 * i + 1] = s + hi;
		carry = c + ((r[2 * i + 1] < s) ? 1u : 0u);
	}
}

// d[0, k) = |x[0, k) - y[0, h)| for h <= k, returns whether x < y
inline bool abs_diff(limb_t* d, const limb_t* x, std::size_t k, const limb_t* y, std::size_t h) noexcept {
	bool x_smaller = false;
	bool x_high    = false;   // y is zero above h: x is the larger if any of x[h, k) is nonzero
	for (std::size_t j = h; j < k; ++j) x_high = x_high || (x[j] != 0);
	if (!x_high) {
		for (std::size_t j = h; j-- > 0;) {
			if (x[j] != y[j]) { x_smaller = (x[j] < y[j]); break; }
		}
	}
	if (x_smaller) {
		sub_n(d, y, x, h);   // x[h, k) is zero, so the difference has h limbs
		for (std::size_t j = h; j < k; ++j) d[j] = 0;
	}
	else {
		limb_t borrow = sub_n(d, x, y, h);
		for (std::size_t j = h; j < k; ++j) {
			const limb_t xj = x[j];
			d[j]   = xj - borrow;
			borrow = (xj < borrow) ? 1u : 0u;
		}
	}
	return x_smaller;
}

// the scratch limbs mul and mul_n need when neither operand is longer than n limbs: mul_n takes
// below 6n + 64, and an unbalanced mul adds a block product per level of its remainder sequence
constexpr std::size_t scratch_limbs(std::size_t n) noexcept { return 16 * n + 128; }

// r[0, 2n) = a[0, n) * b[0, n), or a^2 when square; s holds scratch_limbs(n) limbs.
// Subtractive Karatsuba: with a = a1 B^k + a0 and b likewise, the middle coefficient is
// z0 + z2 - (a0 - a1)(b0 - b1), where the differences have k limbs and no carry.
inline void mul_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t* s, bool square) noexcept {
	if (square ? (n < karatsuba_sqr_threshold) : (n < karatsuba_threshold)) {
		if (square) sqr_basecase(r, a, n); else mul_basecase(r, a, n, b, n);
		return;
	}
	const std::size_t k = (n + 1) / 2;   // the low halves have k limbs, the high halves h <= k
	const std::size_t h = n - k;
	limb_t* da = s;
	limb_t* db = s + k;
	limb_t* t  = s + 2 * k;
	limb_t* ss = s + 4 * k;
	const bool na = abs_diff(da, a, k, a + k, h);
	const bool nb = square ? na : abs_diff(db, b, k, b + k, h);
	mul_n(t, da, square ? da : db, k, ss, square);           // t = |a0 - a1| |b0 - b1|, 2k limbs
	mul_n(r, a, b, k, ss, square);                           // z0 = a0 b0 in r[0, 2k)
	mul_n(r + 2 * k, a + k, b + k, h, ss, square);           // z2 = a1 b1 in r[2k, 2n)
	// m = z0 + z2 -/+ t, 2k + 1 limbs, in ss (the recursions are done with it)
	limb_t* m = ss;
	limb_t carry = add_n(m, r, r + 2 * k, 2 * h);
	for (std::size_t j = 2 * h; j < 2 * k; ++j) {
		m[j]  = r[j] + carry;
		carry = (m[j] < carry) ? 1u : 0u;
	}
	m[2 * k] = carry;
	if (na == nb) m[2 * k] -= sub_n(m, m, t, 2 * k);    // (a0 - a1)(b0 - b1) >= 0
	else          m[2 * k] += add_n(m, m, t, 2 * k);
	// r[k, 2n) += m: the product fits in 2n limbs, so whatever m holds past r's end is zero
	const std::size_t room = 2 * n - k;
	const std::size_t len  = (2 * k + 1 < room) ? 2 * k + 1 : room;
	carry = add_n(r + k, r + k, m, len);
	add_1(r + k + len, room - len, carry);
}

// r[0, m + n) = a[0, m) * b[0, n) for any m, n >= 1; r overlaps neither operand, s holds
// scratch_limbs(max(m, n)) limbs.  An unbalanced product multiplies the longer operand a block of
// the shorter's length at a time.
inline void mul(limb_t* r, const limb_t* a, std::size_t m, const limb_t* b, std::size_t n, limb_t* s) noexcept {
	if (m < n) { const limb_t* tp = a; a = b; b = tp; const std::size_t tn = m; m = n; n = tn; }
	if (n < karatsuba_threshold) { mul_basecase(r, a, m, b, n); return; }
	if (m == n) { mul_n(r, a, b, n, s, false); return; }
	mul_n(r, a, b, n, s, false);   // r[0, 2n)
	limb_t* t = s;                 // a block product, n + c limbs
	for (std::size_t off = n; off < m; off += n) {
		const std::size_t c = (m - off < n) ? (m - off) : n;
		if (c == n) mul_n(t, a + off, b, n, s + 2 * n, false);
		else        mul(t, b, n, a + off, c, s + 2 * n);
		// r[off, off + n) holds the top of the previous blocks; r[off + n, off + n + c) is new
		limb_t carry = add_n(r + off, r + off, t, n);
		for (std::size_t i = 0; i < c; ++i) {
			r[off + n + i] = t[n + i] + carry;
			carry = (r[off + n + i] < carry) ? 1u : 0u;
		}
	}
}

}}}  // namespace sw::universal::efloat_mpn
