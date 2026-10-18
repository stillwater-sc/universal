// logarithm.hpp: logarithmic functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/efloat/math/exponent.hpp>

namespace sw { namespace universal {

// Natural logarithm function for efloat using Newton's method on exp(y) - x = 0
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

	// Newton on exp(y) - x = 0, y <- y + x exp(-y) - 1, finishing at the working precision: the
	// result's precision plus guard bits, with ln 2 computed at that precision (#1712).  The seed
	// carries ~53 bits: x = m 2^e with m in [1, 2), y0 = log(double(m)) + e ln 2.  Each step
	// doubles the correct bits, so the steps run at doubling precisions, 128, 256, ..., and only
	// the last at the working precision; that loop stops once the next correction, ~delta^2 / 2,
	// is below the working precision -- as many steps as the precision needs, not a fixed count.
	const unsigned precision = efloat_detail::result_precision(x);
	const unsigned working   = precision + efloat_detail::exp_log_guard_bits;
	efloat<nlimbs> m(x);
	m.setexponent(0);
	efloat<nlimbs> y = efloat<nlimbs>(std::log(double(m))) + efloat<nlimbs>(x.scale()) * efloat_detail::ln2<nlimbs>(working);
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
	y.set_precision(precision);
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
	result.set_precision(precision);
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
	result.set_precision(precision);
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
		sum.set_precision(precision);
		return sum;
	} else {
		efloat<nlimbs> result = log(efloat<nlimbs>(1.0) + xw);
		result.set_precision(precision);
		return result;
	}
}

}} // namespace sw::universal
