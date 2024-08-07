#pragma once
// dfloat_impl.hpp: implementation of a fixed-size, arbitrary configuration decimal floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>

// supporting types and functions
#include <universal/native/ieee754.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// dfloat exception structure
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

	// converting constructors
	constexpr dfloat(const std::string& stringRep) : _block{} { assign(stringRep); }

	// specific value constructor
	constexpr dfloat(const SpecificValue code) noexcept : _block{} {
		switch (code) {
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::infpos:
			setinf(false);
			break;
		case SpecificValue::infneg:
			setinf(true);
			break;
		case SpecificValue::nar: // approximation as dfloats don't have a NaR
		case SpecificValue::qnan:
			setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	// initializers for native types
	explicit dfloat(signed char iv)        { *this = iv; }
	explicit dfloat(short iv)              { *this = iv; }
	explicit dfloat(int iv)                { *this = iv; }
	explicit dfloat(long iv)               { *this = iv; }
	explicit dfloat(long long iv)          { *this = iv; }
	explicit dfloat(char iv)               { *this = iv; }
	explicit dfloat(unsigned short iv)     { *this = iv; }
	explicit dfloat(unsigned int iv)       { *this = iv; }
	explicit dfloat(unsigned long iv)      { *this = iv; }
	explicit dfloat(unsigned long long iv) { *this = iv; }
	explicit dfloat(float iv)              { *this = iv; }
	explicit dfloat(double iv)             { *this = iv; }
	explicit dfloat(long double iv)        { *this = iv; }

	// assignment operators for native types
	dfloat& operator=(signed char rhs)        { return convert_signed(rhs); }
	dfloat& operator=(short rhs)              { return convert_signed(rhs); }
	dfloat& operator=(int rhs)                { return convert_signed(rhs); }
	dfloat& operator=(long rhs)               { return convert_signed(rhs); }
	dfloat& operator=(long long rhs)          { return convert_signed(rhs); }
	dfloat& operator=(char rhs)               { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned short rhs)     { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned int rhs)       { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned long rhs)      { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned long long rhs) { return convert_unsigned(rhs); }
	dfloat& operator=(float rhs)              { return convert_ieee754(rhs); }
	dfloat& operator=(double rhs)             { return convert_ieee754(rhs); }
	dfloat& operator=(long double rhs)        { return convert_ieee754(rhs); }

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

	// unary operators
	dfloat& operator++() {
		return *this;
	}
	dfloat operator++(int) {
		dfloat tmp(*this);
		operator++();
		return tmp;
	}
	dfloat& operator--() {
		return *this;
	}
	dfloat operator--(int) {
		dfloat tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	void clear()                                         noexcept {  }
	void setzero()                                       noexcept { clear(); }
	void setinf(bool sign = true)                        noexcept { }
	void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { }
	void setsign(bool sign = true)                       noexcept { }
	void setexponent(const std::string& expDigits)       noexcept { }
	void setfraction(const std::string& fracDigits)      noexcept { }
	// use un-interpreted raw bits to set the value of the dfloat
	inline void setbits(uint64_t value) {
		clear();
	}
	
	// create specific number system values of interest
	constexpr dfloat& maxpos() noexcept {
		// maxpos is represented by the pattern 9.999e99
		return *this;
	}
	constexpr dfloat& minpos() noexcept {
		// minpos is represented by the pattern 0.0001e-99
		return *this;
	}
	constexpr dfloat& zero() noexcept {
		// the zero value
		clear();
		return *this;
	}
	constexpr dfloat& minneg() noexcept {
		// minneg is represented by the pattern -0.0001e-99
		return *this;
	}
	constexpr dfloat& maxneg() noexcept {
		// maxneg is represented by the pattern -9.999e99
		return *this;
	}

	dfloat& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	bool iszero() const noexcept { return false; }
	bool isone() const noexcept { return true; }
	bool isodd() const noexcept { return false; }
	bool iseven() const noexcept { return !isodd(); }
	bool ispos() const noexcept { return false; }
	bool isneg() const noexcept { return false; }
	int  scale() const noexcept { return 0; }

	// convert to string containing digits number of digits
	std::string str(size_t nrDigits = 0) const {
		if (iszero()) return std::string("0.0");

		int64_t magnitude = scale();
		if (magnitude > 1 || magnitude < 0) {
			// use scientific notation for non-trivial exponent values
			return std::string("TBD");
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
		std::string final;
		if (isneg()) {
			final = std::string("-") + before_decimal + "." + after_decimal;
		}
		else {
			final = before_decimal + "." + after_decimal;
		}
		return final;
	}

protected:
	bt _block[nrBlocks];

	// HELPER methods

	// convert to native floating-point, use conversion rules to cast down to float and double
	long double toNativeFloatingPoint() const {
		long double ld = 0;
		return ld;
	}

	dfloat& convert_signed(int64_t v) {
		if (0 == v) {
			setzero();
		}
		else {
			// convert 
		}
		return *this;
	}

	 dfloat& convert_unsigned(uint64_t v) {
		if (0 == v) {
			setzero();
		}
		else {
			// convert 
		}
		return *this;
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


////////////////////////    helper functions   /////////////////////////////////

// divide dfloat a and b and return result argument
template<unsigned ndigits, unsigned es, typename BlockType>
void divide(const dfloat<ndigits, es, BlockType>& a, const dfloat<ndigits, es, BlockType>& b, dfloat<ndigits, es, BlockType>& quotient) {
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_binary(const dfloat<ndigits, es, BlockType>& number) {
	std::stringstream s;
	s << "to_binary TBD";
	return s.str();
}

////////////////////////    DFLOAT functions   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> abs(const dfloat<ndigits, es, BlockType>& a) {
	return a; // (a < 0 ? -a : a);
}


////////////////////////  stream operators   /////////////////////////////////


// generate an dfloat format ASCII format
template<unsigned ndigits, unsigned es, typename BlockType>
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
template<unsigned ndigits, unsigned es, typename BlockType>
inline std::istream& operator>>(std::istream& istr, dfloat<ndigits, es, BlockType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators

// read a dfloat ASCII format and make a dfloat out of it
template<unsigned ndigits, unsigned es, typename BlockType>
bool parse(const std::string& number, dfloat<ndigits, es, BlockType>& value) {
	bool bSuccess = false;

	return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - dfloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator==(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return true;
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator!=(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator< (const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return false; // lhs and rhs are the same
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator> (const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator<=(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator>=(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator==(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator==(lhs, dfloat<ndigits, es, BlockType>(rhs));
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator!=(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator< (const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator<(lhs, dfloat<ndigits, es, BlockType>(rhs));
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator> (const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (dfloat<ndigits, es, BlockType>(rhs), lhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator<=(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator>=(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dfloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator==(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator==(dfloat<ndigits, es, BlockType>(lhs), rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator!=(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator< (const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator<(dfloat<ndigits, es, BlockType>(lhs), rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator> (const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator<=(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator>=(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - dfloat binary arithmetic operators
// BINARY ADDITION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator+(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator-(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator*(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator/(const dfloat<ndigits, es, BlockType>& lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	dfloat ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - literal binary arithmetic operators
// BINARY ADDITION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator+(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator+(lhs, dfloat<ndigits, es, BlockType>(rhs));
}
// BINARY SUBTRACTION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator-(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator-(lhs, dfloat<ndigits, es, BlockType>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator*(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator*(lhs, dfloat<ndigits, es, BlockType>(rhs));
}
// BINARY DIVISION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator/(const dfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator/(lhs, dfloat<ndigits, es, BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dfloat binary arithmetic operators
// BINARY ADDITION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator+(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator+(dfloat<ndigits, es, BlockType>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator-(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator-(dfloat<ndigits, es, BlockType>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator*(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator*(dfloat<ndigits, es, BlockType>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned ndigits, unsigned es, typename BlockType>
inline dfloat<ndigits, es, BlockType> operator/(const double lhs, const dfloat<ndigits, es, BlockType>& rhs) {
	return operator/(dfloat<ndigits, es, BlockType>(lhs), rhs);
}

}} // namespace sw::universal
