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
	static constexpr unsigned fbits = 106; // number of fraction digits

	static constexpr int      EXP_BIAS = ((1 << (es - 1u)) - 1l);
	static constexpr int      MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
	static constexpr int      MIN_EXP_NORMAL = 1 - EXP_BIAS;
	static constexpr int      MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP

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
	constexpr dd(signed char iv)                    noexcept { *this = iv; }
	constexpr dd(short iv)                          noexcept { *this = iv; }
	constexpr dd(int iv)                            noexcept { *this = iv; }
	constexpr dd(long iv)                           noexcept { *this = iv; }
	constexpr dd(long long iv)                      noexcept { *this = iv; }
	constexpr dd(char iv)                           noexcept { *this = iv; }
	constexpr dd(unsigned short iv)                 noexcept { *this = iv; }
	constexpr dd(unsigned int iv)                   noexcept { *this = iv; }
	constexpr dd(unsigned long iv)                  noexcept { *this = iv; }
	constexpr dd(unsigned long long iv)             noexcept { *this = iv; }
	constexpr dd(float iv)                          noexcept { *this = iv; }
	constexpr dd(double iv)                         noexcept { *this = iv; }
	constexpr dd(long double iv)                    noexcept { *this = iv; }

	// assignment operators for native types
	constexpr dd& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr dd& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }
	constexpr dd& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }

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

	constexpr void setbit(unsigned index, bool b = true)           noexcept {
		if (index < 64) {
			// set bit in lower limb
		}
		else if (index < 128) {
			// set bit in upper limb
		}
		else {
			// NOP if index out of bounds
		}
	}
	constexpr void setbits(uint64_t value)                         noexcept {
		clear();
	}
	
	// create specific number system values of interest
	constexpr dd& maxpos() noexcept {
		hi = 1.7976931348623157e+308;
		lo = 1.9958403095347196e+292;
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
	constexpr bool iszero()   const noexcept { return false; }
	constexpr bool isone()    const noexcept { return true;  }
	constexpr bool isodd()    const noexcept { return false; }
	constexpr bool iseven()   const noexcept { return !isodd(); }
	constexpr bool ispos()    const noexcept { return false; }
	constexpr bool isneg()    const noexcept { return false; }
	constexpr bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		bool isNaN = false; // (_bits & 0x7F80u) && (_bits & 0x007F);
		bool isNegNaN = isNaN && negative;
		bool isPosNaN = isNaN && !negative;
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
			(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
				(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}
	constexpr bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		bool isInf = false; // (_bits & 0x7F80u);
		bool isNegInf = isInf && negative;
		bool isPosInf = isInf && !negative;
		return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
			(InfType == INF_TYPE_NEGATIVE ? isNegInf :
				(InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}

	bool sign()     const noexcept { return false; }
	int  scale()    const noexcept { return 0; }
	int  exponent() const noexcept { return 0; }
	int  fraction() const noexcept { return 0; }


	// convert to string containing digits number of digits
	std::string str(size_t nrDigits = 0) const {
		if (iszero()) return std::string("0.0");

		int64_t magnitude = scale();
		if (magnitude > 1 || magnitude < 0) {
			// use scientific notation for non-trivial exponent values
			return std::string("TBD");
		}

		std::string str;
		int64_t exponent = 0;

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

	constexpr dd& convert_signed(int64_t v) {
		if (0 == v) {
			setzero();
		}
		else {
			// convert 
		}
		return *this;
	}

	constexpr dd& convert_unsigned(uint64_t v) {
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
	constexpr dd& convert_ieee754(NativeFloat& rhs) {
		clear();
		long long base = (long long)rhs;
		*this = base;
		return *this;
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
