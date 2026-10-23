// logarithm.hpp: logarithmic functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <bit>
#include <cmath>
#include <vector>
#include <universal/number/efloat/math/exponent.hpp>
#include <universal/number/efloat/math/sqrt.hpp>           // sqrt_to
#include <universal/number/efloat/math/trigonometry.hpp>   // pi

namespace sw { namespace universal {

namespace efloat_detail {

	// The log table (#1714): log(j / 512) for j = 362 .. 724, the grid points of [sqrt(1/2), sqrt(2)].
	inline constexpr unsigned log_table_shift = 9;
	inline constexpr unsigned log_table_first = 362;   // 362 / 512 < sqrt(1/2)
	inline constexpr unsigned log_table_last  = 724;   // 724 / 512 > sqrt(2)
	// The AGM replaces the table and series once the working precision passes this many times
	// max(10, k), for an x within 2^-k of 1: the series gains ~2 max(10, k) bits per term, so its
	// cost grows as W M(W) / k and the AGM's as log(W) M(W). The constant is measured: with the
	// series in fixed point (#1795) the two cross near 45000 working bits from the table
	// (k = 10) and near 5000 k for x = 1 + 2^-k, k = 20 .. 40, at 65536 and 131072 bits. With the
	// term-by-term efloat series before it, the constant was 144 (#1771): the AGM ran from 1440
	// bits, and at 8192 bits it took 276 us where the fixed-point series takes 140.
	inline constexpr unsigned log_agm_bits_per_series_bit = 4500;
	// log_working(x, W) is within 2^(scale - W + log_error_bits) of log x: measured, the table and
	// series are within 2^-(W - 3) of it and the AGM, with its own guard bits, 2^-(W + 20).
	inline constexpr unsigned log_error_bits = 16;

	// atanh(v) for a finite |v| < 1/2 at `working` bits, unrounded: with z = v^2,
	//     atanh(v) = v S(z),   S(z) = sum_k z^k / (2k + 1) = 1 + z/3 + z^2/5 + ...,
	// S summed in fixed point by rectangular splitting (fixed_series::arctangent_series without
	// the alternation, #1795). S lies in [1, 1.1), so with F >= working + 24 fraction bits it keeps
	// its relative precision whatever the size of v, as log's result does as x -> 1.
	//
	// Error: the series truncates about N + 2 N / g + m + 2 N / m times for groups of g terms, z
	// once: below 2^-(working + 13). z = v^2 and the product v S each round once at working + 2
	// and working bits.
	template<unsigned nlimbs>
	inline efloat<nlimbs> atanh_working_small(const efloat<nlimbs>& v, unsigned working) {
		using fixed_series::limb_t;
		efloat<nlimbs> result(v);
		result.set_precision(working);
		if (v.iszero()) return result;
		efloat<nlimbs> z(v);
		z.set_precision(working + 2u);
		z *= z;
		const std::size_t n = (working + 24u + 63u) / 64u;   // fraction limbs
		const unsigned    F = static_cast<unsigned>(64 * n);
		std::vector<limb_t> buffer(2 * n + 1);
		limb_t* Z  = buffer.data();          // z 2^F, n limbs
		limb_t* Sv = Z + n;                  // S(z) 2^F, n + 1 limbs
		fixed_series::to_fixed(z, static_cast<int64_t>(F), Z, n);
		std::size_t top = n;
		while (top > 0 && Z[top - 1] == 0u) --top;
		if (top == 0) return result;         // z below 2^-F: S = 1
		const int64_t  a = static_cast<int64_t>(F) - (64 * static_cast<int64_t>(top) - std::countl_zero(Z[top - 1]));   // z < 2^-a
		const unsigned N = fixed_series::series_terms(a, F, fixed_series::divisors::arctangent);
		fixed_series::arctangent_series(Sv, Z, n, N, arctangent_block_length(N), a, false);   // 0.7 sqrt(N), measured again for atanh (#1795)
		// S = Sv 2^-F: the top bit of Sv[n] weighs 2^(64 n + 63 - F) = 2^63
		result *= fixed_series::from_fixed<nlimbs>(Sv, n + 1, 63);
		return result;
	}

