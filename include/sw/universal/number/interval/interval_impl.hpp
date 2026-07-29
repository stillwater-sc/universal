#pragma once
// interval_impl.hpp: implementation of a parameterized interval number type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>
#include <type_traits>
#include <algorithm>
#include <cmath>
#include <ostream>
#include <istream>
#include <sstream>
#include <typeinfo>

#include <universal/number/interval/exceptions.hpp>

#ifndef INTERVAL_THROW_ARITHMETIC_EXCEPTION
#define INTERVAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

namespace sw { namespace universal {

// Outward-rounding helpers for interval arithmetic. Realizing the mathematical
// interval rules in finite precision requires DIRECTED rounding: the lower bound
// toward -inf and the upper bound toward +inf. Otherwise each endpoint can move
// inward by up to half an ulp per operation and the enclosure loses the containment
// property (the Fundamental Theorem of Interval Arithmetic). We round outward with
// nextafter rather than the FPU rounding mode (fesetround) because nextafter is
// Scalar-generic -- the hardware rounding mode has no effect on posit/cfloat/lns
// arithmetic, and interval<posit<...>> is a target use case. (#1234)
namespace interval_detail {
	template<typename Scalar>
	inline Scalar round_down(Scalar x) noexcept {   // toward -inf
		using std::nextafter;
		return nextafter(x, -std::numeric_limits<Scalar>::infinity());
	}
	template<typename Scalar>
	inline Scalar round_up(Scalar x) noexcept {      // toward +inf
		using std::nextafter;
		return nextafter(x, std::numeric_limits<Scalar>::infinity());
	}
	// widen a converted lower bound outward only when the conversion was inexact,
	// so exactly-representable operands keep zero-width endpoints (interval<double>(2)
	// stays [2,2]) while an inexact cross-type cast (interval<float>(0.1)) is enclosed.
	template<typename Scalar, typename T>
	inline Scalar enclose_lo(T v) noexcept {
		Scalar s = static_cast<Scalar>(v);
		return (static_cast<T>(s) == v) ? s : round_down(s);
	}
	template<typename Scalar, typename T>
	inline Scalar enclose_hi(T v) noexcept {
		Scalar s = static_cast<Scalar>(v);
		return (static_cast<T>(s) == v) ? s : round_up(s);
	}

	// --- Stage 2 (#1247): error-free transformations for tight enclosures ---------
	// EFT (Knuth TwoSum / Dekker-FMA TwoProduct) recovers the exact roundoff of an
	// operation, so an endpoint can be widened only when the operation was actually
	// inexact -- yielding 1-ulp-optimal enclosures instead of Stage-1's unconditional
	// outward rounding. But the roundoff term is guaranteed REPRESENTABLE (hence the
	// transform exact, hence containment preserved) ONLY for an IEEE-754-style format
	// with round-to-nearest-even AND gradual underflow (subnormals). We therefore
	// enable the tight path for the native floating-point types ONLY.
	//
	// It is deliberately NOT enabled for the Universal fixed-size types, even the
	// round-to-nearest ones, because the representability precondition fails in ways
	// that SILENTLY BREAK CONTAINMENT (the Fundamental Theorem, #1234):
	//   - cfloat without subnormals (the default): the TwoSum error term underflows to
	//     zero, so an inexact sum is mis-reported as exact and the endpoint is not
	//     widened -- measured at 132 containment breaks / 200k random pairs for
	//     cfloat<16,5>. cfloat WITH subnormals is sound in-range but is not the default.
	//   - posit: tapered precision means the roundoff of a sum/product near the extremes
	//     of the dynamic range need not be representable, so TwoSum/TwoProduct are not
	//     unconditionally exact.
	// Extending Stage 2 to those types (guarded on hasSubnormals / verified per config)
	// is tracked as a follow-up. interval_eft_exact is SAFE BY DEFAULT: every type it
	// does not recognize resolves false and falls back to Stage-1 unconditional outward
	// rounding (correct containment, at most 1 ulp wider) -- so it can never silently
	// lose containment.
	template<typename Scalar>
	struct interval_eft_exact : std::bool_constant<std::is_floating_point_v<Scalar>> {};

