#pragma once
// error_and_gamma.hpp: error and gamma functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// erf, erfc, tgamma and lgamma were stubs through double, accurate to 53 bits in every
// configuration (#2017). They are computed in ereal arithmetic now:
//
//   erf    the series of positive terms (Abramowitz and Stegun 7.1.6), which does not cancel;
//          past the point where erfc is below the precision, 1 - erfc by the asymptotic series
//   erfc   the asymptotic series (DLMF 7.12.1) where it reaches the precision; otherwise
//          1 - erf in a wider configuration that absorbs the cancellation, or where that is
//          not wide enough the continued fraction (DLMF 7.9.2); 1 + erf |x| below 0
//   lgamma Stirling's series (DLMF 5.11.1) at z >= z0, shifted up from x by the recurrence,
//          and below 0 the reflection formula with sin(pi x) taken from the exact x - round(x)
//   tgamma (n - 1)! for a positive integer n, otherwise +/-exp(lgamma x)
//
// The algorithms follow efloat's (#1763), which are correctly rounded by Ziv's strategy at a
// precision chosen per call. ereal's precision is fixed by the configuration, so these are
// computed at it, with the limb budget's extra limbs as guard; they are not correctly rounded.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>
#include <universal/number/einteger/core.hpp>

namespace sw { namespace universal {

namespace ereal_detail {

	// ereal<n, F> from ereal<m, F>: the limbs summed smallest first, which rounds only when
	// the target holds fewer bits
	template<unsigned n, unsigned m, typename FpType>
	inline ereal<n, FpType> convert_limbs(const ereal<m, FpType>& v) {
		ereal<n, FpType> r(0.0);
		const auto& limbs = v.limbs();
		for (std::size_t i = limbs.size(); i-- > 0; ) {
			if (limbs[i] != FpType(0)) r += ereal<n, FpType>(limbs[i]);
		}
		if (r.iszero() && v.signbit()) r = -r;
		return r;
	}

	// the most limbs a configuration with this limb type can have
	template<typename FpType>
	inline constexpr unsigned max_ereal_limbs() {
		return static_cast<unsigned>((-(std::numeric_limits<FpType>::min_exponent - 1)) / std::numeric_limits<FpType>::digits);
	}

	// the wider configuration erfc's 1 - erf runs in: twice the limbs, so that it can lose
	// the configuration's precision to cancellation and keep the rest
	template<unsigned maxlimbs, typename FpType>
	inline constexpr unsigned erfc_wide_limbs() {
		return std::min(2u * maxlimbs + 1u, max_ereal_limbs<FpType>());
	}

	// log2(1 / erfc x) ~= x^2 log2(e): the bits 1 - erf x cancels, and where erfc's asymptotic
	// series has fallen to before its terms grow again
	inline double erfc_bits(double x2) { return x2 * 1.4426950408889634; }

	// 2 / sqrt(pi)
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> two_over_sqrt_pi() {
		using Real = ereal<maxlimbs, FpType>;
		static const Real v = Real(2.0) / sqrt(ereal_pi<maxlimbs, FpType>());
		return v;
	}

	// exp(-x^2) = 2^-n exp(n ln 2 - x^2), with exp's argument within ln(2)/2 of 0: an
	// exp(-x^2) near 2^-577 would keep only the bits above the limb type's floor, 445 of them
	// for double limbs, where the product with erf's series, near 2^577, needs them all
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> scaled_exp_minus(const ereal<maxlimbs, FpType>& x2, long long& n) {
		using Real = ereal<maxlimbs, FpType>;
		n = static_cast<long long>(std::floor(double(x2) / 0.6931471805599453 + 0.5));
		return exp(Real(static_cast<double>(n)) * ereal_ln2<maxlimbs, FpType>() - x2);
	}