	// log(j / 512) = 2 atanh(u / w), u = j - 512, w = j + 512, at `bits` bits: |u / w| < 0.18, and
	// the series gains at least 5 bits a term. In fixed point (#1795) a grid point costs 7-10x
	// less than the term-by-term efloat series it replaced (2048 bits: 16-55 us, from 108-456;
	// 8192 bits: 0.2-0.36 ms, from 1.4-3.3 ms), which matters at high precision, where every new
	// j fills its entry once.
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_grid_point(unsigned j, unsigned bits) {
		const int64_t u = static_cast<int64_t>(j) - (int64_t(1) << log_table_shift);
		const int64_t w = static_cast<int64_t>(j) + (int64_t(1) << log_table_shift);
		efloat<nlimbs> v(static_cast<double>(u));
		v.set_precision(bits);
		if (u == 0) return v;
		v /= efloat<nlimbs>(static_cast<double>(w));
		efloat<nlimbs> sum = atanh_working_small(v, bits);
		sum.setexponent(sum.scale() + 1);   // 2 atanh
		return sum;
	}

	// log(j / 512) at `bits` bits, from a per-thread cache that keeps the widest value computed
	// for each grid point, the way ln2() does: computed on first use, then rounded from.
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_table(unsigned j, unsigned bits) {
		constexpr unsigned N = log_table_last - log_table_first + 1;
		thread_local std::vector<efloat<nlimbs>> cached(N);
		thread_local std::vector<unsigned> cached_bits(N, 0u);
		const unsigned i = j - log_table_first;
		if (cached_bits[i] < bits) {
			cached[i] = log_grid_point<nlimbs>(j, bits + 32);
			cached_bits[i] = bits;
		}
		efloat<nlimbs> value(cached[i]);
		value.set_precision(bits);
		return value;
	}

	// log(x) for a finite x > 0 at `working` bits by a table and a series (#1714), the way mpmath
	// evaluates it below 2500 bits:
	//
	//   x = 2^e m with m in [sqrt(1/2), sqrt(2)), c = j / 512 the nearest grid point to m, and
	//   log x = e ln 2 + log c + 2 atanh(v),  v = (m - c) / (m + c),  |v| < 2^-10.
	//
	// Keeping m around 1 rather than in [1, 2) means a result near zero -- x near 1 -- comes from
	// the series alone (c = 1, log c = 0), with no cancellation against e ln 2. The series
	// atanh(v) = v + v^3 / 3 + v^5 / 5 + ... gains ~21 bits per term.
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_table_series(const efloat<nlimbs>& x, unsigned working) {
		efloat<nlimbs> m(x);
		m.set_precision(working);
		m.setexponent(0);
		int64_t e = x.scale();
		if (double(m) > 1.4142135623730951) {   // which side of sqrt(2) a boundary value lands on is immaterial
			m.setexponent(-1);
			++e;
		}
		const unsigned j = static_cast<unsigned>(std::lround(std::ldexp(double(m), static_cast<int>(log_table_shift))));
		efloat<nlimbs> c(std::ldexp(static_cast<double>(j), -static_cast<int>(log_table_shift)));   // exact
		c.set_precision(working);

		efloat<nlimbs> series(0.0);
		series.set_precision(working);
		const efloat<nlimbs> num = m - c;   // exact: m and c are within 2^-10 of each other
		if (!num.iszero()) {
			series = num / (m + c);   // v
			series.set_precision(working);
			series = atanh_working_small(series, working);
			series.setexponent(series.scale() + 1);   // 2 atanh
		}
		efloat<nlimbs> y = series + log_table<nlimbs>(j, working);
		if (e != 0) {
			// e at the working precision: an efloat made from an integer has the type's default
			// precision, too narrow for an exponent past 2^(nlimbs x 32)
			efloat<nlimbs> exponent;
			exponent.set_precision(working);
			exponent = e;
			y += exponent * ln2<nlimbs>(working);
		}
		return y;
	}

	// Guard bits the AGM carries past its working precision: its ~log2(W) iterations each round
	// a product, a root and a mean, and log s ~ W/2 ln 2 is scaled up by pi and m ln 2.
	inline constexpr unsigned log_agm_guard_bits = 16;

	// floor(log2 |log x|), or up to two less, for a finite x > 0, x != 1: in [1/2, 2), from
	// d = x - 1, which is exact there, with |d| / 2 < |log x| < 2 |d|; elsewhere |log x| > ln 2 / 2
	// and a double estimate from m 2^e, m in [1, 2), is good to one in its last place.
	template<unsigned nlimbs>
	inline int64_t log_magnitude(const efloat<nlimbs>& x) {
		const int64_t e = x.scale();
		if (e == 0 || e == -1) {
			const efloat<nlimbs> d = x - efloat<nlimbs>(1.0);
			return d.scale() - 1;
		}
		efloat<nlimbs> m(x);
		m.setexponent(0);
		const double estimate = std::log(double(m)) + static_cast<double>(e) * 0.6931471805599453;
		return std::ilogb(estimate) - 1;
	}

