#pragma once
// real.hpp: definition of an arbitrary configuration linear floating-point representation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee-754.hpp>
#include <universal/blockbin/blockbinary.hpp>
#include <universal/areal/exceptions.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<size_t nbits, size_t es, typename bt> class real;
template<size_t nbits, size_t es, typename bt> real<nbits,es,bt> abs(const real<nbits,es,bt>& v);

template<size_t nbits, size_t es, typename bt>
void extract_fields(const blockbinary<nbits, bt>& raw_bits, bool& _sign, blockbinary<es, bt>& _exponent, blockbinary<nbits - es - 1, bt>& _fraction) {

}

// fill an real object with mininum positive value
template<size_t nbits, size_t es, typename bt>
real<nbits, es, bt>& minpos(real<nbits, es, bt>& aminpos) {

	return aminpos;
}
// fill an real object with maximum positive value
template<size_t nbits, size_t es, typename bt>
real<nbits, es, bt>& maxpos(real<nbits, es, bt>& amaxpos) {

	return amaxpos;
}
// fill an real object with mininum negative value
template<size_t nbits, size_t es, typename bt>
real<nbits, es, bt>& minneg(real<nbits, es, bt>& aminneg) {

	return aminneg;
}
// fill an real object with maximum negative value
template<size_t nbits, size_t es, typename bt>
real<nbits, es, bt>& maxneg(real<nbits, es, bt>& amaxneg) {

	return amaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t nbits, size_t es, typename bt = uint8_t>
class real {
public:
	static constexpr size_t fbits  = nbits - 1 - es;    // number of fraction bits excluding the hidden bit
	static constexpr size_t fhbits = fbits + 1;         // number of fraction bits including the hidden bit
	static constexpr size_t abits = fhbits + 3;         // size of the addend
	static constexpr size_t mbits = 2 * fhbits;         // size of the multiplier output
	static constexpr size_t divbits = 3 * fhbits + 4;   // size of the divider output

	real() {}

	real(signed char initial_value)        { *this = initial_value; }
	real(short initial_value)              { *this = initial_value; }
	real(int initial_value)                { *this = initial_value; }
	real(long long initial_value)          { *this = initial_value; }
	real(unsigned long long initial_value) { *this = initial_value; }
	real(float initial_value)              { *this = initial_value; }
	real(double initial_value)             { *this = initial_value; }
	real(long double initial_value)        { *this = initial_value; }
	real(const real& rhs)                 { *this = rhs; }

	// assignment operators
	real& operator=(signed char rhs) {
		return *this = (long long)(rhs);
	}
	real& operator=(short rhs) {
		return *this = (long long)(rhs);
	}
	real& operator=(int rhs) {
		return *this = (long long)(rhs);
	}
	real& operator=(long long rhs) {
		return *this;
	}
	real& operator=(unsigned long long rhs) {
		return *this;
	}
	real& operator=(float rhs) {

		return *this;
	}
	real& operator=(double rhs) {

		return *this;
	}
	real& operator=(long double rhs) {

		return *this;
	}

	// arithmetic operators
	// prefix operator
	real operator-() const {				
		return *this;
	}

	real& operator+=(const real& rhs) {
		return *this;
	}
	real& operator+=(double rhs) {
		return *this += real(rhs);
	}
	real& operator-=(const real& rhs) {

		return *this;
	}
	real& operator-=(double rhs) {
		return *this -= real<nbits, es>(rhs);
	}
	real& operator*=(const real& rhs) {

		return *this;
	}
	real& operator*=(double rhs) {
		return *this *= real<nbits, es>(rhs);
	}
	real& operator/=(const real& rhs) {

		return *this;
	}
	real& operator/=(double rhs) {
		return *this /= real<nbits, es>(rhs);
	}
	real& operator++() {
		return *this;
	}
	real operator++(int) {
		real tmp(*this);
		operator++();
		return tmp;
	}
	real& operator--() {
		return *this;
	}
	real operator--(int) {
		real tmp(*this);
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
	template<size_t nnbits, size_t nes, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const real<nnbits,nes,nbt>& r);
	template<size_t nnbits, size_t nes, typename nbt>
	friend std::istream& operator>> (std::istream& istr, real<nnbits,nes,nbt>& r);

	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator==(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator!=(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator< (const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator> (const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator<=(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator>=(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, size_t nes, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const real<nnbits,nes,nbt>& v) {

	return ostr;
}

template<size_t nnbits, size_t nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const real<nnbits,nes,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nnbits, size_t nes, typename nbt>
inline bool operator==(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs) { return false; }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator!=(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator< (const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs) { return false; }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator> (const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator<=(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator>=(const real<nnbits,nes,nbt>& lhs, const real<nnbits,nes,nbt>& rhs) { return !operator< (lhs, rhs); }

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t es, typename bt>
inline real<nbits, es, bt> operator+(const real<nbits, es, bt>& lhs, const real<nbits, es, bt>& rhs) {
	real<nbits, es> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t es, typename bt>
inline real<nbits, es, bt> operator-(const real<nbits, es, bt>& lhs, const real<nbits, es, bt>& rhs) {
	real<nbits, es> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t es, typename bt>
inline real<nbits, es, bt> operator*(const real<nbits, es, bt>& lhs, const real<nbits, es, bt>& rhs) {
	real<nbits, es> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t es, typename bt>
inline real<nbits, es, bt> operator/(const real<nbits, es, bt>& lhs, const real<nbits, es, bt>& rhs) {
	real<nbits, es> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<size_t nbits, size_t es, typename bt>
inline std::string components(const real<nbits,es,bt>& v) {
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
template<size_t nbits, size_t es, typename bt>
real<nbits,es> abs(const real<nbits,es,bt>& v) {
	return real<nbits,es>(false, v.scale(), v.fraction(), v.isZero());
}


}}  // namespace sw::universal
