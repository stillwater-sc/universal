#pragma once
// reduce_pio2.hpp: exact argument reduction modulo pi/2 for the b-posit trigonometric functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sin, cos and tan of a b-posit x need x - k pi/2 to the full precision of the result, and
// x reaches 2^192 in the standard configurations.  Subtracting k times a 212-bit pi/2, as
// quad-double's own reduction does, leaves about 212 - log2|x| bits: some 20 at 2^192.
//
// Payne and Hanek's reduction computes x (2/pi) modulo 4 instead, from as many bits of 2/pi
// as the exponent of x needs.  x = m 2^e is an exact dyadic, so the bits of 2/pi that only
// produce multiples of 4 are skipped, and a 576-bit window below them gives the quadrant and
// a fraction carrying more than 400 good bits, enough even when x lies close to a multiple of
// pi/2.  The fraction is turned back into radians in quad-double.
//
// The table holds the first 1280 bits of 2/pi.  It was computed with exact integer arithmetic
// (Machin's formula for pi to 1408 bits, then floor(2^1281 / pi)), and its leading words
// agree with fdlibm's ipio2 table.  It serves exponents up to 1280 - 576, which covers the
// quad-double engine's |scale| <= 600.
#include <cstdint>
#include <utility>
#include <universal/number/bposit/math/correctly_rounded.hpp>

namespace sw { namespace universal { namespace bposit_math {

inline constexpr std::uint64_t two_over_pi_bits[20] = {
	0xA2F9836E4E441529ull, 0xFC2757D1F534DDC0ull, 0xDB6295993C439041ull, 0xFE5163ABDEBBC561ull,
	0xB7246E3A424DD2E0ull, 0x06492EEA09D1921Cull, 0xFE1DEB1CB129A73Eull, 0xE88235F52EBB4484ull,
	0xE99C7026B45F7E41ull, 0x3991D639835339F4ull, 0x9C845F8BBDF9283Bull, 0x1FF897FFDE05980Full,
	0xEF2F118B5A0A6D1Full, 0x6D367ECF27CB09B7ull, 0x4F463F669E5FEA2Dull, 0x7527BAC7EBE5F17Bull,
	0x3D0739F78A5292EAull, 0x6BFB5FB11F8D5D08ull, 0x56033046FC7B6BABull, 0xF0CFBC209AF4361Dull,
};

// 64 bits of 2/pi starting at bit j (bit 1 is the first bit after the binary point)
inline std::uint64_t two_over_pi_window(int j) {
	const int idx = j - 1;
	const int w = idx / 64, b = idx % 64;
	auto word = [](int k) -> std::uint64_t { return (k >= 0 && k < 20) ? two_over_pi_bits[k] : 0u; };
	std::uint64_t v = word(w) << b;
	if (b != 0) v |= word(w + 1) >> (64 - b);
	return v;
}

// the full 128-bit product of two 64-bit words, portably
inline void mul64(std::uint64_t a, std::uint64_t b, std::uint64_t& hi, std::uint64_t& lo) {
	const std::uint64_t a0 = a & 0xFFFF'FFFFu, a1 = a >> 32;
	const std::uint64_t b0 = b & 0xFFFF'FFFFu, b1 = b >> 32;
	const std::uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
	const std::uint64_t mid = (p00 >> 32) + (p01 & 0xFFFF'FFFFu) + (p10 & 0xFFFF'FFFFu);
	lo = (p00 & 0xFFFF'FFFFu) | (mid << 32);
	hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
}

// x = (q + 4k) pi/2 + r with |r| <= pi/4 (to within rounding), for a finite nonzero b-posit
template<typename B>
std::pair<int, qd> reduce_pio2(const B& x) {
	const auto d = x.decode_fields();
	const std::uint64_t sig = (std::uint64_t(1) << d.nf) | d.frac;
	const int e = d.scale - static_cast<int>(d.nf);              // x = sig 2^e
	if (d.scale < -1) return { 0, to_qd(x) };                    // |x| < 1/2: already reduced

	constexpr int L = 576;                                       // window width, 9 words
	constexpr int W = L / 64;
	const int j0 = (e - 1 > 1) ? e - 1 : 1;                      // earlier bits give multiples of 4
	std::uint64_t win[W];                                        // little endian
	for (int k = 0; k < W; ++k) win[W - 1 - k] = two_over_pi_window(j0 + 64 * k);

	// P = sig * window, 640 bits; x (2/pi) = P 2^-s modulo 4
	std::uint64_t P[W + 1]{};
	for (int k = 0; k < W; ++k) {
		std::uint64_t hi, lo;
		mul64(sig, win[k], hi, lo);
		std::uint64_t s = P[k] + lo;
		std::uint64_t carry = s < lo ? 1u : 0u;
		P[k] = s;
		s = P[k + 1] + hi + carry;
		P[k + 1] = s;                                            // no carry out: P[k + 1] was 0
	}
	const int s = j0 + L - 1 - e;                                // fractional bits of P
	auto bit = [&](int i) -> unsigned { return (i >= 0 && i < 64 * (W + 1)) ? static_cast<unsigned>((P[i / 64] >> (i % 64)) & 1u) : 0u; };

	int q = static_cast<int>(bit(s) + 2u * bit(s + 1));
	// the fraction G = P mod 2^s, or 2^s - G when it is at least one half
	std::uint64_t G[W + 1]{};
	for (int i = 0; i < s; ++i) if (bit(i)) G[i / 64] |= std::uint64_t(1) << (i % 64);
	const bool negate = bit(s - 1) != 0u;
	if (negate) {
		q = (q + 1) & 3;
		std::uint64_t carry = 1;                                 // two's complement within s bits
		for (int k = 0; k <= W; ++k) {
			const std::uint64_t v = ~G[k] + carry;
			carry = (carry && v == 0) ? 1u : 0u;
			G[k] = v;
		}
		for (int i = s; i < 64 * (W + 1); ++i) G[i / 64] &= ~(std::uint64_t(1) << (i % 64));
	}
	auto gbit = [&](int i) -> unsigned { return static_cast<unsigned>((G[i / 64] >> (i % 64)) & 1u); };
	int msb = s - 1;
	while (msb >= 0 && gbit(msb) == 0u) --msb;
	if (msb < 0) return { q, qd(0.0) };

	// G 2^-s in quad-double, from 53-bit chunks summed smallest first
	qd f(0.0);
	for (int c = 3; c >= 0; --c) {
		const int hi = msb - 53 * c;
		std::uint64_t chunk = 0;
		for (int i = hi; i > hi - 53; --i) chunk = (chunk << 1) | (i >= 0 ? gbit(i) : 0u);
		f += qd(std::ldexp(static_cast<double>(chunk), hi - 52 - s));
	}
	const qd r = negate ? -(f * qd_pi_2) : f * qd_pi_2;
	// the reduction ran on |x|: x = -((q + 4k) pi/2 + r) = (-q - 4k) pi/2 - r
	if (d.negative) return { (4 - q) & 3, -r };
	return { q, r };
}

}}} // namespace sw::universal::bposit_math
