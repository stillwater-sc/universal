// sqrt.hpp: square root and cube root functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <universal/number/efloat/math/exponent.hpp>   // can_round, exact_power, round_result

namespace sw { namespace universal {

namespace efloat_detail {

	// r[0, n) = the low n 64-bit limbs of floor(V 2^shift), V the MSB-first limbs v as an
	// integer; shift may be negative. V is read as 64-bit words, least significant first, and
	// shifted once, each word read once: the bit-by-bit assembly from 32-bit halves it replaces
	// cost a twentieth of sqrt at 8192 bits (#1810).
	inline void load_shifted(efloat_mpn::limb_t* r, std::size_t n, const std::vector<uint32_t>& v, int64_t shift) {
		using efloat_mpn::limb_t;
		const std::size_t nv = v.size();
		const int64_t     m  = static_cast<int64_t>((nv + 1) / 2);
		auto word = [&v, nv, m](int64_t j) -> limb_t {   // word j of P = V 2^(32 (2m - nv)), an odd count padded at the bottom
			if (j < 0 || j >= m) return 0u;
			const std::size_t hi = static_cast<std::size_t>(2 * m - 2 - 2 * j);
			return (limb_t(v[hi]) << 32) | ((hi + 1 < nv) ? v[hi + 1] : 0u);
		};
		const int64_t  s = shift - 32 * (2 * m - static_cast<int64_t>(nv));   // floor(P 2^s)
		const int64_t  q = (s >= 0) ? -((s + 63) / 64) : (-s) / 64;          // floor(-s / 64)
		const unsigned c = static_cast<unsigned>(-s - 64 * q);              // in [0, 64)
		limb_t low = word(q);
		for (std::size_t k = 0; k < n; ++k) {
			const limb_t high = word(q + static_cast<int64_t>(k) + 1);
			r[k] = (c == 0u) ? low : ((low >> c) | (high << (64u - c)));
			low = high;
		}
	}

	// sqrt(a) for a finite a > 0, correctly rounded to `precision` bits in the current mode, into
	// result (#1763, #1810). With a = m 4^half, m in [1, 4), F = 64 n - 1 and N = m 2^(2F), the
	// root S = floor(sqrt(N)) fills n 64-bit limbs, its top bit set, and carries at least
	// precision + 2 bits, the guard bit among them; the remainder N - S^2 is the sticky bit.
	// efloat_mpn::sqrt_approx first forms S, or at most sqrt_approx_error(n) above it (6 at 8192
	// bits), without the remainder: precisions are whole 32-bit limbs, so that value carries 30
	// bits or more past the precision, and Ziv's test on it (can_round) decides the rounding
	// wherever it passes. Where it does not
	// -- a root within a unit of a rounding boundary, and every exact square, whose root ends in
	// zeros -- sqrtrem forms S and the remainder exactly. Before, the remainder was formed always,
	// a square of half the root's length that GMP and MPFR skip, and the argument and the root
	// were copied through efloat values on the way in and out.
	template<unsigned nlimbs>
	inline void sqrt_limbs(efloat<nlimbs>& result, const efloat<nlimbs>& a, unsigned precision) {
		using efloat_mpn::limb_t;
		const std::vector<uint32_t>& mb = a.limb_vector();
		const int64_t e    = a.scale();
		const int64_t half = e / 2 - ((e < 0 && e % 2 != 0) ? 1 : 0);   // floor(e / 2), without e - 1 overflowing
		const int64_t frac = 32 * static_cast<int64_t>(mb.size()) - 1 - (e - 2 * half);   // m = M 2^-frac
		// the root rounds at whole 32-bit limbs, so it is sized from them: a request of 33 bits
		// rounds at 64, and the root then needs 66 (before, it was sized from the 33)
		const unsigned plimbs = std::max(1u, (precision + 31) / 32);
		std::size_t n = (32 * static_cast<std::size_t>(plimbs) + 2 + 63) / 64;
		while (2 * (64 * static_cast<int64_t>(n) - 1) < frac) ++n;   // a longer m: N must be an integer
		const int64_t F = 64 * static_cast<int64_t>(n) - 1;

		limb_buffer buffer;
		limb_t* np   = buffer.get(3 * n + efloat_mpn::sqrt_scratch_limbs(n));
		limb_t* sp   = np + 2 * n;
		limb_t* work = sp + n;
		load_shifted(np, 2 * n, mb, 2 * F - frac);
		efloat_mpn::sqrt_approx(sp, np, n, work);
		// S 2^-F lies within sqrt_approx_error(n) + 1 units of its last bit of the root, and its
		// top bit weighs 2^half
		const unsigned lost = static_cast<unsigned>(std::bit_width(efloat_mpn::sqrt_approx_error(n) + 1));
		if (efloat_mpn::can_round(sp, n, 32 * plimbs, static_cast<unsigned>(F) - lost)) {
			result.assign_rounded_limbs(false, sp, n, half, true, plimbs);
			return;
		}
		load_shifted(np, 2 * n, mb, 2 * F - frac);
		const limb_t carry = efloat_mpn::sqrtrem(sp, np, n, work);
		bool sticky = (carry != 0u);
		for (std::size_t k = 0; !sticky && k < n; ++k) sticky = (np[k] != 0u);
		result.assign_rounded_limbs(false, sp, n, half, sticky, plimbs);
	}

