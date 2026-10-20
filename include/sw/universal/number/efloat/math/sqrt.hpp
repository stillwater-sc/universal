// sqrt.hpp: square root and cube root functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sw { namespace universal {

namespace efloat_detail {

	// r[0, n) = the low n 64-bit limbs of floor(V 2^shift), V the MSB-first limbs v as an
	// integer; shift may be negative
	inline void load_shifted(efloat_mpn::limb_t* r, std::size_t n, const std::vector<uint32_t>& v, int64_t shift) {
		const int64_t nv = static_cast<int64_t>(v.size());
		auto limb32 = [&v, nv](int64_t j) -> uint64_t {
			return (j >= 0 && j < nv) ? uint64_t(v[static_cast<std::size_t>(nv - 1 - j)]) : 0u;
		};
		for (std::size_t k = 0; k < n; ++k) {
			const int64_t p = 64 * static_cast<int64_t>(k) - shift;   // the bit of V at bit 0 of r[k]
			uint64_t w = 0;
			if (p > -64 && p < 0) {
				w = (limb32(0) | (limb32(1) << 32)) << static_cast<unsigned>(-p);
			}
			else if (p >= 0) {
				const int64_t  j  = p / 32;
				const unsigned sh = static_cast<unsigned>(p % 32);
				w = limb32(j) | (limb32(j + 1) << 32);
				if (sh != 0u) w = (w >> sh) | (limb32(j + 2) << (64u - sh));
			}
			r[k] = w;
		}
	}

	// sqrt(m) for m in [1, 4), correctly rounded to `precision` bits in the current mode (#1763).
	// With F = 64 n - 1 and N = m 2^(2F), S = floor(sqrt(N)) fills n 64-bit limbs, its top bit
	// set, and carries at least precision + 2 bits, the guard bit among them; the remainder
	// N - S^2 is the sticky bit.  efloat_mpn::sqrtrem forms both exactly.
	template<unsigned nlimbs>
	inline efloat<nlimbs> sqrt_exact(const efloat<nlimbs>& m, unsigned precision) {
		using efloat_mpn::limb_t;
		const std::vector<uint32_t> mb = m.bits();
		const int64_t frac = 32 * static_cast<int64_t>(mb.size()) - 1 - m.scale();   // m = M 2^-frac
		std::size_t n = (static_cast<std::size_t>(precision) + 2 + 63) / 64;
		while (2 * (64 * static_cast<int64_t>(n) - 1) < frac) ++n;   // a longer m: N must be an integer
		const int64_t F = 64 * static_cast<int64_t>(n) - 1;

		limb_t local[512];
		std::vector<limb_t> heap;
		const std::size_t need = 3 * n + efloat_mpn::sqrt_scratch_limbs(n);
		limb_t* np = local;
		if (need > 512) { heap.resize(need); np = heap.data(); }
		limb_t* sp   = np + 2 * n;
		limb_t* work = sp + n;
		load_shifted(np, 2 * n, mb, 2 * F - frac);
		const limb_t carry = efloat_mpn::sqrtrem(sp, np, n, work);
		bool sticky = (carry != 0u);
		for (std::size_t k = 0; !sticky && k < n; ++k) sticky = (np[k] != 0u);

		// S's 32-bit limbs, most significant first, then a limb only the sticky bit sets; S 2^-F
		// is in [1, 2), and it rounds once
		efloat<nlimbs> result(1);
		result.set_precision(static_cast<unsigned>(32 * (2 * n + 1)));
		for (std::size_t i = 0; i < 2 * n; ++i) {
			const limb_t w = sp[n - 1 - i / 2];
			result.setlimb(static_cast<unsigned>(i), (i & 1u) ? static_cast<uint32_t>(w) : static_cast<uint32_t>(w >> 32));
		}
		if (sticky) result.setlimb(static_cast<unsigned>(2 * n), 1u);
		result.setexponent(0);
		result.set_precision(precision);
		return result;
	}

	// sqrt(a) for a finite a > 0 rounded once to `bits` bits: a scaled to m in [1, 4) for the
	// exact root, the exponent halved.  sin's versine takes it at its guard precision (#1761).
	template<unsigned nlimbs>
	inline efloat<nlimbs> sqrt_to(const efloat<nlimbs>& a, unsigned bits) {
		const int64_t e    = a.scale();
		const int64_t half = e / 2 - ((e < 0 && e % 2 != 0) ? 1 : 0);   // floor(e / 2), without e - 1 overflowing
		efloat<nlimbs> m(a);
		m.setexponent(e - 2 * half);                            // m in [1, 4)
		efloat<nlimbs> x = sqrt_exact(m, bits);
		x.setexponent(x.scale() + half);
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

// cbrt: cube root function for efloat using Newton's method
template<unsigned nlimbs>
constexpr efloat<nlimbs> cbrt(const efloat<nlimbs>& a) {
	if (a.iszero() || a.isinf() || a.isnan()) return a;

	// standard cbrt(-x) == -cbrt(x)
	bool is_neg = (a.sign() == -1);
	efloat<nlimbs> abs_a(a);
	abs_a.setsign(false);

	// Initial guess: y0 = 1.0 * 2^(scale / 3)
	efloat<nlimbs> y;
	y.clear();
	y.setexponent(abs_a.scale() / 3);
	y.setlimb(0, 0x80000000);
	const unsigned result_prec = abs_a.get_precision();   // the result's precision: a's own (#1757)
	const unsigned prec = result_prec + 64u;             // guard bits for the iteration
	y.set_precision(prec);
	abs_a.set_precision(prec);

	// 1/3 at that precision: a 1/3 at fewer bits moves the fixed point of the iteration
	efloat<nlimbs> one_third(1.0);
	one_third.set_precision(prec);
	one_third /= 3;
	efloat<nlimbs> two(2.0);

	// Newton iterations for y^3 - abs_a = 0: y_next = 1/3 * (2 * y + abs_a / y^2).
	// Precision-adaptive iteration count with a convergence break, for the same
	// reason as sqrt above (issue #1140): the ~1-bit exponent-only seed needs
	// ~ceil(log2(precision)) doublings to reach full precision.
	int max_iters = 6;
	for (unsigned p = prec; p > 1u; p >>= 1) ++max_iters;
	for (int i = 0; i < max_iters; ++i) {
		efloat<nlimbs> prev(y);
		y = one_third * (two * y + abs_a / (y * y));
		efloat<nlimbs> d(y - prev); d.setsign(false);
		if (d.iszero() || (!y.iszero() && d.scale() < y.scale() - static_cast<int64_t>(prec))) break;
	}

	y.set_precision(result_prec);
	if (!a.carries_precision()) y.clear_precision();
	if (is_neg) {
		y.setsign(true);
	}
	return y;
}

}} // namespace sw::universal
