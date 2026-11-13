#pragma once
// triple.hpp: definition of an abstract (sign, scale, significant) representation of an approximation to a real triple
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iosfwd>    // std::ostream/std::istream in the friend declarations (#1334)
#include <limits>
#include <tuple>

#include <universal/native/ieee754_core.hpp>   // the bit-manipulation half (#1334)
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

// Forward definitions
template<size_t fbits, typename bt> class triple;
template<size_t fbits, typename bt> triple<fbits,bt> abs(const triple<fbits,bt>& v);

// template class representing an abstract real triple using a template size for the number of fraction bits
template<size_t fbits, typename bt>
class triple {
public:
	static constexpr size_t fhbits = fbits + 1;    // number of fraction bits including the hidden bit

	triple() : _sign{false}, _scale{0}, _fraction{0}, _inf{false}, _zero{true}, _nan{false} {}

	triple(bool sign, 
		   int scale, 
		   const blockbinary<fbits,bt>& fraction_without_hidden_bit, 
           bool zero = true, 
		   bool inf = false) 
      : _sign{sign}, 
		_scale{scale}, 
		_fraction{fraction_without_hidden_bit}, 
		_inf{inf}, 
		_zero{zero}, 
		_nan{false} {}


	triple(const triple&) = default;
	triple(triple&&) = default;

	triple& operator=(const triple&) = default;
	triple& operator=(triple&&) = default;

	// decorated constructors
	triple(signed char initial_triple)        { *this = initial_triple; }
	triple(short initial_triple)              { *this = initial_triple; }
	triple(int initial_triple)                { *this = initial_triple; }
	triple(long initial_triple)               { *this = initial_triple; }
	triple(long long initial_triple)          { *this = initial_triple; }
	triple(char initial_triple)               { *this = initial_triple; }
	triple(unsigned short initial_triple)     { *this = initial_triple; }
	triple(unsigned int initial_triple)       { *this = initial_triple; }
	triple(unsigned long initial_triple)      { *this = initial_triple; }
	triple(unsigned long long initial_triple) { *this = initial_triple; }
	triple(float initial_triple)              { *this = initial_triple; }
	triple(double initial_triple) : triple{}  { *this = initial_triple; }
	triple(long double initial_triple)        { *this = initial_triple; }

