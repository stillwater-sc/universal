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
template<size_t nbits, size_t rbits, typename bt> class lns;
template<size_t nbits, size_t rbits, typename bt> lns<nbits, rbits, bt> abs(const lns<nbits, rbits, bt>& v);

// convert a floating-point value to a specific lns configuration. Semantically, p = v, return reference to p
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt>& convert(const triple<nbits, bt>& v, lns<nbits, rbits, bt>& p) {
	if (v.iszero()) {
		return p.setnan();
	}
	if (v.isnan() || v.isinf()) {
		return p.setnan();
	}
	return p;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt>& minpos(lns<nbits, rbits, bt>& lminpos) {
	return lminpos;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& maxpos(lns<nbits, rbits, bt>& lmaxpos) {
	return lmaxpos;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& minneg(lns<nbits, rbits, bt>& lminneg) {
	return lminneg;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& maxneg(lns<nbits, rbits, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t _nbits, size_t _rbits, typename bt = uint8_t>
class lns {
public:
	static constexpr size_t nbits = _nbits;
	static constexpr size_t rbits = _rbits;
	typedef bt BlockType;
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
	lns& operator-=(double rhs) { return *this -= lns(rhs); }
	lns& operator*=(const lns& rhs) {
		this->_bits += rhs._bits;
		return *this; 
	}
	lns& operator*=(double rhs) { return *this *= lns(rhs); }
	lns& operator/=(const lns& rhs) {
		this->_bits -= rhs._bits;
		return *this;
	}
	lns& operator/=(double rhs) { return *this /= lns(rhs); }

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
	void clear() noexcept { _bits.clear(); }
	void setbits(uint64_t v) noexcept { _bits.setbits(v); } // API to be consistent with the other number systems

	// selectors
	constexpr bool iszero() const noexcept { return false; }
	constexpr bool isneg()  const noexcept { return false; }
	constexpr bool ispos()  const noexcept { return false; }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool isnan()  const noexcept { return false; }
	constexpr bool sign()   const noexcept { return false; }
	constexpr int  scale()  const noexcept { return false; }

	constexpr bool at(size_t bitIndex) const noexcept {
		return _bits.at(bitIndex);
	}
	std::string get() const noexcept {
		std::stringstream s;
		s << std::exp(double(_bits.to_long_long()));
		return s.str(); 
	}

	long double to_long_double() const noexcept {
		return std::exp((long double)(_bits.to_long_long()))/scaling;
	}
	double to_double() const noexcept {
		return std::exp(double(_bits.to_long_long()))/scaling;
	}
	float to_float() const noexcept {
		return std::exp(float(_bits.to_long_long()))/scaling;
	}
	// Maybe remove explicit
	explicit operator long double() const noexcept { return to_long_double(); }
	explicit operator double() const noexcept { return to_double(); }
	explicit operator float() const noexcept { return to_float(); }

private:
	blockbinary<nbits,bt>  _bits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const lns<nnbits, rrbits, nbt>& r);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, lns<nnbits, rrbits, nbt>& r);

	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator==(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator< (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator> (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const lns<nnbits, rrbits, nbt>& v) {
	ostr << v.to_double();
	return ostr;
}

template<size_t nnbits, size_t rrbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const lns<nnbits, rrbits, nbt>& v) {
	istr >> v._fraction;
	return istr;
}

// lns - logic operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator==(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return false; }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator< (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return false; }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator> (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator< (lhs, rhs); }

// lns - literal logic operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator==(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return lhs == lns<nnbits, rrbits, nbt>(rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator< (const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return lhs < lns<nnbits, rrbits, nbt>(rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator> (const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator< (lhs, rhs); }

// lns - lns binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator+(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator-(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator*(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator/(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<size_t nbits, size_t rbits, typename bt>
inline std::string to_binary(const lns<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		s << (number.at(static_cast<size_t>(i)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}

template<size_t nbits, size_t rbits, typename bt>
inline std::string components(const lns<nbits, rbits, bt>& v) {
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
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> abs(const lns<nbits, rbits, bt>& v) {
	return lns<nbits, rbits, bt>();
}


}} // namespace sw::universal
