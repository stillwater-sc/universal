#pragma once
// twoparam_impl.hpp: definition of a two-parameter parameterized number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee-754.hpp>
#include <universal/blockbin/blockbinary.hpp>
#include <universal/abstract/triple.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<unsigned nbits, unsigned es, typename bt> class twoparam;
template<unsigned nbits, unsigned es, typename bt> twoparam<nbits, es, bt> abs(const twoparam<nbits, es, bt>& v);

// convert a floating-point value to a specific twoparam configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned es, typename bt>
inline twoparam<nbits, es, bt>& convert(const triple<nbits, es, bt>& v, twoparam<nbits, es, bt>& p) {
	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnan();
		return p;
	}
	return p;
}

template<unsigned nbits, unsigned es, typename bt>
twoparam<nbits, es, bt>& minpos(twoparam<nbits, es, bt>& lminpos) {
	return lminpos;
}
template<unsigned nbits, unsigned es, typename bt>
twoparam<nbits, es, bt>& maxpos(twoparam<nbits, es, bt>& lmaxpos) {
	return lmaxpos;
}
template<unsigned nbits, unsigned es, typename bt>
twoparam<nbits, es, bt>& minneg(twoparam<nbits, es, bt>& lminneg) {
	return lminneg;
}
template<unsigned nbits, unsigned es, typename bt>
twoparam<nbits, es, bt>& maxneg(twoparam<nbits, es, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<unsigned nbits, unsigned es, typename bt = uint8_t>
class twoparam {
public:
	twoparam() {}

	twoparam(const twoparam&) = default;
	twoparam(twoparam&&) = default;

	twoparam& operator=(const twoparam&) = default;
	twoparam& operator=(twoparam&&) = default;

	twoparam(signed char initial_value)        { *this = initial_value; }
	twoparam(short initial_value)              { *this = initial_value; }
	twoparam(int initial_value)                { *this = initial_value; }
	twoparam(long long initial_value)          { *this = initial_value; }
	twoparam(unsigned long long initial_value) { *this = initial_value; }
	twoparam(float initial_value)              { *this = initial_value; }
	twoparam(double initial_value)             { *this = initial_value; }
	twoparam(long double initial_value)        { *this = initial_value; }

	// assignment operators
	twoparam& operator=(signed char rhs) { return *this = (long long)(rhs); }
	twoparam& operator=(short rhs) { return *this = (long long)(rhs); }
	twoparam& operator=(int rhs) { return *this = (long long)(rhs); }
	twoparam& operator=(long long rhs) { return *this; }
	twoparam& operator=(unsigned long long rhs) { return *this; }
	twoparam& operator=(float rhs) { return *this; } 
	twoparam& operator=(double rhs) { return *this; }
	twoparam& operator=(long double rhs) { return *this; }

	// arithmetic operators
	// prefix operator
	twoparam operator-() const {				
		return *this;
	}

	// in-place arithmetic assignment operators
	twoparam& operator+=(const twoparam& rhs) { return *this; }
	twoparam& operator+=(double rhs) { return *this += twoparam(rhs); }
	twoparam& operator-=(const twoparam& rhs) { return *this; }
	twoparam& operator-=(double rhs) { return *this -= twoparam<nbits>(rhs); }
	twoparam& operator*=(const twoparam& rhs) { return *this; }
	twoparam& operator*=(double rhs) { return *this *= twoparam<nbits>(rhs); }
	twoparam& operator/=(const twoparam& rhs) { return *this; }
	twoparam& operator/=(double rhs) { return *this /= twoparam<nbits>(rhs); }

	// prefix/postfix operators
	twoparam& operator++() {
		return *this;
	}
	twoparam operator++(int) {
		twoparam tmp(*this);
		operator++();
		return tmp;
	}
	twoparam& operator--() {
		return *this;
	}
	twoparam operator--(int) {
		twoparam tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	void reset() {	}

	// selectors
	inline bool isneg() const { return false; }
	inline bool iszero() const { return false; }
	inline bool isinf() const { return false; }
	inline bool isnan() const { return false; }
	inline bool sign() const { return false; }
	inline int scale() const { return false; }
	inline std::string get() const { return std::string("tbd"); }


	long double to_long_double() const {
		return 0.0l;
	}
	double to_double() const {
		return 0.0;
	}
	float to_float() const {
		return 0.0f;
	}
	// Maybe remove explicit
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

private:

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const twoparam<nnbits, nes, nbt>& r);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::istream& operator>> (std::istream& istr, twoparam<nnbits, nes, nbt>& r);

	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator==(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator!=(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator< (const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator> (const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator<=(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator>=(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs);
};

////////////////////// operators
template<unsigned nnbits, unsigned nes, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const twoparam<nnbits, nes, nbt>& v) {

	return ostr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const twoparam<nnbits, nes, nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator==(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs) { return false; }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator!=(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator< (const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs) { return false; }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator> (const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator<=(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator>=(const twoparam<nnbits, nes, nbt>& lhs, const twoparam<nnbits, nes, nbt>& rhs) { return !operator< (lhs, rhs); }

// twoparam - twoparam binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
inline twoparam<nbits, es, bt> operator+(const twoparam<nbits, es, bt>& lhs, const twoparam<nbits, es, bt>& rhs) {
	twoparam<nbits> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
inline twoparam<nbits, es, bt> operator-(const twoparam<nbits, es, bt>& lhs, const twoparam<nbits, es, bt>& rhs) {
	twoparam<nbits> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
inline twoparam<nbits, es, bt> operator*(const twoparam<nbits, es, bt>& lhs, const twoparam<nbits, es, bt>& rhs) {
	twoparam<nbits> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
inline twoparam<nbits, es, bt> operator/(const twoparam<nbits, es, bt>& lhs, const twoparam<nbits, es, bt>& rhs) {
	twoparam<nbits> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<unsigned nbits, unsigned es, typename bt>
inline std::string components(const twoparam<nbits, es, bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, unsigned es, typename bt>
twoparam<nbits> abs(const twoparam<nbits, es, bt>& v) {
	return twoparam<nbits>();
}


}}  // namespace sw::universal
