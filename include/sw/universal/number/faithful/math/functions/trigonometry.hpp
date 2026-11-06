#pragma once
// trigonometry.hpp: sin, cos, tan and their inverses for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Compensated trigonometric functions (#1956), evaluated in faithful arithmetic.
//
// sin, cos and tan reduce the argument to t in [-pi/4, pi/4] and a quadrant with the shared
// Payne-Hanek reduction of the multi-double types (#1723): x (2/pi) modulo 4 from as many bits of
// 2/pi as the exponent of x needs, in a 640-bit fixed-point accumulator. The reduced argument
// therefore keeps the pair's precision for any finite x, up to max(), and next to a multiple of
// pi/2, where subtracting k pi/2 with pi/2 to the pair's own precision would lose the bits of the
// distance. Each part of a faithful converts to a double exactly, so the reduction takes the two
// parts as its limbs; a small adapter rebuilds the reduced argument as a faithful, splitting each
// 53-bit chunk into FloatingPointType parts exactly.
// sin(t) and cos(t) then sum their Taylor series, |t| <= pi/4, about 9 terms for float and 14 for
// double; neither series cancels there, cos(t) >= 0.7.
//
// atan2(y, x) takes one Newton step from the FloatingPointType's atan2, a0:
//   a1 = a0 + (y cos a0 - x sin a0) / (x cos a0 + y sin a0),
// the angle of (x, y) rotated by -a0, which doubles the bits of a0. atan(x) = atan2(x, 1),
// asin(x) = atan2(x, sqrt((1 - x)(1 + x))) and acos(x) = atan2(sqrt((1 - x)(1 + x)), x); the
// factored 1 - x^2 keeps its relative accuracy near |x| = 1.
//
// Worst relative error, in units of the pair's epsilon(), over 20000 arguments per range in
// numeric/faithful/trigonometry.cpp (faithful<float> / faithful<double>):
//   sin, cos on [-pi, pi], [-1e4, 1e4], up to max() and next to k pi/2   2.1 / 2.9
//   tan on [-pi, pi], [-1e4, 1e4], up to max() and next to k pi/2       3.6 / 3.5
//   asin, acos (also next to +-1)                                         2.5 / 3.0
//   atan, atan2 (all four quadrants)                                      2.2 / 2.1
//
// Errors follow <cmath>: sin, cos and tan of an infinity, and asin and acos beyond [-1, 1], are
// NaN with EDOM; with FAITHFUL_THROW_ARITHMETIC_EXCEPTION set they throw faithful_domain_error.
// Only hosts whose values convert to double exactly: float, double, and a long double that has
// double's format. The reduction's limbs are doubles, so a wider long double part would lose
// bits or overflow. atan2 follows std's special cases. Only a native FloatingPointType.
#include <algorithm>     // std::max
#include <cerrno>        // errno, EDOM
#include <cmath>         // std::atan2, std::ldexp
#include <limits>
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/constants.hpp>
#include <universal/number/faithful/math/functions/classify.hpp>   // signbit
#include <universal/number/faithful/math/functions/exponent.hpp>   // faithful_detail::scale2
#include <universal/number/faithful/math/functions/sqrt.hpp>
#include <universal/number/shared/multidouble_reduce_pio2.hpp>
#include <universal/number/shared/multidouble_special_constants.hpp>

namespace sw { namespace universal {

namespace faithful_detail {
	// The reduction takes each part of the argument as a double limb, so it serves a
	// FloatingPointType whose values convert to double exactly: float and double, and long double
	// where it has double's format (MSVC, macOS ARM64). A wider long double part has more bits than
	// a double and can exceed DBL_MAX, so there faithful<long double> has no trigonometric
	// functions rather than wrong ones (#1956 review). The test of the property, not of the type,
	// is deliberate: a long double that is a double works correctly.
	template<typename Real>
	concept trig_host = std::is_floating_point_v<Real>
		&& std::numeric_limits<Real>::digits <= std::numeric_limits<double>::digits
		&& std::numeric_limits<Real>::max_exponent <= std::numeric_limits<double>::max_exponent;

	namespace constants {
		// pi / 2: the shared table's pi, halved, which is exact
		inline constexpr double pi_2[4] = {
			multidouble::constants::pi[0] / 2, multidouble::constants::pi[1] / 2,
			multidouble::constants::pi[2] / 2, multidouble::constants::pi[3] / 2 };
	}