	// erf x for a finite x > 0 by the series of positive terms (Abramowitz and Stegun 7.1.6):
	//   erf x = 2 / sqrt(pi) exp(-x^2) sum_{n>=0} t_n,   t_0 = x,  t_n = t_(n-1) 2 x^2 / (2n + 1)
	// Past n > x^2 the terms fall by more than half each, so the tail beyond a term is below it.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> erf_series(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		const int precision = series_precision_bits<maxlimbs, FpType>();
		const Real x2 = x * x;
		const Real two_x2 = x2 * Real(2.0);
		const double x2d = double(x2);
		Real term = x;
		Real sum = x;
		const long long cap = static_cast<long long>(4.0 * x2d) + 4LL * precision + 16;
		for (long long n = 1; n < cap; ++n) {
			term = term * two_x2 / Real(static_cast<double>(2 * n + 1));
			sum += term;
			if (static_cast<double>(n) > x2d && series_converged(term, sum, precision)) break;
		}
		long long n{ 0 };
		const Real e = scaled_exp_minus(x2, n);
		return ldexp(two_over_sqrt_pi<maxlimbs, FpType>() * e * sum, static_cast<int>(-n));
	}

	// whether erfc's asymptotic series reaches the configuration's precision at x^2
	template<unsigned maxlimbs, typename FpType>
	inline bool erfc_asymptotic_reaches(double x2) {
		return erfc_bits(x2) > static_cast<double>(series_precision_bits<maxlimbs, FpType>() + 16);
	}

	// whether erf's series, whose sum grows as exp(x^2) ~= 2^(x^2 log2 e), would leave the
	// limb type's range at x^2. There erfc x is below 2^-(max_exponent - 16), under the
	// precision of 1, and the asymptotic series' error is relative to erfc, so 1 - erfc is
	// accurate even where that series does not reach the configuration's precision: float
	// limbs near x = 9.5 and ereal<19> near x = 27 (#2027 review)
	template<typename FpType>
	inline bool erf_series_overflows(double x2) {
		return erfc_bits(x2) > static_cast<double>(std::numeric_limits<FpType>::max_exponent - 16);
	}

	// erfc x for x > 0 by the asymptotic series (DLMF 7.12.1):
	//   erfc x = exp(-x^2) / (x sqrt(pi)) sum_{n>=0} (-1)^n (2n - 1)!! / (2 x^2)^n
	// whose remainder is smaller than the first omitted term; used only where the terms fall
	// below the precision before they grow again, past n = x^2
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> erfc_asymptotic(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		const int precision = series_precision_bits<maxlimbs, FpType>();
		const Real x2 = x * x;
		const Real e = exp(-x2);
		if (e.iszero()) return Real(0.0);             // below the limb type's range
		const Real two_x2 = x2 * Real(2.0);
		const double x2d = double(x2);
		Real term(1.0), sum(1.0);
		for (long long n = 1; static_cast<double>(n) < x2d; ++n) {
			term = -term * Real(static_cast<double>(2 * n - 1)) / two_x2;
			sum += term;
			if (series_converged(term, sum, precision)) break;
		}
		return e * sum * two_over_sqrt_pi<maxlimbs, FpType>() / (x * Real(2.0));
	}

	// the iterations the continued fraction below needs at x for the configuration's
	// precision: its error after n steps falls as exp(-2 x sqrt(2 n))
	template<unsigned maxlimbs, typename FpType>
	inline double erfc_fraction_steps(double x) {
		const double b = static_cast<double>(series_precision_bits<maxlimbs, FpType>()) * 0.6931471805599453 / (2.0 * x);
		return 0.5 * b * b;
	}

