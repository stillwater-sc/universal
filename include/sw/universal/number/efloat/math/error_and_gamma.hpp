// error_and_gamma.hpp: error and gamma functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <vector>

namespace sw { namespace universal {

namespace efloat_detail {

	// Guard bits erf and erfc carry past exp_log_guard_bits: their series add up to ~2 x^2 + W / 4
	// rounded terms, and exp(-x^2) and 2 / sqrt(pi) are rounded once each
	inline constexpr unsigned erf_guard_bits = 32u;

	// erf_working(x, P) and erfc_working(x, P) are within 2^(scale - P - exp_log_guard_bits +
	// erf_error_bits) of the function's value
	inline constexpr unsigned erf_error_bits = 0u;

	// 2 / sqrt(pi) at `bits` bits, cached like pi
	template<unsigned nlimbs>
	inline efloat<nlimbs> two_over_sqrt_pi(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			efloat<nlimbs> two(2.0);
			two.set_precision(bits + 32u);
			cached = two / sqrt_to(pi<nlimbs>(bits + 32u), bits + 32u);
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

	// x^2 for a finite x, exactly: carried at twice its significant bits
	template<unsigned nlimbs>
	inline efloat<nlimbs> exact_square(const efloat<nlimbs>& x) {
		efloat<nlimbs> a(x);
		if (a.sign() == -1) a = -a;
		a.set_precision(2u * significant_bits(x) + 32u);
		return a * a;
	}

	// erf |x| at `W` bits for a finite x != 0 with x^2 = `x2` well inside exp's range, by the series
	// of positive terms (Abramowitz and Stegun 7.1.6, the way MPFR's mpfr_erf_0 sums it)
	//
	//   erf x = 2 / sqrt(pi) exp(-x^2) sum_{n>=0} t_n,   t_0 = x,  t_n = t_(n-1) 2 x^2 / (2n + 1),
	//
	// which has no cancellation, unlike the alternating Taylor series. Past n > 2 x^2 the terms fall
	// by more than half each, so the tail beyond a term is below that term.
	template<unsigned nlimbs>
	inline efloat<nlimbs> erf_series(const efloat<nlimbs>& x, const efloat<nlimbs>& x2, unsigned W) {
		efloat<nlimbs> t(x);
		if (t.sign() == -1) t = -t;
		t.set_precision(W);
		efloat<nlimbs> two_x2(x2);
		two_x2.set_precision(W);
		two_x2.setexponent(two_x2.scale() + 1);
		const double n_falling = 2.0 * double(x2);
		efloat<nlimbs> sum(t);
		// Once the terms fall, term n lies (scale(sum) - scale(t)) bits below the sum and needs only
		// that many fewer bits: t and 2 x^2 are carried at a precision that falls with the terms, as
		// log's series does
		efloat<nlimbs> two_x2_taper(two_x2);
		unsigned taper = W;
		for (unsigned n = 1; ; ++n) {
			const int64_t below = sum.scale() - t.scale();
			if (below > 0) {
				const unsigned need = (below + 64 < static_cast<int64_t>(W)) ? static_cast<unsigned>(static_cast<int64_t>(W) - below) : 64u;
				if (need + 32u <= taper) {
					taper = need;
					t.set_precision(taper);
					two_x2_taper = two_x2;
					two_x2_taper.set_precision(taper);
				}
			}
			t *= two_x2_taper;
			t /= 2 * n + 1;
			sum += t;
			if (t.iszero() || (static_cast<double>(n) > n_falling && t.scale() < sum.scale() - static_cast<int64_t>(W) - 2)) break;
		}
		efloat<nlimbs> m(x2);
		m = -m;   // exact
		efloat<nlimbs> y = exp_working(m, W - exp_log_guard_bits) * sum;
		y *= two_over_sqrt_pi<nlimbs>(W);
		y.set_precision(W);
		return y;
	}

	// whether erfc's asymptotic series reaches W bits at x: its smallest term, near n = x^2, lies
	// about x^2 log2(e) bits below its first
	inline bool erfc_asymptotic_reaches(double x2, unsigned W) {
		return x2 * log2_e >= static_cast<double>(W) + 8.0;
	}

	// erfc x for x > 0 at `W` bits by the asymptotic series (DLMF 7.12.1)
	//
	//   erfc x = exp(-x^2) / (x sqrt(pi)) sum_{n>=0} (-1)^n (2n - 1)!! / (2 x^2)^n,
	//
	// whose remainder for a real x is smaller than the first omitted term and of its sign (DLMF
	// 7.12(i)); the terms fall to 2^-(W + 2) before they grow again, past n = x^2, where
	// erfc_asymptotic_reaches
	template<unsigned nlimbs>
	inline efloat<nlimbs> erfc_asymptotic(const efloat<nlimbs>& x, const efloat<nlimbs>& x2, unsigned W) {
		efloat<nlimbs> one(1.0);
		one.set_precision(W);
		efloat<nlimbs> u = one / x2;   // 1 / (2 x^2)
		u.setexponent(u.scale() - 1);
		efloat<nlimbs> term(one), sum(one);
		const double smallest = double(x2);
		for (unsigned n = 1; ; ++n) {
			term *= u;
			term *= -static_cast<int>(2 * n - 1);
			if (term.iszero() || term.scale() < -static_cast<int64_t>(W) - 2 || static_cast<double>(n) > smallest) break;
			sum += term;
		}
		efloat<nlimbs> m(x2);
		m = -m;   // exact
		efloat<nlimbs> xw(x);
		xw.set_precision(W);
		efloat<nlimbs> y = exp_working(m, W - exp_log_guard_bits) * sum;
		y *= two_over_sqrt_pi<nlimbs>(W);
		y /= xw;
		y.setexponent(y.scale() - 1);   // 2 / sqrt(pi) / 2
		return y;
	}

	// erf x, unrounded, for a finite x != 0 at which it does not round as +/-1: erf x is odd
	template<unsigned nlimbs>
	inline efloat<nlimbs> erf_working(const efloat<nlimbs>& x, unsigned target) {
		const unsigned W = target + exp_log_guard_bits + erf_guard_bits;
		efloat<nlimbs> y = erf_series(x, exact_square(x), W);
		if (x.sign() == -1) y = -y;
		return y;
	}

	// erfc x, unrounded, for a finite x whose erfc lies in exp's range: 1 + erf |x| below 0; the
	// asymptotic series where it reaches the working precision; elsewhere 1 - erf x, at the working
	// precision plus the bits it cancels, at most log2(1 / erfc x) < x^2 log2(e) + log2(sqrt(pi)
	// (x + sqrt(x^2 + 2)) / 2) (Abramowitz and Stegun 7.1.13)
	template<unsigned nlimbs>
	inline efloat<nlimbs> erfc_working(const efloat<nlimbs>& x, unsigned target) {
		const unsigned W = target + exp_log_guard_bits + erf_guard_bits;
		efloat<nlimbs> one(1.0);
		const efloat<nlimbs> x2 = exact_square(x);
		if (x.sign() == -1) {
			one.set_precision(W);
			return one + erf_series(x, x2, W);
		}
		const double x2d = double(x2);
		if (erfc_asymptotic_reaches(x2d, W)) return erfc_asymptotic(x, x2, W);
		const double xd = double(x);
		const double lost = x2d * log2_e + std::log2(0.8862269254527580 * (xd + std::sqrt(x2d + 2.0)));
		const unsigned cancel = (lost > 0.0) ? static_cast<unsigned>(std::ceil(lost)) + 2u : 2u;
		one.set_precision(W + cancel);
		efloat<nlimbs> y = one - erf_series(x, x2, W + cancel);
		y.set_precision(W);
		return y;
	}

	// whether erfc |x| < 2^-(P + 5), below which erf x rounds as +/-1 and erfc of a negative x as
	// 2: erfc |x| < exp(-x^2) for |x| >= 1
	template<unsigned nlimbs>
	inline bool erfc_negligible(const efloat<nlimbs>& x, unsigned precision) {
		if (x.scale() >= 31) return true;
		const double ax = std::fabs(double(x));
		return ax >= 1.0 && ax * ax * log2_e >= static_cast<double>(precision) + 6.0;
	}

}  // namespace efloat_detail

// erf: the error function, correctly rounded (#1763): odd, exact at 0 and +/-inf, by Ziv's
// strategy elsewhere, where it is transcendental
template<unsigned nlimbs>
inline efloat<nlimbs> erf(const efloat<nlimbs>& x) {
	if (x.isnan() || x.iszero()) return x;   // erf(+/-0) = +/-0
	const unsigned precision = efloat_detail::result_precision(x);
	efloat<nlimbs> one(x.sign() == -1 ? -1.0 : 1.0);
	if (x.isinf()) {
		efloat_detail::round_result(one, precision, x.carries_precision());
		return one;
	}
	// erf x = +/-(1 - erfc |x|), within 2^-(P + 5) of +/-1 and on 0's side of it
	if (efloat_detail::erfc_negligible(x, precision)) return efloat_detail::round_near(one, -x.sign(), precision, x.carries_precision());
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::erf_working(x, target); },
	                                        precision, efloat_detail::erf_error_bits, x.carries_precision());
}

// erfc: the complementary error function 1 - erf x, correctly rounded (#1763): exact at 0 and
// +/-inf, 0 where exp's range ends, and by Ziv's strategy elsewhere
template<unsigned nlimbs>
inline efloat<nlimbs> erfc(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	const unsigned precision = efloat_detail::result_precision(x);
	if (x.iszero() || x.isinf()) {
		// erfc(+/-0) = 1, erfc(+inf) = +0, erfc(-inf) = 2, exactly, at the result's precision
		efloat<nlimbs> result(x.iszero() ? 1.0 : (x.sign() == -1 ? 2.0 : 0.0));
		efloat_detail::round_result(result, precision, x.carries_precision());
		return result;
	}
	if (x.sign() == -1) {
		// erfc x = 2 - erfc |x|, within 2^-(P + 5) of 2 and below it
		if (efloat_detail::erfc_negligible(x, precision)) return efloat_detail::round_near(efloat<nlimbs>(2.0), -1, precision, x.carries_precision());
	}
	else if (x.scale() >= 31 || efloat_detail::exp_beyond_range(efloat_detail::exact_square(x), efloat_detail::log2_e)) {
		// erfc x < exp(-x^2) is 0 past exp's range
		return efloat_detail::exp_out_of_range<nlimbs>(true, precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::erfc_working(x, target); },
	                                        precision, efloat_detail::erf_error_bits, x.carries_precision());
}

namespace efloat_detail {

