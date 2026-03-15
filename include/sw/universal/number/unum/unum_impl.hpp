#pragma once
// unum_impl.hpp: implementation of the unum Type I number system
//
// unum Type I is Gustafson's original universal number format with
// variable-precision exponent and fraction fields, plus a ubit
// (uncertainty bit) that indicates whether the value is exact or
// represents an open interval to the next unum.
//
// Template parameters:
//   esizesize -- number of bits in the exponent-size field
//   fsizesize -- number of bits in the fraction-size field
//   bt        -- block type for underlying storage (default uint8_t)
//
// Bit layout (LSB to MSB within the storage):
//   [ubit(1)] [fsize_field(fsizesize)] [esize_field(esizesize)]
//   [fraction(fsize+1 bits)] [exponent(esize+1 bits)] [sign(1)]
//
// The utag (ubit + fsize + esize fields) occupies fixed positions
// at the bottom of the word. The variable-length fraction, exponent,
// and sign fields sit above, with total width depending on the
// decoded esize and fsize values.
//
// Conventions (per Gustafson, "The End of Error"):
//   esize field value (0..2^esizesize - 1): exponent has esize+1 bits
//   fsize field value (0..2^fsizesize - 1): fraction has fsize bits
//   ubit = 0: exact value
//   ubit = 1: open interval (value, next_unum(value))
//   NaN encoding: all bits set to 1 (quiet NaN)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