	// erfc x for x > 0 by the continued fraction (DLMF 7.9.2), evaluated by Lentz's method:
	//   erfc x = exp(-x^2) / sqrt(pi) / (x + (1/2) / (x + 1 / (x + (3/2) / (x + ...))))
	// It does not cancel, and converges faster the larger x is: it covers the range where
	// 1 - erf cancels more than the wide configuration can absorb, where it converges in a
	// few thousand steps.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> erfc_continued_fraction(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		const int precision = series_precision_bits<maxlimbs, FpType>();
		const Real x2 = x * x;
		const Real e = exp(-x2);
		if (e.iszero()) return Real(0.0);
		Real f = x, C = x, D(0.0);
		const Real one(1.0);
		for (long long n = 1; n < 200000; ++n) {
			const Real a(0.5 * static_cast<double>(n));
			D = x + a * D;
			D = one / D;
			C = x + a / C;
			const Real delta = C * D;
			f *= delta;
			if (series_converged(delta - one, one, precision)) break;
		}
		return e * two_over_sqrt_pi<maxlimbs, FpType>() / (f * Real(2.0));
	}

	// erfc x for a finite x > 0 that is not past the limb type's range
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> erfc_positive(const ereal<maxlimbs, FpType>& x) {
		const double xd = double(x);
		const double x2d = xd * xd;
		if (erfc_asymptotic_reaches<maxlimbs, FpType>(x2d) || erf_series_overflows<FpType>(x2d)) return erfc_asymptotic(x);

		constexpr unsigned W = erfc_wide_limbs<maxlimbs, FpType>();
		using Wide = ereal<W, FpType>;
		// 1 - erf x in the wide configuration, where its extra bits cover the cancellation
		const double headroom = static_cast<double>(series_precision_bits<W, FpType>() - series_precision_bits<maxlimbs, FpType>() - 16);
		// otherwise the continued fraction where it is quick, and failing both, 1 - erf with the
		// cancellation it costs: only the widest configurations, with no wider one to run in,
		// get there, and only for x below about 8
		if (erfc_bits(x2d) >= headroom && erfc_fraction_steps<maxlimbs, FpType>(xd) < 2000.0) {
			return erfc_continued_fraction(x);
		}
		const Wide xw = convert_limbs<W>(x);
		return convert_limbs<maxlimbs>(Wide(1.0) - erf_series(xw));
	}

	// Stirling's series for lgamma at z >= z0 needs its smallest term, near k = pi z, at
	// about 2^(-9.06 z) below the precision: z0 = 0.15 W, as efloat measured (#1797)
	template<unsigned maxlimbs, typename FpType>
	inline constexpr int stirling_z0() {
		return static_cast<int>(0.15 * series_precision_bits<maxlimbs, FpType>()) + 4;
	}

	// a value m 2^e with m in [1/2, 1): Stirling's coefficients grow as (2k)! / (2 pi)^2k and
	// the powers of 1/z fall as fast, so for the widest configurations both leave the limb
	// type's range long before their product does
	template<unsigned maxlimbs, typename FpType>
	struct scaled_value {
		ereal<maxlimbs, FpType> m;
		int e;
	};

	template<unsigned maxlimbs, typename FpType>
	inline void normalize(scaled_value<maxlimbs, FpType>& v) {
		if (v.m.iszero()) return;
		const int k = ilogb(v.m) + 1;
		v.m = ldexp(v.m, -k);
		v.e += k;
	}

