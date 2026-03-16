#pragma once
// ubound.hpp: ubound (unum bound) for interval arithmetic with unum Type I
//
// A ubound represents a contiguous interval on the real number line,
// defined by a pair of exact unums [lower, upper]. When a unum has
// ubit=1, it represents the open interval (x, next_unum(x)). A ubound
// captures this uncertainty as a closed interval of exact endpoints.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <sstream>
#include <cmath>

namespace sw { namespace universal {

// next_exact: given a unum, return the next exact unum value
// by adding 1 ULP to the fraction field (or incrementing the exponent on overflow)
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> next_exact(const unum<esizesize, fsizesize, bt>& u) {
	if (u.isnan()) return u;
	double d = u.to_double();
	unsigned fs = u.fsize();
	// compute 1 ULP for the current esize/fsize configuration
	// ULP = 2^(exponent - bias) * 2^(-fsize) = value_scale * 2^(-fsize)
	if (fs == 0) {
		// no fraction bits: ULP is the distance to the next power of 2
		// the next exact value doubles the exponent increment
		unsigned es = u.esize();
		int bias = (1 << es) - 1;
		int exp = static_cast<int>(u.exponent()) - bias;
		double ulp = std::ldexp(1.0, exp);
		unum<esizesize, fsizesize, bt> result;
		result = d + ulp;
		return result;
	}
	else {
		// with fraction bits: ULP = 2^(exp - bias - fsize)
		unsigned es = u.esize();
		int bias = (1 << es) - 1;
		int exp = static_cast<int>(u.exponent()) - bias;
		double ulp = std::ldexp(1.0, exp - static_cast<int>(fs));
		unum<esizesize, fsizesize, bt> result;
		result = d + ulp;
		return result;
	}
}

// prev_exact: given a unum, return the previous exact unum value
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> prev_exact(const unum<esizesize, fsizesize, bt>& u) {
	if (u.isnan()) return u;
	double d = u.to_double();
	unsigned fs = u.fsize();
	if (fs == 0) {
		unsigned es = u.esize();
		int bias = (1 << es) - 1;
		int exp = static_cast<int>(u.exponent()) - bias;
		double ulp = std::ldexp(1.0, exp);
		unum<esizesize, fsizesize, bt> result;
		result = d - ulp;
		return result;
	}
	else {
		unsigned es = u.esize();
		int bias = (1 << es) - 1;
		int exp = static_cast<int>(u.exponent()) - bias;
		double ulp = std::ldexp(1.0, exp - static_cast<int>(fs));
		unum<esizesize, fsizesize, bt> result;
		result = d - ulp;
		return result;
	}
}

///////////////////////////////////////////////////////////////////////////
// ubound: a closed interval [lower, upper] of exact unum values

template<unsigned esizesize, unsigned fsizesize, typename bt = std::uint8_t>
class ubound {
public:
	using Unum = unum<esizesize, fsizesize, bt>;

	ubound() { _lo.setzero(); _hi.setzero(); }

	ubound(const ubound&) = default;
	ubound(ubound&&) = default;
	ubound& operator=(const ubound&) = default;
	ubound& operator=(ubound&&) = default;

	// construct from a single unum: if exact, [x,x]; if inexact, [x, next(x)]
	explicit ubound(const Unum& u) {
		if (u.isnan()) {
			_lo.setnan();
			_hi.setnan();
		}
		else if (u.ubit()) {
			// inexact: open interval (x, next(x)) -> closed [x, next(x)]
			_lo = u;
			_lo.setbit(0, false);  // clear ubit to get exact lower bound
			_hi = next_exact(_lo);
		}
		else {
			// exact: point interval [x, x]
			_lo = u;
			_hi = u;
		}
	}

	// construct from explicit lower and upper bounds
	ubound(const Unum& lo, const Unum& hi) : _lo(lo), _hi(hi) {}

	// construct from a double: find the tightest unum interval containing v
	explicit ubound(double v) {
		Unum u;
		u = v;
		if (u.ubit()) {
			_lo = u;
			_lo.setbit(0, false);
			_hi = next_exact(_lo);
		}
		else {
			_lo = u;
			_hi = u;
		}
	}

