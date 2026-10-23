#pragma once
// efloat_mpn.hpp: products, quotients, square and cube roots of natural numbers on 64-bit limbs, the kernel of efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// efloat stores its significand as 32-bit limbs, most significant first.  Its multiply packs both
// operands into 64-bit limbs, least significant first, and multiplies those here: a 64 x 64 -> 128
// bit partial product does the work of four 32 x 32 -> 64 bit ones, and above a threshold
// Karatsuba replaces the schoolbook product (#1762).  Its division divides the same way, a 64-bit
// quotient limb per step and divide-and-conquer above a threshold, and its square root is
// Zimmermann's, on the division and the squaring (#1763), and its cube root the same recursion
// carried over to the cube (#1800).  The names follow GMP's mpn layer:
// every operand is a pointer and a limb count, least significant limb first, and nothing
// allocates; the caller provides the scratch space.
#include <bit>
#include <cmath>
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


///////////////////////////////////////////////////////////////////////////////////////////////
// Division (#1763).  The divisor is normalized, its top bit set.  Each quotient limb comes from
// a precomputed reciprocal, as Moller and Granlund describe (Improved division by invariant
// integers, IEEE Trans. Computers, 2011): a 3-by-2 limb estimate that is never more than one too
// small, then one multiply-subtract of the divisor.  Above a threshold, the divide-and-conquer
// division of Burnikel and Ziegler (as GMP's dcpi1_div_qr) splits the quotient in halves and
// pays for each half with one product, so the division costs a few multiplications.

// operands whose quotient and divisor both have at least this many limbs divide by divide-and-conquer
inline constexpr std::size_t dc_div_threshold = 16;

// the high and low limbs of a * b
inline void umul(limb_t a, limb_t b, limb_t& hi, limb_t& lo) noexcept {
#if defined(__SIZEOF_INT128__)
	const uint128_t t = static_cast<uint128_t>(a) * b;
	lo = static_cast<limb_t>(t);
	hi = static_cast<limb_t>(t >> 64);
#else
	mul128(a, b, lo, hi);
#endif
}

// floor((u1 B + u0) / d) for a normalized d and u1 < d, the quotient fits one limb
inline limb_t udiv_2by1(limb_t u1, limb_t u0, limb_t d) noexcept {
#if defined(__SIZEOF_INT128__)
	return static_cast<limb_t>(((static_cast<uint128_t>(u1) << 64) | u0) / d);
#else
	// Knuth D on 32-bit digits (Hacker's Delight, divlu): two quotient digits
	constexpr limb_t b32 = limb_t(1) << 32;
	const limb_t dh = d >> 32, dl = d & 0xFFFF'FFFFu;
	const limb_t uh = u0 >> 32, ul = u0 & 0xFFFF'FFFFu;
	limb_t q1 = u1 / dh, r = u1 - q1 * dh;
	while (q1 >= b32 || q1 * dl > ((r << 32) | uh)) { --q1; r += dh; if (r >= b32) break; }
	const limb_t u21 = (u1 << 32) + uh - q1 * d;
	limb_t q0 = u21 / dh;
	r = u21 - q0 * dh;
	while (q0 >= b32 || q0 * dl > ((r << 32) | ul)) { --q0; r += dh; if (r >= b32) break; }
	return (q1 << 32) | q0;
#endif
}

// the reciprocal of a normalized d: floor((B^2 - 1) / d) - B
inline limb_t invert_limb(limb_t d) noexcept { return udiv_2by1(~d, ~limb_t(0), d); }

// the reciprocal of the normalized two-limb divisor d1 B + d0: floor((B^3 - 1) / (d1 B + d0)) - B
inline limb_t invert_pi1(limb_t d1, limb_t d0) noexcept {
	limb_t v = invert_limb(d1);
	limb_t p = d1 * v + d0;
	if (p < d0) {
		--v;
		const limb_t mask = (p >= d1) ? ~limb_t(0) : 0u;
		p -= d1;
		v += mask;
		p -= mask & d1;
	}
	limb_t t1, t0;
	umul(d0, v, t1, t0);
	p += t1;
	if (p < t1) {
		--v;
		if (p >= d1 && (p > d1 || t0 >= d0)) --v;
	}
	return v;
}

