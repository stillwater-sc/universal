#pragma once
// error_and_gamma.hpp: error and gamma functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// All four functions are evaluated in quad-double, to about 2^-200 relative (#1667: they used
// to be computed in double, which is worth 53 of quad-double's 212 bits):
//
//   erf     (2x / sqrt(pi)) e^-x^2 sum_n (2x^2)^n / (2n + 1)!!, a series of positive terms
//           with no cancellation, for |x| < 6; 1 - erfc(x) beyond, where that is exact enough.
//   erfc    1 - erf(x) for |x| < 3, where erfc keeps at least 196 bits; the continued fraction
//           e^-x^2 / sqrt(pi) / (x + (1/2) / (x + 1 / (x + (3/2) / (x + ...)))) for x >= 3,
//           evaluated backward from a depth of 4200 / x^2 + 40 terms (2^-215, measured with
//           mpmath); 2 - erfc(-x) for negative x.
//   lgamma  the Stirling series for x >= 30; its Taylor series at 1 and at 2, within 1/32,
//           where lgamma vanishes and any other form loses its relative accuracy; elsewhere
//           in (0, 30) the shift lgamma(x + n) - log(x (x + 1) ... (x + n - 1)); and the
//           reflection log(pi / |sin(pi x)|) - lgamma(1 - x) for x < 0.
//   tgamma  e^lgamma(x) for x > 0, pi / (sin(pi x) Gamma(1 - x)) for x < 0.
//
// The poles of the gamma function, 0 and the negative integers, give +inf from lgamma and NaN
// from tgamma.  Close to the negative zeros of lgamma (x near -2.457, -2.747, ...) the reflection
// cancels, and the result carries about 2^-200 absolute rather than relative accuracy.
#include <cmath>
#include <universal/number/qd/math/constants/qd_special_constants.hpp>