	// Knuth TwoSum: s = fl(a+b), e = exact roundoff so that a+b = s+e exactly (RNE).
	template<typename Scalar>
	inline void two_sum(Scalar a, Scalar b, Scalar& s, Scalar& e) noexcept {
		s = a + b;
		Scalar bb = s - a;
		e = (a - (s - bb)) + (b - bb);
	}
	// TwoProduct via FMA: p = fl(a*b), e = fma(a,b,-p) is the exact roundoff (RNE).
	template<typename Scalar>
	inline void two_prod(Scalar a, Scalar b, Scalar& p, Scalar& e) noexcept {
		using std::fma;
		p = a * b;
		e = fma(a, b, -p);
	}
	// enclose the exact value (s + roundoff) whose roundoff has sign 'roundoff':
	// widen down only if the true value is below s, up only if above -- 1-ulp optimal.
	// When the sum/product OVERFLOWED, s is +/-inf and roundoff is non-finite (NaN or
	// +/-inf); the residual sign is then meaningless, so fall back to Stage-1
	// unconditional outward rounding. round_down(+inf) is the largest finite value, which
	// restores containment of the finite-but-unrepresentable true value (e.g. 1e308+1e308).
	template<typename Scalar>
	inline Scalar tight_lo(Scalar s, Scalar roundoff) noexcept {
		using std::isfinite;
		if (!isfinite(roundoff)) return round_down(s);
		return (roundoff < Scalar(0)) ? round_down(s) : s;   // true value at or below s
	}
	template<typename Scalar>
	inline Scalar tight_hi(Scalar s, Scalar roundoff) noexcept {
		using std::isfinite;
		if (!isfinite(roundoff)) return round_up(s);
		return (roundoff > Scalar(0)) ? round_up(s) : s;     // true value at or above s
	}
}

/// <summary>
/// A parameterized interval number type [lo, hi] representing a closed interval.
/// The Scalar type can be any numeric type: float, double, or Universal types like cfloat<>.
///
/// Interval arithmetic follows the standard rules:
///   [a,b] + [c,d] = [a+c, b+d]
///   [a,b] - [c,d] = [a-d, b-c]
///   [a,b] * [c,d] = [min(ac,ad,bc,bd), max(ac,ad,bc,bd)]
///   [a,b] / [c,d] = [a,b] * [1/d, 1/c] (when 0 not in [c,d])
/// </summary>
/// <typeparam name="Scalar">the type used for the lower and upper bounds</typeparam>
template<typename Scalar>
class interval {
public:
	using value_type = Scalar;

	// constructors
	constexpr interval() noexcept : _lo{}, _hi{} {}

	constexpr interval(const interval&) noexcept = default;
	constexpr interval(interval&&) noexcept = default;

	constexpr interval& operator=(const interval&) noexcept = default;
	constexpr interval& operator=(interval&&) noexcept = default;

	// construct from a single value (degenerate interval [v, v])
	// NOTE: NOT constexpr -- a cross-type or unrepresentable value must be enclosed
	// with outward rounding (nextafter), which is not a constexpr operation. (#1234)
	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, Scalar>>>
	interval(T v) noexcept
		: _lo(interval_detail::enclose_lo<Scalar>(v)), _hi(interval_detail::enclose_hi<Scalar>(v)) {}

	// construct from explicit lower and upper bounds (each enclosed outward if the
	// conversion to Scalar is inexact)
	template<typename T, typename U>
	interval(T lo, U hi) noexcept
		: _lo(interval_detail::enclose_lo<Scalar>(lo)), _hi(interval_detail::enclose_hi<Scalar>(hi)) {
		// ensure proper ordering
		if (_lo > _hi) std::swap(_lo, _hi);
	}

	// assignment from a single value (degenerate interval)
	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
	constexpr interval& operator=(T v) noexcept {
		_lo = static_cast<Scalar>(v);
		_hi = static_cast<Scalar>(v);
		return *this;
	}

	// explicit conversion operators
	explicit operator float() const noexcept { return static_cast<float>(mid()); }
	explicit operator double() const noexcept { return static_cast<double>(mid()); }
	explicit operator long double() const noexcept { return static_cast<long double>(mid()); }

