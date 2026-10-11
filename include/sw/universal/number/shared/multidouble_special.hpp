#pragma once
// multidouble_special.hpp: hyperbolic, error and gamma functions of the multi-double types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Shared by dd, qd, dd_cascade, td_cascade and qd_cascade (#1667), which evaluated all of these
// in double, worth 53 of their 106 to 212 bits, and had no tgamma or lgamma at all except qd.
// Each function is computed in the type itself, from its own exp, expm1, log, log1p, sqrt and sin,
// to a relative accuracy close to its precision P = numeric_limits<Real>::digits:
//
//   sinh, tanh       at |x|, from expm1 below 1/2, where e^x - e^-x cancels; tanh saturates to 1
//   cosh             (e^|x| + e^-|x|) / 2
//   asinh            log1p(a + a^2 / (1 + sqrt(1 + a^2))) at a = |x|, log(a) + ln 2 when huge
//   acosh            log1p(t + sqrt(t (t + 2))), t = x - 1, which keeps its accuracy near 1
//   atanh            log1p(2a / (1 - a)) / 2 at a = |x|
//   erf              (2x / sqrt(pi)) e^-x^2 sum_n (2x^2)^n / (2n + 1)!!, positive terms, |x| < 6;
//                    1 - erfc(x) beyond
//   erfc             1 - erf(x) below 3; the continued fraction
//                    e^-x^2 / sqrt(pi) / (x + (1/2) / (x + 1 / (x + (3/2) / (x + ...)))) from 3 on,
//                    evaluated backward from a depth of 4200 / x^2 + 40 (2^-215, measured with
//                    mpmath); 2 - erfc(-x) for negative x
//   lgamma           Stirling from 30; its Taylor series at 1 and at 2, within 1/32, where lgamma
//                    vanishes; the shift lgamma(x + n) - log(x (x + 1) ... (x + n - 1)) elsewhere in
//                    (0, 30); the reflection log(pi / |sin(pi x)|) - lgamma(1 - x) for x < 0
//   tgamma           e^lgamma(x) for x > 0, pi / (sin(pi x) Gamma(1 - x)) for x < 0
//
// The poles of the gamma function, 0 and the negative integers, give +inf from lgamma and NaN
// from tgamma.  Close to the negative zeros of lgamma (x near -2.457, -2.747, ...) the reflection
// cancels, and lgamma carries an absolute rather than a relative accuracy of about 2^-P.
//
// Each type's math/functions header calls these from a non-template wrapper, after the type's own
// exp, log, sqrt and sin are declared.
#include <cmath>
#include <limits>
#include <universal/number/shared/multidouble_exact.hpp>
#include <universal/number/shared/multidouble_special_constants.hpp>

namespace sw { namespace universal { namespace multidouble {

	// the relative size of a term that no longer changes a sum of Real precision
	template<typename Real>
	bool negligible(const Real& term, const Real& sum, int extra = 4) {
		return std::fabs(term[0]) < std::ldexp(std::fabs(sum[0]), -(std::numeric_limits<Real>::digits + extra));
	}

	template<typename Real> Real abs_value(const Real& x) { return x[0] < 0.0 ? Real(-x) : x; }

	/////////////////////////////////////////////////////////////////////////////////////////
	// hyperbolic functions

	template<typename Real>
	Real sinh(const Real& x) {
		if (x.isnan() || x.iszero()) return x;
		const bool negative = x[0] < 0.0;
		const Real a = negative ? Real(-x) : x;
		Real r;
		if (a[0] < 0.5) {
			const Real e = expm1(a);                       // e^a - 1, relative-accurate near 0
			r = (e + e / (e + Real(1.0))) * Real(0.5);
		}
		else {
			const Real e = exp(a);
			r = (e - Real(1.0) / e) * Real(0.5);
		}
		return negative ? Real(-r) : r;
	}

	template<typename Real>
	Real cosh(const Real& x) {
		if (x.isnan()) return x;
		const Real e = exp(abs_value(x));
		return (e + Real(1.0) / e) * Real(0.5);
	}

	template<typename Real>
	Real tanh(const Real& x) {
		if (x.isnan() || x.iszero()) return x;
		const bool negative = x[0] < 0.0;
		const Real a = negative ? Real(-x) : x;
		Real t;
		if (a[0] < 0.5) {
			const Real e = expm1(a + a);
			t = e / (e + Real(2.0));
		}
		else if (a[0] > 0.5 * std::numeric_limits<Real>::digits) {
			t = Real(1.0);                                 // 1 - 2 e^-2a, below half an ulp of 1
		}
		else {
			const Real e = exp(a + a);
			t = Real(1.0) - Real(2.0) / (e + Real(1.0));
		}
		return negative ? Real(-t) : t;
	}