	// The arithmetic the Payne-Hanek reduction needs, Real(double), +=, * and unary minus, over a
	// faithful. A double converts exactly: to one part for a double host, and to three float parts
	// for a float host, 24 + 24 + 5 bits, summed into the pair.
	template<typename Real>
	struct reduction_real {
		faithful<Real> v;
		reduction_real() : v(Real(0), Real(0)) {}
		explicit reduction_real(const faithful<Real>& f) : v(f) {}
		reduction_real(double d) {
			if constexpr (std::numeric_limits<Real>::digits >= std::numeric_limits<double>::digits) {
				v = faithful<Real>(static_cast<Real>(d), Real(0));
			}
			else {
				const Real p1 = static_cast<Real>(d);
				const double r1 = d - static_cast<double>(p1);       // exact
				const Real p2 = static_cast<Real>(r1);
				const Real p3 = static_cast<Real>(r1 - static_cast<double>(p2));
				v = faithful<Real>(p1, Real(0)) + faithful<Real>(p2, Real(0)) + faithful<Real>(p3, Real(0));
			}
		}
		reduction_real& operator+=(const reduction_real& r) { v += r.v; return *this; }
		reduction_real operator*(const reduction_real& r) const { return reduction_real(v * r.v); }
		reduction_real operator-() const { return reduction_real(-v); }
	};

	// x = q pi/2 + t, t in [-pi/4, pi/4]; returns q modulo 4 in {-1, 0, 1, 2}
	template<typename Real>
	inline int reduce_pio2(const faithful<Real>& x, faithful<Real>& t) {
		static const faithful<Real> pio4 = scale2(pair<Real>(constants::pi_2), -1);
		if (!(pio4 < abs(x))) { t = x; return 0; }
		static const reduction_real<Real> pi_2(pair<Real>(constants::pi_2));
		const double limbs[2] = { static_cast<double>(x.high()), static_cast<double>(x.low()) };
		reduction_real<Real> r;
		const int q = multidouble_reduction::reduce_pio2<reduction_real<Real>, 2>(limbs, pi_2, r);
		t = r.v;
		return q;
	}

	// the Taylor series of sin and cos for |t| <= pi/4
	template<typename Real>
	inline faithful<Real> sin_kernel(const faithful<Real>& t) {
		using F = faithful<Real>;
		const Real tiny = std::ldexp(Real(1), -2 * std::numeric_limits<Real>::digits - 4);
		const F t2 = t * t;
		F term = t, sum = t;
		for (int n = 3; n < 60; n += 2) {
			term = -(term * t2) / F(Real((n - 1) * n));
			sum += term;
			if (std::fabs(term.high()) <= tiny * std::fabs(sum.high())) break;
		}
		return sum;
	}
	template<typename Real>
	inline faithful<Real> cos_kernel(const faithful<Real>& t) {
		using F = faithful<Real>;
		const Real tiny = std::ldexp(Real(1), -2 * std::numeric_limits<Real>::digits - 4);
		const F t2 = t * t;
		F term(Real(1)), sum(Real(1));
		for (int n = 2; n < 60; n += 2) {
			term = -(term * t2) / F(Real((n - 1) * n));
			sum += term;
			if (std::fabs(term.high()) <= tiny) break;
		}
		return sum;
	}

	// sin and cos of x from one reduction
	template<typename Real>
	inline void sincos(const faithful<Real>& x, faithful<Real>& s, faithful<Real>& c) {
		faithful<Real> t;
		const int q = reduce_pio2(x, t);
		const faithful<Real> st = sin_kernel(t), ct = cos_kernel(t);
		switch (q) {
		case 0:  s = st;  c = ct;  break;
		case 1:  s = ct;  c = -st; break;
		case 2:  s = -st; c = -ct; break;
		default: s = -ct; c = st;  break;   // q == -1
		}
	}

