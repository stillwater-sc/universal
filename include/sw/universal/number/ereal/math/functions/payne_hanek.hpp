#pragma once
// payne_hanek.hpp: exact 2 pi argument reduction for ereal sin, cos and tan (#2016)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sin and cos used to reduce x by subtracting n 2 pi with the configuration's own pi. The
// rounding error of that pi is multiplied by n, so reducing an x near 2^e lost about e bits:
// ereal<4> agreed with ereal<19> to 107 bits at 1e100 and to none at 1e200.
//
// Payne and Hanek's reduction computes x / (2 pi) modulo 1 instead, from as many bits of
// 1 / (2 pi) as the exponent of x needs. Every limb of an ereal is an exact dyadic S 2^e, so
// it contributes S 2^e / (2 pi) modulo 1, and the bits of 1 / (2 pi) that only produce
// integers are skipped. The contributions are summed exactly in an einteger with L
// fractional bits, folded into [-1/2, 1/2), and turned back into radians by one
// multiplication with 2 pi at the configuration's precision.
//
// The bits of 1 / (2 pi) come from Machin's formula, pi = 16 atan(1/5) - 4 atan(1/239), in
// einteger fixed point, so no table limits the reach: each configuration computes, once,
// enough bits for the largest finite limb its limb type has (about 1500 bits for double
// limbs, 17000 for an 80-bit or 128-bit long double).
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <universal/number/einteger/core.hpp>

namespace sw { namespace universal {

namespace ereal_detail {

	using reduction_int = einteger<std::uint32_t>;

	// atan(1/k) 2^W, truncated, from its alternating series: every term is truncated once,
	// so the sum is within (terms + 1) units of the last place
	inline reduction_int atan_inverse_fixed(long long k, int W) {
		reduction_int power(1);
		power <<= W;
		power /= k;                                 // 2^W / k
		reduction_int sum = power;
		const long long k2 = k * k;
		for (long long n = 1; !power.iszero(); ++n) {
			power /= k2;                            // 2^W / k^(2n + 1)
			reduction_int term = power;
			term /= (2 * n + 1);
			if (n & 1) sum -= term; else sum += term;
		}
		return sum;
	}

	// floor(2^nbits / (2 pi)): the first nbits bits of 1 / (2 pi) after the binary point.
	// pi is carried with 64 guard bits; Machin's sums are within a few thousand units of
	// 2^-W, so the quotient is within one unit.
	inline reduction_int inverse_two_pi_bits(int nbits) {
		const int W = nbits + 64;
		reduction_int pi = atan_inverse_fixed(5, W) * reduction_int(16) - atan_inverse_fixed(239, W) * reduction_int(4);   // pi 2^W
		reduction_int num(1);
		num <<= (nbits + W - 1);                    // 2^nbits 2^W / 2
		num /= pi;
		return num;
	}

	// |v| = S 2^e exactly, for a finite nonzero v of any limb type: the significand is read
	// 24 bits at a time, which every limb type holds exactly, so IBM double-double limbs,
	// whose significand bits need not be contiguous, are read exactly as well
	template<typename FpType>
	inline void exact_dyadic(FpType v, reduction_int& S, int& e) {
		int ex{ 0 };
		FpType m = std::frexp(std::fabs(v), &ex);   // |v| = m 2^ex, m in [0.5, 1)
		S = reduction_int(0);
		int bits = 0;
		while (m != FpType(0) && bits < 4096) {
			m = std::ldexp(m, 24);
			const FpType c = std::floor(m);
			m -= c;
			S <<= 24;
			S += reduction_int(static_cast<long long>(c));
			bits += 24;
		}
		e = ex - bits;
	}

