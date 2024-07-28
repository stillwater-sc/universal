#pragma once
// dd_impl.hpp: implementation of a fixed-size, arbitrary configuration decimal floating-point number system
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
// dd exception structure
#include <universal/number/dd/exceptions.hpp>

namespace sw { namespace universal {

// dd is an unevaluated pair of IEEE-754 doubles that provides a (1,11,106) floating-point triple
class dd {
public:
	static constexpr unsigned nbits = 128;
	static constexpr unsigned es = 11;
	static constexpr unsigned fdigits = 106; // number of fraction digits

	/// trivial constructor
	dd() = default;

	dd(const dd&) = default;
	dd(dd&&) = default;

	dd& operator=(const dd&) = default;
	dd& operator=(dd&&) = default;

	// converting constructors
	dd(const std::string& stringRep) : hi{}, lo{} { assign(stringRep); }

	// specific value constructor
	constexpr dd(const SpecificValue code) noexcept : hi{}, lo{} {
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
		case SpecificValue::nar: // approximation as dds don't have a NaR
		case SpecificValue::qnan:
			setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	// initializers for native types
	explicit dd(signed char iv)        noexcept { *this = iv; }
	explicit dd(short iv)              noexcept { *this = iv; }
	explicit dd(int iv)                noexcept { *this = iv; }
	explicit dd(long iv)               noexcept { *this = iv; }
	explicit dd(long long iv)          noexcept { *this = iv; }
	explicit dd(char iv)               noexcept { *this = iv; }
	explicit dd(unsigned short iv)     noexcept { *this = iv; }
	explicit dd(unsigned int iv)       noexcept { *this = iv; }
	explicit dd(unsigned long iv)      noexcept { *this = iv; }
	explicit dd(unsigned long long iv) noexcept { *this = iv; }
	explicit dd(float iv)              noexcept { *this = iv; }
	explicit dd(double iv)             noexcept { *this = iv; }
	explicit dd(long double iv)        noexcept { *this = iv; }

	// assignment operators for native types
	dd& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	dd& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	dd& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	dd& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	dd& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	dd& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	dd& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	dd& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	dd& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	dd& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	dd& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	dd& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }
	dd& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }

	// prefix operators
	dd operator-() const noexcept {
		dd negated(*this);
		return negated;
	}

	// conversion operators
	explicit operator float() const { return float(toNativeFloatingPoint()); }
	explicit operator double() const { return float(toNativeFloatingPoint()); }
	explicit operator long double() const { return toNativeFloatingPoint(); }

	// arithmetic operators
	dd& operator+=(const dd& rhs) {
		*this = rhs;
		return *this;
	}
	dd& operator-=(const dd& rhs) {
		*this = rhs;
		return *this;
	}
	dd& operator*=(const dd& rhs) {
		*this = rhs;
		return *this;
	}
	dd& operator/=(const dd& rhs) {
		*this = rhs;
		return *this;
	}