	// log(x) for a finite x > 0, x != 1, at `working` bits by the arithmetic-geometric mean, the
	// way MPFR evaluates it (Brent 1976; Brent and Zimmermann, Modern Computer Arithmetic 4.8.2):
	//
	//   s = x 2^m > 2^(W/2),  log s = pi / (2 AGM(1, 4/s)) + O(log(s) / s^2) = pi s / (2 AGM(s, 4)),
	//   log x = log s - m ln 2.
	//
	// The AGM converges quadratically once its means agree, after ~log2(W) steps of a product, an
	// exact root (sqrt_to) and a halving; the last step stops at the half-way agreement, since the
	// arithmetic mean then differs from the limit by (a - b)^2 / (16 a). log s ~ W/2 ln 2 cancels
	// against m ln 2 down to log x, so W carries those cancelled bits, `cancel`, as well.
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_agm(const efloat<nlimbs>& x, unsigned working) {
		// log s < W ln 2 / 2 + 2^12 for any W this runs at, so its bits above log x are fewer than
		// bit_width(working + 2^12) - log_magnitude(x), plus one for the cancelled bits W carries
		const int64_t top    = std::bit_width(working + 4096u);
		const unsigned cancel = static_cast<unsigned>(std::max<int64_t>(0, top - log_magnitude(x) + 1));
		// a multiple of 64, so that nearby precisions share the cached pi and ln 2
		const unsigned W = (working + cancel + log_agm_guard_bits + 63u) & ~63u;
		const int64_t m = static_cast<int64_t>(W / 2 + 8) - x.scale();
		efloat<nlimbs> s(x);
		s.set_precision(W);
		s.setexponent(s.scale() + m);
		efloat<nlimbs> a(s);
		efloat<nlimbs> b(4.0);
		b.set_precision(W);
		for (;;) {
			const efloat<nlimbs> diff = a - b;
			if (diff.iszero() || diff.scale() <= a.scale() - static_cast<int64_t>(W / 2)) break;
			const efloat<nlimbs> product = a * b;
			a += b;
			a.setexponent(a.scale() - 1);
			b = sqrt_to(product, W);
		}
		a += b;   // 2 AGM(s, 4)
		efloat<nlimbs> y = (pi<nlimbs>(W) * s) / a;
		efloat<nlimbs> exponent;
		exponent.set_precision(W);
		exponent = m;
		y -= exponent * ln2<nlimbs>(W);
		return y;
	}

	// log(x) for a finite x > 0 at `working` bits: the AGM at high precision, unless x lies so
	// close to 1 that the series converges in fewer steps than the AGM would take
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_working(const efloat<nlimbs>& x, unsigned working) {
		constexpr int64_t table_bits = 10;   // the table leaves |v| < 2^-10
		if (static_cast<int64_t>(working) <= log_agm_bits_per_series_bit * table_bits || x == efloat<nlimbs>(1.0)) {
			return log_table_series(x, working);   // log 1 = 0 exactly
		}
		const int64_t near_one = std::max<int64_t>(table_bits, -log_magnitude(x) - 1);   // x within 2^-near_one of 1
		if (static_cast<int64_t>(working) <= log_agm_bits_per_series_bit * near_one) {
			return log_table_series(x, working);
		}
		return log_agm(x, working);
	}

	// log2 x = log x / ln 2 and log10 x = log x / ln 10 for a finite x > 0, unrounded, at `target`
	// bits plus guard bits
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_base_working(const efloat<nlimbs>& x, unsigned target, bool ten) {
		const unsigned working = target + exp_log_guard_bits;
		efloat<nlimbs> y = log_working(x, working);
		y /= ten ? ln10<nlimbs>(working) : ln2<nlimbs>(working);
		return y;
	}

	// log(1 + x) for a finite x > -1, x != 0, unrounded, at `target` bits plus guard bits: 1 + x
	// is formed exactly for |x| < 1, at the working precision plus the 1 - scale(x) bits it spans
	// below 1, and rounded only for an x so large that its log does not cancel
	template<unsigned nlimbs>
	inline efloat<nlimbs> log1p_working(const efloat<nlimbs>& x, unsigned target) {
		const unsigned working = target + exp_log_guard_bits + static_cast<unsigned>(std::max<int64_t>(0, 1 - x.scale()));
		efloat<nlimbs> u(x);
		u.set_precision(working);
		u += 1;
		return log_working(u, working);
	}

