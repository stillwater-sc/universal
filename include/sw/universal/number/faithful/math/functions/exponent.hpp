#pragma once
// exponent.hpp: exp, exp2, exp10 and expm1 for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Compensated exponentials (#1955), evaluated in faithful arithmetic. Evaluating through the
// FloatingPointType, std::exp(Real(x)), would keep half of the pair's precision.
//
// exp(x) = 2^m exp(r)^512, with x = m ln2 + 512 r:
//   1. m = round(x / ln2), and x - m ln2 computed EXACTLY, in an exact expansion, with ln 2 split
//      into four FloatingPointType parts. A pair product m * ln2 would leave an absolute error of
//      about |m| ulp(ln2's error part), a relative error of several hundred units in the result
//      once m reaches the hundreds.
//   2. exp(r) - 1 by its Taylor series, |r| <= ln2 / 1024, so a handful of terms suffices.
//   3. nine squarings, each (1 + s)^2 - 1 = 2s + s^2, which keep s = exp(r)^(2^i) - 1 relative,
//      as dd's exp does: the error of s is relative to s, small, and the squarings do not amplify
//      it beyond a few units.
//   4. scale by 2^m.
// exp2(x) reduces by the nearest integer n first, exp2(x) = 2^n exp((x - n) ln2), so that the
// product with ln 2 is never large. exp10(x) = 2^m exp(x ln10 - m ln2), with x ln10 - m ln2 formed
// exactly, as exp's reduction: a pair product x ln10 would be off by |x ln10| units, 70 at x = 30.
// expm1(x) sums its Taylor series for |x| < 1/2, where exp(x) - 1 would cancel, and is
// exp(x) - 1 otherwise.
//
// Worst relative error, in units of the pair's epsilon(), over 20000 arguments per range in
// numeric/faithful/exponential.cpp (faithful<float> / faithful<double>):
//   exp 1.03 / 1.06, exp2 0.91 / 1.01, exp10 0.93 / 0.95, expm1 2.4 / 3.1
// exp's range there stops at log(min()), the full-precision floor: below it the error part is
// subnormal and the pair holds fewer digits, as numeric_limits<faithful>::min() states.
//
// Overflow returns +inf with ERANGE, and underflow below the smallest subnormal returns +0 with
// ERANGE, as <cmath>. Only a native FloatingPointType.
#include <cerrno>        // errno, ERANGE
#include <cmath>         // std::floor, std::ldexp, std::log
#include <limits>
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/functions/truncate.hpp>
#include <universal/number/faithful/math/functions/fractional.hpp>
#include <universal/number/faithful/math/constants.hpp>
#include <universal/numerics/twoprod.hpp>

namespace sw { namespace universal {

namespace faithful_detail {
	// x * 2^k, both parts, exact while neither part leaves the normal range
	template<typename Real>
	inline faithful<Real> scale2(const faithful<Real>& x, int k) {
		return faithful<Real>(std::ldexp(x.high(), k), std::ldexp(x.low(), k));
	}

	// x - m ln2, exactly, rounded once to a pair; m an integer of at most a dozen bits
	template<typename Real>
	inline faithful<Real> reduce_ln2(const faithful<Real>& x, Real m) {
		static const split_constant<Real, 4> ln2 = split<Real, 4>(constants::ln2);
		exact_expansion<Real> e;
		e.add(x.low());
		e.add(x.high());
		for (int i = 3; i >= 0; --i) {
			Real p, err;
			twoProd(m, ln2.part[i], p, err);
			e.add(-err);
			e.add(-p);
		}
		return e.to_pair();
	}

	// x c - m ln2, exactly, with c a four-part constant: the product x c has an absolute error
	// of |x c| units in a pair product, which exp turns into the same relative error, so it is
	// formed exactly, part by part, with the reduction
	template<typename Real>
	inline faithful<Real> reduce_scaled_ln2(const faithful<Real>& x, const split_constant<Real, 4>& c, Real m) {
		static const split_constant<Real, 4> ln2 = split<Real, 4>(constants::ln2);
		exact_expansion<Real> e;
		const Real xp[2] = { x.low(), x.high() };
		for (int i = 3; i >= 0; --i) {
			for (const Real& xi : xp) {
				Real p, err;
				twoProd(xi, c.part[i], p, err);
				e.add(err);
				e.add(p);
			}
			Real p, err;
			twoProd(m, ln2.part[i], p, err);
			e.add(-err);
			e.add(-p);
		}
		return e.to_pair();
	}

	// exp(r) - 1 for |r| <= ln2 / 1024, by its Taylor series
	template<typename Real>
	inline faithful<Real> expm1_kernel(const faithful<Real>& r) {
		using F = faithful<Real>;
		const Real tiny = std::ldexp(Real(1), -2 * std::numeric_limits<Real>::digits - 4);
		F s = r, term = r;
		for (int n = 2; n < 40; ++n) {
			term = term * r / F(Real(n));
			s += term;
			if (std::fabs(term.high()) <= tiny * std::fabs(s.high())) break;
		}
		return s;
	}