	// beyond this, asinh(a) = log(2a) + 1/(4a^2) - ... and the correction is below the precision
	template<typename Real>
	double huge_argument() { return std::ldexp(1.0, std::numeric_limits<Real>::digits / 2 + 4); }

	template<typename Real>
	Real asinh(const Real& x) {
		if (x.isnan() || x.iszero() || x.isinf()) return x;
		const bool negative = x[0] < 0.0;
		const Real a = negative ? Real(-x) : x;
		Real r;
		if (a[0] > huge_argument<Real>()) {
			r = log(a) + constant<Real>(constants::ln2);
		}
		else {
			const Real a2 = a * a;
			r = log1p(a + a2 / (Real(1.0) + sqrt(Real(1.0) + a2)));
		}
		return negative ? Real(-r) : r;
	}

	template<typename Real>
	Real acosh(const Real& x) {
		if (x.isnan()) return x;
		if (x[0] < 1.0) return Real(std::numeric_limits<double>::quiet_NaN());
		if (x.isinf()) return x;
		if (x[0] > huge_argument<Real>()) return log(x) + constant<Real>(constants::ln2);
		const Real t = x - Real(1.0);                      // exact near 1
		if (t[0] < 0.0) return Real(std::numeric_limits<double>::quiet_NaN());
		return log1p(t + sqrt(t * (t + Real(2.0))));
	}

