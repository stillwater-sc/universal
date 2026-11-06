#pragma once
// logarithm.hpp: log, log2, log10 and log1p for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Compensated logarithms (#1955), evaluated in faithful arithmetic.
//
// log(x) = k ln2 + log(b), with x = 2^k b and b in [sqrt(1/2), sqrt(2)):
//   1. k and b from the binary exponent, b = x 2^-k exact. Centering b on 1 keeps log(b) small,
//      |log(b)| <= ln2 / 2, so k ln2 + log(b) never cancels: for k = 0 the result is log(b)
//      itself, and for k != 0, |k ln2| >= ln2 > 2 |log(b)|. Taking b in [1, 2) instead would
//      compute log(1 - tiny) as -ln2 + log(2 - 2 tiny), all cancellation.
//   2. log(b) = 2 atanh(s) = 2 (s + s^3/3 + s^5/5 + ...), s = (b - 1) / (b + 1), |s| <= 0.172:
//      the series needs about 10 terms for float and 21 for double, and b - 1 is exact, so a b
//      near 1 keeps its relative accuracy, where a Newton step on exp would cancel.
// log2(x) = k + log(b) / ln2, exact for a power of two. log10(x) = log(x) / ln10.
// log1p(x) = 2 atanh(x / (2 + x)) for |x| < 1/4, where log(1 + x) would lose the bits of x below
// 1's last bit, and log(1 + x) otherwise.
//
// Worst relative error, in units of the pair's epsilon(), over 20000 arguments per range in
// numeric/faithful/exponential.cpp (faithful<float> / faithful<double>):
//   log 2.6 / 2.7 (arguments near 1), log2 1.3 / 1.4, log10 1.8 / 3.7, log1p 2.7 / 3.4
//
// log(0) is -inf with ERANGE, and a negative argument NaN with EDOM, as <cmath>.
// Only a native FloatingPointType.
#include <cerrno>        // errno, EDOM, ERANGE
#include <cmath>         // std::frexp, std::ldexp, std::sqrt
#include <limits>
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/constants.hpp>
#include <universal/number/faithful/math/functions/exponent.hpp>   // faithful_detail::scale2

namespace sw { namespace universal {

namespace faithful_detail {
	// 2 atanh(s) by its series, for |s| <= 0.2
	template<typename Real>
	inline faithful<Real> two_atanh(const faithful<Real>& s) {
		using F = faithful<Real>;
		const Real tiny = std::ldexp(Real(1), -2 * std::numeric_limits<Real>::digits - 4);
		const F s2 = s * s;
		F term = s, sum = s;
		for (int k = 3; k < 200; k += 2) {
			term = term * s2;
			const F t = term / F(Real(k));
			sum += t;
			if (std::fabs(t.high()) <= tiny * std::fabs(sum.high())) break;
		}
		return scale2(sum, 1);
	}

	// log(b) for b in [sqrt(1/2), sqrt(2))
	template<typename Real>
	inline faithful<Real> log_centered(const faithful<Real>& b) {
		using F = faithful<Real>;
		const F one(Real(1));
		return two_atanh((b - one) / (b + one));
	}

	// x = 2^k b with b in [sqrt(1/2), sqrt(2)), for a finite positive x
	template<typename Real>
	inline faithful<Real> center(const faithful<Real>& x, int& k) {
		int e = 0;
		(void)std::frexp(Real(x), &e);               // Real(x) = f 2^e, f in [1/2, 1)
		k = e - 1;                                   // x 2^-k in [1, 2)
		// scale in two steps so a subnormal x is lifted without leaving the range
		faithful<Real> b = scale2(scale2(x, -(k / 2)), -(k - k / 2));
		static const Real sqrt2 = std::sqrt(Real(2));
		if (Real(b) >= sqrt2) {
			b = scale2(b, -1);
			++k;
		}
		return b;
	}

	// the shared special cases; returns true and sets r when x is not a finite positive value
	template<typename Real>
	inline bool log_special(const faithful<Real>& x, faithful<Real>& r) {
		using F = faithful<Real>;
		if (x.isnan()) { r = x; return true; }
		if (x.iszero()) {
			errno = ERANGE;
			r = F(-std::numeric_limits<Real>::infinity());
			return true;
		}
		if (x.sign()) {
			errno = EDOM;
			r = F(std::numeric_limits<Real>::quiet_NaN());
			return true;
		}
		if (x.isinf()) { r = x; return true; }
		return false;
	}
}

// the natural logarithm
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> log(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	F special;
	if (faithful_detail::log_special(x, special)) return special;
	static const F ln2 = faithful_detail::pair<Real>(faithful_detail::constants::ln2);
	int k = 0;
	const F b = faithful_detail::center(x, k);
	const F lb = faithful_detail::log_centered(b);
	return (k == 0) ? lb : F(Real(k)) * ln2 + lb;
}

// the binary logarithm: exact for a power of two
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> log2(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	F special;
	if (faithful_detail::log_special(x, special)) return special;
	static const F ln2 = faithful_detail::pair<Real>(faithful_detail::constants::ln2);
	int k = 0;
	const F b = faithful_detail::center(x, k);
	const F lb = faithful_detail::log_centered(b) / ln2;
	return (k == 0) ? lb : F(Real(k)) + lb;
}

// the decimal logarithm
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> log10(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	F special;
	if (faithful_detail::log_special(x, special)) return special;
	static const F ln10 = faithful_detail::pair<Real>(faithful_detail::constants::ln10);
	return log(x) / ln10;
}

// log(1 + x), accurate for small x
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> log1p(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	if (x.iszero()) return x;                        // log1p(+-0) = +-0
	if (x.isinf()) {
		if (!x.sign()) return x;
		errno = EDOM;
		return F(std::numeric_limits<Real>::quiet_NaN());
	}
	const F one(Real(1));
	if (x == -one) {
		errno = ERANGE;
		return F(-std::numeric_limits<Real>::infinity());
	}
	if (x < -one) {
		errno = EDOM;
		return F(std::numeric_limits<Real>::quiet_NaN());
	}
	if (std::fabs(Real(x)) < Real(0.25)) return faithful_detail::two_atanh(x / (F(Real(2)) + x));
	return log(one + x);
}

}} // namespace sw::universal