	// 2^m exp(r) for |r| <= ln2 / 2, r already reduced exactly
	template<typename Real>
	inline faithful<Real> exp_reduced(const faithful<Real>& reduced, Real m) {
		using F = faithful<Real>;
		const F r = scale2(reduced, -9);             // r / 512, exact scaling
		F s = expm1_kernel(r);
		for (int i = 0; i < 9; ++i) s = scale2(s, 1) + s * s;   // (1 + s)^2 - 1
		s += F(Real(1));
		// 2^m in two steps, so that a result near the bottom of the range does not pass through a
		// power of two that underflows
		const int mi = static_cast<int>(m);
		const int half = mi / 2;
		return scale2(scale2(s, half), mi - half);
	}

	// exp(x) for a finite x in the range where the result is neither inf nor zero
	template<typename Real>
	inline faithful<Real> exp_finite(const faithful<Real>& x) {
		static const faithful<Real> ln2 = pair<Real>(constants::ln2);
		const Real m = std::floor(Real(x) / ln2.high() + Real(0.5));
		return exp_reduced(reduce_ln2(x, m), m);
	}
}

// e^x
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> exp(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	if (x.isinf()) return x.sign() ? F(Real(0)) : x;
	if (x.iszero()) return F(Real(1));
	static const Real overflowAt  = std::log(std::numeric_limits<Real>::max());
	static const Real underflowAt = std::log(std::numeric_limits<Real>::denorm_min()) - Real(1);
	const Real xv = Real(x);
	if (xv > overflowAt) {
		errno = ERANGE;
		return F(std::numeric_limits<Real>::infinity());
	}
	if (xv < underflowAt) {
		errno = ERANGE;
		return F(Real(0));
	}
	const F r = faithful_detail::exp_finite(x);
	if (r.isinf()) errno = ERANGE;
	return r;
}

// 2^x
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> exp2(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	if (x.isinf()) return x.sign() ? F(Real(0)) : x;
	if (x.iszero()) return F(Real(1));
	const Real xv = Real(x);
	if (xv >= Real(std::numeric_limits<Real>::max_exponent)) {
		errno = ERANGE;
		return F(std::numeric_limits<Real>::infinity());
	}
	if (xv < Real(std::numeric_limits<Real>::min_exponent - std::numeric_limits<Real>::digits - 1)) {
		errno = ERANGE;
		return F(Real(0));
	}
	static const F ln2 = faithful_detail::pair<Real>(faithful_detail::constants::ln2);
	const F n = round(x);
	const F f = x - n;                               // exact: |f| <= 1/2, Sterbenz on the parts
	const F r = (f.iszero()) ? F(Real(1)) : faithful_detail::exp_finite(f * ln2);
	const int ni = static_cast<int>(Real(n));
	const int half = ni / 2;
	const F result = faithful_detail::scale2(faithful_detail::scale2(r, half), ni - half);
	if (result.isinf()) errno = ERANGE;
	return result;
}

// 10^x
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> exp10(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	static const faithful_detail::split_constant<Real, 4> ln10 = faithful_detail::split<Real, 4>(faithful_detail::constants::ln10);
	static const F ln2 = faithful_detail::pair<Real>(faithful_detail::constants::ln2);
	if (x.isnan() || x.isinf() || x.iszero()) return exp(x);
	// the range checks of exp, on x ln10
	const Real y = Real(x) * ln10.part[0];
	if (!(std::fabs(y) < std::log(std::numeric_limits<Real>::max()))) return exp(F(y));
	const Real m = std::floor(y / ln2.high() + Real(0.5));
	const F r = faithful_detail::exp_reduced(faithful_detail::reduce_scaled_ln2(x, ln10, m), m);
	if (r.isinf()) errno = ERANGE;
	return r;
}

// e^x - 1, accurate for small x
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> expm1(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	if (x.isinf()) return x.sign() ? F(Real(-1)) : x;
	if (x.iszero()) return x;                        // expm1(+-0) = +-0
	const Real ax = std::fabs(Real(x));
	// below 2^-10 the series converges in a few terms as it is, and scaling a tiny x down could
	// reach the subnormals
	if (ax < std::ldexp(Real(1), -10)) return faithful_detail::expm1_kernel(x);
	if (ax < Real(0.5)) {
		// expm1(x) = (1 + k)^(2^9) - 1 with k = expm1(x / 2^9): the squaring of exp, which keeps
		// the error relative to the small result, where exp(x) - 1 would cancel
		F s = faithful_detail::expm1_kernel(faithful_detail::scale2(x, -9));
		for (int i = 0; i < 9; ++i) s = faithful_detail::scale2(s, 1) + s * s;
		return s;
	}
	const F e = exp(x);
	return e.isinf() ? e : e - F(Real(1));
}

}} // namespace sw::universal