	// sqrt(a) for a finite a > 0 rounded once to `bits` bits. sin's versine takes it at its guard
	// precision (#1761), and the inverse trigonometric and hyperbolic functions and log's AGM at
	// their working precision.
	template<unsigned nlimbs>
	inline efloat<nlimbs> sqrt_to(const efloat<nlimbs>& a, unsigned bits) {
		efloat<nlimbs> x(a);   // holds the root's limbs without growing, in most cases
		sqrt_limbs(x, a, bits);
		return x;
	}

}  // namespace efloat_detail

// sqrt(a), correctly rounded to a's precision in the current mode
template<unsigned nlimbs>
constexpr efloat<nlimbs> sqrt(const efloat<nlimbs>& a) {
	if (a.iszero()) return a;
	if (a.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (a.isinf() || a.isnan()) return a;

	// the result's precision is a's own (#1757), and the root is exact before it rounds once,
	// so the result is correctly rounded in every mode and Inexact says whether it is exact
	// (#1763).  Before, Newton's iteration ran with 64 guard bits and rounded from them, which
	// misrounded near halfway points and set Inexact on exact roots.
	efloat<nlimbs> x = efloat_detail::sqrt_to(a, a.get_precision());
	if (!a.carries_precision()) x.clear_precision();   // as its arithmetic would (#1757)
	return x;
}

namespace efloat_detail {

	// cbrt(m) for m in [1, 8), correctly rounded to `precision` bits in the current mode (#1800).
	// With F = 64 n - 1 and N = m 2^(3F), S = floor(cbrt(N)) fills n 64-bit limbs, its top bit
	// set, and carries at least precision + 18 bits. efloat_mpn::cbrtrem forms S without its
	// remainder, 0 to cbrt_overshoot units high, so S 2^-F is within 2^-(F - 3) of the root and
	// rounds from there unless its bits from the guard bit on are all 0 or all 1 (can_round):
	// that happens only within a few units of a rounding boundary, at most 2^-25 of arguments
	// (S carries 31 or 63 bits past a precision of whole 32-bit limbs), and for every root of at
	// most precision + 1 bits. Then the exact root and its remainder, the sticky bit, decide,
	// as for sqrt. Before, Newton's iteration ran in efloat arithmetic with 80 guard bits under
	// Ziv's strategy, and an exact cube cost a cube to recognize (#1783).
	template<unsigned nlimbs>
	inline efloat<nlimbs> cbrt_exact(const efloat<nlimbs>& m, unsigned precision) {
		using efloat_mpn::limb_t;
		const std::vector<uint32_t> mb = m.bits();
		const int64_t frac = 32 * static_cast<int64_t>(mb.size()) - 1 - m.scale();   // m = M 2^-frac
		std::size_t n = (static_cast<std::size_t>(precision) + 18 + 63) / 64;
		while (3 * (64 * static_cast<int64_t>(n) - 1) < frac) ++n;   // a longer m: N must be an integer
		const int64_t F = 64 * static_cast<int64_t>(n) - 1;

		// a short m, of b significant bits, can be an exact cube only of a root of about b / 3 bits:
		// where that root needs at most a sixth of n's limbs, its exact remainder says whether m
		// is one, far faster than n limbs would (cbrt(27) at 8192 bits: 0.2x MPFR before #1800,
		// 2.4x without this). A full-width m skips it; an exact cube among those is found below.
		const int64_t sig = static_cast<int64_t>(significant_bits(m));
		const std::size_t ns = static_cast<std::size_t>((sig / 3 + 2 + 63) / 64);
		const int64_t fsig = sig - 1 - m.scale();   // the fraction bits m has
		if (6 * ns <= n) {
			limb_t small[6 * 8 + 1 + efloat_mpn::cbrt_scratch_limbs(8)];
			std::vector<limb_t> heap_small;
			limb_t* sn = small;
			const std::size_t need_small = 6 * ns + 1 + efloat_mpn::cbrt_scratch_limbs(ns);
			if (ns > 8) { heap_small.resize(need_small); sn = heap_small.data(); }
			const int64_t Fs = 64 * static_cast<int64_t>(ns) - 1;
			if (3 * Fs >= fsig) {
				limb_t* ss = sn + 3 * ns;
				limb_t* rs = ss + ns;
				load_shifted(sn, 3 * ns, mb, 3 * Fs - frac);   // drops only m's trailing zeros
				efloat_mpn::cbrtrem(ss, rs, sn, ns, rs + 2 * ns + 1, true);
				bool zero = true;
				for (std::size_t k = 0; zero && k < 2 * ns + 1; ++k) zero = (rs[k] == 0u);
				if (zero) {   // an exact cube: its root, rounded once
					efloat<nlimbs> r(1);
					r.set_precision(static_cast<unsigned>(64 * ns));
					for (std::size_t i = 0; i < 2 * ns; ++i) {
						const limb_t w = ss[ns - 1 - i / 2];
						r.setlimb(static_cast<unsigned>(i), (i & 1u) ? static_cast<uint32_t>(w) : static_cast<uint32_t>(w >> 32));
					}
					r.setexponent(0);
					r.set_precision(precision);
					return r;
				}
			}
		}

		limb_t local[1024];
		std::vector<limb_t> heap;
		const std::size_t need = 6 * n + 1 + efloat_mpn::cbrt_scratch_limbs(n);
		limb_t* np = local;
		if (need > 1024) { heap.resize(need); np = heap.data(); }
		limb_t* sp   = np + 3 * n;
		limb_t* rp   = sp + n;
		limb_t* work = rp + 2 * n + 1;
		load_shifted(np, 3 * n, mb, 3 * F - frac);

		// S's 32-bit limbs, most significant first, then a limb only the sticky bit sets; S 2^-F
		// is in [1, 2)
		auto root = [&](bool sticky) {
			efloat<nlimbs> r(1);
			r.set_precision(static_cast<unsigned>(32 * (2 * n + 1)));
			for (std::size_t i = 0; i < 2 * n; ++i) {
				const limb_t w = sp[n - 1 - i / 2];
				r.setlimb(static_cast<unsigned>(i), (i & 1u) ? static_cast<uint32_t>(w) : static_cast<uint32_t>(w >> 32));
			}
			if (sticky) r.setlimb(static_cast<unsigned>(2 * n), 1u);
			r.setexponent(0);
			return r;
		};
		efloat_mpn::cbrtrem(sp, rp, np, n, work, false);
		efloat<nlimbs> result = root(false);
		if (!can_round(result, precision, static_cast<unsigned>(F) - 3u)) {
			efloat_mpn::cbrtrem(sp, rp, np, n, work, true);
			bool sticky = false;
			for (std::size_t k = 0; !sticky && k < 2 * n + 1; ++k) sticky = (rp[k] != 0u);
			result = root(sticky);
		}
		result.set_precision(precision);
		return result;
	}

	// cbrt(a) for a finite a > 0 rounded once to `bits` bits: a scaled to m in [1, 8) for the
	// root, the exponent divided by 3
	template<unsigned nlimbs>
	inline efloat<nlimbs> cbrt_to(const efloat<nlimbs>& a, unsigned bits) {
		const int64_t e     = a.scale();
		const int64_t third = e / 3 - ((e < 0 && e % 3 != 0) ? 1 : 0);   // floor(e / 3)
		efloat<nlimbs> m(a);
		m.setexponent(e - 3 * third);                                     // m in [1, 8)
		efloat<nlimbs> y = cbrt_exact(m, bits);
		y.setexponent(y.scale() + third);
		return y;
	}

}  // namespace efloat_detail

// cbrt(a), correctly rounded to a's precision in the current mode (#1783, #1800): the root is
// decided from the integer cube root of a's significand, exact where it must be, and rounded
// once. cbrt(-a) = -cbrt(a), rounded as the negative value it is.
template<unsigned nlimbs>
inline efloat<nlimbs> cbrt(const efloat<nlimbs>& a) {
	if (a.iszero() || a.isinf() || a.isnan()) return a;
	const unsigned precision = a.get_precision();
	const bool carries = a.carries_precision();
	const bool negative = (a.sign() == -1);
	efloat<nlimbs> abs_a(a);
	abs_a.setsign(false);
	// a negative root rounds as the negative value: the magnitude in the mirrored mode
	const RoundingMode mode = efloat_rounding_mode;
	if (negative && mode == RoundingMode::RoundTowardPositive) efloat_rounding_mode = RoundingMode::RoundTowardNegative;
	else if (negative && mode == RoundingMode::RoundTowardNegative) efloat_rounding_mode = RoundingMode::RoundTowardPositive;
	efloat<nlimbs> y = efloat_detail::cbrt_to(abs_a, precision);
	efloat_rounding_mode = mode;
	if (negative) y = -y;
	if (!carries) y.clear_precision();
	return y;
}

}} // namespace sw::universal
