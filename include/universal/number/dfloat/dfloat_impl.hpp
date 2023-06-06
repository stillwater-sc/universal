#pragma once
// dfloat_impl.hpp: implementation of a fixed-size, arbitrary configuration decimal floating-point number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>

#include <universal/number/dfloat/exceptions.hpp>

namespace sw { namespace universal {

// dfloat is an adaptive precision decimal floating-point type
template<unsigned _ndigits, unsigned _es, typename bt = std::uint8_t> 
class dfloat {
public:
	static constexpr unsigned ndigits = _ndigits;
	static constexpr unsigned es = _es;
	static constexpr unsigned fdigits = ndigits - 1u - es; // number of fraction digits
	typedef bt BlockType;

	static constexpr unsigned bitsInByte = 8u;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks = 1u + ((ndigits - 1u) / bitsInBlock);
	static constexpr unsigned MSU = nrBlocks - 1u; // MSU == Most Significant Unit, as MSB is already taken
	static constexpr bt       ALL_ONES = bt(~0); // block type specific all 1's value
	static constexpr bt       MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - ndigits));

	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64u - bitsInBlock));
	static constexpr bt       BLOCK_MASK = bt(~0);

	/// trivial constructor
	dfloat() = default;

	dfloat(const dfloat&) = default;
	dfloat(dfloat&&) = default;

	dfloat& operator=(const dfloat&) = default;
	dfloat& operator=(dfloat&&) = default;

	// initializers for native types
	explicit dfloat(const signed char initial_value)        { *this = initial_value; }
	explicit dfloat(const short initial_value)              { *this = initial_value; }
	explicit dfloat(const int initial_value)                { *this = initial_value; }
	explicit dfloat(const long initial_value)               { *this = initial_value; }
	explicit dfloat(const long long initial_value)          { *this = initial_value; }
	explicit dfloat(const char initial_value)               { *this = initial_value; }
	explicit dfloat(const unsigned short initial_value)     { *this = initial_value; }
	explicit dfloat(const unsigned int initial_value)       { *this = initial_value; }
	explicit dfloat(const unsigned long initial_value)      { *this = initial_value; }
	explicit dfloat(const unsigned long long initial_value) { *this = initial_value; }
	explicit dfloat(const float initial_value)              { *this = initial_value; }
	explicit dfloat(const double initial_value)             { *this = initial_value; }
	explicit dfloat(const long double initial_value)        { *this = initial_value; }

	// assignment operators for native types
	dfloat& operator=(const signed char rhs)        { return convert(rhs, *this); }
	dfloat& operator=(const short rhs)              { return convert(rhs, *this); }
	dfloat& operator=(const int rhs)                { return convert(rhs, *this); }
	dfloat& operator=(const long rhs)               { return convert(rhs, *this); }
	dfloat& operator=(const long long rhs)          { return convert(rhs, *this); }
	dfloat& operator=(const char rhs)               { return convert_unsigned(rhs, *this); }
	dfloat& operator=(const unsigned short rhs)     { return convert_unsigned(rhs, *this); }
	dfloat& operator=(const unsigned int rhs)       { return convert_unsigned(rhs, *this); }
	dfloat& operator=(const unsigned long rhs)      { return convert_unsigned(rhs, *this); }
	dfloat& operator=(const unsigned long long rhs) { return convert_unsigned(rhs, *this); }
	dfloat& operator=(const float rhs)              { return convert_ieee754(rhs); }
	dfloat& operator=(const double rhs)             { return convert_ieee754(rhs); }
	dfloat& operator=(const long double rhs)        { return convert_ieee754(rhs); }

	// prefix operators
	dfloat operator-() const {
		dfloat negated(*this);
		return negated;
	}

	// conversion operators
	explicit operator float() const { return float(toNativeFloatingPoint()); }
	explicit operator double() const { return float(toNativeFloatingPoint()); }
	explicit operator long double() const { return toNativeFloatingPoint(); }

	// arithmetic operators
	dfloat& operator+=(const dfloat& rhs) {
		return *this;
	}
	dfloat& operator-=(const dfloat& rhs) {
		return *this;
	}
	dfloat& operator*=(const dfloat& rhs) {
		return *this;
	}
	dfloat& operator/=(const dfloat& rhs) {
		return *this;
	}

	// modifiers
	inline void clear() {  }
	inline void setzero() { clear(); }
	// use un-interpreted raw bits to set the bits of the dfloat
	inline void setBits(unsigned long long value) {
		clear();
	}
	inline dfloat& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	inline bool iszero() const { return false; }
	inline bool isone() const  { return true; }
	inline bool isodd() const  { return false; }
	inline bool iseven() const { return !isodd(); }
	inline bool ispos() const  { return false; }
	inline bool isneg() const   { return false; }
	inline int  scale() const { return 0; }

	// convert to string containing digits number of digits
	std::string str(size_t nrDigits = 0) const {
		if (iszero()) return std::string("0.0");

		int64_t magnitude = scale();
		if (magnitude > 1 || magnitude < 0) {
			// use scientific notation for non-trivial exponent values
			return sci_notation(nrDigits);
		}

		std::string str;
		int64_t exponent = trimmed(nrDigits, str);

		if (magnitude == 0) {
			if (isneg())
				return std::string("-0.0") + str;
			else
				return std::string("0.0") + str;
		}

		std::string before_decimal = "TBD";

		if (exponent >= 0) {
			if (isneg())
				return std::string("-") + before_decimal + ".0";
			else
				return before_decimal + ".0";
		}

		// now the digits after the radix point
		std::string after_decimal = str.substr((size_t)(str.size() + exponent), (size_t)-exponent);
		if (isneg())
			return std::string("-") + before_decimal + "." + after_decimal;
		else
			return before_decimal + "." + after_decimal;

		return std::string("bad");
	}

