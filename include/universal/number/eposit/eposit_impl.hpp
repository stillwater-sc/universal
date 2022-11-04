#pragma once
// eposit.hpp: definition of an adaptive precision tapered floating-point number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>

#include <universal/number/eposit/exceptions.hpp>

namespace sw { namespace universal {

// forward references
class eposit;
inline eposit& convert(int64_t v, eposit& result);
inline eposit& convert_unsigned(uint64_t v, eposit& result);
bool parse(const std::string& number, eposit& v);

// eposit is an adaptive precision linear floating-point type
class eposit {
	using BlockType = uint32_t;
public:
	eposit() : sign(false), exp(0) { }

	eposit(const eposit&) = default;
	eposit(eposit&&) = default;

	eposit& operator=(const eposit&) = default;
	eposit& operator=(eposit&&) = default;

	// initializers for native types
	explicit eposit(const signed char initial_value)        { *this = initial_value; }
	explicit eposit(const short initial_value)              { *this = initial_value; }
	explicit eposit(const int initial_value)                { *this = initial_value; }
	explicit eposit(const long initial_value)               { *this = initial_value; }
	explicit eposit(const long long initial_value)          { *this = initial_value; }
	explicit eposit(const char initial_value)               { *this = initial_value; }
	explicit eposit(const unsigned short initial_value)     { *this = initial_value; }
	explicit eposit(const unsigned int initial_value)       { *this = initial_value; }
	explicit eposit(const unsigned long initial_value)      { *this = initial_value; }
	explicit eposit(const unsigned long long initial_value) { *this = initial_value; }
	explicit eposit(const float initial_value)              { *this = initial_value; }
	explicit eposit(const double initial_value)             { *this = initial_value; }
	explicit eposit(const long double initial_value)        { *this = initial_value; }

	// assignment operators for native types
	eposit& operator=(const signed char rhs)        { return convert(rhs, *this); }
	eposit& operator=(const short rhs)              { return convert(rhs, *this); }
	eposit& operator=(const int rhs)                { return convert(rhs, *this); }
	eposit& operator=(const long rhs)               { return convert(rhs, *this); }
	eposit& operator=(const long long rhs)          { return convert(rhs, *this); }
	eposit& operator=(const char rhs)               { return convert_unsigned(rhs, *this); }
	eposit& operator=(const unsigned short rhs)     { return convert_unsigned(rhs, *this); }
	eposit& operator=(const unsigned int rhs)       { return convert_unsigned(rhs, *this); }
	eposit& operator=(const unsigned long rhs)      { return convert_unsigned(rhs, *this); }
	eposit& operator=(const unsigned long long rhs) { return convert_unsigned(rhs, *this); }
	eposit& operator=(const float rhs)              { return float_assign(rhs); }
	eposit& operator=(const double rhs)             { return float_assign(rhs); }
	eposit& operator=(const long double rhs)        { return float_assign(rhs); }

#ifdef ADAPTER_POSIT_AND_EPOSIT
	// POSIT_CONCEPT_GENERALIZATION
	// TODO: SFINAE to assure we only match a posit<nbits,es> concept
	template<typename PositType>
	eposit& operator=(const PositType& rhs) {
		convert_p2i(rhs, *this);
		return *this;
	}
#endif // ADAPTER_POSIT_AND_EPOSIT

	// prefix operators
	eposit operator-() const {
		eposit negated(*this);
		return negated;
	}

	// conversion operators
	explicit operator float() const { return float(toNativeFloatingPoint()); }
	explicit operator double() const { return float(toNativeFloatingPoint()); }
	explicit operator long double() const { return toNativeFloatingPoint(); }

	// arithmetic operators
	eposit& operator+=(const eposit& rhs) {
		return *this;
	}
	eposit& operator-=(const eposit& rhs) {
		return *this;
	}
	eposit& operator*=(const eposit& rhs) {
		return *this;
	}
	eposit& operator/=(const eposit& rhs) {
		return *this;
	}