	// Stirling's series (DLMF 5.11.1) for a real z > 0:
	//
	//   lgamma(z) = (z - 1/2) log z - z + log(2 pi) / 2 + sum_{k=1}^{m} c_k / z^(2k - 1) + R_m,
	//   c_k = B_2k / (2k (2k - 1)),
	//
	// where the remainder R_m has the sign of the first omitted term and is smaller than it in
	// magnitude (DLMF 5.11(ii)), for every m. The terms are smallest near k = pi z, at about
	// e^(-2 pi z) = 2^(-9.06 z), so z is first shifted up to z0 = 0.15 W, whose smallest term lies
	// 1.36 W bits down, and lgamma(x) = lgamma(x + N) - log(x (x + 1) ... (x + N - 1)). 0.11 W
	// would just reach 2^-W; 0.15 was measured by sweep (#1797), trading shift factors against
	// Stirling terms: at 0.3 lgamma was 5-32% slower, at 0.12 11-16% slower at 128-512 bits and
	// level at 8192. The cost is in the cached coefficients' first fill, which grows with the
	// square of the terms: 1439 terms in 1.1 s at 8192 bits, against 915 in 0.4 s at 0.3.
	inline constexpr double gamma_shift_per_bit = 0.15;

	// lgamma_abs(x, W) is within 2^(E - W + gamma_error_bits) of lgamma x, where E is the scale
	// of its largest part: (z - 1/2) log z, log of the shift's product, or for x < 0 the log of
	// the reflection's sine. Each part is within 2^(E - W + 17) of its value (log's error and the
	// rounding of a product); the shift's grouped product is within 3 N 2^-W <= 2^(E - W + 2) of
	// itself, relative (shift_product), and the tapered Stirling sum within 2^-(W + 5).
	inline constexpr unsigned gamma_error_bits = 22u;

