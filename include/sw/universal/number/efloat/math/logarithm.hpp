// logarithm.hpp: logarithmic functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <vector>
#include <universal/number/efloat/math/exponent.hpp>

namespace sw { namespace universal {

namespace efloat_detail {

	// The log table (#1714): log(j / 512) for j = 362 .. 724, the grid points of [sqrt(1/2), sqrt(2)].
	inline constexpr unsigned log_table_shift = 9;
	inline constexpr unsigned log_table_first = 362;   // 362 / 512 < sqrt(1/2)
	inline constexpr unsigned log_table_last  = 724;   // 724 / 512 > sqrt(2)
	// Past this working precision the table and series lose to Newton on exp.
	inline constexpr unsigned log_series_max_bits = 8192;

	// log(j / 512) = 2 atanh(u / w), u = j - 512, w = j + 512, at `bits` bits. Every term update
	// multiplies by u^2 and divides by w^2 and by 2k + 1 -- small integers, one limb each, so each
	// costs O(n) -- and the terms fall by (u / w)^2, at least 5 bits per term at the table's ends.
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_grid_point(unsigned j, unsigned bits) {
		const int64_t u = static_cast<int64_t>(j) - (int64_t(1) << log_table_shift);
		const int64_t w = static_cast<int64_t>(j) + (int64_t(1) << log_table_shift);
		efloat<nlimbs> sum(0.0);
		sum.set_precision(bits);
		if (u == 0) return sum;
		efloat<nlimbs> power(static_cast<double>(u));
		power.set_precision(bits);
		power /= efloat<nlimbs>(static_cast<double>(w));                   // (u / w)^(2k + 1)
		sum = power;
		const efloat<nlimbs> u2(static_cast<double>(u * u));
		const efloat<nlimbs> w2(static_cast<double>(w * w));
		for (unsigned k = 1; ; ++k) {
			power *= u2;
			power /= w2;
			const efloat<nlimbs> term = power / efloat<nlimbs>(static_cast<double>(2 * k + 1));
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 2) break;
			sum += term;
		}
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

	// log(x) for a finite x > 0 at `working` bits by Newton on exp(y) - x = 0, y <- y + x exp(-y) - 1
	// (#1712): the seed carries ~53 bits, x = m 2^e with m in [1, 2), y0 = log(double(m)) + e ln 2;
	// each step doubles the correct bits, so the steps run at doubling precisions, 128, 256, ...,
	// and only the last at the working precision, stopping once the next correction, ~delta^2 / 2,
	// is below it.
	template<unsigned nlimbs>
	inline efloat<nlimbs> log_newton(const efloat<nlimbs>& x, unsigned working) {
		efloat<nlimbs> m(x);
		m.setexponent(0);
		efloat<nlimbs> y = efloat<nlimbs>(std::log(double(m))) + efloat<nlimbs>(x.scale()) * ln2<nlimbs>(working);
		const efloat<nlimbs> one(1.0);
		for (unsigned bits = 128; bits < working; bits *= 2) {
			efloat<nlimbs> xb(x);
			xb.set_precision(bits);
			y.set_precision(bits);
			y += xb / exp(y) - one;
		}
		efloat<nlimbs> xw(x);
		xw.set_precision(working);
		y.set_precision(working);
		for (int i = 0; i < 64; ++i) {
			const efloat<nlimbs> delta = xw / exp(y) - one;
			y += delta;
			if (delta.iszero() || 2 * delta.scale() < y.scale() - static_cast<int64_t>(working)) break;
		}
		return y;
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
			const efloat<nlimbs> v = num / (m + c);
			const efloat<nlimbs> v2 = v * v;
			efloat<nlimbs> power = v;
			series = v;
			// Term k lies ~21 k bits below the sum, so it needs only that many fewer bits: the
			// running power and v^2 are carried at a precision that falls with the terms. Both
			// operands are cut, because a product takes the wider of its operands' precisions.
			efloat<nlimbs> v2_taper = v2;
			unsigned taper = working;
			for (unsigned k = 1; ; ++k) {
				const int64_t below = series.scale() - power.scale();   // bits the power sits below the sum
				const unsigned need = (below + 64 < static_cast<int64_t>(working)) ? static_cast<unsigned>(static_cast<int64_t>(working) - below) : 64u;
				if (need + 32u <= taper) {
					taper = need;
					power.set_precision(taper);
					v2_taper = v2;
					v2_taper.set_precision(taper);
				}
				power *= v2_taper;
				const efloat<nlimbs> term = power / (2 * k + 1);   // stays at the taper (#1757)
				if (term.iszero() || term.scale() < series.scale() - static_cast<int64_t>(working) - 2) break;
				series += term;
			}
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

}  // namespace efloat_detail

// Natural logarithm function for efloat: a table and a series up to log_series_max_bits, Newton
// on exp beyond (#1714)
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
	const unsigned precision = efloat_detail::result_precision(x);
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	efloat<nlimbs> y = (working <= efloat_detail::log_series_max_bits)
	                 ? efloat_detail::log_table_series(x, working)
	                 : efloat_detail::log_newton(x, working);
	efloat_detail::round_result(y, precision, x.carries_precision());
	return y;
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

	// log2(x) = log(x) / ln 2, both at the working precision, rounded once (#1712)
	const unsigned precision = efloat_detail::result_precision(x);
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	efloat<nlimbs> xw(x);
	xw.set_precision(working);
	efloat<nlimbs> result = log(xw) / efloat_detail::ln2<nlimbs>(working);
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
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

	// log10(x) = log(x) / ln 10, both at the working precision, rounded once (#1712)
	const unsigned precision = efloat_detail::result_precision(x);
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	efloat<nlimbs> xw(x);
	xw.set_precision(working);
	efloat<nlimbs> result = log(xw) / efloat_detail::ln10<nlimbs>(working);
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
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

	// both paths are carried at the working precision and rounded once at the end (#1712)
	const unsigned precision = efloat_detail::result_precision(x);
	efloat<nlimbs> xw(x);
	xw.set_precision(precision + efloat_detail::exp_log_guard_bits);

	// Precision-safe range check completely in efloat space (avoids double underflows)
	if (abs(x) < efloat<nlimbs>(0.375)) {
		// Taylor series for log(1 + x) prevents cancellation as x -> 0; at |x| < 0.375 it gains
		// at least 1.4 bits per term, so it runs until the terms fall below the precision
		efloat<nlimbs> sum(0.0);
		efloat<nlimbs> term(xw);
		efloat<nlimbs> x_pow(xw);

		sum += term;
		for (unsigned n = 2; ; ++n) {
			x_pow = x_pow * xw;
			term = x_pow / efloat<nlimbs>(n);
			
			// Dynamic precision-safe termination: stops when next term cannot affect the sum
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(sum.get_precision())) {
				break;
			}
			if (n % 2 == 0) {
				sum -= term;
			} else {
				sum += term;
			}
		}
		efloat_detail::round_result(sum, precision, x.carries_precision());
		return sum;
	} else {
		efloat<nlimbs> result = log(efloat<nlimbs>(1.0) + xw);
		efloat_detail::round_result(result, precision, x.carries_precision());
		return result;
	}
}

}} // namespace sw::universal
