#pragma once
// bisection_impl.hpp: implementation of the bisection number system
//
// Implements Peter Lindstrom's "Universal Coding of the Reals using
// Bisection" (CoNGA'19). A number system is defined by two functions:
//   - Generator g(x): produces a monotonic bracketing sequence
//   - Refinement f(a, b): bisects a bounded interval
//
// Each bit in the encoding is the outcome of a binary comparison.
// Encoding is binary search; decoding is interval bisection.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <cstdint>
#include <cmath>
#include <cassert>
#include <limits>
#include <string>
#include <sstream>
#include <iomanip>

namespace sw { namespace universal {

// sentinel for +/-infinity in bisection interval arithmetic
constexpr double bisection_inf = std::numeric_limits<double>::infinity();

// --------------------------------------------------------------------
// Bisection encode: real -> p-bit two's complement integer
// --------------------------------------------------------------------

/// Encode a real number x into a p-bit signed integer y using the
/// bisection framework. The encoding is monotonic: x < x' => y < y'.
///
/// Algorithm (Lindstrom, CoNGA'19 Section 2.5):
///   1. Start with interval (-inf, +inf)
///   2. For each bit position (MSB to LSB):
///      - Compute bisection point x_m = bisect(x_l, x_h)
///      - If x < x_m: emit 0, narrow to [x_l, x_m)
///      - Else:        emit 1, narrow to [x_m, x_h)
///   3. Round to nearest at the last step
///
/// The binary representation y is treated as two's complement so that
/// comparisons on the encoding correspond directly to comparisons on
/// the reals.
/// Compute the bisection point for interval [xl, xh] using the full
/// set of bisection rules: bootstrapping, negation, reciprocation,
/// bracketing (generator), and refinement.
template<typename Generator, typename Refinement>
inline double bisection_point(double xl, double xh, Generator g, Refinement f) {
	// Bootstrapping rules
	if (xl == -bisection_inf && xh == bisection_inf) return 0.0;
	if (xl == -bisection_inf && xh == 0.0)           return -1.0;
	if (xl == 0.0 && xh == bisection_inf)             return 1.0;

	// Negation: f(a, b) = -f(-b, -a) for a < b < 0
	if (xh < 0.0) {
		return -bisection_point(-xh, -xl, g, f);
	}

	// Reciprocation: f(0, b) = [f(b^-1, +inf)]^-1
	if (xl == 0.0) {
		return 1.0 / bisection_point(1.0 / xh, bisection_inf, g, f);
	}

	// Negative side with xl < 0 < xh should not occur in a well-formed
	// bisection (the interval is always on one side of zero after the
	// first bisection step).

	// Bracketing: [a_i, +inf) -> g(a_i)
	if (xh == bisection_inf) {
		return g(xl);
	}

	// Refinement: finite interval [a, b]
	return f(xl, xh);
}

template<typename Generator, typename Refinement>
inline int64_t bisection_encode(double x, unsigned p, Generator g, Refinement f) {
	if (std::isnan(x)) {
		// NaN: all ones (two's complement -1 before sign flip = max negative)
		return -(int64_t(1) << (p - 1));
	}

	double xl = -bisection_inf;
	double xh = bisection_inf;
	int64_t y = 0;

	for (unsigned i = 0; i < p; ++i) {
		double xm = bisection_point(xl, xh, g, f);

		y <<= 1;
		if (x >= xm) {
			y |= 1;
			xl = xm;
		}
		else {
			xh = xm;
		}
	}

	// Rounding: if x is exactly equal to the lower endpoint xl,
	// no rounding needed. Otherwise, generate one more bisection
	// step to determine which adjacent value x rounds to.
	if (x != xl) {
		double xm = bisection_point(xl, xh, g, f);

		if (x >= xm) {
			if (x == xm && (y & 1) == 0) {
				// tie: y is even, round down (keep y)
			}
			else {
				++y;
			}
		}
	}

	// Two's complement sign flip (Section 2.5):
	// flip the MSB so that y = 0 maps to x = 0
	y ^= (int64_t(1) << (p - 1));

	// Sign-extend from p bits to int64 so that the stored value
	// matches what setbits() produces and operator== works correctly.
	if (p < 64 && (y & (int64_t(1) << (p - 1)))) {
		uint64_t mask = (uint64_t(1) << p) - 1;
		y |= ~static_cast<int64_t>(mask);
	}

	return y;
}

// --------------------------------------------------------------------
// Bisection decode: p-bit two's complement integer -> real
// --------------------------------------------------------------------

/// Decode a p-bit signed integer y back to a real number x.
/// Reverses the encoding by reading bits MSB to LSB and narrowing
/// the interval accordingly.
template<typename Generator, typename Refinement>
inline double bisection_decode(int64_t y, unsigned p, Generator g, Refinement f) {
	// Undo two's complement sign flip
	y ^= (int64_t(1) << (p - 1));

	// NaN check: y == minimum signed value
	if (y == -(int64_t(1) << (p - 1))) {
		return std::numeric_limits<double>::quiet_NaN();
	}

	double xl = -bisection_inf;
	double xh = bisection_inf;

	for (unsigned i = 0; i < p; ++i) {
		double xm = bisection_point(xl, xh, g, f);

		// Read bit (MSB first)
		int bit = (y >> (p - 1 - i)) & 1;
		if (bit) {
			xl = xm;
		}
		else {
			xh = xm;
		}
	}

	// Return the lower endpoint of the final interval.
	// For +/-inf endpoints, return the finite one.
	if (xl == -bisection_inf) return xh;
	if (xh == bisection_inf) return xl;
	return xl;
}

// --------------------------------------------------------------------
// The bisection class
// --------------------------------------------------------------------

/// bisection<Generator, Refinement, nbits, bt>
///
/// A number system defined by a generator (bracketing sequence) and
/// a refinement (interval bisection) function. The encoding stores
/// nbits of precision in a two's complement integer.
template<typename Generator, typename Refinement, unsigned _nbits, typename bt>
class bisection {
public:
	static constexpr unsigned nbits = _nbits;
	using BlockType = bt;
	static_assert(nbits <= 64, "bisection currently supports up to 64 bits; wider types require blockbinary storage");
	static_assert(nbits >= 2, "bisection requires at least 2 bits (sign + one value bit)");

