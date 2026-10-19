#pragma once
// limb_division.hpp: long division of multi-limb unsigned integers, a 32-bit word at a time
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Knuth's Algorithm D (TAOCP Vol. 2, 4.3.1), in the form of Hacker's Delight's divmnu: each
// 32-bit quotient digit is estimated from the top 64 bits of the running remainder and the top
// 32 bits of the divisor, corrected at most twice, and confirmed by one multiply-subtract of
// the divisor. A division therefore costs O(m n) word operations for an m-limb dividend and an
// n-limb divisor, where the shift-and-subtract loops it replaces cost a full-width compare,
// subtract and shift for every quotient BIT (#1742).
//
// The kernel is constexpr and allocation-free: the caller provides the scratch space, so a
// fixed-size number system can call it with arrays sized at compile time.
#include <cstddef>
#include <cstdint>

namespace sw { namespace universal { namespace limb {

// count of leading zero bits of a nonzero 32-bit word
inline constexpr unsigned leading_zeros(std::uint32_t x) noexcept {
	unsigned n = 0;
	if (x <= 0x0000'FFFFu) { n += 16; x <<= 16; }
	if (x <= 0x00FF'FFFFu) { n += 8;  x <<= 8; }
	if (x <= 0x0FFF'FFFFu) { n += 4;  x <<= 4; }
	if (x <= 0x3FFF'FFFFu) { n += 2;  x <<= 2; }
	if (x <= 0x7FFF'FFFFu) { n += 1; }
	return n;
}

// q = floor(u / v), with u and v unsigned integers of m and n 32-bit limbs, least significant
// limb first, and r = u - q v when r is not null. Returns whether the remainder is nonzero.
//
//   q   : m - n + 1 limbs when m >= n (written in full); untouched when m < n, where q = 0
//   r   : n limbs, or null when only the remainder's being nonzero is wanted
//   un  : scratch, m + 1 limbs
//   vn  : scratch, n limbs
//
// Preconditions: n >= 1 and v[n-1] != 0 (strip the divisor's leading zero limbs first).
inline constexpr bool divide(const std::uint32_t* u, std::size_t m, const std::uint32_t* v, std::size_t n,
                             std::uint32_t* q, std::uint32_t* r, std::uint32_t* un, std::uint32_t* vn) noexcept {
	constexpr std::uint64_t base = std::uint64_t(1) << 32;
	if (m < n) {
		// the quotient is 0 and the remainder is u itself
		bool nonzero = false;
		for (std::size_t i = 0; i < n; ++i) {
			const std::uint32_t limb = (i < m) ? u[i] : 0u;
			if (r != nullptr) r[i] = limb;
			nonzero = nonzero || (limb != 0);
		}
		return nonzero;
	}

	if (n == 1) {
		// a single-limb divisor: one 64-by-32 division per limb
		std::uint64_t rem = 0;
		for (std::size_t j = m; j-- > 0;) {
			const std::uint64_t cur = (rem << 32) | u[j];
			q[j] = static_cast<std::uint32_t>(cur / v[0]);
			rem = cur % v[0];
		}
		if (r != nullptr) r[0] = static_cast<std::uint32_t>(rem);
		return rem != 0;
	}

	// normalize: shift so the divisor's top bit is set, which bounds the digit estimate's error
	const unsigned s = leading_zeros(v[n - 1]);
	for (std::size_t i = n - 1; i > 0; --i) {
		vn[i] = static_cast<std::uint32_t>((std::uint64_t(v[i]) << s) | (s == 0 ? 0u : (std::uint64_t(v[i - 1]) >> (32u - s))));
	}
	vn[0] = static_cast<std::uint32_t>(std::uint64_t(v[0]) << s);
	un[m] = (s == 0) ? 0u : static_cast<std::uint32_t>(std::uint64_t(u[m - 1]) >> (32u - s));
	for (std::size_t i = m - 1; i > 0; --i) {
		un[i] = static_cast<std::uint32_t>((std::uint64_t(u[i]) << s) | (s == 0 ? 0u : (std::uint64_t(u[i - 1]) >> (32u - s))));
	}
	un[0] = static_cast<std::uint32_t>(std::uint64_t(u[0]) << s);

	for (std::size_t j = m - n + 1; j-- > 0;) {
		// estimate the digit from the top two limbs of the remainder
		const std::uint64_t top = (std::uint64_t(un[j + n]) << 32) | un[j + n - 1];
		std::uint64_t qhat = top / vn[n - 1];
		std::uint64_t rhat = top % vn[n - 1];
		while (qhat >= base || qhat * vn[n - 2] > ((rhat << 32) | un[j + n - 2])) {
			--qhat;
			rhat += vn[n - 1];
			if (rhat >= base) break;
		}
		// un[j .. j+n] -= qhat * vn
		std::uint64_t carry = 0;
		std::int64_t borrow = 0;
		for (std::size_t i = 0; i < n; ++i) {
			const std::uint64_t p = qhat * vn[i] + carry;
			carry = p >> 32;
			const std::int64_t t = std::int64_t(un[i + j]) - borrow - std::int64_t(p & 0xFFFF'FFFFull);
			un[i + j] = static_cast<std::uint32_t>(t);
			borrow = (t < 0) ? 1 : 0;
		}
		const std::int64_t t = std::int64_t(un[j + n]) - borrow - std::int64_t(carry);
		un[j + n] = static_cast<std::uint32_t>(t);
		if (t < 0) {
			// the estimate was one too large: add the divisor back
			--qhat;
			std::uint64_t c = 0;
			for (std::size_t i = 0; i < n; ++i) {
				const std::uint64_t sum = std::uint64_t(un[i + j]) + vn[i] + c;
				un[i + j] = static_cast<std::uint32_t>(sum);
				c = sum >> 32;
			}
			un[j + n] = static_cast<std::uint32_t>(std::uint64_t(un[j + n]) + c);
		}
		q[j] = static_cast<std::uint32_t>(qhat);
	}
	// the remainder is un[0 .. n), shifted left by s: shift it back
	if (r != nullptr) {
		for (std::size_t i = 0; i + 1 < n; ++i) {
			r[i] = static_cast<std::uint32_t>((std::uint64_t(un[i]) >> s) | (s == 0 ? 0u : (std::uint64_t(un[i + 1]) << (32u - s))));
		}
		r[n - 1] = static_cast<std::uint32_t>(std::uint64_t(un[n - 1]) >> s);
	}
	for (std::size_t i = 0; i < n; ++i) if (un[i] != 0) return true;
	return false;
}

// the same, when only whether the remainder is nonzero is wanted
inline constexpr bool divide(const std::uint32_t* u, std::size_t m, const std::uint32_t* v, std::size_t n,
                             std::uint32_t* q, std::uint32_t* un, std::uint32_t* vn) noexcept {
	return divide(u, m, v, n, q, nullptr, un, vn);
}

// Gather the blocks of a multi-block number into 32-bit limbs, least significant first, and
// scatter limbs back into blocks. block(i) returns block i (BlockType of 8, 16, 32 or 64 bits);
// set(i, b) stores block i. Bits beyond nrLimbs * 32 are not read or written.
template<typename BlockType, typename Get>
constexpr void gather(Get&& block, unsigned nrBlocks, std::uint32_t* limbs, unsigned nrLimbs) noexcept {
	constexpr unsigned bitsInBlock = sizeof(BlockType) * 8u;
	for (unsigned i = 0; i < nrLimbs; ++i) limbs[i] = 0;
	for (unsigned i = 0; i < nrBlocks; ++i) {
		const std::uint64_t word = static_cast<std::uint64_t>(block(i));
		for (unsigned k = 0; k < bitsInBlock; k += 32u) {
			const unsigned bit = i * bitsInBlock + k;
			if (bit / 32u < nrLimbs) limbs[bit / 32u] |= static_cast<std::uint32_t>((word >> k) << (bit % 32u));
		}
	}
}
template<typename BlockType, typename Set>
constexpr void scatter(const std::uint32_t* limbs, unsigned nrLimbs, Set&& set, unsigned nrBlocks) noexcept {
	constexpr unsigned bitsInBlock = sizeof(BlockType) * 8u;
	for (unsigned i = 0; i < nrBlocks; ++i) {
		std::uint64_t word = 0;
		for (unsigned k = 0; k < bitsInBlock; k += 32u) {
			const unsigned bit = i * bitsInBlock + k;
			if (bit / 32u < nrLimbs) word |= std::uint64_t(limbs[bit / 32u] >> (bit % 32u)) << k;
		}
		set(i, static_cast<BlockType>(word));
	}
}

}}}  // namespace sw::universal::limb
