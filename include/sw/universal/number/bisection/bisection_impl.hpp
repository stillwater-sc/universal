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
#include <type_traits>

#include <universal/internal/blockbinary/blockbinary.hpp>

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

// -- Bit-abstraction helpers for dual-path int64_t / blockbinary -----
//
// These allow bisection_encode / bisection_decode to work identically
// with int64_t (nbits <= 64) and blockbinary (nbits > 64).

namespace bisection_detail {

/// Create a value of type B with exactly one bit set at position `pos`.
/// Uses uint64_t for the int64_t path to avoid UB from left-shifting
/// into the sign bit when pos == 63.
template<typename B>
inline B bit_mask(unsigned pos) {
	if constexpr (std::is_same_v<B, int64_t>) {
		return static_cast<int64_t>(uint64_t(1) << pos);
	} else {
		B v{};
		v.setbit(pos);
		return v;
	}
}

/// Read the bit at position `pos` in value `v`.
/// Uses uint64_t cast for the int64_t path to avoid
/// implementation-defined right-shift of signed values.
template<typename B>
inline bool bit_test(const B& v, unsigned pos) {
	if constexpr (std::is_same_v<B, int64_t>) {
		return (static_cast<uint64_t>(v) >> pos) & 1;
	} else {
		return v.test(pos);
	}
}

/// Sign-extend from p bits to the full width of B.
/// For int64_t this is required when p < 64; for blockbinary it is a
/// no-op because the storage is already exactly nbits wide.
template<typename B>
inline void sign_extend(B& y, unsigned p) {
	if constexpr (std::is_same_v<B, int64_t>) {
		if (p < 64 && (y & (int64_t(1) << (p - 1)))) {
			uint64_t mask = (uint64_t(1) << p) - 1;
			y |= ~static_cast<int64_t>(mask);
		}
	}
	(void)y; (void)p;  // suppress unused-parameter warnings on the blockbinary path
}

/// NaN sentinel: the minimum signed value for a p-bit two's complement
/// integer (1000...0 pattern).
template<typename B>
inline B nan_encoding(unsigned p) {
	if constexpr (std::is_same_v<B, int64_t>) {
		if (p < 64)
			return -(int64_t(1) << (p - 1));
		else
			return std::numeric_limits<int64_t>::min();
	} else {
		B v{};
		v.maxneg();  // sets the 1000...0 pattern for Signed blockbinary
		return v;
	}
}

/// Two's complement negation.
template<typename B>
inline B negate_bits(const B& v) {
	if constexpr (std::is_same_v<B, int64_t>) {
		return -v;
	} else {
		return -v;  // blockbinary has operator-()
	}
}

} // namespace bisection_detail

// -- Encode / Decode --------------------------------------------------
//
// B is the storage type (int64_t or blockbinary).
// R is the auxiliary real type (double, dd, or qd).

template<typename R, typename B, typename Generator, typename Refinement>
inline B bisection_encode(const R& x, unsigned p, Generator g, Refinement f) {
	using namespace bisection_detail;
	using std::isnan;
	if (isnan(x)) {
		return nan_encoding<B>(p);
	}

	const R pinf = bisection_aux_inf<R>();
	R xl = -pinf;
	R xh =  pinf;
	B y{};

	for (unsigned i = 0; i < p; ++i) {
		R xm = bisection_point<R>(xl, xh, g, f);

		y <<= 1;
		if (x >= xm) {
			y |= B(1);
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
			if (x == xm && !bit_test(y, 0u)) {
				// tie: y is even, round down (keep y)
			}
			else {
				++y;
			}
		}
	}

	// Two's complement sign flip (Section 2.5):
	// flip the MSB so that y = 0 maps to x = 0
	y ^= bit_mask<B>(p - 1);

	// Sign-extend (int64_t only; blockbinary is already nbits-wide)
	sign_extend(y, p);

	return y;
}