	// whether a finite x > 0 is 10^j for an integer j >= 0, and j: 10^j = 5^j 2^j has more than
	// 2.32 j significant bits, so j is at most significant_bits(x) / 2.32
	template<unsigned nlimbs>
	inline bool is_power_of_ten(const efloat<nlimbs>& x, int64_t& j) {
		efloat<nlimbs> m(x);
		m.setexponent(0);
		j = std::llround(std::log10(double(m)) + static_cast<double>(x.scale()) * 0.30102999566398120);
		if (j < 0 || static_cast<double>(j) * 2.32 > static_cast<double>(significant_bits(x)) + 1.0) return false;
		if (j == 0) return x == efloat<nlimbs>(1.0);
		return exact_power(efloat<nlimbs>(10.0), static_cast<uint64_t>(j)) == x;
	}

	// log2, log10 and log1p carry a division or the rounding of 1 + x past log's error
	inline constexpr unsigned log_derived_error_bits = log_error_bits + 4u;

}  // namespace efloat_detail

// Natural logarithm function for efloat: a table and a series (#1714), the arithmetic-geometric
// mean at high precision (#1763)
template<unsigned nlimbs>
constexpr efloat<nlimbs> log(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision()); // maintain dynamic precision contract
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	// Signbit check captures both negative normal values and negative infinity
	if (x.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.isinf()) return x;

	// computed at the working precision -- the result's precision plus guard bits -- and rounded
	// once (#1712)
	// Ziv's strategy (#1763): log x is transcendental for x != 1, and log(1 - d) = -d - d^2/2 - ...
	// for a short d is the common near miss
	const unsigned precision = efloat_detail::result_precision(x);
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::log_working(x, target + efloat_detail::exp_log_guard_bits); },
	                                        precision, efloat_detail::log_error_bits, x.carries_precision());
}

// Logarithm base 2 function for efloat
template<unsigned nlimbs>
constexpr efloat<nlimbs> log2(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision());
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	if (x.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.isinf()) return x;

	// correctly rounded (#1763): exact at a power of 2, and by Ziv's strategy elsewhere, where
	// log2 x is irrational
	const unsigned precision = efloat_detail::result_precision(x);
	if (efloat_detail::significant_bits(x) == 1) {
		efloat<nlimbs> result;
		result.set_precision(64);
		result = x.scale();
		efloat_detail::round_result(result, precision, x.carries_precision());
		return result;
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::log_base_working(x, target, false); },
	                                        precision, efloat_detail::log_derived_error_bits, x.carries_precision());
}

// Logarithm base 10 function for efloat
template<unsigned nlimbs>
constexpr efloat<nlimbs> log10(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision());
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	if (x.sign() == -1) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.isinf()) return x;

	// correctly rounded (#1763): exact at a power of 10, and by Ziv's strategy elsewhere, where
	// log10 x is irrational
	const unsigned precision = efloat_detail::result_precision(x);
	int64_t j = 0;
	if (efloat_detail::is_power_of_ten(x, j)) {
		efloat<nlimbs> result;
		result.set_precision(64);
		result = j;
		efloat_detail::round_result(result, precision, x.carries_precision());
		return result;
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::log_base_working(x, target, true); },
	                                        precision, efloat_detail::log_derived_error_bits, x.carries_precision());
}

// Natural logarithm of 1 + x function for efloat
template<unsigned nlimbs>
constexpr efloat<nlimbs> log1p(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;
	if (x < efloat<nlimbs>(-1.0)) {
		efloat<nlimbs> nan;
		nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x == efloat<nlimbs>(-1.0)) {
		efloat<nlimbs> neg_inf;
		neg_inf.setinf(true);
		if (!std::is_constant_evaluated()) {
			neg_inf.set_precision(x.get_precision());
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return neg_inf;
	}
	if (x.isinf()) {
		if (x.sign() == -1) {
			efloat<nlimbs> nan;
			nan.setnan();
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
			}
			return nan;
		}
		return x;
	}

	// correctly rounded (#1763): log1p x is transcendental for x != 0
	const unsigned precision = efloat_detail::result_precision(x);
	// log(1 + x) = x - x^2/2 + ..., within 2^-(P + 4) of x for |x| < 2^-(P + 4), below it
	if (x.scale() < -static_cast<int64_t>(precision) - 5) {
		return efloat_detail::round_near(x, -1, precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::log1p_working(x, target); },
	                                        precision, efloat_detail::log_derived_error_bits, x.carries_precision());
}

}} // namespace sw::universal