	// the einteger n 2^scale as an ereal, rounded: its leading `keep` bits, read in chunks the
	// limb type holds exactly and summed smallest first
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> einteger_to_ereal(const reduction_int& n, int scale, int keep) {
		using Real = ereal<maxlimbs, FpType>;
		if (n.iszero()) return Real(0.0);
		reduction_int a = n;
		const bool negative = a.isneg();
		if (negative) a.setsign(false);
		constexpr int w = std::min(24, std::numeric_limits<FpType>::digits);
		const int msb = a.findMsb();
		const int chunks = (keep + w - 1) / w;
		Real result(0.0);
		for (int c = chunks - 1; c >= 0; --c) {
			const int lo = msb - w + 1 - w * c;     // the chunk is bits [lo, lo + w - 1]
			reduction_int t = a;
			if (lo > 0) t >>= lo; else t <<= -lo;
			reduction_int high = t;
			high >>= w;
			high <<= w;
			t -= high;
			const long long v = static_cast<long long>(t);
			if (v != 0) result += ldexp(Real(static_cast<FpType>(v)), lo + scale);
		}
		return negative ? -result : result;
	}

	// the bits of 1 / (2 pi) a configuration needs, computed once: L fractional bits of the
	// accumulator below the largest limb's exponent
	template<unsigned maxlimbs, typename FpType>
	struct inverse_two_pi_table {
		static constexpr int precision = series_precision_bits<maxlimbs, FpType>();
		// L: the working precision, the bits of one limb's significand (whose product with the
		// table's tail is the truncation error), and 128 guard bits for an argument close to a
		// multiple of 2 pi
		static constexpr int L = precision + std::max(std::numeric_limits<FpType>::digits, 64) + 128;
		// a limb's lowest set bit lies below 2^max_exponent, and needs table bits up to e + L
		static constexpr int nbits = std::numeric_limits<FpType>::max_exponent + L + 1;

		static const reduction_int& bits() {
			static const reduction_int table = inverse_two_pi_bits(nbits);
			return table;
		}
	};

	/// a - 2 pi n in [-pi, pi), n the nearest integer to a / (2 pi), for a finite a
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> payne_hanek_two_pi(const ereal<maxlimbs, FpType>& a, const ereal<maxlimbs, FpType>& two_pi) {
		using Real  = ereal<maxlimbs, FpType>;
		using Table = inverse_two_pi_table<maxlimbs, FpType>;
		constexpr int L  = Table::L;
		constexpr int NT = Table::nbits;
		const reduction_int& T = Table::bits();

		// A = sum over the limbs of S 2^e / (2 pi), in units of 2^-L, modulo 2^L
		reduction_int A(0);
		for (FpType limb : a.limbs()) {
			if (limb == FpType(0) || !std::isfinite(limb)) continue;
			reduction_int S;
			int e{ 0 };
			exact_dyadic(limb, S, e);
			const int N = e + L;                    // the last table bit this limb needs
			if (N < 1) continue;                    // the limb lies below 2^-(L - digits): no effect
			// bits j0 .. N of 1 / (2 pi): the bits before j0 only give S 2^(e - j) >= 1, integers
			const int j0 = std::max(1, e + 1);
			reduction_int B = T;
			B >>= (NT - N);                         // floor(2^N / (2 pi))
			const int width = N - j0 + 1;
			reduction_int high = B;
			high >>= width;
			high <<= width;
			B -= high;
			reduction_int product = S * B;          // S 2^e (bits j0 .. N) = product 2^-L
			if (limb < FpType(0)) A -= product; else A += product;
		}
		reduction_int M(1);
		M <<= L;
		reduction_int r = A % M;                    // truncated: the sign of A
		if (r.isneg()) r += M;                      // in [0, 2^L)
		reduction_int half(1);
		half <<= (L - 1);
		if (r >= half) r -= M;                      // in [-2^(L-1), 2^(L-1))

		// r 2^-L of a turn, back to radians at the configuration's precision
		const Real turns = einteger_to_ereal<maxlimbs, FpType>(r, -L, Table::precision + 64);
		return turns * two_pi;
	}

}  // namespace ereal_detail

}}  // namespace sw::universal