	constexpr triple& operator=(signed char rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr triple& operator=(short rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr triple& operator=(int rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr triple& operator=(long rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	// the integer and floating-point assignments were empty stubs, so a triple held whatever its
	// members were left at: triple(2.5) read back 1.0 (#2053)
	constexpr triple& operator=(long long rhs) {
		// the magnitude in the unsigned domain: -rhs overflows for LLONG_MIN
		const bool negative = (rhs < 0);
		assign_magnitude(negative ? (0ull - static_cast<unsigned long long>(rhs)) : static_cast<unsigned long long>(rhs));
		_sign = negative && !_zero;
		return *this;
	}
	triple& operator=(char rhs) {
		return *this = static_cast<long long>(rhs);   // plain char may be signed
	}
	triple& operator=(unsigned short rhs) {
		return *this = static_cast<unsigned long long>(rhs);
	}
	triple& operator=(unsigned int rhs) {
		return *this = static_cast<unsigned long long>(rhs);
	}
	triple& operator=(unsigned long rhs) {
		return *this = static_cast<unsigned long long>(rhs);
	}
	triple& operator=(unsigned long long rhs) {
		assign_magnitude(rhs);
		return *this;
	}
	triple& operator=(float rhs)       { return assign_real(rhs); }
	triple& operator=(double rhs)      { return assign_real(rhs); }
	triple& operator=(long double rhs) { return assign_real(rhs); }

	// operators
	constexpr triple operator-() const {
		triple negated(*this);   // keeps NaN, which the (sign, scale, fraction, zero, inf) constructor dropped
		negated._sign = !_sign;
		return negated;
	}

	// modifiers
	constexpr void reset() & {
		_sign  = false;
		_scale = 0;
		_inf = false;
		_zero = false;
		_nan = false;
		_fraction.reset(); // not constexpr
                // _fraction= bitblock<fbits>{}; // work around
	}
	void set(bool sign, int scale, blockbinary<fbits,bt> fraction_without_hidden_bit, bool zero, bool inf, bool nan = false) {
		_sign     = sign;
		_scale    = scale;
		_fraction = fraction_without_hidden_bit;
		_zero     = zero;
		_inf      = inf;
		_nan      = nan;
	}
	void setzero() {
		_zero     = true;
		_sign     = false;
		_inf      = false;
		_nan      = false;
		_scale    = 0;
		_fraction.reset();
	}
	void setinf() {      // this maps to NaR on the posit side, and that has a sign = 1
		_inf      = true;
		_sign     = true;
		_zero     = false;
		_nan      = false;
		_scale    = 0;
		_fraction.reset();
	}
	void setnan() {		// this will also map to NaR
		_nan      = true;
		_sign     = true;
		_zero     = false;
		_inf      = false;
		_scale    = 0;
		_fraction.reset();
	}
	inline void setExponent(int e) { _scale = e; }
	inline bool isneg() const { return _sign; }
	inline bool ispos() const { return !_sign; }
	inline constexpr bool iszero() const { return _zero; }
	inline constexpr bool isinf() const { return _inf; }
	inline constexpr bool isnan() const { return _nan; }
	inline bool sign() const { return _sign; }
	inline int scale() const { return _scale; }
	blockbinary<fbits,bt> fraction() const { return _fraction; }

	int sign_value() const { return (_sign ? -1 : 1); }
	double scale_value() const {
		if (_zero) return (long double)(0.0);
		return std::pow((long double)2.0, (long double)_scale);
	}
	template<typename Ty = double>
	Ty fraction_value() const {
		if (_zero) return (long double)0.0;
		Ty v = 1.0;
		Ty scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_fraction.test(i)) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}
	// the special values first: the product read an infinity or a NaN as 1.0 * 2^scale
	template<typename Real>
	Real to_native() const {
		if (_nan) return std::numeric_limits<Real>::quiet_NaN();
		if (_inf) return _sign ? -std::numeric_limits<Real>::infinity() : std::numeric_limits<Real>::infinity();
		if (_zero) return _sign ? -Real(0) : Real(0);
		return static_cast<Real>(sign_value()) * std::ldexp(fraction_value<Real>(), _scale);
	}
	long double to_long_double() const { return to_native<long double>(); }
	double to_double() const           { return to_native<double>(); }
	float to_float() const             { return to_native<float>(); }
	// Maybe remove explicit
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

private:
	// a non-zero integer magnitude as 1.fff * 2^scale, rounded to fbits to nearest, ties to even
	constexpr void assign_magnitude(unsigned long long v) {
		reset();
		if (v == 0) { _zero = true; return; }
		int msb = 63;
		while (((v >> msb) & 1ull) == 0) --msb;
		_scale = msb;
		const unsigned long long below = (msb == 0) ? 0ull : (v & ((1ull << msb) - 1ull));   // the bits under the hidden bit
		const int nbelow = msb;
		if (nbelow <= static_cast<int>(fbits)) {
			for (int i = 0; i < nbelow; ++i) {
				if ((below >> i) & 1ull) _fraction.setbit(static_cast<unsigned>(static_cast<int>(fbits) - nbelow + i));
			}
			return;
		}
		const int drop = nbelow - static_cast<int>(fbits);
		unsigned long long kept = below >> drop;
		const unsigned long long half = 1ull << (drop - 1);
		const unsigned long long rest = below & ((half << 1) - 1ull);
		if (rest > half || (rest == half && (kept & 1ull))) {
			++kept;
			if constexpr (fbits < 64) {
				if (kept >> fbits) { kept = 0; ++_scale; }   // 1.11..1 + ulp = 10.00..0
			}
		}
		for (unsigned i = 0; i < fbits && i < 64; ++i) {
			if ((kept >> i) & 1ull) _fraction.setbit(i);
		}
	}
	// a floating-point value as 1.fff * 2^scale, rounded to fbits to nearest, ties to even. The
	// fraction bits come from doubling the significand, which is exact in binary floating
	// point, so subnormals and every long double format need no special handling.
	template<typename Real>
	triple& assign_real(Real rhs) {
		reset();
		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_zero = true;
			_sign = std::signbit(rhs);
			return *this;
		case FP_INFINITE:
			_inf  = true;
			_sign = (rhs < 0);   // was always true: +inf read back as -inf
			return *this;
		case FP_NAN:
			_nan  = true;
			_sign = true;
			return *this;
		default:
			break;
		}
		_sign = std::signbit(rhs);
		int e = 0;
		Real f = std::frexp(std::fabs(rhs), &e);   // f in [0.5, 1)
		_scale = e - 1;
		f = f * Real(2) - Real(1);                  // the bits below the hidden bit, in [0, 1)
		for (int i = static_cast<int>(fbits) - 1; i >= 0; --i) {
			f *= Real(2);
			if (f >= Real(1)) {
				_fraction.setbit(static_cast<unsigned>(i));
				f -= Real(1);
			}
		}
		// what is left is the discarded part, in units of the last kept bit: round to nearest even
		const bool lsb = (fbits > 0) ? _fraction.test(0) : true;
		if (f > Real(0.5) || (f == Real(0.5) && lsb)) {
			bool carry = true;
			for (unsigned i = 0; i < fbits && carry; ++i) {
				carry = _fraction.test(i);
				_fraction.setbit(i, !carry);
			}
			if (carry) ++_scale;   // the fraction wrapped: 1.11..1 + ulp = 10.00..0
		}
		return *this;
	}

	bool                  _sign;
	int                   _scale;
	blockbinary<fbits,bt> _fraction;   // managing the significand as a 2's complement number
	bool                  _inf;
	bool                  _zero;
	bool                  _nan;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nfbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const triple<nfbits,nbt>& r);
	template<size_t nfbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, triple<nfbits,nbt>& r);