	// Trivially constructible: no in-class initializers
	bisection() : _bits{ 0 } {}

	bisection(const bisection&) = default;
	bisection(bisection&&) = default;
	bisection& operator=(const bisection&) = default;
	bisection& operator=(bisection&&) = default;

	// Constructors from native types
	bisection(int v)                { *this = static_cast<double>(v); }
	bisection(unsigned v)           { *this = static_cast<double>(v); }
	bisection(long long v)          { *this = static_cast<double>(v); }
	bisection(float v)              { *this = static_cast<double>(v); }
	bisection(double v)             { *this = v; }

	// Assignment from double: the core encode path
	bisection& operator=(double rhs) {
		if (std::isinf(rhs)) {
			// +inf: max positive encoding; -inf: max negative encoding
			if (rhs > 0) maxpos(); else maxneg();
			return *this;
		}
		_bits = bisection_encode(rhs, nbits, Generator{}, Refinement{});
		return *this;
	}
	bisection& operator=(int rhs) { return *this = static_cast<double>(rhs); }
	bisection& operator=(unsigned rhs) { return *this = static_cast<double>(rhs); }
	bisection& operator=(long long rhs) { return *this = static_cast<double>(rhs); }
	bisection& operator=(float rhs) { return *this = static_cast<double>(rhs); }

	// Conversion to double: the core decode path
	explicit operator double() const {
		return bisection_decode(_bits, nbits, Generator{}, Refinement{});
	}
	explicit operator float() const { return static_cast<float>(double(*this)); }
	explicit operator long long() const { return static_cast<long long>(double(*this)); }

	// -- Arithmetic operators (decode-compute-encode) --------------

	bisection operator-() const {
		if (isnan()) return *this;  // NaN negation returns NaN
		bisection neg;
		neg._bits = -_bits;
		return neg;
	}

	bisection& operator+=(const bisection& rhs) {
		double result = double(*this) + double(rhs);
		return *this = result;
	}
	bisection& operator-=(const bisection& rhs) {
		double result = double(*this) - double(rhs);
		return *this = result;
	}
	bisection& operator*=(const bisection& rhs) {
		double result = double(*this) * double(rhs);
		return *this = result;
	}
	bisection& operator/=(const bisection& rhs) {
		double result = double(*this) / double(rhs);
		return *this = result;
	}

	// Prefix increment/decrement (move to next/prev representable value)
	bisection& operator++() { ++_bits; return *this; }
	bisection& operator--() { --_bits; return *this; }
	bisection operator++(int) { bisection tmp(*this); ++_bits; return tmp; }
	bisection operator--(int) { bisection tmp(*this); --_bits; return tmp; }

	// -- Comparison operators (directly on two's complement bits) --