	// prefix operators
	constexpr interval operator-() const noexcept {
		return interval(-_hi, -_lo);
	}
	constexpr interval operator+() const noexcept {
		return *this;
	}

	// arithmetic operators
	interval& operator+=(const interval& rhs) noexcept {
		// [a,b] + [c,d] = [a+c, b+d]. EFT tightens: widen an endpoint only when the
		// sum was actually inexact (Stage 2, #1247); otherwise round outward (Stage 1).
		if constexpr (interval_detail::interval_eft_exact<Scalar>::value) {
			Scalar slo, elo, shi, ehi;
			interval_detail::two_sum(_lo, rhs._lo, slo, elo);
			interval_detail::two_sum(_hi, rhs._hi, shi, ehi);
			_lo = interval_detail::tight_lo(slo, elo);
			_hi = interval_detail::tight_hi(shi, ehi);
		}
		else {
			_lo = interval_detail::round_down(Scalar(_lo + rhs._lo));
			_hi = interval_detail::round_up(Scalar(_hi + rhs._hi));
		}
		return *this;
	}

	interval& operator-=(const interval& rhs) noexcept {
		// [a,b] - [c,d] = [a-d, b-c]
		if constexpr (interval_detail::interval_eft_exact<Scalar>::value) {
			Scalar slo, elo, shi, ehi;
			interval_detail::two_sum(_lo, Scalar(-rhs._hi), slo, elo);   // a - d
			interval_detail::two_sum(_hi, Scalar(-rhs._lo), shi, ehi);   // b - c
			_lo = interval_detail::tight_lo(slo, elo);
			_hi = interval_detail::tight_hi(shi, ehi);
		}
		else {
			Scalar newLo = interval_detail::round_down(Scalar(_lo - rhs._hi));
			Scalar newHi = interval_detail::round_up(Scalar(_hi - rhs._lo));
			_lo = newLo;
			_hi = newHi;
		}
		return *this;
	}

	interval& operator*=(const interval& rhs) noexcept {
		// [a,b] * [c,d] = [min(ac,ad,bc,bd), max(ac,ad,bc,bd)]
		if constexpr (interval_detail::interval_eft_exact<Scalar>::value) {
			// enclose each corner product to 1 ulp via TwoProduct, then take min/max
			Scalar dlo[4], dhi[4];
			const Scalar corners[4][2] = { {_lo, rhs._lo}, {_lo, rhs._hi}, {_hi, rhs._lo}, {_hi, rhs._hi} };
			for (int k = 0; k < 4; ++k) {
				Scalar p, e;
				interval_detail::two_prod(corners[k][0], corners[k][1], p, e);
				dlo[k] = interval_detail::tight_lo(p, e);
				dhi[k] = interval_detail::tight_hi(p, e);
			}
			_lo = std::min({dlo[0], dlo[1], dlo[2], dlo[3]});
			_hi = std::max({dhi[0], dhi[1], dhi[2], dhi[3]});
		}
		else {
			Scalar ac = _lo * rhs._lo;
			Scalar ad = _lo * rhs._hi;
			Scalar bc = _hi * rhs._lo;
			Scalar bd = _hi * rhs._hi;
			_lo = interval_detail::round_down(std::min({ac, ad, bc, bd}));
			_hi = interval_detail::round_up(std::max({ac, ad, bc, bd}));
		}
		return *this;
	}