	// modifiers
	inline void clear() { sign = false; exp = 0; coef.clear(); }
	inline void setzero() { clear(); }
	// use un-interpreted raw bits to set the bits of the eposit
	inline void set_raw_bits(unsigned long long value) {
		clear();
	}
	inline eposit& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	inline bool iszero() const { return !sign && coef.size() == 0; }
	inline bool isone() const  { return true; }
	inline bool isodd() const  { return false; }
	inline bool iseven() const { return !isodd(); }
	inline bool ispos() const  { return !sign; }
	inline bool ineg() const   { return sign; }
	inline int64_t scale() const { return exp + int64_t(coef.size()); }

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
			if (sign)
				return std::string("-0.0") + str;
			else
				return std::string("0.0") + str;
		}

		std::string before_decimal = std::to_string(coef.back());

		if (exponent >= 0) {
			if (sign)
				return std::string("-") + before_decimal + ".0";
			else
				return before_decimal + ".0";
		}

		// now the digits after the radix point
		std::string after_decimal = str.substr((size_t)(str.size() + exponent), (size_t)-exponent);
		if (sign)
			return std::string("-") + before_decimal + "." + after_decimal;
		else
			return before_decimal + "." + after_decimal;

		return std::string("bad");
	}

	void test(bool _sign, int _exp, std::vector<BlockType>& _coef) {
		sign = _sign;
		coef = _coef;
		exp = _exp;
	}
protected:
	bool                   sign;  // sign of the number: -1 if true, +1 if false, zero is positive
	int64_t                exp;   // exponent of the number
	std::vector<BlockType> coef;  // coefficients of the polynomial

	// HELPER methods

	// convert to native floating-point, use conversion rules to cast down to float and double
	long double toNativeFloatingPoint() const {
		long double ld = 0;
		return ld;
	}

	template<typename Ty>
	eposit& float_assign(Ty& rhs) {
		clear();
		long long base = (long long)rhs;
		*this = base;
		return *this;
	}

	// convert to string with nrDigits of significant digits and return the scale
	// value = str + "10^" + scale
	int64_t trimmed(size_t nrDigits, std::string& number) const {
		if (coef.size() == 0) return 0;
		int64_t exponent = exp;
		size_t length = coef.size();
		size_t index = 0; 
		if (nrDigits == 0) {
			nrDigits = length * 9;
		}
		else {
			size_t nrSegments = (nrDigits + 17) / 9;
			if (nrSegments < length) {
				index = length - nrSegments;
				exponent += index;
				length = nrSegments;
			}
		}
		exponent *= 9;
		char segment[] = "012345678";
		number.clear();
		size_t i = length;
		while (i-- > 0) {
			BlockType w = coef[i];
			for (int i = 8; i >= 0; --i) {
				segment[i] = w % 10 + '0';
				w /= 10;
			}
			number += segment;
		}

		// process leading zeros
		size_t lz = 0;
		while (number[lz] == '0') ++lz;
		nrDigits += lz;
		if (nrDigits < number.size()) {
			exponent += number.size() - nrDigits;
			number.resize(nrDigits);
		}

		return exponent;
	}

	std::string sci_notation(size_t nrDigits) const {
		if (coef.size() == 0) return std::string("0.0");
		std::string str;
		int64_t exponent = trimmed(nrDigits, str);
		// remove leading zeros
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) { return (ch != '0'); }));
		exponent += str.size() - 1;
		str = str.substr(0, 1) + "." + &str[1];
		if (exponent != 0) {
			str += "*10^";
			str += std::to_string(exponent);
		}
		if (sign) str = std::string("-") + str;
		return str;
	}

private:

	// eposit - eposit logic comparisons
	friend bool operator==(const eposit& lhs, const eposit& rhs);

	// eposit - literal logic comparisons
	friend bool operator==(const eposit& lhs, const long long rhs);

	// literal - eposit logic comparisons
	friend bool operator==(const long long lhs, const eposit& rhs);

	// find the most significant bit set
	friend signed findMsb(const eposit& v);
};

