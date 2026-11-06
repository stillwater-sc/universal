#pragma once
// hyperbolic.hpp: sinh, cosh, tanh, asinh, acosh and atanh for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Compensated hyperbolic functions (#1957), built on the compensated exp, expm1, log and log1p of
// #1955, in forms that do not cancel:
//   sinh(x)  = (s + s / (1 + s)) / 2 with s = expm1(|x|): no e^x - e^-x near 0
//   cosh(x)  = (e + 1 / e) / 2 with e = exp(|x|)
//   tanh(x)  = s / (s + 2) with s = expm1(2|x|)
//   asinh(x) = log1p(|x| + x^2 / (1 + sqrt(1 + x^2))), and log(2|x|) once x^2 would overflow
//   acosh(x) = log1p(t + sqrt(2t + t^2)) with t = x - 1, exact, and log(2x) for a large x
//   atanh(x) = log1p(2|x| / (1 - |x|)) / 2
// sinh and cosh past the overflow point of exp use exp(|x|/2)^2 / 2, so a result just below
// max() is not lost to an intermediate e^|x| that overflows.
//
// Worst relative error, in units of the pair's epsilon(), over 20000 arguments per range in
// numeric/faithful/hyperbolic_and_power.cpp (faithful<float> / faithful<double>):
//   sinh 2.7 / 3.1, cosh 1.0 / 1.0, tanh 3.5 / 3.9, asinh 3.1 / 3.5, acosh 3.6 / 3.6 (near 1),
//   atanh 2.9 / 3.8 (gcc and clang)
//
// Errors follow <cmath>: acosh(x < 1) and atanh(|x| > 1) are NaN with EDOM, or throw
// faithful_domain_error with FAITHFUL_THROW_ARITHMETIC_EXCEPTION set; atanh(+-1) is +-inf
// with ERANGE, and an overflowing sinh or cosh is inf with ERANGE. Only a native FloatingPointType.
#include <cerrno>        // errno, EDOM, ERANGE
#include <cmath>
#include <limits>
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/constants.hpp>
#include <universal/number/faithful/math/functions/exponent.hpp>
#include <universal/number/faithful/math/functions/logarithm.hpp>
#include <universal/number/faithful/math/functions/sqrt.hpp>
#include <universal/number/faithful/math/functions/trigonometry.hpp>   // faithful_detail::domain_error

namespace sw { namespace universal {

namespace faithful_detail {
	// e^|x| / 2 for |x| past exp's overflow point: exp(|x| / 2)^2 / 2, with |x| / 2 exact
	template<typename Real>
	inline faithful<Real> half_exp_large(const faithful<Real>& ax) {
		const faithful<Real> h = exp(scale2(ax, -1));
		const faithful<Real> r = scale2(h, -1) * h;
		if (r.isinf()) errno = ERANGE;
		return r;
	}
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> sinh(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	const F ax = abs(x);
	static const Real overflowAt = std::log(std::numeric_limits<Real>::max());
	F r;
	if (Real(ax) > overflowAt) {
		r = faithful_detail::half_exp_large(ax);
	}
	else {
		const F s = expm1(ax);
		r = faithful_detail::scale2(s + s / (F(Real(1)) + s), -1);
	}
	return x.sign() ? -r : r;
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> cosh(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	if (x.isinf()) return abs(x);
	if (x.iszero()) return F(Real(1));
	const F ax = abs(x);
	static const Real overflowAt = std::log(std::numeric_limits<Real>::max());
	if (Real(ax) > overflowAt) return faithful_detail::half_exp_large(ax);
	const F e = exp(ax);
	return faithful_detail::scale2(e + F(Real(1)) / e, -1);
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> tanh(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || x.iszero()) return x;
	const F one(Real(1));
	// beyond (digits + 2) ln2, 1 - tanh(|x|) = 2 / (e^2|x| + 1) is below half the pair's epsilon
	static const Real saturateAt = Real(std::numeric_limits<Real>::digits + 2) * Real(0.6931471805599453);
	const F ax = abs(x);
	F r;
	if (x.isinf() || Real(ax) > saturateAt) {
		r = one;
	}
	else {
		const F s = expm1(faithful_detail::scale2(ax, 1));
		r = s / (s + F(Real(2)));
	}
	return x.sign() ? -r : r;
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> asinh(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	static const F ln2 = faithful_detail::pair<Real>(faithful_detail::constants::ln2);
	const F ax = abs(x);
	const F one(Real(1));
	F r;
	// past 2^(digits + 1), sqrt(1 + x^2) = |x| to the pair's precision, and x^2 overflows sooner
	if (Real(ax) > std::ldexp(Real(1), std::numeric_limits<Real>::digits + 1)) {
		r = log(ax) + ln2;
	}
	else {
		const F x2 = ax * ax;
		r = log1p(ax + x2 / (one + sqrt(one + x2)));
	}
	return x.sign() ? -r : r;
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> acosh(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	const F one(Real(1));
	if (x < one) return faithful_detail::domain_error<Real>("acosh");
	if (x.isinf()) return x;
	static const F ln2 = faithful_detail::pair<Real>(faithful_detail::constants::ln2);
	if (Real(x) > std::ldexp(Real(1), std::numeric_limits<Real>::digits + 1)) return log(x) + ln2;
	const F t = x - one;                             // exact: x >= 1
	return log1p(t + sqrt(faithful_detail::scale2(t, 1) + t * t));
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> atanh(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || x.iszero()) return x;
	const F one(Real(1));
	const F ax = abs(x);
	if (one < ax) return faithful_detail::domain_error<Real>("atanh");
	if (ax == one) {
		errno = ERANGE;
		return F(x.sign() ? -std::numeric_limits<Real>::infinity() : std::numeric_limits<Real>::infinity());
	}
	const F r = faithful_detail::scale2(log1p(faithful_detail::scale2(ax, 1) / (one - ax)), -1);
	return x.sign() ? -r : r;
}

}} // namespace sw::universal
