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
// (legacy double-typed constant kept for backward source compatibility;
//  the encode/decode pipeline now uses per-AuxReal infinity via numeric_limits)
constexpr double bisection_inf = std::numeric_limits<double>::infinity();

// Helper: +infinity in the auxiliary real type R.
template<typename R>
inline R bisection_aux_inf() { return std::numeric_limits<R>::infinity(); }

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
///
/// R is the auxiliary real type: double, dd, or qd. All intermediate
/// arithmetic runs in R so that encoding accuracy scales with the
/// precision of the auxiliary type, not with double.
template<typename R, typename Generator, typename Refinement>
inline R bisection_point(const R& xl, const R& xh, Generator g, Refinement f) {
	const R pinf = bisection_aux_inf<R>();
	const R ninf = -pinf;
	// Bootstrapping rules
	if (xl == ninf && xh == pinf) return R(0);
	if (xl == ninf && xh == R(0)) return R(-1);
	if (xl == R(0) && xh == pinf) return R(1);

	// Negation: f(a, b) = -f(-b, -a) for a < b < 0
	if (xh < R(0)) {
		return -bisection_point<R>(-xh, -xl, g, f);
	}

	// Reciprocation: f(0, b) = [f(b^-1, +inf)]^-1
	if (xl == R(0)) {
		return R(1) / bisection_point<R>(R(1) / xh, pinf, g, f);
	}

	// Bracketing: [a_i, +inf) -> g(a_i)
	if (xh == pinf) {
		return g(xl);
	}

	// Refinement: finite interval [a, b]
	return f(xl, xh);
}