	// c_k = B_2k / (2k (2k - 1)) for k = 1 .. m, from the tangent numbers T_k, which are
	// integers and computed exactly (Brent and Harvey 2011):
	//   c_k = (-1)^(k - 1) T_k / ((2k - 1) 4^k (4^k - 1))
	// computed once per configuration, the quotient formed in einteger and kept scaled
	template<unsigned maxlimbs, typename FpType>
	inline const std::vector<scaled_value<maxlimbs, FpType>>& stirling_coefficients() {
		using Int    = einteger<std::uint32_t>;
		using Scaled = scaled_value<maxlimbs, FpType>;
		static const std::vector<Scaled> c = [] {
			// enough terms for z0: the smallest term is near k = pi z0
			const int m = static_cast<int>(3.2 * stirling_z0<maxlimbs, FpType>()) + 8;
			std::vector<Int> T(static_cast<std::size_t>(m) + 1);
			T[1] = Int(1);
			for (int k = 2; k <= m; ++k) T[k] = T[k - 1] * Int(k - 1);
			for (int k = 2; k <= m; ++k) {
				for (int j = k; j <= m; ++j) T[j] = T[j - 1] * Int(j - k) + T[j] * Int(j - k + 2);
			}
			const int keep = series_precision_bits<maxlimbs, FpType>() + 64;
			std::vector<Scaled> out;
			out.reserve(static_cast<std::size_t>(m));
			for (int k = 1; k <= m; ++k) {
				Int four_k(1);
				four_k <<= 2 * k;
				const Int d = Int(2 * k - 1) * (four_k - Int(1));   // (2k - 1)(4^k - 1)
				// q = T_k 2^s / d with keep + 2 bits, then c_k = q 2^-(s + 2k)
				const int s = keep + 2 - (T[k].findMsb() - d.findMsb());
				Int q = T[k];
				if (s >= 0) q <<= s; else q >>= -s;
				q /= d;
				const int top = q.findMsb();
				Scaled v{ einteger_to_ereal<maxlimbs, FpType>(q, -(top + 1), keep), top + 1 - s - 2 * k };
				if (!(k & 1)) v.m = -v.m;
				out.push_back(v);
			}
			return out;
		}();
		return c;
	}

	// log(2 pi) / 2
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> half_log_two_pi() {
		using Real = ereal<maxlimbs, FpType>;
		static const Real v = log(ereal_pi<maxlimbs, FpType>() * Real(2.0)) * Real(0.5);
		return v;
	}

	// lgamma x for a finite x > 0:
	//   lgamma z = (z - 1/2) log z - z + log(2 pi) / 2 + sum_k c_k / z^(2k - 1)
	// at z = x + N >= z0, and lgamma x = lgamma z - log(x (x + 1) ... (x + N - 1))
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> lgamma_positive(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		const int precision = series_precision_bits<maxlimbs, FpType>();
		const double z0 = static_cast<double>(stirling_z0<maxlimbs, FpType>());
		const double xd = double(x);
		const long long N = (xd < z0) ? static_cast<long long>(std::ceil(z0 - xd)) : 0;

		// log of the shift's product, taken in pieces before the product leaves the range
		Real log_product(0.0);
		if (N > 0) {
			const int limit = std::numeric_limits<FpType>::max_exponent / 2;
			Real product = x;
			for (long long j = 1; j < N; ++j) {
				product *= (x + Real(static_cast<double>(j)));
				if (ilogb(product) > limit) { log_product += log(product); product = Real(1.0); }
			}
			log_product += log(product);
		}
		const Real z = x + Real(static_cast<double>(N));

		// the Stirling sum, from c_1 / z down; the powers of 1/z are kept scaled like c_k
		const auto& c = stirling_coefficients<maxlimbs, FpType>();
		const Real w = Real(1.0) / (z * z);
		scaled_value<maxlimbs, FpType> zp{ Real(1.0) / z, 0 };
		normalize(zp);
		Real series(0.0);
		for (std::size_t k = 0; k < c.size(); ++k) {
			const Real term = ldexp(c[k].m * zp.m, c[k].e + zp.e);
			series += term;
			if (series_converged(term, series, precision + 8)) break;
			zp.m *= w;
			normalize(zp);
		}
		const Real half(0.5);
		return (z - half) * log(z) - z + half_log_two_pi<maxlimbs, FpType>() + series - log_product;
	}

	// whether x, finite, is a negative integer or zero: a pole of gamma
	template<unsigned maxlimbs, typename FpType>
	inline bool is_gamma_pole(const ereal<maxlimbs, FpType>& x) {
		return x.iszero() || (x.isneg() && is_integer(x));
	}