	// accessors
	const Unum& lower() const { return _lo; }
	const Unum& upper() const { return _hi; }

	// is this a point interval (zero width)?
	bool ispoint() const { return _lo.to_double() == _hi.to_double(); }

	// is either bound NaN?
	bool isnan() const { return _lo.isnan() || _hi.isnan(); }

	// width of the interval
	double width() const { return _hi.to_double() - _lo.to_double(); }

	// midpoint of the interval
	double midpoint() const { return (_lo.to_double() + _hi.to_double()) / 2.0; }

	// does this interval contain a value?
	bool contains(double v) const {
		return v >= _lo.to_double() && v <= _hi.to_double();
	}

	// interval arithmetic: [a,b] + [c,d] = [a+c, b+d]
	ubound& operator+=(const ubound& rhs) {
		if (isnan() || rhs.isnan()) { _lo.setnan(); _hi.setnan(); return *this; }
		double lo = _lo.to_double() + rhs._lo.to_double();
		double hi = _hi.to_double() + rhs._hi.to_double();
		_lo = lo;
		_hi = hi;
		return *this;
	}

	// interval arithmetic: [a,b] - [c,d] = [a-d, b-c]
	ubound& operator-=(const ubound& rhs) {
		if (isnan() || rhs.isnan()) { _lo.setnan(); _hi.setnan(); return *this; }
		double lo = _lo.to_double() - rhs._hi.to_double();
		double hi = _hi.to_double() - rhs._lo.to_double();
		_lo = lo;
		_hi = hi;
		return *this;
	}

	// interval arithmetic: [a,b] * [c,d] = [min(ac,ad,bc,bd), max(ac,ad,bc,bd)]
	ubound& operator*=(const ubound& rhs) {
		if (isnan() || rhs.isnan()) { _lo.setnan(); _hi.setnan(); return *this; }
		double a = _lo.to_double(), b = _hi.to_double();
		double c = rhs._lo.to_double(), d = rhs._hi.to_double();
		double ac = a * c, ad = a * d, bc = b * c, bd = b * d;
		double lo = std::min({ac, ad, bc, bd});
		double hi = std::max({ac, ad, bc, bd});
		_lo = lo;
		_hi = hi;
		return *this;
	}

	// interval arithmetic: [a,b] / [c,d]
	// undefined if [c,d] contains 0
	ubound& operator/=(const ubound& rhs) {
		if (isnan() || rhs.isnan()) { _lo.setnan(); _hi.setnan(); return *this; }
		double c = rhs._lo.to_double(), d = rhs._hi.to_double();
		if (c <= 0.0 && d >= 0.0) {
			// divisor interval contains zero
			_lo.setnan();
			_hi.setnan();
			return *this;
		}
		double a = _lo.to_double(), b = _hi.to_double();
		double ac = a / c, ad = a / d, bc = b / c, bd = b / d;
		double lo = std::min({ac, ad, bc, bd});
		double hi = std::max({ac, ad, bc, bd});
		_lo = lo;
		_hi = hi;
		return *this;
	}

private:
	Unum _lo;  // lower bound (exact, ubit=0)
	Unum _hi;  // upper bound (exact, ubit=0)
};

// binary operators
template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator+(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> sum(lhs);
	sum += rhs;
	return sum;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator-(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> diff(lhs);
	diff -= rhs;
	return diff;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator*(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> prod(lhs);
	prod *= rhs;
	return prod;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator/(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> quot(lhs);
	quot /= rhs;
	return quot;
}

// IO
template<unsigned esizesize, unsigned fsizesize, typename bt>
std::ostream& operator<<(std::ostream& ostr, const ubound<esizesize, fsizesize, bt>& ub) {
	if (ub.isnan()) {
		ostr << "[NaN]";
	}
	else if (ub.ispoint()) {
		ostr << '[' << ub.lower() << ']';
	}
	else {
		ostr << '[' << ub.lower() << ", " << ub.upper() << ']';
	}
	return ostr;
}

}} // namespace sw::universal