	interval& operator/=(const interval& rhs) {
		// [a,b] / [c,d] = [a,b] * [1/d, 1/c] when 0 not in [c,d]
#if INTERVAL_THROW_ARITHMETIC_EXCEPTION
		if (rhs.contains_zero()) {
			throw interval_divide_by_zero();
		}
#else
		if (rhs.contains_zero()) {
			// Division by interval containing zero results in [-inf, inf]
			_lo = -std::numeric_limits<Scalar>::infinity();
			_hi = std::numeric_limits<Scalar>::infinity();
			return *this;
		}
#endif
		// Compute reciprocal of rhs: [1/d, 1/c]. EFT (fma residual) widens each endpoint
		// only when 1/x was inexact; the subsequent *= tightens the products too.
		Scalar recipLo, recipHi;
		if constexpr (interval_detail::interval_eft_exact<Scalar>::value) {
			using std::fma;
			// residual r = fma(s, x, -1) = (s - 1/x) * x, so sign(s - 1/x) = sign(r)*sign(x).
			// Compare signs directly rather than forming r*x: the product can underflow to
			// zero for a tiny denominator (|r| ~ ulp(1), |x| subnormal), which would wrongly
			// report "exact" and skip the widening.
			Scalar d = rhs._hi, s = Scalar(1) / d;
			Scalar r = fma(s, d, Scalar(-1));
			// recipLo must stay <= 1/d: widen down iff s overestimates 1/d, i.e. sign(r)==sign(d).
			recipLo = (r != Scalar(0) && ((r > Scalar(0)) == (d > Scalar(0)))) ? interval_detail::round_down(s) : s;
			Scalar c = rhs._lo, s2 = Scalar(1) / c;
			Scalar r2 = fma(s2, c, Scalar(-1));
			// recipHi must stay >= 1/c: widen up iff s2 underestimates 1/c, i.e. sign(r2)!=sign(c).
			recipHi = (r2 != Scalar(0) && ((r2 > Scalar(0)) != (c > Scalar(0)))) ? interval_detail::round_up(s2) : s2;
		}
		else {
			recipLo = interval_detail::round_down(Scalar(Scalar(1) / rhs._hi));
			recipHi = interval_detail::round_up(Scalar(Scalar(1) / rhs._lo));
		}
		interval reciprocal;
		reciprocal.setlo(recipLo);
		reciprocal.sethi(recipHi);
		return *this *= reciprocal;
	}

	// arithmetic with scalar
	template<typename T>
	interval& operator+=(T rhs) noexcept { return *this += interval(rhs); }
	template<typename T>
	interval& operator-=(T rhs) noexcept { return *this -= interval(rhs); }
	template<typename T>
	interval& operator*=(T rhs) noexcept { return *this *= interval(rhs); }
	template<typename T>
	interval& operator/=(T rhs) { return *this /= interval(rhs); }

	// modifiers
	constexpr void clear() noexcept { _lo = Scalar{}; _hi = Scalar{}; }
	constexpr void setzero() noexcept { clear(); }

	constexpr void setinf(bool sign = true) noexcept {
		if (sign) {
			_lo = -std::numeric_limits<Scalar>::infinity();
			_hi = -std::numeric_limits<Scalar>::infinity();
		}
		else {
			_lo = std::numeric_limits<Scalar>::infinity();
			_hi = std::numeric_limits<Scalar>::infinity();
		}
	}

	constexpr void setnan() noexcept {
		_lo = std::numeric_limits<Scalar>::quiet_NaN();
		_hi = std::numeric_limits<Scalar>::quiet_NaN();
	}

	// set lower and upper bounds explicitly
	constexpr void set(Scalar lo, Scalar hi) noexcept {
		_lo = lo;
		_hi = hi;
		if (_lo > _hi) std::swap(_lo, _hi);
	}

	constexpr void setlo(Scalar lo) noexcept { _lo = lo; }
	constexpr void sethi(Scalar hi) noexcept { _hi = hi; }

	// selectors
	constexpr Scalar lo() const noexcept { return _lo; }
	constexpr Scalar hi() const noexcept { return _hi; }
	constexpr Scalar lower() const noexcept { return _lo; }
	constexpr Scalar upper() const noexcept { return _hi; }

	// midpoint of the interval
	constexpr Scalar mid() const noexcept {
		return (_lo + _hi) / Scalar(2);
	}

	// radius (half-width) of the interval -- rounded UP so that mid() +/- rad()
	// still covers [lo, hi] (#1234). Not constexpr: nextafter is a runtime op.
	Scalar rad() const noexcept {
		return interval_detail::round_up(Scalar((_hi - _lo) / Scalar(2)));
	}

	// width of the interval -- rounded UP so it never underestimates the true width
	Scalar width() const noexcept {
		return interval_detail::round_up(Scalar(_hi - _lo));
	}

