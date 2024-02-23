#pragma once
// takum_impl.hpp: definition of a arbitrary, fixed-size takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee-754.hpp>
#include <universal/blockbin/blockbinary.hpp>
#include <universal/abstract/triple.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<unsigned nbits, unsigned es, typename bt> class takum;
template<unsigned nbits, unsigned es, typename bt> takum<nbits, es, bt> abs(const takum<nbits, es, bt>& v);

// convert a floating-point value to a specific takum configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt>& convert(const triple<nbits, es, bt>& v, takum<nbits, es, bt>& p) {
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
takum<nbits, es, bt>& minpos(takum<nbits, es, bt>& lminpos) {
	return lminpos;
}
template<unsigned nbits, unsigned es, typename bt>
takum<nbits, es, bt>& maxpos(takum<nbits, es, bt>& lmaxpos) {
	return lmaxpos;
}
template<unsigned nbits, unsigned es, typename bt>
takum<nbits, es, bt>& minneg(takum<nbits, es, bt>& lminneg) {
	return lminneg;
}
template<unsigned nbits, unsigned es, typename bt>
takum<nbits, es, bt>& maxneg(takum<nbits, es, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<unsigned nbits, unsigned es, typename bt = uint8_t>
class takum {
public:
	takum() {}

	takum(const takum&) = default;
	takum(takum&&) = default;

	takum& operator=(const takum&) = default;
	takum& operator=(takum&&) = default;

	takum(signed char initial_value)        { *this = initial_value; }
	takum(short initial_value)              { *this = initial_value; }
	takum(int initial_value)                { *this = initial_value; }
	takum(long long initial_value)          { *this = initial_value; }
	takum(unsigned long long initial_value) { *this = initial_value; }
	takum(float initial_value)              { *this = initial_value; }
	takum(double initial_value)             { *this = initial_value; }
	takum(long double initial_value)        { *this = initial_value; }

	// assignment operators
	takum& operator=(signed char rhs) { return *this = (long long)(rhs); }
	takum& operator=(short rhs) { return *this = (long long)(rhs); }
	takum& operator=(int rhs) { return *this = (long long)(rhs); }
	takum& operator=(long long rhs) { return *this; }
	takum& operator=(unsigned long long rhs) { return *this; }
	takum& operator=(float rhs) { return *this; } 
	takum& operator=(double rhs) { return *this; }
	takum& operator=(long double rhs) { return *this; }

	// arithmetic operators
	// prefix operator
	takum operator-() const {				
		return *this;
	}

	// in-place arithmetic assignment operators
	takum& operator+=(const takum& rhs) { return *this; }
	takum& operator+=(double rhs) { return *this += takum(rhs); }
	takum& operator-=(const takum& rhs) { return *this; }
	takum& operator-=(double rhs) { return *this -= takum<nbits>(rhs); }
	takum& operator*=(const takum& rhs) { return *this; }
	takum& operator*=(double rhs) { return *this *= takum<nbits>(rhs); }
	takum& operator/=(const takum& rhs) { return *this; }
	takum& operator/=(double rhs) { return *this /= takum<nbits>(rhs); }

	// prefix/postfix operators
	takum& operator++() {
		return *this;
	}
	takum operator++(int) {
		takum tmp(*this);
		operator++();
		return tmp;
	}
	takum& operator--() {
		return *this;
	}
	takum operator--(int) {
		takum tmp(*this);
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
	friend std::ostream& operator<< (std::ostream& ostr, const takum<nnbits, nes, nbt>& r);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::istream& operator>> (std::istream& istr, takum<nnbits, nes, nbt>& r);

	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator==(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator!=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator< (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator> (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator<=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator>=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
};

////////////////////// operators
template<unsigned nnbits, unsigned nes, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const takum<nnbits, nes, nbt>& v) {

	return ostr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const takum<nnbits, nes, nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator==(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return false; }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator!=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator< (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return false; }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator> (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator<=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator>=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return !operator< (lhs, rhs); }

// takum - takum binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator+(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator-(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator*(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator/(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<unsigned nbits, unsigned es, typename bt>
inline std::string components(const takum<nbits, es, bt>& v) {
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
takum<nbits> abs(const takum<nbits, es, bt>& v) {
	return takum<nbits>();
}


}}  // namespace sw::universal