// q = floor((n1 B + n0) / d), r = the remainder, for n1 < d and v = invert_limb(d)
inline void udiv_qr_2by1(limb_t& q, limb_t& r, limb_t n1, limb_t n0, limb_t d, limb_t v) noexcept {
	limb_t qh, ql;
	umul(n1, v, qh, ql);
	ql += n0;
	qh += n1 + 1u + ((ql < n0) ? 1u : 0u);
	r = n0 - qh * d;
	if (r > ql) { --qh; r += d; }
	if (r >= d) { ++qh; r -= d; }
	q = qh;
}

// q = floor((n2 B^2 + n1 B + n0) / (d1 B + d0)) and the two-limb remainder r1 B + r0, for
// n2 B + n1 < d1 B + d0 and v = invert_pi1(d1, d0)
inline void udiv_qr_3by2(limb_t& q, limb_t& r1, limb_t& r0, limb_t n2, limb_t n1, limb_t n0, limb_t d1, limb_t d0, limb_t v) noexcept {
	limb_t q0;
	umul(n2, v, q, q0);
	q0 += n1;
	q  += n2 + ((q0 < n1) ? 1u : 0u);
	r1 = n1 - d1 * q;
	// (r1, r0) = (r1, n0) - (d1, d0) - d0 q
	limb_t borrow = (n0 < d0) ? 1u : 0u;
	r0 = n0 - d0;
	r1 = r1 - d1 - borrow;
	limb_t t1, t0;
	umul(d0, q, t1, t0);
	borrow = (r0 < t0) ? 1u : 0u;
	r0 -= t0;
	r1 = r1 - t1 - borrow;
	++q;
	const limb_t mask = (r1 >= q0) ? ~limb_t(0) : 0u;
	q += mask;
	const limb_t a0 = mask & d0;
	r0 += a0;
	r1 += (mask & d1) + ((r0 < a0) ? 1u : 0u);
	if (r1 >= d1 && (r1 > d1 || r0 >= d0)) {
		++q;
		borrow = (r0 < d0) ? 1u : 0u;
		r0 -= d0;
		r1 = r1 - d1 - borrow;
	}
}

// r[0, n) -= a[0, n) * b, returns the borrow out of the top
inline limb_t submul_1(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
#if defined(__SIZEOF_INT128__)
		const uint128_t t  = static_cast<uint128_t>(a[i]) * b + carry;
		const limb_t    lo = static_cast<limb_t>(t);
		const limb_t    hi = static_cast<limb_t>(t >> 64);
#else
		limb_t hi, lo;
		umul(a[i], b, hi, lo);
		lo += carry;
		hi += (lo < carry) ? 1u : 0u;
#endif
		const limb_t ri = r[i];
		r[i]  = ri - lo;
		carry = hi + ((ri < lo) ? 1u : 0u);
	}
	return carry;
}

// r[0, n) -= c, returns the borrow out of the top
inline limb_t sub_1(limb_t* r, std::size_t n, limb_t c) noexcept {
	for (std::size_t i = 0; i < n && c != 0; ++i) {
		const limb_t ri = r[i];
		r[i] = ri - c;
		c = (ri < c) ? 1u : 0u;
	}
	return c;
}

// whether a[0, n) >= b[0, n)
inline bool geq_n(const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	for (std::size_t i = n; i-- > 0;) {
		if (a[i] != b[i]) return a[i] > b[i];
	}
	return true;
}

// q[0, nn - 1) = the low limbs of floor(np[0, nn) / d), the remainder replaces np[0]; returns
// the quotient's top limb, 0 or 1.  d normalized, v = invert_limb(d).
inline limb_t divrem_1(limb_t* q, limb_t* np, std::size_t nn, limb_t d, limb_t v) noexcept {
	limb_t r  = np[nn - 1];
	limb_t qh = 0;
	if (r >= d) { r -= d; qh = 1; }
	for (std::size_t i = nn - 1; i-- > 0;) udiv_qr_2by1(q[i], r, r, np[i], d, v);
	np[0] = r;
	return qh;
}