	template<typename Real>
	Real atanh(const Real& x) {
		if (x.isnan() || x.iszero()) return x;
		const bool negative = x[0] < 0.0;
		const Real a = negative ? Real(-x) : x;
		const Real d = Real(1.0) - a;
		if (d[0] < 0.0) return Real(std::numeric_limits<double>::quiet_NaN());
		if (d.iszero()) return Real(negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity());
		const Real r = log1p((a + a) / d) * Real(0.5);
		return negative ? Real(-r) : r;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// error functions

	namespace special_detail {

		// sum_n (2a^2)^n / (2n + 1)!!, a >= 0: every term positive
		template<typename Real>
		Real erf_series(const Real& a) {
			const Real t = (a * a) * Real(2.0);
			Real term(1.0), sum(1.0);
			for (int n = 1; n < 4000; ++n) {
				term = term * t / Real(static_cast<double>(2 * n + 1));
				sum += term;
				if (negligible(term, sum)) break;
			}
			return sum;
		}

		// erf(a) for 0 <= a < 6
		template<typename Real>
		Real erf_positive(const Real& a) {
			const Real two_over_sqrtpi = constant<Real>(constants::one_over_sqrtpi) * Real(2.0);
			return two_over_sqrtpi * a * exp(-(a * a)) * erf_series(a);
		}

		// erfc(a) for a >= 3, by the continued fraction
		template<typename Real>
		Real erfc_cf(const Real& a) {
			const double x = a[0];
			const int depth = static_cast<int>(4200.0 / (x * x)) + 40;
			Real t = a;
			for (int n = depth; n >= 1; --n) t = a + Real(0.5 * n) / t;
			return exp(-(a * a)) * constant<Real>(constants::one_over_sqrtpi) / t;
		}

	}  // namespace special_detail

	template<typename Real>
	Real erf(const Real& x) {
		if (x.isnan() || x.iszero()) return x;
		if (x.isinf()) return Real(x[0] < 0.0 ? -1.0 : 1.0);
		const bool negative = x[0] < 0.0;
		const Real a = negative ? Real(-x) : x;
		Real r;
		if (a[0] < 6.0)       r = special_detail::erf_positive(a);
		else if (a[0] > 27.3) r = Real(1.0);                  // erfc below the smallest double
		else                  r = Real(1.0) - special_detail::erfc_cf(a);
		return negative ? Real(-r) : r;
	}

	template<typename Real>
	Real erfc(const Real& x) {
		if (x.isnan()) return x;
		if (x.isinf()) return Real(x[0] < 0.0 ? 2.0 : 0.0);
		if (x[0] < 0.0) {
			const Real a = -x;
			if (a[0] < 6.0)  return Real(1.0) + special_detail::erf_positive(a);
			if (a[0] > 27.3) return Real(2.0);
			return Real(2.0) - special_detail::erfc_cf(a);
		}
		if (x[0] < 3.0)  return Real(1.0) - special_detail::erf_positive(x);
		if (x[0] > 27.3) return Real(0.0);                    // below the smallest double
		return special_detail::erfc_cf(x);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// gamma functions

	namespace special_detail {

		// 0 or a negative integer
		template<typename Real>
		bool gamma_pole(const Real& x) { return x[0] <= 0.0 && multidouble::floor(x) == x; }

		// lgamma(x) for x >= 30, by the Stirling series
		template<typename Real>
		Real lgamma_stirling(const Real& x) {
			Real r = (x - Real(0.5)) * log(x) - x + constant<Real>(constants::half_log_2pi);
			const Real x2 = x * x;
			Real p = x;                                        // x^(2k - 1)
			for (int k = 0; k < constants::stirling_terms; ++k) {
				const Real term = constant<Real>(constants::stirling[k]) / p;
				r += term;
				if (negligible(term, r)) break;
				p *= x2;
			}
			return r;
		}

		// sum_k c_k e^k for k >= 2, with c indexed from k = 2
		template<typename Real>
		Real lgamma_taylor(const double (*c)[4], const Real& e) {
			Real r(0.0), p = e * e;
			const Real scale = abs_value(e);
			for (int k = 0; k < constants::lgamma_taylor_terms; ++k) {
				const Real term = constant<Real>(c[k]) * p;
				r += term;
				if (std::fabs(term[0]) < std::ldexp(std::fabs(r[0]) + scale[0], -(std::numeric_limits<Real>::digits + 6))) break;
				p *= e;
			}
			return r;
		}

		// lgamma(x) for x > 0
		template<typename Real>
		Real lgamma_positive(const Real& x) {
			if (x[0] >= 30.0) return lgamma_stirling(x);
			const Real gamma = constant<Real>(constants::euler_gamma);
			const Real e1 = x - Real(1.0), e2 = x - Real(2.0);
			if (std::fabs(e1[0]) < 0.03125) return -gamma * e1 + lgamma_taylor(constants::lgamma_taylor_at_1, e1);
			if (std::fabs(e2[0]) < 0.03125) return (Real(1.0) - gamma) * e2 + lgamma_taylor(constants::lgamma_taylor_at_2, e2);
			// lgamma(x) = lgamma(x + n) - log(x (x + 1) ... (x + n - 1)), x + n >= 30
			Real product(1.0), y = x;
			while (y[0] < 30.0) { product *= y; y += Real(1.0); }
			return lgamma_stirling(y) - log(product);
		}

		// sin(pi x) for a non-integer x, from the exact fractional part
		template<typename Real>
		Real sin_pi(const Real& x) {
			const Real n = multidouble::floor(x);
			Real f = x - n;                                    // in (0, 1)
			if (f > Real(0.5)) f = Real(1.0) - f;              // sin(pi f) = sin(pi (1 - f))
			const Real s = sin(constant<Real>(constants::pi) * f);
			// sin(pi x) = (-1)^n sin(pi f): the parity of n, from its limbs
			bool odd = false;
			for (int i = 0; i < limbs<Real>; ++i) {
				if (std::fabs(n[i]) < 9007199254740992.0 && std::fmod(n[i], 2.0) != 0.0) odd = !odd;
			}
			return odd ? Real(-s) : s;
		}

	}  // namespace special_detail

	// the logarithm of |Gamma(x)|; +inf at the poles
	template<typename Real>
	Real lgamma(const Real& x) {
		if (x.isnan()) return x;
		if (x.isinf()) return Real(std::numeric_limits<double>::infinity());
		if (special_detail::gamma_pole(x)) return Real(std::numeric_limits<double>::infinity());
		if (x[0] > 0.0) return special_detail::lgamma_positive(x);
		// reflection: Gamma(x) Gamma(1 - x) = pi / sin(pi x)
		const Real s = special_detail::sin_pi(x);
		return log(constant<Real>(constants::pi) / abs_value(s)) - special_detail::lgamma_positive(Real(Real(1.0) - x));
	}

	// the gamma function; NaN at the poles
	template<typename Real>
	Real tgamma(const Real& x) {
		if (x.isnan()) return x;
		if (x.isinf()) return x[0] > 0.0 ? x : Real(std::numeric_limits<double>::quiet_NaN());
		if (special_detail::gamma_pole(x)) return Real(std::numeric_limits<double>::quiet_NaN());
		if (x[0] > 0.0) {
			if (x[0] > 171.7) return Real(std::numeric_limits<double>::infinity());   // beyond the largest double
			return exp(special_detail::lgamma_positive(x));
		}
		// reflection: Gamma(x) = pi / (sin(pi x) Gamma(1 - x))
		const Real pi = constant<Real>(constants::pi);
		const Real one_minus = Real(1.0) - x;
		const Real s = special_detail::sin_pi(x);
		if (one_minus[0] > 171.0) {
			// Gamma(1 - x) overflows double, while Gamma(x) itself need not underflow: close to a
			// negative integer sin(pi x) is small too.  Form the quotient in logarithms instead.
			const Real magnitude = exp(log(pi / abs_value(s)) - special_detail::lgamma_positive(one_minus));
			return s[0] < 0.0 ? Real(-magnitude) : magnitude;
		}
		return pi / (s * exp(special_detail::lgamma_positive(one_minus)));
	}

}}}  // namespace sw::universal::multidouble