	// unary operators
	dd& operator++() {
		return *this;
	}
	dd operator++(int) {
		dd tmp(*this);
		operator++();
		return tmp;
	}
	dd& operator--() {
		return *this;
	}
	dd operator--(int) {
		dd tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear()                                         noexcept {  }
	constexpr void setzero()                                       noexcept { clear(); }
	constexpr void setinf(bool sign = true)                        noexcept { }
	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { }
	constexpr void setsign(bool sign = true)                       noexcept { }
	constexpr void setexponent(const std::string& expDigits)       noexcept { }
	constexpr void setfraction(const std::string& fracDigits)      noexcept { }
	// use un-interpreted raw bits to set the value of the dd
	constexpr void setbits(uint64_t value)                         noexcept {
		clear();
	}
	
	// create specific number system values of interest
	constexpr dd& maxpos() noexcept {
		hi = 1.0f;
		lo = 0.0f;
		return *this;
	}
	constexpr dd& minpos() noexcept {
		hi = 1.0f;
		lo = 0.0f;
		return *this;
	}
	constexpr dd& zero() noexcept {
		// the zero value
		clear();
		return *this;
	}
	constexpr dd& minneg() noexcept {
		hi = 1.0f;
		lo = 0.0f;
		return *this;
	}
	constexpr dd& maxneg() noexcept {
		hi = 1.0f;
		lo = 0.0f;
		return *this;
	}

	dd& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	bool iszero() const noexcept { return false; }
	bool isone()  const noexcept { return true;  }
	bool isodd()  const noexcept { return false; }
	bool iseven() const noexcept { return !isodd(); }
	bool ispos()  const noexcept { return false; }
	bool isneg()  const noexcept { return false; }
	int  scale()  const noexcept { return 0; }

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
	double hi, lo;

	// HELPER methods

	// convert to native floating-point, use conversion rules to cast down to float and double
	long double toNativeFloatingPoint() const {
		long double ld = 0;
		return ld;
	}

	dd& convert_signed(int64_t v) {
		if (0 == v) {
			setzero();
		}
		else {
			// convert 
		}
		return *this;
	}

	dd& convert_unsigned(uint64_t v) {
		if (0 == v) {
			setzero();
		}
		else {
			// convert 
		}
		return *this;
	}

	// no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
	template<typename NativeFloat>
	dd& convert_ieee754(NativeFloat& rhs) {
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

	// dd - dd logic comparisons
	friend bool operator==(const dd& lhs, const dd& rhs);

	// dd - literal logic comparisons
	friend bool operator==(const dd& lhs, const double rhs);

	// literal - dd logic comparisons
	friend bool operator==(const double lhs, const dd& rhs);

};


////////////////////////    helper functions   /////////////////////////////////


inline std::string to_binary(const dd& number) {
	std::stringstream s;
	s << "to_binary TBD";
	return s.str();
}

////////////////////////    DFLOAT functions   /////////////////////////////////

inline dd abs(const dd& a) {
	return a; // (a < 0 ? -a : a);
}


////////////////////////  stream operators   /////////////////////////////////


// generate an dd format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const dd& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the dd into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));

	return ostr << ss.str();
}

// read an ASCII dd format
inline std::istream& operator>>(std::istream& istr, dd& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators

// read a dd ASCII format and make a dd out of it
bool parse(const std::string& number, dd& value) {
	bool bSuccess = false;

	return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - dd binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
inline bool operator==(const dd& lhs, const dd& rhs) {
	return true;
}

inline bool operator!=(const dd& lhs, const dd& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const dd& lhs, const dd& rhs) {
	return false; // lhs and rhs are the same
}

inline bool operator> (const dd& lhs, const dd& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const dd& lhs, const dd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const dd& lhs, const dd& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
inline bool operator==(const dd& lhs, const double rhs) {
	return operator==(lhs, dd(rhs));
}

inline bool operator!=(const dd& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const dd& lhs, const double rhs) {
	return operator<(lhs, dd(rhs));
}

inline bool operator> (const dd& lhs, const double rhs) {
	return operator< (dd(rhs), lhs);
}

inline bool operator<=(const dd& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const dd& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dd binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const double lhs, const dd& rhs) {
	return operator==(dd(lhs), rhs);
}

inline bool operator!=(const double lhs, const dd& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const double lhs, const dd& rhs) {
	return operator<(dd(lhs), rhs);
}

inline bool operator> (const double lhs, const dd& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const double lhs, const dd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const double lhs, const dd& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - dd binary arithmetic operators
// BINARY ADDITION
inline dd operator+(const dd& lhs, const dd& rhs) {
	dd sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
inline dd operator-(const dd& lhs, const dd& rhs) {
	dd diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
inline dd operator*(const dd& lhs, const dd& rhs) {
	dd mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
inline dd operator/(const dd& lhs, const dd& rhs) {
	dd ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - literal binary arithmetic operators
// BINARY ADDITION
inline dd operator+(const dd& lhs, const double rhs) {
	return operator+(lhs, dd(rhs));
}
// BINARY SUBTRACTION
inline dd operator-(const dd& lhs, const double rhs) {
	return operator-(lhs, dd(rhs));
}
// BINARY MULTIPLICATION
inline dd operator*(const dd& lhs, const double rhs) {
	return operator*(lhs, dd(rhs));
}
// BINARY DIVISION
inline dd operator/(const dd& lhs, const double rhs) {
	return operator/(lhs, dd(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dd binary arithmetic operators
// BINARY ADDITION
inline dd operator+(const double lhs, const dd& rhs) {
	return operator+(dd(lhs), rhs);
}
// BINARY SUBTRACTION
inline dd operator-(const double lhs, const dd& rhs) {
	return operator-(dd(lhs), rhs);
}
// BINARY MULTIPLICATION
inline dd operator*(const double lhs, const dd& rhs) {
	return operator*(dd(lhs), rhs);
}
// BINARY DIVISION
inline dd operator/(const double lhs, const dd& rhs) {
	return operator/(dd(lhs), rhs);
}

}} // namespace sw::universal