	// magnitude: max of |lo| and |hi|
	constexpr Scalar mag() const noexcept {
		using std::abs;
		return std::max(abs(_lo), abs(_hi));
	}

	// mignitude: min of |lo| and |hi| if interval doesn't contain 0, else 0
	constexpr Scalar mig() const noexcept {
		if (contains_zero()) return Scalar(0);
		using std::abs;
		return std::min(abs(_lo), abs(_hi));
	}

	// predicates
	constexpr bool iszero() const noexcept {
		return _lo == Scalar(0) && _hi == Scalar(0);
	}

	constexpr bool isdegenerate() const noexcept {
		return _lo == _hi;
	}

	constexpr bool isnan() const noexcept {
		using std::isnan;
		return isnan(_lo) || isnan(_hi);
	}

	constexpr bool isinf() const noexcept {
		using std::isinf;
		return isinf(_lo) || isinf(_hi);
	}

	constexpr bool isfinite() const noexcept {
		return !isnan() && !isinf();
	}

	// returns true if the interval contains zero
	constexpr bool contains_zero() const noexcept {
		return _lo <= Scalar(0) && Scalar(0) <= _hi;
	}

	// returns true if the interval contains the value v
	constexpr bool contains(Scalar v) const noexcept {
		return _lo <= v && v <= _hi;
	}

	// returns true if the interval is entirely positive
	constexpr bool ispos() const noexcept {
		return _lo > Scalar(0);
	}

	// returns true if the interval is entirely negative
	constexpr bool isneg() const noexcept {
		return _hi < Scalar(0);
	}

	// returns true if this interval is a subset of other
	constexpr bool subset_of(const interval& other) const noexcept {
		return other._lo <= _lo && _hi <= other._hi;
	}

	// returns true if this interval is a proper subset of other
	constexpr bool proper_subset_of(const interval& other) const noexcept {
		return subset_of(other) && (other._lo < _lo || _hi < other._hi);
	}

	// returns true if intervals overlap
	constexpr bool overlaps(const interval& other) const noexcept {
		return _lo <= other._hi && other._lo <= _hi;
	}

private:
	Scalar _lo;  // lower bound
	Scalar _hi;  // upper bound

	// friend declarations
	template<typename S>
	friend std::ostream& operator<<(std::ostream& ostr, const interval<S>& v);
	template<typename S>
	friend std::istream& operator>>(std::istream& istr, interval<S>& v);