// Schoolbook division: q[0, nn - dn) = the low limbs of floor(np[0, nn) / dp[0, dn)), the
// remainder replaces np[0, dn); returns the quotient's top limb, 0 or 1.  dn >= 2, dp normalized,
// v = invert_pi1 of its top two limbs.
inline limb_t sbpi1_div_qr(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn, limb_t v) noexcept {
	const std::size_t qn = nn - dn;
	limb_t* top = np + qn;
	limb_t  qh  = 0;
	if (geq_n(top, dp, dn)) { sub_n(top, top, dp, dn); qh = 1; }
	const limb_t d1 = dp[dn - 1], d0 = dp[dn - 2];
	limb_t n1 = np[nn - 1];
	for (std::size_t j = qn; j-- > 0;) {
		// divide np[j, j + dn] by dp; n1 is np[j + dn], already reduced below d
		limb_t* w = np + j;
		limb_t qj;
		if (n1 == d1 && w[dn - 1] == d0) {
			// the top two limbs equal the divisor's, so the quotient limb is B - 1: the borrow
			// out of w[0, dn) cancels n1
			qj = ~limb_t(0);
			submul_1(w, dp, dn, qj);
			n1 = w[dn - 1];
		}
		else {
			limb_t n0;
			udiv_qr_3by2(qj, n1, n0, n1, w[dn - 1], w[dn - 2], d1, d0, v);
			const limb_t cy  = submul_1(w, dp, dn - 2, qj);
			const limb_t cy1 = (n0 < cy) ? 1u : 0u;
			n0 -= cy;
			const limb_t cy2 = (n1 < cy1) ? 1u : 0u;
			n1 -= cy1;
			w[dn - 2] = n0;
			if (cy2 != 0u) {   // the estimate was one too large: add the divisor back
				n1 += d1 + add_n(w, w, dp, dn - 1);
				--qj;
			}
		}
		q[j] = qj;
	}
	np[dn - 1] = n1;
	return qh;
}

// the scratch limbs div_qr needs for a divisor of dn limbs
constexpr std::size_t div_scratch_limbs(std::size_t dn) noexcept { return dn + scratch_limbs(dn); }

// sbpi1_div_qr for np[0, 2n) and dp[0, n) by divide-and-conquer: the high half of the quotient
// from the top 3n/2 limbs and the high half of the divisor, corrected by the product of that
// half with the low half of the divisor, then the low half the same way.  tp holds
// div_scratch_limbs(n) limbs.
inline limb_t dcpi1_div_qr_n(limb_t* q, limb_t* np, const limb_t* dp, std::size_t n, limb_t v, limb_t* tp) noexcept {
	const std::size_t lo = n / 2;
	const std::size_t hi = n - lo;
	limb_t qh = (hi < dc_div_threshold) ? sbpi1_div_qr(q + lo, np + 2 * lo, 2 * hi, dp + lo, hi, v)
	                                    : dcpi1_div_qr_n(q + lo, np + 2 * lo, dp + lo, hi, v, tp);
	mul(tp, q + lo, hi, dp, lo, tp + n);
	limb_t cy = sub_n(np + lo, np + lo, tp, n);
	if (qh != 0u) cy += sub_n(np + n, np + n, dp, lo);
	while (cy != 0u) {   // the estimate was too large: at most twice
		qh -= sub_1(q + lo, hi, 1u);
		cy -= add_n(np + lo, np + lo, dp, n);
	}
	const limb_t ql = (lo < dc_div_threshold) ? sbpi1_div_qr(q, np + hi, 2 * lo, dp + hi, lo, v)
	                                          : dcpi1_div_qr_n(q, np + hi, dp + hi, lo, v, tp);
	mul(tp, dp, hi, q, lo, tp + n);
	cy = sub_n(np, np, tp, n);
	if (ql != 0u) cy += sub_n(np + lo, np + lo, dp, hi);
	while (cy != 0u) {
		sub_1(q, lo, 1u);
		cy -= add_n(np, np, dp, n);
	}
	return qh;
}