	template<size_t nfbits, typename nbt>
	friend bool operator==(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator!=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator< (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator> (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator<=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator>=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
};

// the fraction bits as an unsigned number: blockbinary's own ordering is 2's complement
template<unsigned N, typename nbt, BinaryNumberType E>
inline int compare_fraction_bits(const blockbinary<N, nbt, E>& a, const blockbinary<N, nbt, E>& b) {
	for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
		const bool x = a.test(static_cast<unsigned>(i)), y = b.test(static_cast<unsigned>(i));
		if (x != y) return x ? 1 : -1;
	}
	return 0;
}

// IEEE-754 semantics: NaN is unordered, +0 == -0, -inf < every finite value < +inf (every
// infinity used to compare as -inf, and the fractions were compared as signed numbers)
template<size_t nfbits, typename nbt>
inline bool operator==(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) {
	if (lhs._nan || rhs._nan) return false;
	if (lhs._zero || rhs._zero) return lhs._zero && rhs._zero;
	if (lhs._inf || rhs._inf) return lhs._inf && rhs._inf && lhs._sign == rhs._sign;
	return lhs._sign == rhs._sign && lhs._scale == rhs._scale && compare_fraction_bits(lhs._fraction, rhs._fraction) == 0;
}
template<size_t nfbits, typename nbt>
inline bool operator!=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nfbits, typename nbt>
inline bool operator< (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) {
	if (lhs._nan || rhs._nan) return false;
	if (lhs._inf) return lhs._sign && !(rhs._inf && rhs._sign);
	if (rhs._inf) return !rhs._sign;
	if (lhs._zero) return rhs._zero ? false : !rhs._sign;
	if (rhs._zero) return lhs._sign;
	if (lhs._sign != rhs._sign) return lhs._sign;
	int order = (lhs._scale != rhs._scale) ? (lhs._scale < rhs._scale ? -1 : 1) : compare_fraction_bits(lhs._fraction, rhs._fraction);
	if (order == 0) return false;
	return lhs._sign ? (order > 0) : (order < 0);
}
template<size_t nfbits, typename nbt>
inline bool operator> (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nfbits, typename nbt>
inline bool operator<=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return operator< (lhs, rhs) || operator==(lhs, rhs); }
template<size_t nfbits, typename nbt>
inline bool operator>=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return operator< (rhs, lhs) || operator==(lhs, rhs); }

/// Magnitude of a scientific notation triple (equivalent to turning the sign bit off).
template<size_t nfbits, typename BlockType>
triple<nfbits,BlockType> abs(const triple<nfbits,BlockType>& v) {
	return v.isneg() ? -v : v;   // keeps NaN and infinity, which the constructor dropped
}


}}  // namespace sw::universal