	bool operator==(const bisection& rhs) const { return _bits == rhs._bits; }
	bool operator!=(const bisection& rhs) const { return _bits != rhs._bits; }
	bool operator< (const bisection& rhs) const { return _bits <  rhs._bits; }
	bool operator<=(const bisection& rhs) const { return _bits <= rhs._bits; }
	bool operator> (const bisection& rhs) const { return _bits >  rhs._bits; }
	bool operator>=(const bisection& rhs) const { return _bits >= rhs._bits; }

	// -- Bit manipulation -----------------------------------------

	void setbits(uint64_t v) {
		// Mask to nbits and sign-extend
		uint64_t mask = (nbits < 64) ? ((uint64_t(1) << nbits) - 1) : ~uint64_t(0);
		_bits = static_cast<int64_t>(v & mask);
		// Sign extend
		if (nbits < 64 && (_bits & (int64_t(1) << (nbits - 1)))) {
			_bits |= ~static_cast<int64_t>(mask);
		}
	}

	uint64_t getbits() const {
		uint64_t mask = (nbits < 64) ? ((uint64_t(1) << nbits) - 1) : ~uint64_t(0);
		return static_cast<uint64_t>(_bits) & mask;
	}

	void setbit(unsigned index, bool v = true) {
		if (v)
			_bits |= (int64_t(1) << index);
		else
			_bits &= ~(int64_t(1) << index);
	}

	bool at(unsigned index) const {
		return (_bits >> index) & 1;
	}

	// -- Special value setters ------------------------------------

	void setzero() { _bits = 0; }

	void maxpos() {
		uint64_t mask = (nbits < 64) ? ((uint64_t(1) << nbits) - 1) : ~uint64_t(0);
		_bits = static_cast<int64_t>(mask >> 1);  // 0111...1
	}

	void minpos() {
		_bits = 1;  // smallest positive encoding
	}

	void maxneg() {
		// most negative: 1000...1 in raw bits -> after sign flip this is
		// the smallest (most negative) value
		if (nbits < 64)
			_bits = -(int64_t(1) << (nbits - 1)) + 1;
		else
			_bits = std::numeric_limits<int64_t>::min() + 1;
	}

	void minneg() {
		_bits = -1;  // largest negative (closest to zero)
	}

	void setnan() {
		// NaN: minimum signed value (all-ones pattern before sign flip)
		if (nbits < 64)
			_bits = -(int64_t(1) << (nbits - 1));
		else
			_bits = std::numeric_limits<int64_t>::min();
	}

	void setinf(bool negative = false) {
		// Bisection has no infinity encoding; saturate to maxpos/maxneg
		if (negative) maxneg(); else maxpos();
	}

	// -- State queries --------------------------------------------

	bool iszero() const { return _bits == 0; }
	bool sign()   const { return _bits < 0; }
	bool isnan()  const {
		int64_t nan_val = (nbits < 64) ? -(int64_t(1) << (nbits - 1))
		                               : std::numeric_limits<int64_t>::min();
		return _bits == nan_val;
	}
	bool isinf()  const { return false; }  // bisection has no infinity

private:
	int64_t _bits;

	// Grant free functions access
	template<typename G, typename R, unsigned n, typename b>
	friend std::string to_binary(const bisection<G, R, n, b>&, bool);
};

// -- Free binary operators ----------------------------------------

template<typename G, typename R, unsigned n, typename b>
inline bisection<G, R, n, b> operator+(bisection<G, R, n, b> lhs, const bisection<G, R, n, b>& rhs) {
	return lhs += rhs;
}
template<typename G, typename R, unsigned n, typename b>
inline bisection<G, R, n, b> operator-(bisection<G, R, n, b> lhs, const bisection<G, R, n, b>& rhs) {
	return lhs -= rhs;
}
template<typename G, typename R, unsigned n, typename b>
inline bisection<G, R, n, b> operator*(bisection<G, R, n, b> lhs, const bisection<G, R, n, b>& rhs) {
	return lhs *= rhs;
}
template<typename G, typename R, unsigned n, typename b>
inline bisection<G, R, n, b> operator/(bisection<G, R, n, b> lhs, const bisection<G, R, n, b>& rhs) {
	return lhs /= rhs;
}

template<typename G, typename R, unsigned n, typename b>
bisection<G, R, n, b> abs(const bisection<G, R, n, b>& v) {
	return (v.sign()) ? -v : v;
}

// -- Stream I/O ---------------------------------------------------

template<typename G, typename R, unsigned n, typename b>
inline std::ostream& operator<<(std::ostream& ostr, const bisection<G, R, n, b>& v) {
	double d = double(v);
	return ostr << d;
}

}} // namespace sw::universal