	// log2 of a bound on |c_k / z^(2k - 1)| for z = 2^log2z: log2(2 zeta(2k) (2k)! / ((2 pi)^2k 2k
	// (2k - 1))) - (2k - 1) log2 z, with zeta(2k) <= 2
	inline double stirling_log2_term(unsigned k, double log2z) {
		const double log2_2pi = 2.6514961294723187;
		const double kk = 2.0 * k;
		return 2.0 + std::lgamma(kk + 1.0) / 0.6931471805599453 - kk * log2_2pi - std::log2(kk * (kk - 1.0)) - (kk - 1.0) * log2z;
	}

	// the number of Stirling terms whose first omitted term lies below 2^-(W + 8) at z >= z0
	inline unsigned stirling_terms(double log2z, unsigned W) {
		for (unsigned k = 1; ; ++k) {
			if (stirling_log2_term(k, log2z) < -static_cast<double>(W) - 8.0) return k - 1;
		}
	}

	// c_k = B_2k / (2k (2k - 1)) for k = 1 .. m at `bits` bits, from the tangent numbers T_k
	// (Brent and Harvey 2011; Brent and Zimmermann, Modern Computer Arithmetic 4.7.2):
	//
	//   B_2k = (-1)^(k - 1) 2k T_k / (4^k (4^k - 1)),   c_k = (-1)^(k - 1) T_k / ((2k - 1) 4^k (4^k - 1)).
	//
	// The T_k come from m^2 / 2 updates T_j = (j - k) T_(j - 1) + (j - k + 2) T_j, all of positive
	// numbers by small integers, so each T_k is within 4 m 2^-bits of itself relative; the bits
	// carry 2 bit_width(m) + 8 more for it. Cached per thread like log's table: the widest
	// precision and the most terms computed so far, then rounded from. A refill costs m^2 / 2
	// products, and the working precision varies with x by a few bits (#1797), so a refill takes
	// 1/32 more terms and 64 more bits than asked for, lest such arguments refill it in turn.
	template<unsigned nlimbs>
	inline const std::vector<efloat<nlimbs>>& stirling_coefficients(unsigned m, unsigned bits) {
		thread_local std::vector<efloat<nlimbs>> cached;
		thread_local unsigned cached_bits = 0;
		if (m < cached.size() && bits <= cached_bits) return cached;
		m = std::max<unsigned>(m, static_cast<unsigned>(cached.size() == 0 ? 0 : cached.size() - 1));
		m += m / 32u + 2u;
		bits = std::max(bits, cached_bits) + 64u;
		const unsigned wide = bits + 2u * static_cast<unsigned>(std::bit_width(m)) + 8u;
		std::vector<efloat<nlimbs>> T(m + 1);
		T[1] = 1;
		T[1].set_precision(wide);
		for (unsigned k = 2; k <= m; ++k) T[k] = T[k - 1] * static_cast<int>(k - 1);
		for (unsigned k = 2; k <= m; ++k) {
			for (unsigned j = k; j <= m; ++j) {   // in place: one temporary an update
				T[j] *= static_cast<int>(j - k + 2);
				if (j > k) T[j] += T[j - 1] * static_cast<int>(j - k);
			}
		}
		cached.assign(m + 1, efloat<nlimbs>(0.0));
		for (unsigned k = 1; k <= m; ++k) {
			efloat<nlimbs> d(1.0);   // 4^k - 1, exact
			d.set_precision(2u * k + 8u);
			d.setexponent(2 * static_cast<int64_t>(k));
			d -= 1;
			efloat<nlimbs> c = T[k] / static_cast<int>(2 * k - 1);
			c /= d;
			c.set_precision(bits + 32u);
			c.setexponent(c.scale() - 2 * static_cast<int64_t>(k));
			if (k % 2 == 0) c = -c;
			cached[k] = c;
		}
		cached_bits = bits;
		return cached;
	}

