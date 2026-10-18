#pragma once
// multidouble_reduce_pio2.hpp: exact trigonometric argument reduction for the multi-double types (#1723)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sin, cos and sincos of dd, qd, dd_cascade, td_cascade and qd_cascade reduced the argument by
// subtracting nint(x / 2pi) 2pi, with 2pi held to the type's own 106, 159 or 212 bits.  The
// rounding error of that constant is multiplied by the quotient, so the reduced argument lost
// about log2|x| bits, and an argument near a multiple of pi/2 lost the bits of its distance to
// it as well: dd sin(1e22) was wrong at 1e-11 relative, td_cascade sin(1e50) returned 0.10159
// for -0.48050, and past ~1e100 the reduction failed outright.
//
// Payne and Hanek's reduction computes x (2/pi) modulo 4 instead, from as many bits of 2/pi as
// the exponent of x needs.  Every limb of a multi-double value is an exact dyadic sig 2^e, so
// each contributes sig 2^e (2/pi) modulo 4, and the bits of 2/pi that only produce multiples
// of 4 are skipped.  The contributions are summed in a fixed-point accumulator with FRAC = 640
// fractional bits and two integer bits (the quadrant), in two's complement so a negative limb
// subtracts.  The fraction, folded into [-1/2, 1/2), is turned back into radians by one
// multiplication with the type's pi/2: a reduced argument t carries the full precision of the
// type unless |t| < 2^-(640 - 212 - 8), far closer to a multiple of pi/2 than any double or
// multi-double argument of the types' range is likely to come (the closest double is 2^-61
// away, at 6381956970095103 2^797).
//
// The table holds the first 1792 bits of 2/pi, computed as floor(2^1793 / pi) with mpmath at
// two working precisions that agree; its first 1280 bits agree with the independently computed
// table of number/bposit/math/reduce_pio2.hpp.  A limb's lowest set bit is at most 2^971, which
// needs bits up to 970 + 704 = 1674.
#include <cmath>
#include <cstdint>

namespace sw { namespace universal { namespace multidouble_reduction {

	inline constexpr std::uint64_t two_over_pi_bits[28] = {
		0xA2F9836E4E441529ull, 0xFC2757D1F534DDC0ull, 0xDB6295993C439041ull, 0xFE5163ABDEBBC561ull,
		0xB7246E3A424DD2E0ull, 0x06492EEA09D1921Cull, 0xFE1DEB1CB129A73Eull, 0xE88235F52EBB4484ull,
		0xE99C7026B45F7E41ull, 0x3991D639835339F4ull, 0x9C845F8BBDF9283Bull, 0x1FF897FFDE05980Full,
		0xEF2F118B5A0A6D1Full, 0x6D367ECF27CB09B7ull, 0x4F463F669E5FEA2Dull, 0x7527BAC7EBE5F17Bull,
		0x3D0739F78A5292EAull, 0x6BFB5FB11F8D5D08ull, 0x56033046FC7B6BABull, 0xF0CFBC209AF4361Dull,
		0xA9E391615EE61B08ull, 0x6599855F14A06840ull, 0x8DFFD8804D732731ull, 0x06061556CA73A8C9ull,
		0x60E27BC08C6B47C4ull, 0x19C367CDDCE8092Aull, 0x8359C4768B961CA6ull, 0xDDAF44D15719053Eull,
	};

	constexpr int FRAC  = 640;          // fractional bits of the accumulator
	constexpr int WORDS = 11;           // accumulator words: 704 bits, of which FRAC + 2 are used
	constexpr int WIN   = 11;           // words of 2/pi multiplied into each limb: 704 bits

