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
	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, Scalar>>>
	constexpr interval(T v) noexcept : _lo(static_cast<Scalar>(v)), _hi(static_cast<Scalar>(v)) {}

	// construct from explicit lower and upper bounds
	template<typename T, typename U>
	constexpr interval(T lo, U hi) noexcept
		: _lo(static_cast<Scalar>(lo)), _hi(static_cast<Scalar>(hi)) {
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
		// [a,b] + [c,d] = [a+c, b+d]
		_lo = _lo + rhs._lo;
		_hi = _hi + rhs._hi;
		return *this;
	}

	interval& operator-=(const interval& rhs) noexcept {
		// [a,b] - [c,d] = [a-d, b-c]
		Scalar newLo = _lo - rhs._hi;
		Scalar newHi = _hi - rhs._lo;
		_lo = newLo;
		_hi = newHi;
		return *this;
	}

	interval& operator*=(const interval& rhs) noexcept {
		// [a,b] * [c,d] = [min(ac,ad,bc,bd), max(ac,ad,bc,bd)]
		Scalar ac = _lo * rhs._lo;
		Scalar ad = _lo * rhs._hi;
		Scalar bc = _hi * rhs._lo;
		Scalar bd = _hi * rhs._hi;

		_lo = std::min({ac, ad, bc, bd});
		_hi = std::max({ac, ad, bc, bd});
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
		// Compute reciprocal of rhs: [1/d, 1/c]
		interval reciprocal(Scalar(1) / rhs._hi, Scalar(1) / rhs._lo);
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

	// radius (half-width) of the interval
	constexpr Scalar rad() const noexcept {
		return (_hi - _lo) / Scalar(2);
	}

	// width of the interval
	constexpr Scalar width() const noexcept {
		return _hi - _lo;
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
	if (x.contains_zero()) {
		Scalar maxSq = x.mag() * x.mag();
		return interval<Scalar>(Scalar(0), maxSq);
	}
	else {
		Scalar loSq = x.lo() * x.lo();
		Scalar hiSq = x.hi() * x.hi();
		return interval<Scalar>(std::min(loSq, hiSq), std::max(loSq, hiSq));
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
	Scalar lo = x.lo() < Scalar(0) ? Scalar(0) : sqrt(x.lo());
	Scalar hi = sqrt(x.hi());
	return interval<Scalar>(lo, hi);
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