// fill helpers
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt>& minpos(unum<esizesize, fsizesize, bt>& u) {
	u.clear();
	u.setbit(0, false);  // ubit = 0 (exact)
	// fsize = 0 (0 fraction bits), esize = 0 (1 exponent bit)
	// exponent = 0, fraction = (none), sign = 0
	// This gives the value (+, 2^(0 - bias), 1.0) = minpos
	// For esize=0: 1 exponent bit, bias = 0, exponent value 0 -> 2^0 = 1
	// Actually minpos with esize=0, fsize=0 is 1.0 (smallest exact positive)
	// The true minpos is with the smallest nonzero encoding
	// For now set the minimal nonzero encoding: sign=0, exp=0..01, frac=0, ubit=0
	unsigned utag = 1u + fsizesize + esizesize;
	u.setbit(utag, true);  // set LSB of exponent field to 1
	return u;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt>& maxpos(unum<esizesize, fsizesize, bt>& u) {
	u.setnan();  // set all bits
	u.setbit(0, false);   // ubit = 0 (exact)
	// clear the sign bit: need to find it based on max esize/fsize
	unsigned maxes = (1u << esizesize) - 1u;
	unsigned maxfs = (1u << fsizesize) - 1u;
	unsigned nbits_used = 1u + (maxes + 1u) + maxfs + esizesize + fsizesize + 1u;
	u.setbit(nbits_used - 1u, false);  // clear sign bit
	return u;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt>& minneg(unum<esizesize, fsizesize, bt>& u) {
	minpos(u);
	// set the sign bit
	unsigned utag = 1u + fsizesize + esizesize;
	u.setbit(utag + 1u, true);  // sign bit for minimal encoding
	return u;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt>& maxneg(unum<esizesize, fsizesize, bt>& u) {
	maxpos(u);
	// set the sign bit
	unsigned maxes = (1u << esizesize) - 1u;
	unsigned maxfs = (1u << fsizesize) - 1u;
	unsigned nbits_used = 1u + (maxes + 1u) + maxfs + esizesize + fsizesize + 1u;
	u.setbit(nbits_used - 1u, true);  // set sign bit
	return u;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt>& qnan(unum<esizesize, fsizesize, bt>& u) {
	u.setnan();
	return u;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt>& snan(unum<esizesize, fsizesize, bt>& u) {
	u.setnan();
	return u;
}

///////////////////////////////////////////////////////////////////////////
// unum Type I class

template<unsigned esizesize, unsigned fsizesize, typename bt>
class unum {
public:
	// maximum exponent field size in bits
	static constexpr unsigned maxesize    = (1u << esizesize);
	// maximum fraction field size in bits
	static constexpr unsigned maxfsize    = (1u << fsizesize) - 1u;
	// utag size: ubit + fsizesize + esizesize
	static constexpr unsigned utagsize    = 1u + fsizesize + esizesize;
	// maximum total bits in a unum word
	// sign(1) + max_exponent(maxesize) + max_fraction(maxfsize) + esize_field(esizesize) + fsize_field(fsizesize) + ubit(1)
	static constexpr unsigned maxbits     = 1u + maxesize + maxfsize + esizesize + fsizesize + 1u;

	// storage type
	using StorageType = blockbinary<maxbits, bt>;

	unum() { _bits.clear(); }

	unum(const unum&) = default;
	unum(unum&&) = default;
	unum& operator=(const unum&) = default;
	unum& operator=(unum&&) = default;

	// constructors from native types (Phase 2 - stubs for now)
	unum(signed char initial_value)        { _bits.clear(); *this = initial_value; }
	unum(short initial_value)              { _bits.clear(); *this = initial_value; }
	unum(int initial_value)                { _bits.clear(); *this = initial_value; }
	unum(long long initial_value)          { _bits.clear(); *this = initial_value; }
	unum(unsigned long long initial_value) { _bits.clear(); *this = initial_value; }
	unum(float initial_value)              { _bits.clear(); *this = initial_value; }
	unum(double initial_value)             { _bits.clear(); *this = initial_value; }
	unum(long double initial_value)        { _bits.clear(); *this = initial_value; }

	// assignment operators (Phase 2 - stubs)
	unum& operator=(signed char rhs)        { return *this = static_cast<long long>(rhs); }
	unum& operator=(short rhs)              { return *this = static_cast<long long>(rhs); }
	unum& operator=(int rhs)                { return *this = static_cast<long long>(rhs); }
	unum& operator=(long long rhs)          { _bits.clear(); return *this; }
	unum& operator=(unsigned long long rhs) { _bits.clear(); return *this; }
	unum& operator=(float rhs)              { _bits.clear(); return *this; }
	unum& operator=(double rhs)             { _bits.clear(); return *this; }
	unum& operator=(long double rhs)        { _bits.clear(); return *this; }

	// arithmetic operators (Phase 4 - stubs)
	unum operator-() const { unum neg(*this); return neg; }
	unum& operator+=(const unum& rhs) { return *this; }
	unum& operator-=(const unum& rhs) { return *this; }
	unum& operator*=(const unum& rhs) { return *this; }
	unum& operator/=(const unum& rhs) { return *this; }
	unum& operator++() { return *this; }
	unum  operator++(int) { unum tmp(*this); operator++(); return tmp; }
	unum& operator--() { return *this; }
	unum  operator--(int) { unum tmp(*this); operator--(); return tmp; }

	// modifiers
	void clear() { _bits.clear(); }
	void setzero() { _bits.clear(); }
	void setnan() {
		// NaN encoding: all bits set in the maximum-width configuration
		for (unsigned i = 0; i < maxbits; ++i) _bits.setbit(i, true);
	}
	void setbits(uint64_t v) { _bits.setbits(v); }
	void setbit(unsigned i, bool v = true) {
		if (i < maxbits) _bits.setbit(i, v);
	}

	// raw bit access
	StorageType bits() const { return _bits; }
	bool at(unsigned i) const {
		if (i < maxbits) return _bits.at(i);
		return false;
	}

	// decode the utag fields (always at fixed positions)
	bool  ubit()       const { return _bits.at(0); }
	unsigned fsize()   const {
		unsigned fs = 0;
		for (unsigned i = 0; i < fsizesize; ++i) {
			if (_bits.at(1u + i)) fs |= (1u << i);
		}
		return fs;
	}
	unsigned esize()   const {
		unsigned es = 0;
		for (unsigned i = 0; i < esizesize; ++i) {
			if (_bits.at(1u + fsizesize + i)) es |= (1u << i);
		}
		return es;
	}

	// number of bits actually used by this unum word
	unsigned nbits_used() const {
		unsigned es = esize();
		unsigned fs = fsize();
		return 1u + (es + 1u) + fs + esizesize + fsizesize + 1u;
	}

	// decode the sign bit (position depends on esize/fsize)
	bool sign() const {
		return _bits.at(nbits_used() - 1u);
	}

	// decode the exponent field (esize+1 bits above the utag+fraction)
	uint64_t exponent() const {
		unsigned es = esize();
		unsigned fs = fsize();
		unsigned exp_start = utagsize + fs;
		uint64_t exp = 0;
		for (unsigned i = 0; i < es + 1u; ++i) {
			if (_bits.at(exp_start + i)) exp |= (1ull << i);
		}
		return exp;
	}

	// decode the fraction field (fsize bits above the utag)
	uint64_t fraction() const {
		unsigned fs = fsize();
		uint64_t frac = 0;
		for (unsigned i = 0; i < fs; ++i) {
			if (_bits.at(utagsize + i)) frac |= (1ull << i);
		}
		return frac;
	}

	// selectors
	bool iszero() const {
		// zero: sign=0, exponent=all zeros, fraction=all zeros, ubit=0
		// simplification: all bits are 0
		for (unsigned i = 0; i < maxbits; ++i) {
			if (_bits.at(i)) return false;
		}
		return true;
	}
	bool isnan() const {
		// NaN: all bits set to 1 in max-width configuration
		for (unsigned i = 0; i < maxbits; ++i) {
			if (!_bits.at(i)) return false;
		}
		return true;
	}
	bool isneg()  const { return sign(); }
	bool ispos()  const { return !sign(); }
	bool isinf()  const { return false; }  // unum Type I has no infinity encoding
	bool exact()  const { return !ubit(); }

	// conversion to native types (Phase 2 - basic stub via decoded fields)
	double to_double() const {
		if (iszero()) return 0.0;
		if (isnan()) return std::numeric_limits<double>::quiet_NaN();
		// basic conversion using decoded fields
		unsigned es = esize();
		unsigned fs = fsize();
		uint64_t exp = exponent();
		uint64_t frac = fraction();
		bool s = sign();
		// bias = 2^es - 1 (standard IEEE-like bias for es+1 exponent bits)
		int bias = (1 << es) - 1;
		int e = static_cast<int>(exp) - bias;
		// hidden bit: value = 1.fraction * 2^e
		double f = 1.0;
		for (unsigned i = 0; i < fs; ++i) {
			if (frac & (1ull << (fs - 1u - i))) {
				f += std::ldexp(1.0, -static_cast<int>(i + 1));
			}
		}
		double value = std::ldexp(f, e);
		return s ? -value : value;
	}
	float to_float() const { return static_cast<float>(to_double()); }
	long double to_long_double() const { return static_cast<long double>(to_double()); }

	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }
	explicit operator long double() const { return to_long_double(); }

private:
	StorageType _bits;

	// friend declarations
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend std::ostream& operator<<(std::ostream& ostr, const unum<nesizesize, nfsizesize, nbt>& v);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend std::istream& operator>>(std::istream& istr, unum<nesizesize, nfsizesize, nbt>& v);

	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator==(const unum<nesizesize, nfsizesize, nbt>& lhs, const unum<nesizesize, nfsizesize, nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator!=(const unum<nesizesize, nfsizesize, nbt>& lhs, const unum<nesizesize, nfsizesize, nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator< (const unum<nesizesize, nfsizesize, nbt>& lhs, const unum<nesizesize, nfsizesize, nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator> (const unum<nesizesize, nfsizesize, nbt>& lhs, const unum<nesizesize, nfsizesize, nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator<=(const unum<nesizesize, nfsizesize, nbt>& lhs, const unum<nesizesize, nfsizesize, nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator>=(const unum<nesizesize, nfsizesize, nbt>& lhs, const unum<nesizesize, nfsizesize, nbt>& rhs);
};

////////////////////// IO operators

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const unum<esizesize, fsizesize, bt>& v) {
	if (v.isnan()) {
		ostr << "NaN";
	}
	else if (v.iszero()) {
		ostr << 0;
	}
	else {
		// use decoded double value
		double d = v.to_double();
		ostr << d;
	}
	return ostr;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::istream& operator>>(std::istream& istr, unum<esizesize, fsizesize, bt>& v) {
	double d;
	istr >> d;
	v = d;
	return istr;
}

////////////////////// comparison operators (Phase 3 - basic bit comparison for now)

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline bool operator==(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	return lhs._bits == rhs._bits;
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline bool operator!=(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline bool operator< (const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	return lhs.to_double() < rhs.to_double();  // stub: compare via double
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline bool operator> (const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	return operator<(rhs, lhs);
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline bool operator<=(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	return !operator>(lhs, rhs);
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline bool operator>=(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	return !operator<(lhs, rhs);
}

////////////////////// binary arithmetic operators (Phase 4 - stubs)

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum<esizesize, fsizesize, bt> operator+(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	unum<esizesize, fsizesize, bt> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum<esizesize, fsizesize, bt> operator-(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	unum<esizesize, fsizesize, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum<esizesize, fsizesize, bt> operator*(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	unum<esizesize, fsizesize, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum<esizesize, fsizesize, bt> operator/(const unum<esizesize, fsizesize, bt>& lhs, const unum<esizesize, fsizesize, bt>& rhs) {
	unum<esizesize, fsizesize, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

////////////////////// helper functions

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::string components(const unum<esizesize, fsizesize, bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << "zero";
	}
	else if (v.isnan()) {
		s << "NaN";
	}
	else {
		s << (v.sign() ? "-" : "+")
		  << " esize:" << (v.esize() + 1u) << " fsize:" << v.fsize()
		  << " exp:" << v.exponent() << " frac:" << v.fraction()
		  << " ubit:" << v.ubit();
	}
	return s.str();
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> abs(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> a(v);
	// clear the sign bit
	unsigned sign_pos = a.nbits_used() - 1u;
	a.setbit(sign_pos, false);
	return a;
}

}} // namespace sw::universal
