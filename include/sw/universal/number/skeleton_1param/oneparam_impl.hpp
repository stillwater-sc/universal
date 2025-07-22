#pragma once
// oneparam_impl.hpp: definition of a one-parameter parameterized number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee-754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<size_t nbits, typename bt> class oneparam;
template<size_t nbits, typename bt> oneparam<nbits,bt> abs(const oneparam<nbits,bt>& v);

// convert a floating-point value to a specific oneparam configuration. Semantically, p = v, return reference to p
template<size_t nbits, typename bt>
inline oneparam<nbits, bt>& convert(const triple<nbits,bt>& v, oneparam<nbits,bt>& p) {
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

template<size_t nbits, typename bt>
oneparam<nbits, bt>& minpos(oneparam<nbits, bt>& lminpos) {
	return lminpos;
}
template<size_t nbits, typename bt>
oneparam<nbits, bt>& maxpos(oneparam<nbits, bt>& lmaxpos) {
	return lmaxpos;
}
template<size_t nbits, typename bt>
oneparam<nbits, bt>& minneg(oneparam<nbits, bt>& lminneg) {
	return lminneg;
}
template<size_t nbits, typename bt>
oneparam<nbits, bt>& maxneg(oneparam<nbits, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t nbits, typename bt = uint8_t>
class oneparam {
public:
	oneparam() {}

	oneparam(const oneparam&) = default;
	oneparam(oneparam&&) = default;

	oneparam& operator=(const oneparam&) = default;
	oneparam& operator=(oneparam&&) = default;

	oneparam(signed char initial_value)        { *this = initial_value; }
	oneparam(short initial_value)              { *this = initial_value; }
	oneparam(int initial_value)                { *this = initial_value; }
	oneparam(long long initial_value)          { *this = initial_value; }
	oneparam(unsigned long long initial_value) { *this = initial_value; }
	oneparam(float initial_value)              { *this = initial_value; }
	oneparam(double initial_value)             { *this = initial_value; }
	oneparam(long double initial_value)        { *this = initial_value; }

	// assignment operators
	oneparam& operator=(signed char rhs) { return *this = (long long)(rhs); }
	oneparam& operator=(short rhs) { return *this = (long long)(rhs); }
	oneparam& operator=(int rhs) { return *this = (long long)(rhs); }
	oneparam& operator=(long long rhs) { return *this; }
	oneparam& operator=(unsigned long long rhs) { return *this; }
	oneparam& operator=(float rhs) { return *this; } 
	oneparam& operator=(double rhs) { return *this; }
	oneparam& operator=(long double rhs) { return *this; }

	// arithmetic operators
	// prefix operator
	oneparam operator-() const {				
		return *this;
	}

	// in-place arithmetic assignment operators
	oneparam& operator+=(const oneparam& rhs) { return *this; }
	oneparam& operator+=(double rhs) { return *this += oneparam(rhs); }
	oneparam& operator-=(const oneparam& rhs) { return *this; }
	oneparam& operator-=(double rhs) { return *this -= oneparam<nbits>(rhs); }
	oneparam& operator*=(const oneparam& rhs) { return *this; }
	oneparam& operator*=(double rhs) { return *this *= oneparam<nbits>(rhs); }
	oneparam& operator/=(const oneparam& rhs) { return *this; }
	oneparam& operator/=(double rhs) { return *this /= oneparam<nbits>(rhs); }

	// prefix/postfix operators
	oneparam& operator++() {
		return *this;
	}
	oneparam operator++(int) {
		oneparam tmp(*this);
		operator++();
		return tmp;
	}
	oneparam& operator--() {
		return *this;
	}
	oneparam operator--(int) {
		oneparam tmp(*this);
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
	template<size_t nnbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const oneparam<nnbits,nbt>& r);
	template<size_t nnbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, oneparam<nnbits,nbt>& r);

	template<size_t nnbits, typename nbt>
	friend bool operator==(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator!=(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator< (const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator> (const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator<=(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator>=(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const oneparam<nnbits,nbt>& v) {

	return ostr;
}

template<size_t nnbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const oneparam<nnbits,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nnbits, typename nbt>
inline bool operator==(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs) { return false; }
template<size_t nnbits, typename nbt>
inline bool operator!=(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator< (const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs) { return false; }
template<size_t nnbits, typename nbt>
inline bool operator> (const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, typename nbt>
inline bool operator<=(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator>=(const oneparam<nnbits,nbt>& lhs, const oneparam<nnbits,nbt>& rhs) { return !operator< (lhs, rhs); }

// oneparam - oneparam binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, typename bt>
inline oneparam<nbits, bt> operator+(const oneparam<nbits, bt>& lhs, const oneparam<nbits, bt>& rhs) {
	oneparam<nbits> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, typename bt>
inline oneparam<nbits, bt> operator-(const oneparam<nbits, bt>& lhs, const oneparam<nbits, bt>& rhs) {
	oneparam<nbits> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, typename bt>
inline oneparam<nbits, bt> operator*(const oneparam<nbits, bt>& lhs, const oneparam<nbits, bt>& rhs) {
	oneparam<nbits> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, typename bt>
inline oneparam<nbits, bt> operator/(const oneparam<nbits, bt>& lhs, const oneparam<nbits, bt>& rhs) {
	oneparam<nbits> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<size_t nbits, typename bt>
inline std::string components(const oneparam<nbits,bt>& v) {
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
template<size_t nbits, typename bt>
oneparam<nbits> abs(const oneparam<nbits,bt>& v) {
	return oneparam<nbits>();
}


}}  // namespace sw::universal