	// log(2 pi) / 2 at `bits` bits, cached
	template<unsigned nlimbs>
	inline efloat<nlimbs> half_log_2pi(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			cached = log_working(pi_scaled<nlimbs>(bits + 32u, 1), bits + 32u);
			cached.setexponent(cached.scale() - 1);
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

	// sum_{k=1}^{m} c_k / z^(2k - 1) at `W` bits for z >= z0, by Horner's scheme in w = 1 / z^2,
	//   S_m = c_m,   S_k = c_k + w S_(k+1),   sum = S_1 / z,
	// one product a term. S_k is about c_k, and c_k / z^(2k - 1) lies d_k bits below the first
	// term, so S_k is formed at W - d_k + 16 bits (tapered, as exp's series is, #1754): its
	// rounding then weighs below 2^-(W + 16) of the first term, under 2^-(W + 5) for all m terms
	template<unsigned nlimbs>
	inline efloat<nlimbs> stirling_sum(const efloat<nlimbs>& z, double log2z, unsigned m, unsigned W) {
		const std::vector<efloat<nlimbs>>& c = stirling_coefficients<nlimbs>(m, W);
		efloat<nlimbs> one(1.0);
		one.set_precision(W);
		const efloat<nlimbs> inv = one / z;
		const efloat<nlimbs> w   = inv * inv;
		const double first = stirling_log2_term(1, log2z);
		auto bits = [&](unsigned k) {
			const double p = static_cast<double>(W) + 16.0 - (first - stirling_log2_term(k, log2z));
			return (p >= W) ? W : std::max(64u, static_cast<unsigned>(p));
		};
		efloat<nlimbs> S(c[m]);
		S.set_precision(bits(m));
		for (unsigned k = m - 1; k >= 1; --k) {
			const unsigned p = bits(k);
			efloat<nlimbs> wk(w);
			wk.set_precision(p);
			S.set_precision(p);
			S *= wk;
			efloat<nlimbs> ck(c[k]);
			ck.set_precision(p);
			S += ck;
		}
		S.set_precision(W);
		return S * inv;
	}

	// x (x + 1) ... (x + N - 1) at `W` bits for an x > 0, the factors after x in groups: the
	// product of x + j0, ..., x + j0 + g - 1 is the polynomial sum_i e_i x^i, whose coefficients
	// are natural numbers below prod (j0 + t + 1) < 2^62, from the powers x^2 .. x^g computed
	// once. A group then costs g products by a 64-bit integer and one full product, not g full
	// products. All its terms are positive, so it is within (2 g + 1) 2^-W of itself, relative,
	// and the whole product within 3 N 2^-W. 8 was measured by sweep (#1797): 4 and 6 were
	// level with it within 2% (1-2% slower at 8192 bits), 12 and 16 1-10% slower; at 0.15 W
	// the product is a small part of the time.
	inline constexpr unsigned gamma_shift_group = 8u;

	template<unsigned nlimbs>
	inline efloat<nlimbs> shift_product(const efloat<nlimbs>& x, int64_t N, unsigned W) {
		efloat<nlimbs> product(x);
		product.set_precision(W);
		const unsigned G = static_cast<unsigned>(std::min<int64_t>(gamma_shift_group, N - 1));
		std::vector<efloat<nlimbs>> power(G + 1);   // x^i at W bits
		if (G >= 1) power[1] = product;
		for (unsigned i = 2; i <= G; ++i) power[i] = power[i - 1] * power[1];
		uint64_t e[gamma_shift_group + 1];
		for (int64_t j0 = 1; j0 < N;) {
			// the coefficients of prod_{t < g} (x + j0 + t), lowest first
			unsigned g = 0;
			uint64_t bound = 1;   // prod (j0 + t + 1), above every coefficient
			e[0] = 1;
			while (g < G && j0 + g < N) {
				const uint64_t a = static_cast<uint64_t>(j0) + g;
				if (bound > (uint64_t(1) << 62) / (a + 1)) break;
				bound *= a + 1;
				e[g + 1] = e[g];
				for (unsigned i = g; i >= 1; --i) e[i] = e[i - 1] + a * e[i];
				e[0] *= a;
				++g;
			}
			efloat<nlimbs> group(power[g]);
			for (unsigned i = g - 1; i >= 1; --i) group += power[i] * e[i];
			group += e[0];
			product *= group;
			j0 += g;
		}
		return product;
	}

	// lgamma x for a finite x > 0 at `W` bits, within 2^(E - W + gamma_error_bits) of it
	template<unsigned nlimbs>
	inline efloat<nlimbs> lgamma_positive(const efloat<nlimbs>& x, unsigned W, int64_t& E) {
		const double z0 = std::max(16.0, std::ceil(gamma_shift_per_bit * W));
		const int64_t N = (x.scale() < 62 && double(x) < z0) ? static_cast<int64_t>(std::ceil(z0 - double(x))) : 0;
		efloat<nlimbs> z(x);
		z.set_precision(W);
		if (N > 0) z += efloat<nlimbs>(N);
		const efloat<nlimbs> L = log_working(z, W);
		efloat<nlimbs> half(0.5);
		efloat<nlimbs> y = (z - half) * L;
		E = y.scale() + 1;
		y -= z;
		y += half_log_2pi<nlimbs>(W);

		// the series, from c_1 / z down, below 1 / (12 z) in all
		const double log2z = (z.scale() < 1000) ? std::log2(double(z)) : static_cast<double>(z.scale());
		const unsigned m = stirling_terms(log2z, W);
		if (m > 0) y += stirling_sum(z, log2z, m, W);

		// lgamma x = lgamma(x + N) - log(x (x + 1) ... (x + N - 1))
		if (N > 0) {
			const efloat<nlimbs> log_product = log_working(shift_product(x, N, W), W);
			if (!log_product.iszero()) E = std::max(E, log_product.scale() + 1);
			y -= log_product;
		}
		return y;
	}

	// lgamma x = log |gamma x| for a finite x that is not 0 or a negative integer, at `W` bits,
	// within 2^(E - W + gamma_error_bits) of it; negative is set where gamma x < 0. Below 0 by
	// the reflection gamma(x) gamma(1 - x) = pi / sin(pi x), with x = n + f, n = round(x) and f
	// exact, so sin(pi x) = (-1)^n sin(pi f) keeps its relative precision near the poles.
	template<unsigned nlimbs>
	inline efloat<nlimbs> lgamma_abs(const efloat<nlimbs>& x, unsigned W, int64_t& E, bool& negative) {
		negative = false;
		if (x.sign() != -1) return lgamma_positive(x, W, E);
		const efloat<nlimbs> n = round(x);
		const efloat<nlimbs> f = x - n;   // exact: |f| <= 1/2 on x's grid
		efloat<nlimbs> n_half(n);
		if (!n_half.iszero()) n_half.setexponent(n_half.scale() - 1);
		negative = is_integer(n_half) == (f.sign() == -1);   // (-1)^n sign(f) < 0
		efloat<nlimbs> pif = pi<nlimbs>(W) * f;
		pif.set_precision(W);
		efloat<nlimbs> s = trig_working(pif, W, trig_function::sine);
		if (s.sign() == -1) s = -s;
		s.set_precision(W);
		const efloat<nlimbs> log_sine = log_working(s, W);
		efloat<nlimbs> t(1.0);   // 1 - x
		t.set_precision(W);
		t -= x;
		int64_t E_reflected = 0;
		efloat<nlimbs> y = log_working(pi<nlimbs>(W), W) - log_sine - lgamma_positive(t, W, E_reflected);
		E = std::max<int64_t>(E_reflected, log_sine.iszero() ? 2 : log_sine.scale() + 1);
		return y;
	}

	// The scale E that lgamma_abs(x, W, E) reports, estimated in double before any pass (#1797):
	// for x > 0 the larger of (z - 1/2) log z and log of the shift's product, about lgamma z +
	// z - lgamma x, at z = x + N; below 0 also the log of the reflection's sine. One bit high
	// rather than low: a low estimate costs a second pass.
	template<unsigned nlimbs>
	inline int64_t lgamma_parts_scale(const efloat<nlimbs>& x, unsigned W) {
		if (x.scale() >= 1000) return x.scale() + 12;   // x log x, log x < 2^11
		const double z0 = std::max(16.0, std::ceil(gamma_shift_per_bit * W));
		auto positive = [z0](double t) {
			const double z = (t < z0) ? t + std::ceil(z0 - t) : t;
			const double main = (z - 0.5) * std::log(z);
			const double product = (t < z0) ? std::fabs(main - z + 0.92 - std::lgamma(t)) : 0.0;
			return std::max(main, product);
		};
		const double xd = double(x);
		double part = 0.0;
		if (x.scale() < -1000) {
			part = positive(1.0) - 0.6931471805599453 * static_cast<double>(x.scale());   // and -log |x|
		} else if (xd > 0.0) {
			part = positive(xd);
		} else {
			const double f = xd - std::nearbyint(xd);
			const double sine = (f == 0.0) ? 0.0 : std::fabs(std::log(std::fabs(std::sin(3.141592653589793 * f))));
			part = std::max(positive(1.0 - xd), sine);
		}
		return (part < 1.0) ? 2 : static_cast<int64_t>(std::floor(std::log2(part))) + 2;
	}

	// the scale of lgamma x for an x at which it is not 0, estimated in double: next to the zeros
	// at 1 and 2 from lgamma(1 + d) ~= -gamma d and lgamma(2 + d) ~= (1 - gamma) d, as a double x
	// would lose d; elsewhere from std::lgamma, which may be far off next to the zeros below 0
	// (lgamma_working retries there)
	template<unsigned nlimbs>
	inline int64_t lgamma_scale_estimate(const efloat<nlimbs>& x) {
		if (x.scale() >= 1000) return x.scale() - 1;
		if (x.scale() < -1000) return static_cast<int64_t>(std::floor(std::log2(-0.6931471805599453 * static_cast<double>(x.scale()))));   // -log |x|
		for (int zero = 1; zero <= 2; ++zero) {
			const efloat<nlimbs> d = x - zero;
			if (d.iszero()) return 0;
			if (d.scale() < -8) return d.scale() - zero;   // gamma = 0.577, 1 - gamma = 0.423
		}
		const double l = std::fabs(std::lgamma(double(x)));
		return (l == 0.0 || !std::isfinite(l)) ? 0 : static_cast<int64_t>(std::floor(std::log2(l)));
	}

	// the working precision lgamma_working predicts for `needed` bits below lgamma x: needed + 8
	// and the cancellation E - scale(lgamma x). E depends on W through the shift, by a bit or so,
	// hence the iteration.
	template<unsigned nlimbs>
	inline int64_t lgamma_precision(const efloat<nlimbs>& x, int64_t needed) {
		const int64_t s = lgamma_scale_estimate(x);
		int64_t W = needed + 8;
		for (int i = 0; i < 3; ++i) W = needed + 8 + std::max<int64_t>(0, lgamma_parts_scale(x, static_cast<unsigned>(W)) - s);
		return W;
	}

	// the working precision tgamma_working predicts for `needed` bits below 1, absolute: needed + 8 + E
	template<unsigned nlimbs>
	inline int64_t tgamma_precision(const efloat<nlimbs>& x, int64_t needed) {
		int64_t W = needed + 8;
		for (int i = 0; i < 3; ++i) W = needed + 8 + std::max<int64_t>(0, lgamma_parts_scale(x, static_cast<unsigned>(W)));
		return W;
	}

	// lgamma x, unrounded, within 2^(scale - target - exp_log_guard_bits) of it, for an x at which
	// it is not 0: the working precision is raised by the cancellation lgamma_abs's parts show, E
	// less the scale of lgamma x. That is predicted (#1797): the shift's own cancellation, about
	// log2 z0 bits, and near the zeros at 1 and 2 the distance from them. Where the prediction
	// falls short, next to the zeros below 0, the pass is repeated with the shortfall added.
	template<unsigned nlimbs>
	inline efloat<nlimbs> lgamma_working(const efloat<nlimbs>& x, unsigned target) {
		const int64_t needed = static_cast<int64_t>(target + exp_log_guard_bits + gamma_error_bits);
		int64_t W = lgamma_precision(x, needed);
		efloat<nlimbs> y;
		for (int retry = 0; retry < 16; ++retry) {
			int64_t E = 0;
			bool negative = false;
			y = lgamma_abs(x, static_cast<unsigned>(W), E, negative);
			const int64_t s = y.iszero() ? E - W : y.scale();
			if (W >= E - s + needed) break;
			W = E - s + needed + 16;   // E grows with W, by the shift, a bit at most
		}
		return y;
	}

	// gamma x = (-1)^negative exp(lgamma x), unrounded, within 2^(scale - target -
	// exp_log_guard_bits + exp_error_bits + 1) of it, for an x whose result lies in exp's range:
	// lgamma x is within 2^-(target + exp_log_guard_bits + 1) of itself, its error in exp's result.
	// That needs W >= E + needed, with E predicted (#1797).
	template<unsigned nlimbs>
	inline efloat<nlimbs> tgamma_working(const efloat<nlimbs>& x, unsigned target) {
		const int64_t needed = static_cast<int64_t>(target + exp_log_guard_bits + gamma_error_bits) + 1;
		int64_t W = tgamma_precision(x, needed);
		efloat<nlimbs> L;
		bool negative = false;
		for (int retry = 0; retry < 4; ++retry) {   // E grows with the shift, which grows with W
			int64_t E = 0;
			L = lgamma_abs(x, static_cast<unsigned>(W), E, negative);
			if (W >= E + needed) break;
			W = E + needed + 16;
		}
		efloat<nlimbs> y = exp_working(L, target);
		if (negative) y = -y;
		return y;
	}

	// (n - 1)! for an integer n >= 1, exactly, while it has at most `limit` bits; false past it
	template<unsigned nlimbs>
	inline bool exact_factorial(int64_t n, unsigned limit, efloat<nlimbs>& result) {
		unsigned bits = 1;
		for (int64_t j = 2; j < n; ++j) {
			bits += static_cast<unsigned>(std::bit_width(static_cast<uint64_t>(j)));
			if (bits > limit) return false;
		}
		result = 1;
		result.set_precision(bits + 32u);
		for (int64_t j = 2; j < n; ++j) result *= efloat<nlimbs>(j);
		return true;
	}

}  // namespace efloat_detail

// tgamma: the Gamma function, correctly rounded (#1763): exact at a positive integer whose
// factorial could lie on a rounding boundary, and by Ziv's strategy elsewhere, from Stirling's
// series and the reflection formula below 0
template<unsigned nlimbs>
inline efloat<nlimbs> tgamma(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	const unsigned precision = efloat_detail::result_precision(x);
	if (x.iszero() || (x.isinf() && x.sign() != -1)) {
		efloat<nlimbs> result;
		result.setinf(x.sign() == -1);   // gamma(+/-0) = +/-inf, gamma(+inf) = +inf
		if (x.iszero()) efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		return result;
	}
	if (x.isinf() || (x.sign() == -1 && efloat_detail::is_integer(x))) {
		efloat<nlimbs> nan;   // gamma(-inf) and the poles at the negative integers
		nan.setnan();
		efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	// (n - 1)! exactly where it has at most twice the bits a result rounds to: a wider factorial
	// has more than precision + 1 bits above its trailing zeros, so it is no rounding boundary
	if (efloat_detail::is_integer(x) && x.scale() < 62) {
		efloat<nlimbs> factorial;
		if (efloat_detail::exact_factorial(efloat_detail::integer_value(x), 2u * (precision + efloat_detail::exp_log_guard_bits), factorial)) {
			efloat_detail::round_result(factorial, precision, x.carries_precision());
			return factorial;
		}
	}
	// 0 or inf where exp's range ends, judged from lgamma x at a coarse precision; for 2^-2^56 <
	// |x| < 2^50, |lgamma x| < 2^57 is well inside it, and that pass is skipped (#1797)
	if (x.scale() >= 50 || x.scale() < -(int64_t(1) << 56)) {
		int64_t E = 0;
		bool negative = false;
		const efloat<nlimbs> estimate = efloat_detail::lgamma_abs(x, 64u, E, negative);
		if (efloat_detail::exp_beyond_range(estimate, efloat_detail::log2_e)) {
			efloat<nlimbs> result = efloat_detail::exp_out_of_range<nlimbs>(estimate.sign() == -1, precision, x.carries_precision());
			if (negative) result = -result;
			return result;
		}
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::tgamma_working(x, target); },
	                                        precision, efloat_detail::exp_error_bits + 1u, x.carries_precision());
}

// lgamma: log |gamma x|, correctly rounded (#1763): exactly 0 at 1 and 2, and by Ziv's strategy
// elsewhere, where it is transcendental
template<unsigned nlimbs>
inline efloat<nlimbs> lgamma(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	const unsigned precision = efloat_detail::result_precision(x);
	if (x.isinf()) {
		efloat<nlimbs> result;
		result.setinf(false);   // lgamma(+/-inf) = +inf
		return result;
	}
	if (x.iszero() || (x.sign() == -1 && efloat_detail::is_integer(x))) {
		efloat<nlimbs> result;   // the poles at 0 and the negative integers
		result.setinf(false);
		efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		return result;
	}
	if (x == efloat<nlimbs>(1.0) || x == efloat<nlimbs>(2.0)) {
		efloat<nlimbs> zero(0.0);
		efloat_detail::round_result(zero, precision, x.carries_precision());
		return zero;
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::lgamma_working(x, target); },
	                                        precision, 0u, x.carries_precision());
}

}} // namespace sw::universal