template<typename R, typename Generator, typename Refinement>
inline int64_t bisection_encode(const R& x, unsigned p, Generator g, Refinement f) {
	using std::isnan;
	if (isnan(x)) {
		// NaN: all ones (two's complement -1 before sign flip = max negative)
		return -(int64_t(1) << (p - 1));
	}

	const R pinf = bisection_aux_inf<R>();
	R xl = -pinf;
	R xh =  pinf;
	int64_t y = 0;

	for (unsigned i = 0; i < p; ++i) {
		R xm = bisection_point<R>(xl, xh, g, f);

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
		R xm = bisection_point<R>(xl, xh, g, f);

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

/// Decode a p-bit signed integer y back to a real number x in the
/// auxiliary real type R. Reverses the encoding by reading bits MSB
/// to LSB and narrowing the interval accordingly.
template<typename R, typename Generator, typename Refinement>
inline R bisection_decode(int64_t y, unsigned p, Generator g, Refinement f) {
	// Undo two's complement sign flip
	y ^= (int64_t(1) << (p - 1));

	// NaN check: y == minimum signed value
	if (y == -(int64_t(1) << (p - 1))) {
		return std::numeric_limits<R>::quiet_NaN();
	}

	const R pinf = bisection_aux_inf<R>();
	R xl = -pinf;
	R xh =  pinf;

	for (unsigned i = 0; i < p; ++i) {
		R xm = bisection_point<R>(xl, xh, g, f);

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
	if (xl == -pinf) return xh;
	if (xh ==  pinf) return xl;
	return xl;
}

// --------------------------------------------------------------------
// The bisection class
// --------------------------------------------------------------------

/// bisection<Generator, Refinement, nbits, bt, AuxReal>
///
/// A number system defined by a generator (bracketing sequence) and
/// a refinement (interval bisection) function. The encoding stores
/// nbits of precision in a two's complement integer.
///
/// AuxReal is the auxiliary real type used for the interval bisection
/// arithmetic. The default (double) is correct for nbits up to ~50.
/// For wider encodings (or whenever you need higher round-trip ULP
/// accuracy) instantiate with sw::universal::dd (~31 digits) or
/// sw::universal::qd (~63 digits).
template<typename Generator, typename Refinement, unsigned _nbits, typename bt, typename AuxReal>
class bisection {
public:
	static constexpr unsigned nbits = _nbits;
	using BlockType = bt;
	using aux_real_type = AuxReal;
	static_assert(nbits <= 64, "bisection currently supports up to 64 bits; wider types require blockbinary storage (issue #687 follow-up)");
	static_assert(nbits >= 2, "bisection requires at least 2 bits (sign + one value bit)");

	// Trivially constructible: no in-class initializers
	bisection() : _bits{ 0 } {}

	bisection(const bisection&) = default;
	bisection(bisection&&) = default;
	bisection& operator=(const bisection&) = default;
	bisection& operator=(bisection&&) = default;

	// Constructors from native types -- always go through AuxReal so
	// the encoding accuracy reflects the chosen auxiliary precision.
	bisection(int v)       { *this = AuxReal(v); }
	bisection(unsigned v)  { *this = AuxReal(v); }
	bisection(long long v) { *this = AuxReal(v); }
	bisection(float v)     { *this = AuxReal(v); }
	bisection(double v)    { *this = AuxReal(v); }

	// Assignment from double is the canonical, always-present overload.
	// It is wide enough for all native arithmetic types and avoids
	// overload ambiguity with operator=(const AuxReal&) when AuxReal == double.
	bisection& operator=(double rhs) { return assign_from_aux(AuxReal(rhs)); }
	bisection& operator=(int rhs)       { return *this = static_cast<double>(rhs); }
	bisection& operator=(unsigned rhs)  { return *this = static_cast<double>(rhs); }
	bisection& operator=(long long rhs) { return *this = static_cast<double>(rhs); }
	bisection& operator=(float rhs)     { return *this = static_cast<double>(rhs); }
	// When AuxReal != double, also accept AuxReal directly so callers
	// that have a high-precision input avoid the lossy double round-trip.
	template<typename A = AuxReal,
	         typename = std::enable_if_t<!std::is_same<A, double>::value>>
	bisection& operator=(const AuxReal& rhs) { return assign_from_aux(rhs); }

	// Canonical decode to double is always available so legacy callers
	// (stream I/O, compare-with-double, math wrappers below) keep working.
	explicit operator double() const {
		return static_cast<double>(decode_aux());
	}
	// When AuxReal != double, also expose the high-precision decode.
	template<typename A = AuxReal,
	         typename = std::enable_if_t<!std::is_same<A, double>::value>>
	explicit operator AuxReal() const { return decode_aux(); }
	explicit operator float() const { return static_cast<float>(static_cast<double>(*this)); }
	explicit operator long long() const { return static_cast<long long>(static_cast<double>(*this)); }

	// -- Arithmetic operators (decode-compute-encode) --------------

	bisection operator-() const {
		if (isnan()) return *this;  // NaN negation returns NaN
		bisection neg;
		neg._bits = -_bits;
		return neg;
	}

	bisection& operator+=(const bisection& rhs) {
		return assign_from_aux(decode_aux() + rhs.decode_aux());
	}
	bisection& operator-=(const bisection& rhs) {
		return assign_from_aux(decode_aux() - rhs.decode_aux());
	}
	bisection& operator*=(const bisection& rhs) {
		return assign_from_aux(decode_aux() * rhs.decode_aux());
	}
	bisection& operator/=(const bisection& rhs) {
		return assign_from_aux(decode_aux() / rhs.decode_aux());
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

	// Core decode in the auxiliary precision -- the single source of truth
	// used by every public conversion and arithmetic operator.
	AuxReal decode_aux() const {
		return bisection_decode<AuxReal>(_bits, nbits, Generator{}, Refinement{});
	}

	// Core encode helper -- handles the inf saturation rule once.
	bisection& assign_from_aux(const AuxReal& rhs) {
		using std::isinf;
		if (isinf(rhs)) {
			if (rhs > AuxReal(0)) maxpos(); else maxneg();
			return *this;
		}
		_bits = bisection_encode<AuxReal>(rhs, nbits, Generator{}, Refinement{});
		return *this;
	}

	// Grant free functions access
	template<typename G, typename R, unsigned n, typename b, typename A>
	friend std::string to_binary(const bisection<G, R, n, b, A>&, bool);
};

// -- Free binary operators ----------------------------------------

template<typename G, typename R, unsigned n, typename b, typename A>
inline bisection<G, R, n, b, A> operator+(bisection<G, R, n, b, A> lhs, const bisection<G, R, n, b, A>& rhs) {
	return lhs += rhs;
}
template<typename G, typename R, unsigned n, typename b, typename A>
inline bisection<G, R, n, b, A> operator-(bisection<G, R, n, b, A> lhs, const bisection<G, R, n, b, A>& rhs) {
	return lhs -= rhs;
}
template<typename G, typename R, unsigned n, typename b, typename A>
inline bisection<G, R, n, b, A> operator*(bisection<G, R, n, b, A> lhs, const bisection<G, R, n, b, A>& rhs) {
	return lhs *= rhs;
}
template<typename G, typename R, unsigned n, typename b, typename A>
inline bisection<G, R, n, b, A> operator/(bisection<G, R, n, b, A> lhs, const bisection<G, R, n, b, A>& rhs) {
	return lhs /= rhs;
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> abs(const bisection<G, R, n, b, A>& v) {
	return (v.sign()) ? -v : v;
}

// -- Math functions (decode-compute-encode via double) ---------------
// These wrappers always go through double; they exist so that bisection
// types satisfy the same generic-math API as the rest of Universal.

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> sqrt(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::sqrt(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> log(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::log(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> exp(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::exp(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> sin(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::sin(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> cos(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::cos(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> tan(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::tan(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> asin(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::asin(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> acos(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::acos(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> atan(const bisection<G, R, n, b, A>& v) {
	return bisection<G, R, n, b, A>(std::atan(double(v)));
}

template<typename G, typename R, unsigned n, typename b, typename A>
bisection<G, R, n, b, A> pow(const bisection<G, R, n, b, A>& base, const bisection<G, R, n, b, A>& exp) {
	return bisection<G, R, n, b, A>(std::pow(double(base), double(exp)));
}

// -- Stream I/O ---------------------------------------------------

template<typename G, typename R, unsigned n, typename b, typename A>
inline std::ostream& operator<<(std::ostream& ostr, const bisection<G, R, n, b, A>& v) {
	double d = double(v);
	return ostr << d;
}

}} // namespace sw::universal
