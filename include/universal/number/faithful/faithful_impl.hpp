#pragma once
// faithful_impl.hpp: definition of a faithfully rounded number system
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/abstract/triple.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<typename FloatingPointType> class faithful;
template<typename FloatingPointType> faithful<FloatingPointType> abs(const faithful<FloatingPointType>& v);

// convert a floating-point value to a specific faithful configuration. Semantically, p = v, return reference to p
template<unsigned nbits, typename BlockType, typename FloatingPointType>
inline faithful<FloatingPointType>& convert(const triple<nbits, BlockType>& v, faithful<FloatingPointType>& p) {
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

template<typename FloatingPointType>
faithful<FloatingPointType>& minpos(faithful<FloatingPointType>& lminpos) {
	return lminpos;
}
template<typename FloatingPointType>
faithful<FloatingPointType>& maxpos(faithful<FloatingPointType>& lmaxpos) {
	return lmaxpos;
}
template<typename FloatingPointType>
faithful<FloatingPointType>& minneg(faithful<FloatingPointType>& lminneg) {
	return lminneg;
}
template<typename FloatingPointType>
faithful<FloatingPointType>& maxneg(faithful<FloatingPointType>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<typename FloatingPointType = uint8_t>
class faithful {
public:
	faithful() {}

	faithful(const faithful&) = default;
	faithful(faithful&&) = default;

	faithful& operator=(const faithful&) = default;
	faithful& operator=(faithful&&) = default;

	faithful(signed char initial_value)        { *this = initial_value; }
	faithful(short initial_value)              { *this = initial_value; }
	faithful(int initial_value)                { *this = initial_value; }
	faithful(long long initial_value)          { *this = initial_value; }
	faithful(unsigned long long initial_value) { *this = initial_value; }
	faithful(float initial_value)              { *this = initial_value; }
	faithful(double initial_value)             { *this = initial_value; }
	faithful(long double initial_value)        { *this = initial_value; }

	// assignment operators
	faithful& operator=(signed char rhs) { return *this = (long long)(rhs); }
	faithful& operator=(short rhs) { return *this = (long long)(rhs); }
	faithful& operator=(int rhs) { return *this = (long long)(rhs); }
	faithful& operator=(long long rhs) { return *this; }
	faithful& operator=(unsigned long long rhs) { return *this; }
	faithful& operator=(float rhs) { return *this; } 
	faithful& operator=(double rhs) { return *this; }
	faithful& operator=(long double rhs) { return *this; }

	// arithmetic operators
	// prefix operator
	faithful operator-() const {				
		return *this;
	}

	// in-place arithmetic assignment operators
	faithful& operator+=(const faithful& rhs) { return *this; }
	faithful& operator+=(double rhs) { return *this += faithful(rhs); }
	faithful& operator-=(const faithful& rhs) { return *this; }
	faithful& operator-=(double rhs) { return *this -= faithful<FloatingPointType>(rhs); }
	faithful& operator*=(const faithful& rhs) { return *this; }
	faithful& operator*=(double rhs) { return *this *= faithful<FloatingPointType>(rhs); }
	faithful& operator/=(const faithful& rhs) { return *this; }
	faithful& operator/=(double rhs) { return *this /= faithful<FloatingPointType>(rhs); }

	// prefix/postfix operators
	faithful& operator++() {
		return *this;
	}
	faithful operator++(int) {
		faithful tmp(*this);
		operator++();
		return tmp;
	}
	faithful& operator--() {
		return *this;
	}
	faithful operator--(int) {
		faithful tmp(*this);
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
	template<typename FPType>
	friend std::ostream& operator<< (std::ostream& ostr, const faithful<FPType>& r);
	template<typename FPType>
	friend std::istream& operator>> (std::istream& istr, faithful<FPType>& r);

	template<typename FPType>
	friend bool operator==(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator!=(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator< (const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator> (const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator<=(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator>=(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
};

////////////////////// operators
template<typename FPType>
inline std::ostream& operator<<(std::ostream& ostr, const faithful<FPType>& v) {

	return ostr;
}

template<typename FPType>
inline std::istream& operator>>(std::istream& istr, const faithful<FPType>& v) {
	istr >> v._fraction;
	return istr;
}

template<typename FPType>
inline bool operator==(const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return false; }
template<typename FPType>
inline bool operator!=(const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return !operator==(lhs, rhs); }
template<typename FPType>
inline bool operator< (const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return false; }
template<typename FPType>
inline bool operator> (const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return  operator< (rhs, lhs); }
template<typename FPType>
inline bool operator<=(const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return !operator> (lhs, rhs); }
template<typename FPType>
inline bool operator>=(const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return !operator< (lhs, rhs); }

// faithful - faithful binary arithmetic operators
// BINARY ADDITION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator+(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator-(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator*(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator/(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<typename FloatingPointType>
inline std::string components(const faithful<FloatingPointType>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero";
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite";
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<typename FloatingPointType>
faithful<FloatingPointType> abs(const faithful<FloatingPointType>& v) {
	return faithful<FloatingPointType>();
}


}}  // namespace sw::universal