	template<typename S>
	friend bool operator==(const interval<S>& lhs, const interval<S>& rhs);
	template<typename S>
	friend bool operator!=(const interval<S>& lhs, const interval<S>& rhs);
	template<typename S>
	friend bool operator<(const interval<S>& lhs, const interval<S>& rhs);
	template<typename S>
	friend bool operator>(const interval<S>& lhs, const interval<S>& rhs);
	template<typename S>
	friend bool operator<=(const interval<S>& lhs, const interval<S>& rhs);
	template<typename S>
	friend bool operator>=(const interval<S>& lhs, const interval<S>& rhs);
};

////////////////////// operators

// stream output
template<typename Scalar>
inline std::ostream& operator<<(std::ostream& ostr, const interval<Scalar>& v) {
	return ostr << '[' << v._lo << ", " << v._hi << ']';
}

// stream input
template<typename Scalar>
inline std::istream& operator>>(std::istream& istr, interval<Scalar>& v) {
	Scalar lo, hi;
	char c;
	istr >> c >> lo >> c >> hi >> c;  // expects format [lo, hi]
	v._lo = lo;
	v._hi = hi;
	return istr;
}

// comparison operators
// Two intervals are equal if both bounds are equal
template<typename Scalar>
inline bool operator==(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	return lhs._lo == rhs._lo && lhs._hi == rhs._hi;
}

template<typename Scalar>
inline bool operator!=(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	return !(lhs == rhs);
}

// Interval ordering: lhs < rhs if lhs.hi < rhs.lo (lhs is entirely before rhs)
template<typename Scalar>
inline bool operator<(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	return lhs._hi < rhs._lo;
}

template<typename Scalar>
inline bool operator>(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	return rhs < lhs;
}

template<typename Scalar>
inline bool operator<=(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	return !(lhs > rhs);
}

template<typename Scalar>
inline bool operator>=(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	return !(lhs < rhs);
}

// comparison with scalar
template<typename Scalar, typename T>
inline bool operator==(const interval<Scalar>& lhs, T rhs) {
	return lhs == interval<Scalar>(rhs);
}

template<typename Scalar, typename T>
inline bool operator==(T lhs, const interval<Scalar>& rhs) {
	return interval<Scalar>(lhs) == rhs;
}

template<typename Scalar, typename T>
inline bool operator!=(const interval<Scalar>& lhs, T rhs) {
	return !(lhs == rhs);
}

template<typename Scalar, typename T>
inline bool operator!=(T lhs, const interval<Scalar>& rhs) {
	return !(lhs == rhs);
}

// binary arithmetic operators
template<typename Scalar>
inline interval<Scalar> operator+(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	interval<Scalar> result(lhs);
	result += rhs;
	return result;
}

template<typename Scalar>
inline interval<Scalar> operator-(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	interval<Scalar> result(lhs);
	result -= rhs;
	return result;
}

template<typename Scalar>
inline interval<Scalar> operator*(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	interval<Scalar> result(lhs);
	result *= rhs;
	return result;
}

template<typename Scalar>
inline interval<Scalar> operator/(const interval<Scalar>& lhs, const interval<Scalar>& rhs) {
	interval<Scalar> result(lhs);
	result /= rhs;
	return result;
}

// scalar-interval operations
template<typename Scalar, typename T>
inline interval<Scalar> operator+(const interval<Scalar>& lhs, T rhs) {
	return lhs + interval<Scalar>(rhs);
}

template<typename Scalar, typename T>
inline interval<Scalar> operator+(T lhs, const interval<Scalar>& rhs) {
	return interval<Scalar>(lhs) + rhs;
}

template<typename Scalar, typename T>
inline interval<Scalar> operator-(const interval<Scalar>& lhs, T rhs) {
	return lhs - interval<Scalar>(rhs);
}

template<typename Scalar, typename T>
inline interval<Scalar> operator-(T lhs, const interval<Scalar>& rhs) {
	return interval<Scalar>(lhs) - rhs;
}

template<typename Scalar, typename T>
inline interval<Scalar> operator*(const interval<Scalar>& lhs, T rhs) {
	return lhs * interval<Scalar>(rhs);
}

template<typename Scalar, typename T>
inline interval<Scalar> operator*(T lhs, const interval<Scalar>& rhs) {
	return interval<Scalar>(lhs) * rhs;
}

template<typename Scalar, typename T>
inline interval<Scalar> operator/(const interval<Scalar>& lhs, T rhs) {
	return lhs / interval<Scalar>(rhs);
}

template<typename Scalar, typename T>
inline interval<Scalar> operator/(T lhs, const interval<Scalar>& rhs) {
	return interval<Scalar>(lhs) / rhs;
}

////////////////////// mathematical functions

// absolute value of an interval
template<typename Scalar>
inline interval<Scalar> abs(const interval<Scalar>& x) {
	using std::abs;
	if (x.contains_zero()) {
		return interval<Scalar>(Scalar(0), x.mag());
	}
	else if (x.isneg()) {
		return interval<Scalar>(abs(x.hi()), abs(x.lo()));
	}
	else {
		return x;
	}
}

// square of an interval
template<typename Scalar>
inline interval<Scalar> sqr(const interval<Scalar>& x) {
	if constexpr (interval_detail::interval_eft_exact<Scalar>::value) {
		if (x.contains_zero()) {
			Scalar p, e; interval_detail::two_prod(x.mag(), x.mag(), p, e);
			return interval<Scalar>(Scalar(0), interval_detail::tight_hi(p, e));
		}
		Scalar pl, el, ph, eh;
		interval_detail::two_prod(x.lo(), x.lo(), pl, el);
		interval_detail::two_prod(x.hi(), x.hi(), ph, eh);
		Scalar lo = std::min(interval_detail::tight_lo(pl, el), interval_detail::tight_lo(ph, eh));
		Scalar hi = std::max(interval_detail::tight_hi(pl, el), interval_detail::tight_hi(ph, eh));
		return interval<Scalar>(lo, hi);
	}
	else if (x.contains_zero()) {
		Scalar maxSq = interval_detail::round_up(Scalar(x.mag() * x.mag()));
		return interval<Scalar>(Scalar(0), maxSq);   // lower bound 0 is exact
	}
	else {
		Scalar loSq = x.lo() * x.lo();
		Scalar hiSq = x.hi() * x.hi();
		return interval<Scalar>(interval_detail::round_down(std::min(loSq, hiSq)),
		                        interval_detail::round_up(std::max(loSq, hiSq)));
	}
}

// square root of an interval
template<typename Scalar>
inline interval<Scalar> sqrt(const interval<Scalar>& x) {
	using std::sqrt;
#if INTERVAL_THROW_ARITHMETIC_EXCEPTION
	if (x.hi() < Scalar(0)) {
		throw interval_negative_sqrt_arg();
	}
#endif
	// sqrt is inexact in general. A clamped-to-zero lower bound (negative sqrt argument)
	// stays exactly 0. EFT (fma residual s*s - x) widens an endpoint only when sqrt was
	// actually inexact (#1247); otherwise round outward unconditionally (#1234).
	if constexpr (interval_detail::interval_eft_exact<Scalar>::value) {
		using std::fma;
		Scalar lo;
		if (x.lo() < Scalar(0)) {
			lo = Scalar(0);
		}
		else {
			Scalar s = sqrt(x.lo());
			Scalar r = fma(s, s, -x.lo());                          // s*s - lo; >0 => s above sqrt(lo)
			lo = (r > Scalar(0)) ? interval_detail::round_down(s) : s;
		}
		Scalar sh = sqrt(x.hi());
		Scalar rh = fma(sh, sh, -x.hi());                           // sh*sh - hi; <0 => sh below sqrt(hi)
		Scalar hi = (rh < Scalar(0)) ? interval_detail::round_up(sh) : sh;
		return interval<Scalar>(lo, hi);
	}
	else {
		Scalar lo = x.lo() < Scalar(0) ? Scalar(0) : interval_detail::round_down(Scalar(sqrt(x.lo())));
		Scalar hi = interval_detail::round_up(Scalar(sqrt(x.hi())));
		return interval<Scalar>(lo, hi);
	}
}

// power function
template<typename Scalar>
inline interval<Scalar> pow(const interval<Scalar>& x, int n) {
	if (n == 0) return interval<Scalar>(Scalar(1));
	if (n == 1) return x;
	if (n < 0) return interval<Scalar>(Scalar(1)) / pow(x, -n);

	// Even power
	if (n % 2 == 0) {
		return sqr(pow(x, n / 2));
	}
	// Odd power
	return x * pow(x, n - 1);
}

// intersection of two intervals (returns empty interval if no overlap)
template<typename Scalar>
inline interval<Scalar> intersect(const interval<Scalar>& a, const interval<Scalar>& b) {
	Scalar lo = std::max(a.lo(), b.lo());
	Scalar hi = std::min(a.hi(), b.hi());
	if (lo > hi) {
		// empty intersection - return NaN interval
		return interval<Scalar>(std::numeric_limits<Scalar>::quiet_NaN(),
		                        std::numeric_limits<Scalar>::quiet_NaN());
	}
	return interval<Scalar>(lo, hi);
}

// hull (union) of two intervals
template<typename Scalar>
inline interval<Scalar> hull(const interval<Scalar>& a, const interval<Scalar>& b) {
	return interval<Scalar>(std::min(a.lo(), b.lo()), std::max(a.hi(), b.hi()));
}

////////////////////// utility functions

// string conversion
template<typename Scalar>
inline std::string to_string(const interval<Scalar>& v) {
	std::stringstream s;
	s << v;
	return s.str();
}

// type tag for reporting
template<typename Scalar>
inline std::string type_tag(const interval<Scalar>& = {}) {
	std::stringstream s;
	s << "interval<" << typeid(Scalar).name() << '>';
	return s.str();
}

}} // namespace sw::universal