inline eposit& convert(int64_t v, eposit& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

inline eposit& convert_unsigned(uint64_t v, eposit& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

////////////////////////    MPFLOAT functions   /////////////////////////////////


inline eposit abs(const eposit& a) {
	return a; // (a < 0 ? -a : a);
}


// findMsb takes an eposit reference and returns the position of the most significant bit, -1 if v == 0

inline signed findMsb(const eposit& v) {
	return -1; // no significant bit found, all bits are zero
}

////////////////////////    INTEGER operators   /////////////////////////////////

// divide eposit a and b and return result argument

void divide(const eposit& a, const eposit& b, eposit& quotient) {
}

/// stream operators

// read a eposit ASCII format and make a binary eposit out of it

bool parse(const std::string& number, eposit& value) {
	bool bSuccess = false;

	return bSuccess;
}

// generate an eposit format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const eposit& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the eposit into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));

	return ostr << ss.str();
}

// read an ASCII eposit format

inline std::istream& operator>>(std::istream& istr, eposit& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// eposit - eposit binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths

inline bool operator==(const eposit& lhs, const eposit& rhs) {
	return true;
}

inline bool operator!=(const eposit& lhs, const eposit& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const eposit& lhs, const eposit& rhs) {
	return false; // lhs and rhs are the same
}

inline bool operator> (const eposit& lhs, const eposit& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const eposit& lhs, const eposit& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const eposit& lhs, const eposit& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// eposit - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const eposit& lhs, const long long rhs) {
	return operator==(lhs, eposit(rhs));
}

inline bool operator!=(const eposit& lhs, const long long rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const eposit& lhs, const long long rhs) {
	return operator<(lhs, eposit(rhs));
}

inline bool operator> (const eposit& lhs, const long long rhs) {
	return operator< (eposit(rhs), lhs);
}

inline bool operator<=(const eposit& lhs, const long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const eposit& lhs, const long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - eposit binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(const long long lhs, const eposit& rhs) {
	return operator==(eposit(lhs), rhs);
}

inline bool operator!=(const long long lhs, const eposit& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const long long lhs, const eposit& rhs) {
	return operator<(eposit(lhs), rhs);
}

inline bool operator> (const long long lhs, const eposit& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const long long lhs, const eposit& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const long long lhs, const eposit& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// eposit - eposit binary arithmetic operators
// BINARY ADDITION

inline eposit operator+(const eposit& lhs, const eposit& rhs) {
	eposit sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION

inline eposit operator-(const eposit& lhs, const eposit& rhs) {
	eposit diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION

inline eposit operator*(const eposit& lhs, const eposit& rhs) {
	eposit mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION

inline eposit operator/(const eposit& lhs, const eposit& rhs) {
	eposit ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// eposit - literal binary arithmetic operators
// BINARY ADDITION

inline eposit operator+(const eposit& lhs, const long long rhs) {
	return operator+(lhs, eposit(rhs));
}
// BINARY SUBTRACTION

inline eposit operator-(const eposit& lhs, const long long rhs) {
	return operator-(lhs, eposit(rhs));
}
// BINARY MULTIPLICATION

inline eposit operator*(const eposit& lhs, const long long rhs) {
	return operator*(lhs, eposit(rhs));
}
// BINARY DIVISION

inline eposit operator/(const eposit& lhs, const long long rhs) {
	return operator/(lhs, eposit(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - eposit binary arithmetic operators
// BINARY ADDITION

inline eposit operator+(const long long lhs, const eposit& rhs) {
	return operator+(eposit(lhs), rhs);
}
// BINARY SUBTRACTION

inline eposit operator-(const long long lhs, const eposit& rhs) {
	return operator-(eposit(lhs), rhs);
}
// BINARY MULTIPLICATION

inline eposit operator*(const long long lhs, const eposit& rhs) {
	return operator*(eposit(lhs), rhs);
}
// BINARY DIVISION

inline eposit operator/(const long long lhs, const eposit& rhs) {
	return operator/(eposit(lhs), rhs);
}

}} // namespace sw::universal
