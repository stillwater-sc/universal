#pragma once
// lns_impl.hpp: implementation of an arbitrary logarithmic number system configuration
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>

namespace sw { namespace universal {
		
// Forward definitions
template<size_t nbits, typename bt> class lns;
template<size_t nbits, typename bt> lns<nbits,bt> abs(const lns<nbits,bt>& v);

// convert a floating-point value to a specific lns configuration. Semantically, p = v, return reference to p
template<size_t nbits, typename bt>
inline lns<nbits, bt>& convert(const triple<nbits,bt>& v, lns<nbits,bt>& p) {
	if (v.iszero()) {
		return p.setnan();
	}
	if (v.isnan() || v.isinf()) {
		return p.setnan();
	}
	return p;
}

template<size_t nbits, typename bt>
lns<nbits, bt>& minpos(lns<nbits, bt>& lminpos) {
	return lminpos;
}
template<size_t nbits, typename bt>
lns<nbits, bt>& maxpos(lns<nbits, bt>& lmaxpos) {
	return lmaxpos;
}
template<size_t nbits, typename bt>
lns<nbits, bt>& minneg(lns<nbits, bt>& lminneg) {
	return lminneg;
}
template<size_t nbits, typename bt>
lns<nbits, bt>& maxneg(lns<nbits, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t _nbits, typename bt = uint8_t>
class lns {
public:
	static constexpr size_t nbits = _nbits;
	typedef bt BlockType;
	static constexpr size_t rbits = nbits >> 1;
	static constexpr double scaling = double(1ull << rbits);
	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;


	lns() : _bits{ 0 } {}

	lns(const lns&) = default;
	lns(lns&&) = default;

	lns& operator=(const lns&) = default;
	lns& operator=(lns&&) = default;

	lns(signed char initial_value)        { *this = initial_value; }
	lns(short initial_value)              { *this = initial_value; }
	lns(int initial_value)                { *this = initial_value; }
	lns(long long initial_value)          { *this = initial_value; }
	lns(unsigned long long initial_value) { *this = initial_value; }
	lns(float initial_value)              { *this = initial_value; }
	lns(double initial_value)             { *this = initial_value; }
	lns(long double initial_value)        { *this = initial_value; }

	// assignment operators
	lns& operator=(signed char rhs) { return *this = (long long)(rhs); }
	lns& operator=(short rhs) { return *this = (long long)(rhs); }
	lns& operator=(int rhs) { return *this = (long long)(rhs); }
	lns& operator=(long long rhs) { return *this; }
	lns& operator=(unsigned long long rhs) { return *this; }
	constexpr lns& operator=(float rhs) { _bits = (long long)(std::log(rhs) * scaling); return *this; }
	constexpr lns& operator=(double rhs) { _bits = (long long)(std::log(rhs) * scaling); return *this; }
	lns& operator=(long double rhs) { _bits = (long long)(std::log(rhs) * scaling); return *this; }

	// arithmetic operators
	// prefix operator
	lns operator-() const {				
		return *this;
	}

	// in-place arithmetic assignment operators
	lns& operator+=(const lns& rhs) { return *this; }
	lns& operator+=(double rhs) { return *this += lns(rhs); }
	lns& operator-=(const lns& rhs) { return *this; }
	lns& operator-=(double rhs) { return *this -= lns<nbits>(rhs); }
	lns& operator*=(const lns& rhs) {
		this->_bits += rhs._bits;
		return *this; 
	}
	lns& operator*=(double rhs) { return *this *= lns<nbits>(rhs); }
	lns& operator/=(const lns& rhs) {
		this->_bits -= rhs._bits;
		return *this;
	}
	lns& operator/=(double rhs) { return *this /= lns<nbits>(rhs); }

	// prefix/postfix operators
	lns& operator++() {
		return *this;
	}
	lns operator++(int) {
		lns tmp(*this);
		operator++();
		return tmp;
	}
	lns& operator--() {
		return *this;
	}
	lns operator--(int) {
		lns tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	void clear() { _bits.clear(); }
	void setbits(uint64_t v) { _bits.setbits(v); } // API to be consistent with the other number systems

	// selectors
	constexpr bool iszero() const { return false; }
	constexpr bool isneg() const { return false; }
	constexpr bool ispos() const { return false; }
	constexpr bool isinf() const { return false; }
	constexpr bool isnan() const { return false; }
	constexpr bool sign() const { return false; }
	constexpr int  scale() const { return false; }

	constexpr bool at(size_t bitIndex) const noexcept {
		return _bits.at(bitIndex);
	}
	std::string get() const { 
		std::stringstream s;
		s << std::exp(double(_bits.to_long_long()));
		return s.str(); 
	}

	long double to_long_double() const {
		return std::exp((long double)(_bits.to_long_long()))/scaling;
	}
	double to_double() const {
		return std::exp(double(_bits.to_long_long()))/scaling;
	}
	float to_float() const {
		return std::exp(float(_bits.to_long_long()))/scaling;
	}
	// Maybe remove explicit
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

private:
	blockbinary<nbits,bt>  _bits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const lns<nnbits,nbt>& r);
	template<size_t nnbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, lns<nnbits,nbt>& r);

	template<size_t nnbits, typename nbt>
	friend bool operator==(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator!=(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator< (const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator> (const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator<=(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator>=(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const lns<nnbits,nbt>& v) {
	ostr << v.to_double();
	return ostr;
}

template<size_t nnbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const lns<nnbits,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

// lns - logic operators
template<size_t nnbits, typename nbt>
inline bool operator==(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs) { return false; }
template<size_t nnbits, typename nbt>
inline bool operator!=(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator< (const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs) { return false; }
template<size_t nnbits, typename nbt>
inline bool operator> (const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, typename nbt>
inline bool operator<=(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator>=(const lns<nnbits,nbt>& lhs, const lns<nnbits,nbt>& rhs) { return !operator< (lhs, rhs); }

// lns - literal logic operators
template<size_t nnbits, typename nbt>
inline bool operator==(const lns<nnbits, nbt>& lhs, double rhs) { return lhs == lns<nnbits,nbt>(rhs); }
template<size_t nnbits, typename nbt>
inline bool operator!=(const lns<nnbits, nbt>& lhs, double rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator< (const lns<nnbits, nbt>& lhs, double rhs) { return lhs < lns<nnbits, nbt>(rhs); }
template<size_t nnbits, typename nbt>
inline bool operator> (const lns<nnbits, nbt>& lhs, double rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, typename nbt>
inline bool operator<=(const lns<nnbits, nbt>& lhs, double rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator>=(const lns<nnbits, nbt>& lhs, double rhs) { return !operator< (lhs, rhs); }

// lns - lns binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, typename bt>
inline lns<nbits, bt> operator+(const lns<nbits, bt>& lhs, const lns<nbits, bt>& rhs) {
	lns<nbits> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, typename bt>
inline lns<nbits, bt> operator-(const lns<nbits, bt>& lhs, const lns<nbits, bt>& rhs) {
	lns<nbits> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, typename bt>
inline lns<nbits, bt> operator*(const lns<nbits, bt>& lhs, const lns<nbits, bt>& rhs) {
	lns<nbits> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, typename bt>
inline lns<nbits, bt> operator/(const lns<nbits, bt>& lhs, const lns<nbits, bt>& rhs) {
	lns<nbits> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<size_t nbits, typename bt>
inline std::string to_binary(const lns<nbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		s << (number.at(static_cast<size_t>(i)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}

template<size_t nbits, typename bt>
inline std::string components(const lns<nbits,bt>& v) {
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
lns<nbits> abs(const lns<nbits,bt>& v) {
	return lns<nbits>();
}


}} // namespace sw::universal