	// a domain error: a throw when FAITHFUL_THROW_ARITHMETIC_EXCEPTION is set, else NaN with EDOM
	template<typename Real>
	inline faithful<Real> domain_error(const char* function) {
#if FAITHFUL_THROW_ARITHMETIC_EXCEPTION
		throw faithful_domain_error(function);
#else
		(void)function;
		errno = EDOM;
		return faithful<Real>(std::numeric_limits<Real>::quiet_NaN(), Real(0));
#endif
	}
}

template<typename FloatingPointType>
	requires faithful_detail::trig_host<FloatingPointType>
inline faithful<FloatingPointType> sin(const faithful<FloatingPointType>& x) {
	if (x.isnan()) return x;
	if (x.isinf()) return faithful_detail::domain_error<FloatingPointType>("sin");
	if (x.iszero()) return x;                        // sin(+-0) = +-0
	faithful<FloatingPointType> s, c;
	faithful_detail::sincos(x, s, c);
	return s;
}

template<typename FloatingPointType>
	requires faithful_detail::trig_host<FloatingPointType>
inline faithful<FloatingPointType> cos(const faithful<FloatingPointType>& x) {
	if (x.isnan()) return x;
	if (x.isinf()) return faithful_detail::domain_error<FloatingPointType>("cos");
	faithful<FloatingPointType> s, c;
	faithful_detail::sincos(x, s, c);
	return c;
}

template<typename FloatingPointType>
	requires faithful_detail::trig_host<FloatingPointType>
inline faithful<FloatingPointType> tan(const faithful<FloatingPointType>& x) {
	if (x.isnan()) return x;
	if (x.isinf()) return faithful_detail::domain_error<FloatingPointType>("tan");
	if (x.iszero()) return x;
	faithful<FloatingPointType> s, c;
	faithful_detail::sincos(x, s, c);
	return s / c;
}

// the angle of (x, y), in [-pi, pi]
template<typename FloatingPointType>
	requires faithful_detail::trig_host<FloatingPointType>
inline faithful<FloatingPointType> atan2(const faithful<FloatingPointType>& y, const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || y.isnan()) return F(std::numeric_limits<Real>::quiet_NaN(), Real(0));
	static const F pi_2 = faithful_detail::pair<Real>(faithful_detail::constants::pi_2);
	static const F pi = faithful_detail::scale2(pi_2, 1);
	static const F pi_4 = faithful_detail::scale2(pi_2, -1);
	const bool yneg = signbit(y), xneg = signbit(x);
	// the special cases of std::atan2
	if (y.iszero()) {
		if (xneg) return yneg ? -pi : pi;           // atan2(+-0, -0 or x < 0) = +-pi
		return y;                                    // atan2(+-0, +0 or x > 0) = +-0
	}
	if (x.isinf() || y.isinf()) {
		F a;
		if (x.isinf() && y.isinf()) a = xneg ? pi - pi_4 : pi_4;
		else if (x.isinf()) a = xneg ? pi : F(Real(0));
		else a = pi_2;
		return yneg ? -a : a;
	}
	if (x.iszero()) return yneg ? -pi_2 : pi_2;
	// scale (x, y) by a common power of two, which leaves the angle alone, so that the products
	// of the Newton step cannot overflow near max() or lose bits near the bottom of the range
	F xs = x, ys = y;
	const int k = std::max(std::ilogb(Real(x)), std::ilogb(Real(y)));
	if (k > std::numeric_limits<Real>::max_exponent / 2 || k < -std::numeric_limits<Real>::max_exponent / 2) {
		xs = faithful_detail::scale2(faithful_detail::scale2(x, -(k / 2)), -(k - k / 2));
		ys = faithful_detail::scale2(faithful_detail::scale2(y, -(k / 2)), -(k - k / 2));
	}
	// one Newton step from the FloatingPointType's angle
	const F a0(std::atan2(Real(ys), Real(xs)), Real(0));
	F s, c;
	faithful_detail::sincos(a0, s, c);
	return a0 + (ys * c - xs * s) / (xs * c + ys * s);
}

template<typename FloatingPointType>
	requires faithful_detail::trig_host<FloatingPointType>
inline faithful<FloatingPointType> atan(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	if (x.iszero()) return x;                        // atan(+-0) = +-0
	return atan2(x, faithful<Real>(Real(1)));
}

template<typename FloatingPointType>
	requires faithful_detail::trig_host<FloatingPointType>
inline faithful<FloatingPointType> asin(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	const F one(Real(1));
	if (one < abs(x)) return faithful_detail::domain_error<Real>("asin");
	if (x.iszero()) return x;                        // asin(+-0) = +-0
	return atan2(x, sqrt((one - x) * (one + x)));
}

template<typename FloatingPointType>
	requires faithful_detail::trig_host<FloatingPointType>
inline faithful<FloatingPointType> acos(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan()) return x;
	const F one(Real(1));
	if (one < abs(x)) return faithful_detail::domain_error<Real>("acos");
	return atan2(sqrt((one - x) * (one + x)), x);
}

}} // namespace sw::universal
