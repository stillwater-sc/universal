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
#include <universal/number/posit/math/posit_limbs.hpp>

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
			using namespace posit_limbs;
			constexpr unsigned fbits = Posit::fbits;
			constexpr std::size_t n = (fbits + 3u + 63u) / 64u;   // R has 64 n >= fbits + 3 bits
			constexpr unsigned rbits = static_cast<unsigned>(64u * n);

			// a = S * 2^(scale - fbits), S in [2^fbits, 2^(fbits+1)): the fraction's nf bits sit
			// below the hidden bit, exponent bits cut off by the regime are zero
			limb_t w[limbs(nbits)];
			gather(a, w);
			const Fields f = decode<nbits, es>(w);
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
			return encode(p, halfExponent + static_cast<int>(rbits) - 1, R, n, sticky);
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