/// Decode a p-bit signed integer y back to a real number x in the
/// auxiliary real type R. Reverses the encoding by reading bits MSB
/// to LSB and narrowing the interval accordingly.
template<typename R, typename B, typename Generator, typename Refinement>
inline R bisection_decode(const B& y_in, unsigned p, Generator g, Refinement f) {
	using namespace bisection_detail;

	// NaN check on the STORED encoding, BEFORE undoing the sign flip.
	// The NaN sentinel (minimum signed value, 1000...0) must be detected
	// before XOR because the XOR of the zero encoding produces the same
	// bit pattern as the pre-flip NaN in fixed-width representations
	// (blockbinary). For int64_t with sign extension the post-XOR check
	// also worked, but this ordering is correct for both storage types.
	if (y_in == nan_encoding<B>(p)) {
		return std::numeric_limits<R>::quiet_NaN();
	}

	B y = y_in;
	// Undo two's complement sign flip
	y ^= bit_mask<B>(p - 1);

	const R pinf = bisection_aux_inf<R>();
	R xl = -pinf;
	R xh =  pinf;

	for (unsigned i = 0; i < p; ++i) {
		R xm = bisection_point<R>(xl, xh, g, f);

		// Read bit (MSB first)
		if (bit_test(y, p - 1 - i)) {
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

	// Dual-path storage: int64_t for small types (zero overhead),
	// blockbinary for wide types (nbits > 64).
	using bits_type = std::conditional_t<(nbits <= 64),
		int64_t,
		blockbinary<nbits, bt, BinaryNumberType::Signed>>;

	static_assert(nbits >= 2, "bisection requires at least 2 bits (sign + one value bit)");

	bisection() : _bits{} {}

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
		neg._bits = bisection_detail::negate_bits(_bits);
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
		if constexpr (std::is_same_v<bits_type, int64_t>) {
			uint64_t mask = (nbits < 64) ? ((uint64_t(1) << nbits) - 1) : ~uint64_t(0);
			_bits = static_cast<int64_t>(v & mask);
			if (nbits < 64 && (_bits & (int64_t(1) << (nbits - 1)))) {
				_bits |= ~static_cast<int64_t>(mask);
			}
		} else {
			_bits.setbits(v);
		}
	}

	/// Return the low 64 bits of the encoding.
	/// For nbits <= 64 this is the complete encoding (masked to nbits).
	/// For nbits > 64 this is LOSSY -- use at(index) to read individual
	/// bits for the full encoding. The setbits()/getbits() pair does NOT
	/// round-trip for wide types; it is provided as a convenience for
	/// initialization from small constants.
	uint64_t getbits() const {
		if constexpr (std::is_same_v<bits_type, int64_t>) {
			uint64_t mask = (nbits < 64) ? ((uint64_t(1) << nbits) - 1) : ~uint64_t(0);
			return static_cast<uint64_t>(_bits) & mask;
		} else {
			return static_cast<unsigned long long>(_bits);
		}
	}

	void setbit(unsigned index, bool v = true) {
		if constexpr (std::is_same_v<bits_type, int64_t>) {
			// Route through getbits/setbits to re-normalize sign extension
			// when modifying bits in the nbits-wide window.
			uint64_t raw = getbits();
			if (v) raw |= (uint64_t(1) << index);
			else   raw &= ~(uint64_t(1) << index);
			setbits(raw);
		} else {
			_bits.setbit(index, v);
		}
	}

	bool at(unsigned index) const {
		return bisection_detail::bit_test(_bits, index);
	}

	// -- Special value setters ------------------------------------

	void setzero() {
		if constexpr (std::is_same_v<bits_type, int64_t>) { _bits = 0; }
		else { _bits.setzero(); }
	}

	void maxpos() {
		if constexpr (std::is_same_v<bits_type, int64_t>) {
			uint64_t mask = (nbits < 64) ? ((uint64_t(1) << nbits) - 1) : ~uint64_t(0);
			_bits = static_cast<int64_t>(mask >> 1);  // 0111...1
		} else {
			_bits.maxpos();  // blockbinary Signed: 0111...1
		}
	}

	void minpos() {
		if constexpr (std::is_same_v<bits_type, int64_t>) { _bits = 1; }
		else { _bits.minpos(); }  // 0000...001
	}

	void maxneg() {
		if constexpr (std::is_same_v<bits_type, int64_t>) {
			if (nbits < 64) _bits = -(int64_t(1) << (nbits - 1)) + 1;
			else            _bits = std::numeric_limits<int64_t>::min() + 1;
		} else {
			// maxneg = 1000...0001 (one past the NaN sentinel)
			_bits.maxneg();  // 1000...0
			++_bits;         // 1000...1
		}
	}

	void minneg() {
		if constexpr (std::is_same_v<bits_type, int64_t>) { _bits = -1; }
		else { _bits.minneg(); }  // 1111...1 for Signed = -1
	}

	void setnan() {
		_bits = bisection_detail::nan_encoding<bits_type>(nbits);
	}

	void setinf(bool negative = false) {
		if (negative) maxneg(); else maxpos();
	}

	// -- State queries --------------------------------------------

	bool iszero() const {
		if constexpr (std::is_same_v<bits_type, int64_t>) { return _bits == 0; }
		else { return _bits.iszero(); }
	}
	bool sign() const {
		if constexpr (std::is_same_v<bits_type, int64_t>) { return _bits < 0; }
		else { return _bits.sign(); }
	}
	bool isnan() const {
		return _bits == bisection_detail::nan_encoding<bits_type>(nbits);
	}
	bool isinf() const { return false; }

private:
	bits_type _bits;

	AuxReal decode_aux() const {
		return bisection_decode<AuxReal, bits_type>(_bits, nbits, Generator{}, Refinement{});
	}

	bisection& assign_from_aux(const AuxReal& rhs) {
		using std::isinf;
		if (isinf(rhs)) {
			if (rhs > AuxReal(0)) maxpos(); else maxneg();
			return *this;
		}
		_bits = bisection_encode<AuxReal, bits_type>(rhs, nbits, Generator{}, Refinement{});
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
