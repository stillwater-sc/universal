#pragma once
// pow.hpp: x^y for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// pow (#1957), built on the compensated exp and log of #1955.
//   - y an integer with |y| <= 64: binary powering in faithful arithmetic, exact whenever the
//     result is representable as a pair, as pow(3, 4) = 81 and pow(2, -10) are.
//   - otherwise x^y = 2^n exp(f ln2 + y log(b)), with x = 2^k b, b in [sqrt(1/2), sqrt(2)), and
//     y k = n + f split exactly, in an exact expansion, into an integer n and |f| <= 1/2. Folding y k ln2 into
//     a power of two keeps it out of exp's argument, where its rounding error would become a
//     relative error of |y k| units; what remains is the error of y log(b), which grows with
//     |y log(b)|: the pair's log(b) is accurate to a few units of its own magnitude, and y
//     multiplies that.
// Accuracy, therefore: a few units of epsilon() while |y log x| <= 1, and growing linearly beyond,
// about 1.5 |y log x| units on the exp/log path and about |y| units for binary powering. This is a
// limit of pair precision, not of the algorithm: exp(w) needs w to an absolute epsilon(), and a
// pair y log(b) holds it to a relative one. dd's pow has the same limit. A caller that needs
// x^y for a large y to full precision needs a wider type for the logarithm.
//
// The special cases follow std::pow (C99 F.10.4.4): pow(x, +-0) = 1 and pow(1, y) = 1 even for
// NaN; pow(-1, +-inf) = 1; a zero base gives +-0 or +-inf by the sign and parity of y, with
// ERANGE for the pole; an infinite base or exponent follows the magnitude of |x| against 1; a
// negative finite x with a non-integer y is NaN with EDOM, or throws faithful_domain_error with
// FAITHFUL_THROW_ARITHMETIC_EXCEPTION set; overflow is inf with ERANGE.
// Only a native FloatingPointType.
#include <cerrno>        // errno, EDOM, ERANGE
#include <cmath>
#include <limits>
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/constants.hpp>
#include <universal/number/faithful/math/functions/exponent.hpp>
#include <universal/number/faithful/math/functions/logarithm.hpp>
#include <universal/number/faithful/math/functions/truncate.hpp>
#include <universal/number/faithful/math/functions/trigonometry.hpp>   // faithful_detail::domain_error
#include <universal/number/faithful/math/functions/fractional.hpp>   // faithful_detail::exact_expansion
#include <universal/numerics/twoprod.hpp>

namespace sw { namespace universal {

namespace faithful_detail {
	// is y an integer, and if so, is it odd
	template<typename Real>
	inline bool is_integer(const faithful<Real>& y) { return !y.isinf() && !y.isnan() && floor(y) == y; }
	template<typename Real>
	inline bool is_odd_integer(const faithful<Real>& y) {
		if (!is_integer(y)) return false;
		const faithful<Real> h = scale2(y, -1);
		return floor(h) != h;
	}

	// x^n by binary powering
	template<typename Real>
	inline faithful<Real> ipow(faithful<Real> x, int n) {
		const bool reciprocal = n < 0;
		unsigned m = static_cast<unsigned>(reciprocal ? -n : n);
		faithful<Real> r(Real(1));
		while (m != 0) {
			if (m & 1u) r *= x;
			m >>= 1;
			if (m != 0) x *= x;
		}
		return reciprocal ? faithful<Real>(Real(1)) / r : r;
	}
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> pow(const faithful<FloatingPointType>& x, const faithful<FloatingPointType>& y) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	using faithful_detail::is_integer;
	using faithful_detail::is_odd_integer;
	const F one(Real(1)), zero(Real(0));
	constexpr Real inf = std::numeric_limits<Real>::infinity();
	if (y.iszero() || x == one) return one;          // also for a NaN x or y
	if (x.isnan() || y.isnan()) return F(std::numeric_limits<Real>::quiet_NaN());

	// a zero base
	if (x.iszero()) {
		const bool odd = is_odd_integer(y);
		if (y.sign()) {
			errno = ERANGE;
			return F((odd && signbit(x)) ? -inf : inf);
		}
		return (odd && signbit(x)) ? x : zero;
	}
	// an infinite exponent
	if (y.isinf()) {
		const F ax = abs(x);
		if (ax == one) return one;                    // pow(-1, +-inf) = 1
		const bool grows = (one < ax) != y.sign();
		return grows ? F(inf) : zero;
	}
	// an infinite base
	if (x.isinf()) {
		const bool odd = is_odd_integer(y);
		if (x.sign()) {
			if (y.sign()) return odd ? F(-Real(0)) : zero;
			return odd ? F(-inf) : F(inf);
		}
		return y.sign() ? zero : F(inf);
	}

	// a negative base needs an integer exponent
	bool negate = false;
	F ax = x;
	if (x.sign()) {
		if (!is_integer(y)) return faithful_detail::domain_error<Real>("pow");
		negate = is_odd_integer(y);
		ax = -x;
	}

	F r;
	if (is_integer(y) && std::fabs(Real(y)) <= Real(64)) {
		r = faithful_detail::ipow(ax, static_cast<int>(Real(y)));
		if (r.isinf() || r.iszero()) errno = ERANGE;   // overflow, or underflow of a nonzero base
	}
	else {
		static const F ln2 = faithful_detail::pair<Real>(faithful_detail::constants::ln2);
		int k = 0;
		const F b = faithful_detail::center(ax, k);
		const F lb = faithful_detail::log_centered(b);
		// y k = n + f, exactly: y k as four twoProd terms in an exact expansion, n its nearest
		// integer, and f = y k - n rounded once to a pair. A pair product y * k would round the
		// error part, and the lost bits would come back as a relative error of |y k| units.
		faithful_detail::exact_expansion<Real> yk;
		{
			const Real kr = Real(k);
			Real p1, e1, p2, e2;
			twoProd(y.high(), kr, p1, e1);
			twoProd(y.low(), kr, p2, e2);
			yk.add(e2);
			yk.add(p2);
			yk.add(e1);
			yk.add(p1);
		}
		const Real nv = std::nearbyint(yk.approx());
		// a |y k| this large puts the result far outside the range; the estimate below says which way
		const bool huge = !(std::fabs(nv) < std::ldexp(Real(1), std::numeric_limits<Real>::digits - 2));
		F w;
		if (!huge) {
			yk.add(-nv);
			w = yk.to_pair() * ln2 + y * lb;
		}
		// 2^n exp(w), with the range checks on the combined exponent
		const Real estimate = huge ? nv : nv + Real(w) * Real(1.4426950408889634);   // log2 of the result
		if (estimate > Real(std::numeric_limits<Real>::max_exponent + 1)) {
			errno = ERANGE;
			r = F(inf);
		}
		else if (estimate < Real(std::numeric_limits<Real>::min_exponent - std::numeric_limits<Real>::digits - 2)) {
			errno = ERANGE;
			r = zero;
		}
		else {
			// reduce w itself, w = m ln2 + r exactly, and apply 2^(m + n) once at the end: exp(w) on
			// its own overflows or underflows whenever |w| is past exp's range, though 2^n brings the
			// result back, as for pow(0.7f, 270) = 2^-270 exp(270 log(1.4)) with exp(90.8) > FLT_MAX
			const Real m = std::floor(Real(w) / ln2.high() + Real(0.5));
			r = faithful_detail::exp_reduced(faithful_detail::reduce_ln2(w, m), m + nv);
			if (r.isinf() || r.iszero()) errno = ERANGE;
		}
	}
	return negate ? -r : r;
}

}} // namespace sw::universal