namespace sw { namespace universal {

// defined in exponent.hpp, logarithm.hpp and trigonometry.hpp, which mathlib.hpp includes after this file
inline qd exp(const qd&);
inline qd log(const qd&);
inline qd sin(const qd&);

namespace qd_special {

// sum_n (2a^2)^n / (2n + 1)!!, a >= 0: every term positive
inline qd erf_series(const qd& a) {
	const qd t = mul_pwr2(sqr(a), 2.0);
	qd term(1.0), sum(1.0);
	for (int n = 1; n < 2000; ++n) {
		term = term * t / qd(static_cast<double>(2 * n + 1));
		sum += term;
		if (term[0] < std::ldexp(sum[0], -216)) break;
	}
	return sum;
}

// erf(a) for 0 <= a < 6
inline qd erf_positive(const qd& a) {
	return qd_2_sqrtpi * a * exp(-sqr(a)) * erf_series(a);
}

// erfc(a) for a >= 3, by the continued fraction
inline qd erfc_cf(const qd& a) {
	const double x = a[0];
	const int depth = static_cast<int>(4200.0 / (x * x)) + 40;
	qd t = a;
	for (int n = depth; n >= 1; --n) t = a + qd(0.5 * n) / t;
	return exp(-sqr(a)) * qd_1_sqrtpi / t;
}

// x is a pole of the gamma function: 0 or a negative integer
inline bool gamma_pole(const qd& x) { return x[0] <= 0.0 && floor(x) == x; }

// lgamma(x) for x >= 30, by the Stirling series
inline qd lgamma_stirling(const qd& x) {
	qd r = (x - qd(0.5)) * log(x) - x + qd_half_log_2pi;
	const qd x2 = sqr(x);
	qd p = x;                                               // x^(2k - 1)
	for (int k = 0; k < qd_stirling_terms; ++k) {
		const qd term = qd_stirling[k] / p;
		r += term;
		if (std::fabs(term[0]) < std::ldexp(std::fabs(r[0]), -216)) break;
		p *= x2;
	}
	return r;
}

// sum_k c_k e^k for k >= 2, with c indexed from k = 2
inline qd lgamma_taylor(const qd* c, const qd& e) {
	qd r(0.0), p = sqr(e);
	for (int k = 0; k < qd_lgamma_taylor_terms; ++k) {
		const qd term = c[k] * p;
		r += term;
		if (std::fabs(term[0]) < std::ldexp(std::fabs(r[0]) + std::fabs(e[0]), -218)) break;
		p *= e;
	}
	return r;
}

// lgamma(x) for x > 0
inline qd lgamma_positive(const qd& x) {
	if (x[0] >= 30.0) return lgamma_stirling(x);
	const qd e1 = x - qd(1.0), e2 = x - qd(2.0);
	if (std::fabs(e1[0]) < 0.03125) return -qd_euler_gamma * e1 + lgamma_taylor(qd_lgamma_taylor_at_1, e1);
	if (std::fabs(e2[0]) < 0.03125) return (qd(1.0) - qd_euler_gamma) * e2 + lgamma_taylor(qd_lgamma_taylor_at_2, e2);
	// lgamma(x) = lgamma(x + n) - log(x (x + 1) ... (x + n - 1)), x + n >= 30
	qd product(1.0), y = x;
	while (y[0] < 30.0) { product *= y; y += qd(1.0); }
	return lgamma_stirling(y) - log(product);
}

// sin(pi x) for a non-integer x, from the exact fractional part
inline qd sin_pi(const qd& x) {
	const qd n = floor(x);
	qd f = x - n;                                           // in (0, 1)
	if (f > qd(0.5)) f = qd(1.0) - f;                       // sin(pi f) = sin(pi (1 - f))
	const qd s = sin(qd_pi * f);
	// sin(pi x) = (-1)^n sin(pi f)
	const bool odd = std::fmod(n[0], 2.0) != 0.0;
	return odd ? -s : s;
}

} // namespace qd_special

// erf(x) = 2 / sqrt(pi) times the integral from 0 to x of e^(-t^2) dt
inline qd erf(const qd& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return qd(0.0);
	const bool negative = x[0] < 0.0;
	const qd a = negative ? -x : x;
	qd r;
	if (a[0] < 6.0) r = qd_special::erf_positive(a);
	else            r = qd(1.0) - qd_special::erfc_cf(a);
	return negative ? -r : r;
}

// erfc(x) = 1 - erf(x), accurate where it is small
inline qd erfc(const qd& x) {
	if (x.isnan()) return x;
	if (x[0] < 0.0) {
		const qd a = -x;
		return qd(1.0) + ((a[0] < 6.0) ? qd_special::erf_positive(a) : qd(1.0) - qd_special::erfc_cf(a));
	}
	if (x[0] < 3.0) return qd(1.0) - qd_special::erf_positive(x);
	if (x[0] > 27.3) return qd(0.0);                        // below the smallest double
	return qd_special::erfc_cf(x);
}

// the logarithm of |Gamma(x)|; +inf at the poles
inline qd lgamma(const qd& x) {
	if (x.isnan()) return x;
	if (x.isinf()) return qd(SpecificValue::infpos);
	if (qd_special::gamma_pole(x)) return qd(SpecificValue::infpos);
	if (x[0] > 0.0) return qd_special::lgamma_positive(x);
	// reflection: Gamma(x) Gamma(1 - x) = pi / sin(pi x)
	const qd s = qd_special::sin_pi(x);
	return log(qd_pi / (s[0] < 0.0 ? -s : s)) - qd_special::lgamma_positive(qd(1.0) - x);
}

// the gamma function; NaN at the poles
inline qd tgamma(const qd& x) {
	if (x.isnan()) return x;
	if (qd_special::gamma_pole(x)) return qd(SpecificValue::qnan);
	if (x[0] > 0.0) {
		if (x[0] > 171.7) return qd(SpecificValue::infpos);  // beyond the largest double
		return exp(qd_special::lgamma_positive(x));
	}
	// reflection: Gamma(x) = pi / (sin(pi x) Gamma(1 - x))
	const qd one_minus = qd(1.0) - x;
	const qd s = qd_special::sin_pi(x);
	if (one_minus[0] > 171.0) {
		// Gamma(1 - x) overflows double, while Gamma(x) itself need not underflow: close to a
		// negative integer sin(pi x) is small too.  Form the quotient in logarithms instead.
		const qd magnitude = exp(log(qd_pi / (s[0] < 0.0 ? -s : s)) - qd_special::lgamma_positive(one_minus));
		return s[0] < 0.0 ? -magnitude : magnitude;
	}
	return qd_pi / (s * exp(qd_special::lgamma_positive(one_minus)));
}

}} // namespace sw::universal