	// 64 bits of 2/pi starting at bit j (bit 1 is the first bit after the binary point)
	inline std::uint64_t two_over_pi_window(int j) {
		const int idx = j - 1;
		const int w = idx / 64, b = idx % 64;
		auto word = [](int k) -> std::uint64_t { return (k >= 0 && k < 28) ? two_over_pi_bits[k] : 0u; };
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

	// A += limb (2/pi) modulo 4, A holding FRAC fractional bits and two integer bits
	inline void accumulate(std::uint64_t (&A)[WORDS], double limb) {
		if (limb == 0.0 || !std::isfinite(limb)) return;
		int ex{ 0 };
		const double m = std::frexp(std::fabs(limb), &ex);              // |limb| = m 2^ex, m in [0.5, 1)
		const std::uint64_t sig = static_cast<std::uint64_t>(std::ldexp(m, 53));   // exact, subnormals included
		const int e = ex - 53;                                           // |limb| = sig 2^e
		if (ex < -(FRAC + 4)) return;                                    // below the accumulator

		// the bits of 2/pi before j0 give sig 2^(e - j) with e - j >= 2: multiples of 4
		const int j0 = (e - 1 > 1) ? e - 1 : 1;
		std::uint64_t win[WIN];                                          // little endian
		for (int k = 0; k < WIN; ++k) win[WIN - 1 - k] = two_over_pi_window(j0 + 64 * k);
		std::uint64_t P[WIN + 1]{};                                      // P = sig * window
		for (int k = 0; k < WIN; ++k) {
			std::uint64_t hi, lo;
			mul64(sig, win[k], hi, lo);
			const std::uint64_t s = P[k] + lo;
			const std::uint64_t carry = (s < lo) ? 1u : 0u;
			P[k] = s;
			P[k + 1] = hi + carry;                                       // no carry out: P[k + 1] was 0
		}
		// P has j0 + 64 WIN - 1 - e >= 702 fractional bits; align it to FRAC
		const int shift = j0 + 64 * WIN - 1 - e - FRAC;
		std::uint64_t C[WORDS];
		for (int k = 0; k < WORDS; ++k) {
			const int pos = shift + 64 * k;
			const int w = pos / 64, b = pos % 64;
			const std::uint64_t lo = (w <= WIN) ? P[w] : 0u;
			const std::uint64_t hi = (w + 1 <= WIN) ? P[w + 1] : 0u;
			C[k] = (b != 0) ? ((lo >> b) | (hi << (64 - b))) : lo;
		}
		if (limb > 0.0) {
			std::uint64_t carry = 0;
			for (int k = 0; k < WORDS; ++k) {
				const std::uint64_t s = A[k] + C[k];
				const std::uint64_t c1 = (s < A[k]) ? 1u : 0u;
				A[k] = s + carry;
				carry = c1 | ((A[k] < s) ? 1u : 0u);
			}
		}
		else {
			std::uint64_t borrow = 0;
			for (int k = 0; k < WORDS; ++k) {
				const std::uint64_t d = A[k] - C[k];
				const std::uint64_t b1 = (A[k] < C[k]) ? 1u : 0u;
				A[k] = d - borrow;
				borrow = b1 | ((d < borrow) ? 1u : 0u);
			}
		}
		A[WORDS - 1] &= 3u;                                              // modulo 4
	}

	/// <summary>
	/// a = (j + 4n) pi/2 + t with |t| <= pi/4, for the finite value a = sum of the limbs
	/// </summary>
	/// <param name="limbs">the limbs of a, in any order</param>
	/// <param name="pi_2">pi/2 to the precision of Real</param>
	/// <param name="t">the reduced argument, to the precision of Real</param>
	/// <returns>the quadrant j in {-1, 0, 1, 2}</returns>
	template<typename Real, int N>
	int reduce_pio2(const double (&limbs)[N], const Real& pi_2, Real& t) {
		std::uint64_t A[WORDS]{};
		for (int i = 0; i < N; ++i) accumulate(A, limbs[i]);
		int q = static_cast<int>(A[WORDS - 1] & 3u);

		// the fraction G = A mod 2^FRAC, folded to 2^FRAC - G when it is at least one half
		std::uint64_t G[WORDS - 1];
		for (int k = 0; k < WORDS - 1; ++k) G[k] = A[k];
		const bool negate = ((G[WORDS - 2] >> 63) & 1u) != 0u;
		if (negate) {
			q = (q + 1) & 3;
			std::uint64_t carry = 1;
			for (int k = 0; k < WORDS - 1; ++k) {
				const std::uint64_t v = ~G[k] + carry;
				carry = (carry != 0u && v == 0u) ? 1u : 0u;
				G[k] = v;
			}
		}
		int msb = -1;
		for (int k = WORDS - 2; k >= 0 && msb < 0; --k) {
			if (G[k] == 0u) continue;
			int b = 63;
			while (((G[k] >> b) & 1u) == 0u) --b;
			msb = 64 * k + b;
		}
		if (msb < 0) {
			t = Real(0.0);
		}
		else {
			// G 2^-FRAC from five 53-bit chunks, summed smallest first: 265 bits
			auto chunk = [&](int lo) -> std::uint64_t {                  // bits [lo, lo + 52]
				std::uint64_t v = 0;
				if (lo >= 0) {
					const int w = lo / 64, b = lo % 64;
					const std::uint64_t x0 = (w < WORDS - 1) ? G[w] : 0u;
					const std::uint64_t x1 = (w + 1 < WORDS - 1) ? G[w + 1] : 0u;
					v = (b != 0) ? ((x0 >> b) | (x1 << (64 - b))) : x0;
				}
				else if (lo > -53) {
					v = G[0] << (-lo);
				}
				return v & ((std::uint64_t(1) << 53) - 1u);
			};
			Real f(0.0);
			for (int c = 4; c >= 0; --c) {
				const int lo = msb - 52 - 53 * c;
				f += Real(std::ldexp(static_cast<double>(chunk(lo)), lo - FRAC));
			}
			t = f * pi_2;
			if (negate) t = -t;
		}
		return (q == 3) ? -1 : q;
	}

}}}  // namespace sw::universal::multidouble_reduction
