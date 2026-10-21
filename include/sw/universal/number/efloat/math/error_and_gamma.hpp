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

// erf: Error function
template<unsigned nlimbs>
inline efloat<nlimbs> erf(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	return efloat<nlimbs>(std::erf(double(x)));
}

// erfc: Complementary error function
template<unsigned nlimbs>
inline efloat<nlimbs> erfc(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	return efloat<nlimbs>(std::erfc(double(x)));
}

namespace efloat_detail {

	// Stirling's series (DLMF 5.11.1) for a real z > 0:
	//
	//   lgamma(z) = (z - 1/2) log z - z + log(2 pi) / 2 + sum_{k=1}^{m} c_k / z^(2k - 1) + R_m,
	//   c_k = B_2k / (2k (2k - 1)),
	//
	// where the remainder R_m has the sign of the first omitted term and is smaller than it in
	// magnitude (DLMF 5.11(ii)), for every m. The terms are smallest near k = pi z, at about
	// e^(-2 pi z) = 2^(-9.06 z), so z is first shifted up to z0 = 0.3 W, whose smallest term lies
	// 2.7 W bits down, and lgamma(x) = lgamma(x + N) - log(x (x + 1) ... (x + N - 1)).
	inline constexpr double gamma_shift_per_bit = 0.3;

	// lgamma_abs(x, W) is within 2^(E - W + gamma_error_bits) of lgamma x, where E is the scale
	// of its largest part: (z - 1/2) log z, log of the shift's product, or for x < 0 the log of
	// the reflection's sine. Each part is within 2^(E - W + 17) of its value (log's error and the
	// rounding of a product); the shift's N factors add N 2^-W <= 2^(E - W) relative to the product.
	inline constexpr unsigned gamma_error_bits = 22u;

	// the number of Stirling terms whose first omitted term lies below 2^-(W + 8) at z >= z0, for
	// z = 2^log2z: log2 |c_k / z^(2k - 1)| <= log2(2 zeta(2k) (2k)! / ((2 pi)^2k 2k (2k - 1))) -
	// (2k - 1) log2 z, with zeta(2k) <= 2
	inline unsigned stirling_terms(double log2z, unsigned W) {
		const double log2_2pi = 2.6514961294723187;
		for (unsigned k = 1; ; ++k) {
			const double kk = 2.0 * k;
			const double log2_term = 2.0 + std::lgamma(kk + 1.0) / 0.6931471805599453 - kk * log2_2pi - std::log2(kk * (kk - 1.0)) - (kk - 1.0) * log2z;
			if (log2_term < -static_cast<double>(W) - 8.0) return k - 1;
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
	// precision and the most terms computed so far, then rounded from.
	template<unsigned nlimbs>
	inline const std::vector<efloat<nlimbs>>& stirling_coefficients(unsigned m, unsigned bits) {
		thread_local std::vector<efloat<nlimbs>> cached;
		thread_local unsigned cached_bits = 0;
		if (m < cached.size() && bits <= cached_bits) return cached;
		m = std::max<unsigned>(m, static_cast<unsigned>(cached.size() == 0 ? 0 : cached.size() - 1));
		bits = std::max(bits, cached_bits);
		const unsigned wide = bits + 2u * static_cast<unsigned>(std::bit_width(m)) + 8u;
		std::vector<efloat<nlimbs>> T(m + 1);
		T[1] = 1;
		T[1].set_precision(wide);
		for (unsigned k = 2; k <= m; ++k) T[k] = T[k - 1] * static_cast<int>(k - 1);
		for (unsigned k = 2; k <= m; ++k) {
			for (unsigned j = k; j <= m; ++j) T[j] = T[j - 1] * static_cast<int>(j - k) + T[j] * static_cast<int>(j - k + 2);
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
		if (m > 0) {
			const std::vector<efloat<nlimbs>>& c = stirling_coefficients<nlimbs>(m, W);
			efloat<nlimbs> one(1.0);
			one.set_precision(W);
			efloat<nlimbs> power = one / z;   // 1 / z^(2k - 1)
			const efloat<nlimbs> inv2 = power * power;
			efloat<nlimbs> series(0.0);
			series.set_precision(W);
			for (unsigned k = 1; k <= m; ++k) {
				efloat<nlimbs> ck(c[k]);
				ck.set_precision(W);
				series += ck * power;
				if (k < m) power *= inv2;
			}
			y += series;
		}

		// lgamma x = lgamma(x + N) - log(x (x + 1) ... (x + N - 1))
		if (N > 0) {
			efloat<nlimbs> product(x);
			product.set_precision(W);
			for (int64_t j = 1; j < N; ++j) {
				efloat<nlimbs> factor(x);
				factor.set_precision(W);
				factor += efloat<nlimbs>(j);
				product *= factor;
			}
			const efloat<nlimbs> log_product = log_working(product, W);
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

	// lgamma x, unrounded, within 2^(scale - target - exp_log_guard_bits) of it, for an x at which
	// it is not 0: the working precision is raised by the cancellation its value shows, up to the
	// distance of x from the zeros at 1, 2 and below 0, as trig's argument reduction is
	template<unsigned nlimbs>
	inline efloat<nlimbs> lgamma_working(const efloat<nlimbs>& x, unsigned target) {
		const int64_t needed = static_cast<int64_t>(target + exp_log_guard_bits + gamma_error_bits);
		unsigned W = target + exp_log_guard_bits + gamma_error_bits + 8u;
		efloat<nlimbs> y;
		for (int retry = 0; retry < 16; ++retry) {
			int64_t E = 0;
			bool negative = false;
			y = lgamma_abs(x, W, E, negative);
			const int64_t s = y.iszero() ? E - static_cast<int64_t>(W) : y.scale();
			if (static_cast<int64_t>(W) >= E - s + needed) break;
			W = static_cast<unsigned>(std::max<int64_t>(2 * static_cast<int64_t>(W), E - s + needed + 8));
		}
		return y;
	}

	// gamma x = (-1)^negative exp(lgamma x), unrounded, within 2^(scale - target -
	// exp_log_guard_bits + exp_error_bits + 1) of it, for an x whose result lies in exp's range:
	// lgamma x is within 2^-(target + exp_log_guard_bits + 1) of itself, its error in exp's result
	template<unsigned nlimbs>
	inline efloat<nlimbs> tgamma_working(const efloat<nlimbs>& x, unsigned target) {
		const int64_t needed = static_cast<int64_t>(target + exp_log_guard_bits + gamma_error_bits) + 1;
		int64_t W = needed + 8;
		efloat<nlimbs> L;
		bool negative = false;
		for (int retry = 0; retry < 4; ++retry) {   // E grows with the shift, which grows with W
			int64_t E = 0;
			L = lgamma_abs(x, static_cast<unsigned>(W), E, negative);
			if (W >= E + needed) break;
			W = E + needed + 8;
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
	// 0 or inf where exp's range ends, judged from lgamma x at a coarse precision
	int64_t E = 0;
	bool negative = false;
	const efloat<nlimbs> estimate = efloat_detail::lgamma_abs(x, 64u, E, negative);
	if (efloat_detail::exp_beyond_range(estimate, efloat_detail::log2_e)) {
		efloat<nlimbs> result = efloat_detail::exp_out_of_range<nlimbs>(estimate.sign() == -1, precision, x.carries_precision());
		if (negative) result = -result;
		return result;
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