protected:
	bt _blocks[nrBlocks];

	// HELPER methods

	// convert to native floating-point, use conversion rules to cast down to float and double
	long double toNativeFloatingPoint() const {
		long double ld = 0;
		return ld;
	}

	template<typename Ty>
	dfloat& convert_ieee754(Ty& rhs) {
		clear();
		long long base = (long long)rhs;
		*this = base;
		return *this;
	}

	// convert to string with nrDigits of significant digits and return the scale
	// value = str + "10^" + scale
	int64_t trimmed(size_t nrDigits, std::string& number) const {

		return 0;
	}

	std::string sci_notation(size_t nrDigits) const {

		return std::string("TBD");
	}

private:

	// dfloat - dfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend bool operator==(const dfloat<N, E, B>& lhs, const dfloat<N, E, B>& rhs);

	// dfloat - literal logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend bool operator==(const dfloat<N, E, B>& lhs, const double rhs);

	// literal - dfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend bool operator==(const double lhs, const dfloat<N, E, B>& rhs);

};

template<size_t ndigits, size_t es, typename BlockType, typename NativeFloat>
inline dfloat<ndigits, es, BlockType>& convert(int64_t v, dfloat<ndigits, es, BlockType>& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType>& convert_unsigned(uint64_t v, dfloat<ndigits, es, BlockType>& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

////////////////////////    MPFLOAT functions   /////////////////////////////////

template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> abs(const dfloat<ndigits, es, BlockType>& a) {
	return a; // (a < 0 ? -a : a);
}


////////////////////////    INTEGER operators   /////////////////////////////////

// divide dfloat a and b and return result argument
template<size_t ndigits, size_t es, typename BlockType>
void divide(const dfloat<ndigits, es, BlockType>& a, const dfloat<ndigits, es, BlockType>& b, dfloat<ndigits, es, BlockType>& quotient) {
}

/// stream operators

// read a dfloat ASCII format and make a binary dfloat out of it
template<size_t ndigits, size_t es, typename BlockType>
bool parse(const std::string& number, dfloat<ndigits, es, BlockType>& value) {
	bool bSuccess = false;

	return bSuccess;
}

// generate an dfloat format ASCII format
template<size_t ndigits, size_t es, typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const dfloat<ndigits, es, BlockType>& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the dfloat into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));

	return ostr << ss.str();
}

// read an ASCII dfloat format
template<size_t ndigits, size_t es, typename BlockType>
inline std::istream& operator>>(std::istream& istr, dfloat<ndigits, es, BlockType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - dfloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<size_t ndigits, size_t es, typename BlockType>
inline bool operator==(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return true;
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator!=(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator< (const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return false; // lhs and rhs are the same
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator> (const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator<=(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator>=(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t ndigits, size_t es, typename BlockType>
inline bool operator==(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator==(lhs, dfloat<ndigits, es, BlockType>(rhs));
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator!=(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator< (const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator<(lhs, dfloat<ndigits, es, BlockType>(rhs));
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator> (const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (dfloat<ndigits, es, BlockType>(rhs), lhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator<=(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator>=(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dfloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator==(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator==(dfloat<ndigits, es, BlockType>(lhs), rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator!=(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator< (const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator<(dfloat<ndigits, es, BlockType>(lhs), rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator> (const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator<=(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<size_t ndigits, size_t es, typename BlockType>
inline bool operator>=(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - dfloat binary arithmetic operators
// BINARY ADDITION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator+(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator-(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator*(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator/(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - literal binary arithmetic operators
// BINARY ADDITION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator+(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator+(lhs, dfloat<ndigits, es, BlockType>(rhs));
}
// BINARY SUBTRACTION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator-(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator-(lhs, dfloat<ndigits, es, BlockType>(rhs));
}
// BINARY MULTIPLICATION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator*(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator*(lhs, dfloat<ndigits, es, BlockType>(rhs));
}
// BINARY DIVISION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator/(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator/(lhs, dfloat<ndigits, es, BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dfloat binary arithmetic operators
// BINARY ADDITION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator+(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator+(dfloat<ndigits, es, BlockType>(lhs), rhs);
}
// BINARY SUBTRACTION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator-(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator-(dfloat<ndigits, es, BlockType>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator*(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator*(dfloat<ndigits, es, BlockType>(lhs), rhs);
}
// BINARY DIVISION
template<size_t ndigits, size_t es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator/(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator/(dfloat<ndigits, es, BlockType>(lhs), rhs);
}

}} // namespace sw::universal