	// lgamma x = log |gamma x| for a finite x that is not a pole; `negative` reports gamma x < 0.
	// Below 1/2 the reflection gamma(x) gamma(1 - x) = pi / sin(pi x), with sin(pi x) =
	// (-1)^n sin(pi f) for n = round(x) and the exact f = x - n, which keeps its precision near
	// the poles.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> lgamma_abs(const ereal<maxlimbs, FpType>& x, bool& negative) {
		using Real = ereal<maxlimbs, FpType>;
		negative = false;
		if (!x.isneg()) return lgamma_positive(x);
		const Real n = round(x);
		const Real f = x - n;                         // exact, in [-1/2, 1/2]
		const Real pi = ereal_pi<maxlimbs, FpType>();
		Real s = sin(pi * f);
		const bool odd = is_odd_integer(n);
		if (odd) s = -s;                              // sin(pi x)
		negative = s.isneg();                         // gamma(1 - x) > 0
		return log(pi) - log(abs(s)) - lgamma_positive(Real(1.0) - x);
	}

}  // namespace ereal_detail

	// erf: the error function. Odd; exact at 0 and +/-inf.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> erf(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		if (x.isnan() || x.iszero()) return x;        // erf(+/-0) = +/-0 (C99 F.10.5.1)
		if (x.isinf()) return x.isneg() ? Real(-1.0) : Real(1.0);
		const Real ax = abs(x);
		const double ad = double(ax);
		const double a2 = ad * ad;
		Real r = (ereal_detail::erfc_asymptotic_reaches<maxlimbs, FpType>(a2) || ereal_detail::erf_series_overflows<FpType>(a2))
		             ? Real(1.0) - ereal_detail::erfc_asymptotic(ax)
		             : ereal_detail::erf_series(ax);
		return x.isneg() ? -r : r;
	}

	// erfc: the complementary error function 1 - erf x, without its cancellation for x > 0
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> erfc(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		if (x.isnan()) return x;
		if (x.iszero()) return Real(1.0);
		if (x.isinf()) return x.isneg() ? Real(2.0) : Real(0.0);   // C99 F.10.5.2
		if (x.isneg()) return Real(1.0) + erf(-x);
		return ereal_detail::erfc_positive(x);
	}

	// lgamma: log |gamma x|. +inf at the poles and at +/-inf (C99 F.10.5.3), exactly 0 at 1 and 2.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> lgamma(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		if (x.isnan()) return x;
		if (x.isinf()) return ereal_detail::signed_inf<maxlimbs, FpType>(false);
		if (ereal_detail::is_gamma_pole(x)) return ereal_detail::signed_inf<maxlimbs, FpType>(false);
		if (x == Real(1.0) || x == Real(2.0)) return Real(0.0);
		bool negative{ false };
		return ereal_detail::lgamma_abs(x, negative);
	}

	// tgamma: the Gamma function. +/-inf at +/-0, NaN at a negative integer and at -inf,
	// +inf at +inf (C99 F.10.5.4); (n - 1)! at a positive integer n.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> tgamma(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;
		if (x.isnan()) return x;
		if (x.iszero()) return ereal_detail::signed_inf<maxlimbs, FpType>(x.signbit());
		if (x.isinf()) return x.isneg() ? ereal_detail::quiet_nan<maxlimbs, FpType>() : x;
		if (x.isneg() && ereal_detail::is_integer(x)) return ereal_detail::quiet_nan<maxlimbs, FpType>();
		if (ereal_detail::is_integer(x) && double(x) <= 200.0) {
			// (n - 1)!, exact while it fits the configuration
			const long long n = static_cast<long long>(double(x));
			Real f(1.0);
			for (long long k = 2; k < n; ++k) {
				f *= Real(static_cast<double>(k));
				if (f.isinf()) return f;
			}
			return f;
		}
		// exp(lgamma x) carries lgamma's absolute error as a relative one, which grows with
		// |lgamma x|: near 0, where lgamma(1e-20) is 46, tgamma(x + 1) / x, whose lgamma is
		// near 0, keeps the bits
		if (!x.isneg() && x < Real(1.0)) return tgamma(x + Real(1.0)) / x;
		bool negative{ false };
		const Real lg = ereal_detail::lgamma_abs(x, negative);
		const Real r = exp(lg);
		return negative ? -r : r;
	}

}} // namespace sw::universal