// A quotient shorter than the divisor, qn = nn - dn < dn: divide the top 2 qn limbs of np by the
// top qn limbs of dp, then subtract the quotient times the divisor's remaining limbs and correct.
inline limb_t dcpi1_div_qr_short(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn, limb_t v, limb_t* tp) noexcept {
	const std::size_t qn = nn - dn;
	const std::size_t in = dn - qn;
	limb_t qh = dcpi1_div_qr_n(q, np + in, dp + in, qn, v, tp);
	mul(tp, q, qn, dp, in, tp + dn);   // dn limbs
	limb_t cy = sub_n(np, np, tp, dn);
	if (qh != 0u) cy += sub_n(np + qn, np + qn, dp, in);
	while (cy != 0u) {
		qh -= sub_1(q, qn, 1u);
		cy -= add_n(np, np, dp, dn);
	}
	return qh;
}

// q[0, nn - dn) = the low limbs of floor(np[0, nn) / dp[0, dn)), the remainder replaces
// np[0, dn); returns the quotient's top limb, 0 or 1.  nn >= dn >= 1, dp normalized (its top bit
// set) and overlapping neither np nor q, s holds div_scratch_limbs(dn) limbs.
inline limb_t div_qr(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn, limb_t* s) noexcept {
	if (dn == 1) return divrem_1(q, np, nn, dp[0], invert_limb(dp[0]));
	const limb_t      v  = invert_pi1(dp[dn - 1], dp[dn - 2]);
	const std::size_t qn = nn - dn;
	if (dn < dc_div_threshold || qn < dc_div_threshold) return sbpi1_div_qr(q, np, nn, dp, dn, v);
	if (qn < dn) return dcpi1_div_qr_short(q, np, nn, dp, dn, v, s);
	// a long quotient: a first block of qn mod dn limbs (or dn), then blocks of dn limbs, each
	// dividing the 2 dn limbs below the remainder of the last
	const std::size_t first = (qn % dn == 0) ? dn : qn % dn;
	std::size_t off = qn - first;
	limb_t qh;
	if (first == dn)                   qh = dcpi1_div_qr_n(q + off, np + off, dp, dn, v, s);
	else if (first < dc_div_threshold) qh = sbpi1_div_qr(q + off, np + off, dn + first, dp, dn, v);
	else                               qh = dcpi1_div_qr_short(q + off, np + off, dn + first, dp, dn, v, s);
	while (off > 0) {
		off -= dn;
		dcpi1_div_qr_n(q + off, np + off, dp, dn, v, s);
	}
	return qh;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Square root with remainder (#1763): Zimmermann's Karatsuba square root (INRIA RR-3805, 1999),
// as GMP's mpn_sqrtrem.  With N = a3 b^3 + a2 b^2 + a1 b + a0, (s', r') = sqrtrem(a3 b + a2),
// (q, u) = divrem(r' b + a1, 2 s'), s = s' b + q and r = u b + a0 - q^2, then one correction
// when r < 0: the square root costs one division and one square of half its length per level,
// and both S = floor(sqrt(N)) and the remainder are exact.

// floor(sqrt(a)) for a >= 2^62, r = a - s^2: a double estimate, corrected
inline limb_t sqrtrem1(limb_t a, limb_t& r) noexcept {
	limb_t s = static_cast<limb_t>(std::sqrt(static_cast<double>(a)));
	if (s > 0xFFFF'FFFFu) s = 0xFFFF'FFFFu;   // the double rounded up to 2^32
	while (s * s > a) --s;
	while (s < 0xFFFF'FFFFu && (s + 1) * (s + 1) <= a) ++s;
	r = a - s * s;
	return s;
}

// sp[0] = floor(sqrt(np[0, 2))) for np[1] >= 2^62; the remainder, at most 2 sp[0], is rp[0]
// and the returned bit above it.  rp may be np.  The same step as dc_sqrtrem on 32-bit halves.
inline limb_t sqrtrem2(limb_t* sp, limb_t* rp, const limb_t* np) noexcept {
	const limb_t n0 = np[0];
	limb_t r1;
	const limb_t s1 = sqrtrem1(np[1], r1);   // s1 in [2^31, 2^32), r1 <= 2 s1
	limb_t k = 0;
	while (r1 >= s1) { r1 -= s1; ++k; }      // k s1 2^32 / (2 s1) = k 2^31 exactly
	const limb_t t = (r1 << 32) | (n0 >> 32);
	const limb_t d = 2 * s1;
	const limb_t q = t / d + (k << 31);      // at most 2^32
	const limb_t u = t % d;                  // below 2^33
	// r = u 2^32 + (n0 mod 2^32) - q^2 as a low limb and a signed top
	limb_t  rlo = (u << 32) | (n0 & 0xFFFF'FFFFu);
	int64_t rhi = static_cast<int64_t>(u >> 32);
	if (q == (limb_t(1) << 32)) {
		rhi -= 1;
	}
	else {
		const limb_t qq = q * q;
		rhi -= (rlo < qq) ? 1 : 0;
		rlo -= qq;
	}
	// s = s1 2^32 + q, which reaches 2^64 only when the correction below takes it back
	limb_t slo = (s1 << 32) + q;
	limb_t shi = (slo < (s1 << 32)) ? 1u : 0u;
	if (rhi < 0) {   // r += 2 s - 1, s -= 1, as r += s, --s, r += s
		rlo += slo;
		rhi += static_cast<int64_t>(shi + ((rlo < slo) ? 1u : 0u));
		shi -= (slo == 0u) ? 1u : 0u;
		--slo;
		rlo += slo;
		rhi += static_cast<int64_t>(shi + ((rlo < slo) ? 1u : 0u));
	}
	sp[0] = slo;
	rp[0] = rlo;
	return static_cast<limb_t>(rhi);
}

// the scratch limbs sqrtrem needs for a root of n limbs
constexpr std::size_t sqrt_scratch_limbs(std::size_t n) noexcept { return n + 2 + div_scratch_limbs(n); }

// sp[0, n) = floor(sqrt(np[0, 2n))) for n >= 2 and np[2n - 1] >= 2^62; the remainder replaces
// np[0, n), with the returned limb, 0 or 1, above it.  s holds sqrt_scratch_limbs(n) limbs.
inline limb_t dc_sqrtrem(limb_t* sp, limb_t* np, std::size_t n, limb_t* s) noexcept {
	const std::size_t l = n / 2;
	const std::size_t h = n - l;
	// (s', r') for the top 2h limbs: s' in sp[l, n), r' in np[2l, 2l + h) and q
	limb_t q = (h == 1) ? sqrtrem2(sp + l, np + 2 * l, np + 2 * l) : dc_sqrtrem(sp + l, np + 2 * l, h, s);
	if (q != 0u) sub_n(np + 2 * l, np + 2 * l, sp + l, h);   // r' - s' < s': divide by s', then halve
	limb_t* qp = s;   // l + 1 limbs
	qp[l] = div_qr(qp, np + l, n, sp + l, h, s + l + 1);
	q += qp[l];
	limb_t c = qp[0] & 1u;   // (r' b + a1) / s' = 2 q + c: the remainder by 2 s' gains c s'
	for (std::size_t i = 0; i + 1 < l; ++i) sp[i] = (qp[i] >> 1) | (qp[i + 1] << 63);
	sp[l - 1] = (qp[l - 1] >> 1) | (q << 63);
	q >>= 1;
	if (c != 0u) c = add_n(np + l, np + l, sp + l, h);
	mul_n(np + n, sp, sp, l, s, true);   // q^2 above the remainder, where a3 and a2 were
	const limb_t b = q + sub_n(np, np, np + n, 2 * l);
	int64_t cc = static_cast<int64_t>(c) - static_cast<int64_t>((l == h) ? b : sub_1(np + 2 * l, 1, b));
	if (cc < 0) {   // r < 0: r += 2 s - 1, s -= 1
		q = add_1(sp + l, h, q);
		cc += static_cast<int64_t>(addmul_1(np, sp, n, 2u) + 2 * q);
		cc -= static_cast<int64_t>(sub_1(np, n, 1u));
		sub_1(sp, n, 1u);
	}
	return static_cast<limb_t>(cc);
}

// sp[0, n) = floor(sqrt(np[0, 2n))) for np[2n - 1] >= 2^62; the remainder replaces np[0, n),
// with the returned limb, 0 or 1, above it.  s holds sqrt_scratch_limbs(n) limbs.
inline limb_t sqrtrem(limb_t* sp, limb_t* np, std::size_t n, limb_t* s) noexcept {
	return (n == 1) ? sqrtrem2(sp, np, np) : dc_sqrtrem(sp, np, n, s);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Cube root with remainder (#1800): the recursion of Zimmermann's square root (dc_sqrtrem above)
// carried over to the cube. With N = N' B^(3l) + A2 B^(2l) + A0, (S', R') = cbrtrem(N') and
// (Q, U) = divrem(R' B^l + A2, 3 S'^2), the root is S = S' B^l + Q and its remainder
//     R = N - S^3 = U B^(2l) + A0 - Q^2 (3 S' B^l + Q),
// since S^3 - S'^3 B^(3l) = 3 S'^2 Q B^(2l) + Q^2 (3 S' B^l + Q). Q drops the last two terms,
// so it is never too small and at most a few units too large (cbrt_overshoot: measured at most 2,
// and 0 when the root has an odd number of limbs, l < h); each unit is
// corrected by R += 3 S^2 - 3 S + 1, S -= 1, with 3 S^2 = 3 S'^2 B^(2l) + 6 S' Q B^l + 3 Q^2
// from the parts already formed. A level costs a square of h limbs, a division of an l-limb
// quotient, and a product of 2l by n limbs for the remainder. The top level can skip the
// remainder: cbrt rounds from S alone unless S lies within cbrt_overshoot of a rounding boundary.

// r[0, 3) = s^3
inline void cube_1(limb_t* r, limb_t s) noexcept {
	limb_t h, l, a1, a0, b1, b0;
	umul(s, s, h, l);
	umul(l, s, a1, a0);
	umul(h, s, b1, b0);
	r[0] = a0;
	r[1] = a1 + b0;
	r[2] = b1 + ((r[1] < b0) ? 1u : 0u);
}

// r[0, n) = a[0, n) << c, 0 < c < 64, returns the bits shifted out of the top; r may be a
inline limb_t lshift(limb_t* r, const limb_t* a, std::size_t n, unsigned c) noexcept {
	limb_t out = a[n - 1] >> (64u - c);
	for (std::size_t i = n - 1; i > 0; --i) r[i] = (a[i] << c) | (a[i - 1] >> (64u - c));
	r[0] = a[0] << c;
	return out;
}

// r[0, n) = a[0, n) >> c, 0 < c < 64; r may be a
inline void rshift(limb_t* r, const limb_t* a, std::size_t n, unsigned c) noexcept {
	for (std::size_t i = 0; i + 1 < n; ++i) r[i] = (a[i] >> c) | (a[i + 1] << (64u - c));
	r[n - 1] = a[n - 1] >> c;
}

// r[0, n) = the low n limbs of floor(a[0, na) 2^shift), for a shift of either sign; returns
// whether the bits a right shift drops below r are nonzero (#1802). r overlaps not a.
inline bool load_window(limb_t* r, std::size_t n, const limb_t* a, std::size_t na, std::int64_t shift) noexcept {
	const std::int64_t m = static_cast<std::int64_t>(na);
	auto at = [a, m](std::int64_t j) -> limb_t { return (j >= 0 && j < m) ? a[j] : 0u; };
	const std::int64_t q = (shift >= 0) ? -((shift + 63) / 64) : (-shift) / 64;   // floor(-shift / 64)
	const unsigned     c = static_cast<unsigned>(-shift - 64 * q);                  // in [0, 64)
	for (std::size_t k = 0; k < n; ++k) {
		const std::int64_t j = q + static_cast<std::int64_t>(k);   // r[k] holds a[j] >> c and a[j + 1] << (64 - c)
		r[k] = (c == 0u) ? at(j) : ((at(j) >> c) | (at(j + 1) << (64u - c)));
	}
	if (shift >= 0) return false;
	for (std::int64_t j = 0; j < q && j < m; ++j) {
		if (a[j] != 0u) return true;
	}
	return c != 0u && q < m && (a[q] & ((limb_t(1) << c) - 1u)) != 0u;
}

// sp[0] = floor(cbrt(np[0, 3))) for np[2] >= 2^61, rp[0, 3) = the remainder: a double estimate,
// within about 2^12 of the root, one Newton step in double, and an exact correction
inline void cbrtrem_1(limb_t* sp, limb_t* rp, const limb_t* np) noexcept {
	const double two64 = 18446744073709551616.0;
	const double c = std::cbrt(std::ldexp(static_cast<double>(np[2]), 128) + std::ldexp(static_cast<double>(np[1]), 64));
	limb_t s = (c >= two64) ? ~limb_t(0) : static_cast<limb_t>(c);
	limb_t t[3];
	cube_1(t, s);
	const bool below = !geq_n(np, t, 3);   // N < s^3
	limb_t d[3];
	if (below) sub_n(d, t, np, 3); else sub_n(d, np, t, 3);
	const double step = (std::ldexp(static_cast<double>(d[2]), 128) + std::ldexp(static_cast<double>(d[1]), 64) + static_cast<double>(d[0])) /
	                    (3.0 * static_cast<double>(s) * static_cast<double>(s));
	const limb_t k = static_cast<limb_t>(step);
	if (below) s -= k; else s = (~limb_t(0) - s < k) ? ~limb_t(0) : s + k;
	for (;;) {   // s^3 <= N
		cube_1(t, s);
		if (geq_n(np, t, 3)) break;
		--s;
	}
	while (s != ~limb_t(0)) {   // (s + 1)^3 > N
		limb_t u[3];
		cube_1(u, s + 1);
		if (!geq_n(np, u, 3)) break;
		++s;
		t[0] = u[0]; t[1] = u[1]; t[2] = u[2];
	}
	sp[0] = s;
	sub_n(rp, np, t, 3);
}

// the scratch limbs cbrtrem needs for a root of n limbs: a level's own 5n + 8, and below them
// the larger of the recursion's, the division's (17 n + 162) and the remainder's (25 n + 152)
constexpr std::size_t cbrt_scratch_limbs(std::size_t n) noexcept { return 64 * n + 512; }

// the most units the root without its remainder can lie above floor(cbrt(N)): Q's excess is
// below Q^2 / (S' B^l) + Q^3 / (3 S'^2 B^(2l)) + R / (3 S'^2 B^(2l)) < 2 + 1 + 1
inline constexpr limb_t cbrt_overshoot = 4;

// sp[0, n) = floor(cbrt(np[0, 3n))) for np[3n - 1] >= 2^61, and rp[0, 2n + 1) = N - S^3 when
// `exact`; without it, S is the root plus 0 to cbrt_overshoot units and rp is left alone.
// s holds cbrt_scratch_limbs(n) limbs.
inline void cbrtrem(limb_t* sp, limb_t* rp, const limb_t* np, std::size_t n, limb_t* s, bool exact = true) noexcept {
	if (n == 1) {   // exact either way
		limb_t r[3];
		cbrtrem_1(sp, exact ? rp : r, np);
		return;
	}
	const std::size_t l = n / 2;
	const std::size_t h = n - l;
	limb_t* Rh = s;                    // 2h + 1: R'
	limb_t* D  = Rh + 2 * h + 1;       // 2h + 1: 3 S'^2
	limb_t* X  = D + 2 * h + 1;        // l + 2h + 2: R' B^l + A2, then the remainder of the division
	limb_t* q  = X + l + 2 * h + 2;    // l + 2h + 2: the quotient
	limb_t* w  = q + l + 2 * h + 2;
	limb_t* const Sh = sp + l;         // S'
	cbrtrem(Sh, Rh, np + 3 * l, h, w, true);

	// D = 3 S'^2, normalized for the division by a shift of c bits
	mul_n(D, Sh, Sh, h, w, true);
	D[2 * h] = mul_1(D, D, 2 * h, 3u);
	const std::size_t dn = (D[2 * h] != 0u) ? 2 * h + 1 : 2 * h;
	const unsigned c = static_cast<unsigned>(std::countl_zero(D[dn - 1]));
	for (std::size_t i = 0; i < l; ++i) X[i] = np[2 * l + i];
	for (std::size_t i = 0; i < 2 * h + 1; ++i) X[l + i] = Rh[i];
	std::size_t xn = l + 2 * h + 1;
	X[xn] = 0;
	if (c != 0u) {
		lshift(D, D, dn, c);
		X[xn] = lshift(X, X, xn, c);
	}
	++xn;
	const limb_t qtop = div_qr(q, X, xn, D, dn, w);
	const std::size_t qn = xn - dn;
	q[qn] = qtop;
	if (c != 0u) {   // U and D back to scale
		rshift(X, X, dn, c);
		rshift(D, D, dn, c);
	}
	limb_t* U = X;   // dn limbs, and one more for the clamp below
	U[dn] = 0;
	// Q < B^l: the root's low part, since S < (S' + 1) B^l. A larger Q is reduced to B^l - 1,
	// and U gains D for each unit, so that U = X - D Q still holds.
	std::size_t qlen = qn + 1;
	while (qlen > l && q[qlen - 1] == 0u) --qlen;
	if (qlen > l) {   // Q = B^l + q[0] at most: k = q[0] + 1 units above B^l - 1
		U[dn] = addmul_1(U, D, dn, q[0] + 1u);
		for (std::size_t i = 0; i < l; ++i) q[i] = ~limb_t(0);
	}
	for (std::size_t i = 0; i < l; ++i) sp[i] = q[i];
	if (!exact) return;

	// R = U B^(2l) + A0 - Q^2 (3 S' B^l + Q), in 2n + 3 limbs, two's complement
	const std::size_t rn = 2 * n + 3;
	limb_t* R  = w;                    // rn
	limb_t* Q2 = R + rn;               // 2l: Q^2
	limb_t* T  = Q2 + 2 * l;           // n + 1: 3 S' B^l + Q
	limb_t* P  = T + n + 1;            // 2l + n + 1: Q^2 T
	limb_t* G  = P + 2 * l + n + 1;    // rn: 3 S^2 - 3 S + 1
	limb_t* SQ = G + rn;               // n: S' Q
	limb_t* ww = SQ + n;
	for (std::size_t i = 0; i < rn; ++i) R[i] = 0;
	for (std::size_t i = 0; i < 2 * l; ++i) R[i] = np[i];
	add_n(R + 2 * l, R + 2 * l, U, dn + 1);
	mul_n(Q2, sp, sp, l, ww, true);
	for (std::size_t i = 0; i < l; ++i) T[i] = sp[i];
	T[n] = mul_1(T + l, Sh, h, 3u);
	mul(P, Q2, 2 * l, T, n + 1, ww);
	const limb_t borrow = sub_n(R, R, P, 2 * l + n + 1);
	sub_1(R + 2 * l + n + 1, rn - (2 * l + n + 1), borrow);
	if ((R[rn - 1] >> 63) != 0u) {   // S too large: R += 3 S^2 - 3 S + 1, S -= 1, until R >= 0
		mul(SQ, Sh, h, sp, l, ww);   // S' Q
		do {
			for (std::size_t i = 0; i < rn; ++i) G[i] = 0;
			for (std::size_t i = 0; i < dn; ++i) G[2 * l + i] = D[i];
			add_1(G + l + n, rn - l - n, addmul_1(G + l, SQ, n, 6u));
			add_1(G + 2 * l, rn - 2 * l, addmul_1(G, Q2, 2 * l, 3u));
			sub_1(G + n, rn - n, submul_1(G, sp, n, 3u));
			add_1(G, rn, 1u);
			add_n(R, R, G, rn);
			// Q^2 -= 2 Q - 1, S' Q -= S', Q -= 1 (Q >= 1: S never falls below S' B^l)
			sub_1(Q2 + l, l, submul_1(Q2, sp, l, 2u));
			add_1(Q2, 2 * l, 1u);
			sub_1(SQ + h, n - h, sub_n(SQ, SQ, Sh, h));
			sub_1(sp, l, 1u);
		} while ((R[rn - 1] >> 63) != 0u);
	}
	for (std::size_t i = 0; i < 2 * n + 1; ++i) rp[i] = R[i];
}

}}}  // namespace sw::universal::efloat_mpn
