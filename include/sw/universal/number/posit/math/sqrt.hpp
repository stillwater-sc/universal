#pragma once
// sqrt.hpp: sqrt functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <universal/native/ieee754.hpp>   // posit.hpp users get to_binary(float) and friends through here
#include <universal/internal/limbs/limb_kernels.hpp>   // sqrtrem, for posits wider than 64 bits

namespace sw { namespace universal {

	namespace posit_sqrt_detail {

		// a 128-bit unsigned integer, for the square root of a 64-bit significand
		struct u128 { std::uint64_t hi, lo; };

		// a * b, exactly, from 32-bit halves: portable, no __int128 and no intrinsics
		inline u128 mul(std::uint64_t a, std::uint64_t b) noexcept {
			const std::uint64_t a0 = a & 0xFFFF'FFFFull, a1 = a >> 32, b0 = b & 0xFFFF'FFFFull, b1 = b >> 32;
			const std::uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
			const std::uint64_t mid = (p00 >> 32) + (p01 & 0xFFFF'FFFFull) + (p10 & 0xFFFF'FFFFull);
			return { p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32), (mid << 32) | (p00 & 0xFFFF'FFFFull) };
		}
		inline int compare(const u128& a, const u128& b) noexcept {
			if (a.hi != b.hi) return a.hi < b.hi ? -1 : 1;
			if (a.lo != b.lo) return a.lo < b.lo ? -1 : 1;
			return 0;
		}
		inline u128 subtract(const u128& a, const u128& b) noexcept {   // a >= b
			return { a.hi - b.hi - (a.lo < b.lo ? 1u : 0u), a.lo - b.lo };
		}
		inline double to_double(const u128& a) noexcept { return std::ldexp(double(a.hi), 64) + double(a.lo); }

		// the integer square root of N, N in [2^126, 2^128): R = floor(sqrt(N)), in [2^63, 2^64).
		// A double seed and one correction from the exact residual give R within a unit or two;
		// the exact fix-up then makes it right, so the result never depends on the seed's accuracy.
		inline std::uint64_t isqrt(const u128& N) noexcept {
			constexpr std::uint64_t Rmax = ~0ull;
			const double r0 = std::sqrt(to_double(N));
			std::uint64_t R = (r0 >= 0x1.0p64) ? Rmax : static_cast<std::uint64_t>(r0);
			const u128 R2 = mul(R, R);
			const bool above = compare(N, R2) < 0;
			const double delta = to_double(above ? subtract(R2, N) : subtract(N, R2)) / (2.0 * double(R));
			const std::uint64_t c = static_cast<std::uint64_t>(delta + 0.5);
			if (above) R -= c;
			else R = (c > Rmax - R) ? Rmax : R + c;
			while (compare(mul(R, R), N) > 0) --R;
			while (R != Rmax && compare(mul(R + 1, R + 1), N) <= 0) ++R;
			return R;
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// posits wider than 64 bits, on 64-bit limbs, least significant first (#1846)
		using limb_kernels::limb_t;

		constexpr std::size_t limbs(unsigned bits) noexcept { return (bits + 63u) / 64u; }
		inline bool test(const limb_t* w, unsigned i) noexcept { return ((w[i / 64u] >> (i % 64u)) & 1u) != 0u; }
		inline void set(limb_t* w, unsigned i) noexcept { w[i / 64u] |= limb_t(1) << (i % 64u); }

		// the encoding of a posit, gathered from its blocks
		template<unsigned nbits, unsigned es, typename bt>
		inline void wide_gather(const posit<nbits, es, bt>& a, limb_t* w) noexcept {
			constexpr unsigned bitsInBlock = sizeof(bt) * 8u;
			const auto bits = a.bits();
			for (std::size_t i = 0; i < limbs(nbits); ++i) w[i] = 0u;
			for (unsigned i = 0; i < bits.nrBlocks; ++i) {
				const unsigned pos = i * bitsInBlock;
				w[pos / 64u] |= limb_t(bits.block(i)) << (pos % 64u);
			}
		}

		// a positive posit's scale, and its nf fraction bits, which are w[0, nf) on return: the
		// regime and exponent bits above them are cleared
		struct WideFields { int scale; unsigned nf; };
		template<unsigned nbits, unsigned es>
		inline WideFields wide_decode(limb_t* w) noexcept {
			const unsigned top = nbits - 2u;   // the first bit after the sign
			const bool positiveRegime = test(w, top);
			unsigned run = 1;
			while (run < nbits - 1u && test(w, top - run) == positiveRegime) ++run;
			const int k = positiveRegime ? static_cast<int>(run) - 1 : -static_cast<int>(run);
			const int below = static_cast<int>(nbits) - 2 - static_cast<int>(run);   // bits after the terminator
			int exponent = 0;
			for (int i = 1; i <= static_cast<int>(es); ++i) {
				const int j = below - i;
				exponent = (exponent << 1) | ((j >= 0 && test(w, static_cast<unsigned>(j))) ? 1 : 0);
			}
			const unsigned nf = (below > static_cast<int>(es)) ? static_cast<unsigned>(below) - es : 0u;
			for (std::size_t i = nf / 64u; i < limbs(nbits); ++i) {
				w[i] = (i == nf / 64u) ? (w[i] & ((limb_t(1) << (nf % 64u)) - 1u)) : 0u;
			}
			return { k * (1 << es) + exponent, nf };
		}

		// p = sig * 2^(scale - (64 n - 1)), sig in [2^(64 n - 1), 2^(64 n)), with sticky standing for
		// nonzero bits below sig: rounded once to nearest, ties to even, and never to zero or NaR
		template<std::size_t n, unsigned nbits, unsigned es, typename bt>
		inline posit<nbits, es, bt>& wide_encode(posit<nbits, es, bt>& p, int scale, const limb_t* sig, bool sticky) noexcept {
			constexpr unsigned sbits = static_cast<unsigned>(64u * n);
			const int k = scale >> es;   // floor(scale / 2^es)
			const unsigned run = (k >= 0) ? static_cast<unsigned>(k) + 1u : static_cast<unsigned>(-k);
			if (run >= nbits - 1u) {   // the regime fills the posit: past maxpos or below minpos
				return (k >= 0) ? p.maxpos() : p.minpos();
			}
			// the unrounded body, T bits: the regime and its terminator, es exponent bits, and sig
			// below its hidden bit
			constexpr std::size_t B = limbs(sbits + es + nbits);   // T < sbits + es + nbits
			limb_t body[B]{};
			for (std::size_t i = 0; i < n; ++i) body[i] = sig[i];
			body[(sbits - 1u) / 64u] &= ~(limb_t(1) << ((sbits - 1u) % 64u));
			const unsigned exponent = static_cast<unsigned>(scale - k * (1 << es));
			for (unsigned i = 0; i < es; ++i) {
				if ((exponent >> i) & 1u) set(body, sbits - 1u + i);
			}
			const unsigned terminator = sbits - 1u + es;
			if (k >= 0) { for (unsigned i = 1; i <= run; ++i) set(body, terminator + i); }
			else set(body, terminator);
			const unsigned T = terminator + run + 1u;

			// the top nbits - 1 bits, and the guard bit below them
			constexpr std::size_t L = limbs(nbits);
			limb_t r[L];
			const bool dropped = limb_kernels::load_window(r, L, body, B, static_cast<std::int64_t>(nbits) - static_cast<std::int64_t>(T));
			const bool guard = (r[0] & 1u) != 0u;
			limb_kernels::rshift(r, r, L, 1u);
			if (guard && (dropped || sticky || (r[0] & 1u) != 0u)) limb_kernels::add_1(r, L, 1u);

			constexpr unsigned bitsInBlock = sizeof(bt) * 8u;
			blockbinary<nbits, bt, BinaryNumberType::Signed> bits;
			bits.clear();
			for (unsigned i = 0; i < bits.nrBlocks; ++i) {
				const unsigned pos = i * bitsInBlock;
				bits.setblock(i, static_cast<bt>(r[pos / 64u] >> (pos % 64u)));
			}
			return p.setbits(bits);
		}

	} // namespace posit_sqrt_detail

	// sqrt, correctly rounded at every nbits: the integer square root of the significand, the
	// remainder as the sticky bit, then one rounding through posit's encoder.  sqrt(0) = 0,
	// sqrt(NaR) = NaR, and a negative argument is NaR.
	template<unsigned nbits, unsigned es, typename bt>
	inline posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>& a) {
		using Posit = posit<nbits, es, bt>;
		if (a.iszero() || a.isnar()) return a;
		if (a.isneg()) return Posit(SpecificValue::nar);
		if constexpr (nbits > 64) {
			// the same construction on 64-bit limbs (#1846): R = floor(sqrt(N)) of n limbs and its
			// remainder from limb_kernels::sqrtrem, both exact, then one rounding
			using namespace posit_sqrt_detail;
			constexpr unsigned fbits = Posit::fbits;
			constexpr std::size_t n = (fbits + 3u + 63u) / 64u;   // R has 64 n >= fbits + 3 bits
			constexpr unsigned rbits = static_cast<unsigned>(64u * n);

			// a = S * 2^(scale - fbits), S in [2^fbits, 2^(fbits+1)): the fraction's nf bits sit
			// below the hidden bit, exponent bits cut off by the regime are zero
			limb_t w[limbs(nbits)];
			wide_gather(a, w);
			const WideFields f = wide_decode<nbits, es>(w);
			const int ea = f.scale - static_cast<int>(fbits);

			// N = S * 2^shift in [2^(2 rbits - 2), 2^(2 rbits)) with ea - shift even
			unsigned shift = 2u * rbits - 1u - fbits;
			if (((ea - static_cast<int>(shift)) & 1) != 0) --shift;
			limb_t N[2 * n];
			limb_kernels::load_window(N, 2 * n, w, limbs(nbits), static_cast<std::int64_t>(shift + fbits - f.nf));
			N[(shift + fbits) / 64u] |= limb_t(1) << ((shift + fbits) % 64u);
			const int halfExponent = (ea - static_cast<int>(shift)) / 2;

			limb_t R[n];
			limb_t scratch[limb_kernels::sqrt_scratch_limbs(n)];
			bool sticky = limb_kernels::sqrtrem(R, N, n, scratch) != 0u;   // the remainder replaces N[0, n)
			for (std::size_t i = 0; i < n; ++i) sticky = sticky || (N[i] != 0u);

			Posit p;
			return wide_encode<n>(p, halfExponent + static_cast<int>(rbits) - 1, R, sticky);
		}
		else {
			using namespace posit_sqrt_detail;
			// decode a = S * 2^(scale - 63), S in [2^63, 2^64), from the bits after the sign, left-aligned;
			// exponent bits cut off by the regime are zero
			const std::uint64_t body = static_cast<std::uint64_t>(a.encoding()) << (65u - nbits);
			const bool positiveRegime = (body >> 63) != 0;
			const int run = positiveRegime ? std::countl_one(body) : std::countl_zero(body);
			const int k = positiveRegime ? run - 1 : -run;
			std::uint64_t rest = (run + 1 >= 64) ? 0ull : (body << (run + 1));
			int exponent = 0;
			if constexpr (es > 0) {
				exponent = static_cast<int>(rest >> (64u - es));
				rest <<= es;
			}
			const int scale = k * (1 << es) + exponent;
			const std::uint64_t S = (1ull << 63) | (rest >> 1);

			// N = S * 2^shift with scale - 63 - shift even, so sqrt(a) = sqrt(N) * 2^((scale - 63 - shift) / 2)
			const int ea = scale - 63;
			const unsigned shift = ((ea - 64) % 2 == 0) ? 64u : 63u;
			const u128 N = (shift == 64u) ? u128{ S, 0ull } : u128{ S >> 1, S << 63 };
			const int halfExponent = (ea - static_cast<int>(shift)) / 2;

			// R in [2^63, 2^64): its 64 bits are far more than the posit keeps, so R and the sticky
			// bit decide every rounding exactly
			const std::uint64_t R = isqrt(N);
			const bool sticky = compare(mul(R, R), N) != 0;
			const std::uint64_t fraction = (R << 1) | (sticky ? 1ull : 0ull);   // R without its hidden bit, then sticky
			Posit p;
			return p.setpositive(halfExponent + 63, fraction, 64u);
		}
	}

	// reciprocal sqrt
	template<unsigned nbits, unsigned es, typename bt>
	inline posit<nbits, es, bt> rsqrt(const posit<nbits, es, bt>& a) {
		posit<nbits, es, bt> v = sqrt(a);
		return v.reciprocal();
	}

}} // namespace sw::universal
