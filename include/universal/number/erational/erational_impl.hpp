#pragma once
// erational_impl.hpp: implementation of adaptive precision decimal erational arithmetic type
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <sstream>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>
#include <regex>
#include <algorithm>

#include <universal/native/ieee754.hpp>
#include <universal/string/strmanip.hpp>
#include <universal/number/erational/exceptions.hpp>
#include <universal/number/edecimal/edecimal.hpp>

namespace sw { namespace universal {

/// <summary>
/// Adaptive precision rational number system type
/// </summary>
/// The erational is comprised of two adaptive precision decimals representing the numerator and denominator.
/// The digits of both are managed as a vector with the digit for 10^0 stored at index 0, 10^1 stored at index 1, etc.
class erational {
public:
	erational() { setzero(); }

	erational(const erational&) = default;
	erational(erational&&) = default;

	erational& operator=(const erational&) = default;
	erational& operator=(erational&&) = default;

	// initializers for native types
	erational(char initial_value)               { *this = initial_value; }
	erational(short initial_value)              { *this = initial_value; }
	erational(int initial_value)                { *this = initial_value; }
	erational(long initial_value)               { *this = initial_value; }
	erational(long long initial_value)          { *this = initial_value; }
	erational(unsigned char initial_value)      { *this = initial_value; }
	erational(unsigned short initial_value)     { *this = initial_value; }
	erational(unsigned int initial_value)       { *this = initial_value; }
	erational(unsigned long initial_value)      { *this = initial_value; }
	erational(unsigned long long initial_value) { *this = initial_value; }
	erational(float initial_value)              { *this = initial_value; }
	erational(double initial_value)             { *this = initial_value; }


	// assignment operators for native types
	erational& operator=(const std::string& digits) {
		parse(digits);
		return *this;
	}
	erational& operator=(signed char rhs)        { return from_signed<signed char>(rhs); }
	erational& operator=(short rhs)              { return from_signed<short>(rhs); }
	erational& operator=(int rhs)                { return from_signed<int>(rhs); }
	erational& operator=(long rhs)               { return from_signed<long>(rhs); }
	erational& operator=(long long rhs)          { return from_signed<long long>(rhs); }
	erational& operator=(unsigned char rhs)      { return from_unsigned<unsigned char>(rhs); }
	erational& operator=(unsigned short rhs)     { return from_unsigned<unsigned short>(rhs); }
	erational& operator=(unsigned int rhs)       { return from_unsigned<unsigned int>(rhs); }
	erational& operator=(unsigned long rhs)      { return from_unsigned<unsigned long>(rhs); }
	erational& operator=(unsigned long long rhs) { return from_unsigned<unsigned long long>(rhs); }
	erational& operator=(float rhs)              { return from_ieee754<float>(rhs); }
	erational& operator=(double rhs)             { return from_ieee754<double>(rhs); }

#if LONG_DOUBLE_SUPPORT
	erational(long double initial_value)         { *this = initial_value; }
	erational& operator=(long double rhs)        { return from_ieee754<long double>(rhs); }
#endif

	// unitary operators
	erational operator-() const {
		erational tmp(*this);
		tmp.setsign(!tmp.sign());
		return tmp;
	}
	erational operator++(int) { // postfix
		erational tmp(*this);
		++numerator;
		return tmp;
	}
	erational& operator++() { // prefix
		++numerator;
		return *this;
	}
	erational operator--(int) { // postfix
		erational tmp(*this);
		--numerator;
		return tmp;
	}
	erational& operator--() { // prefix
		--numerator;
		return *this;
	}

	// arithmetic operators
	erational& operator+=(const erational& rhs) {
		edecimal a = (negative ? -numerator : numerator);
		edecimal b = denominator;
		edecimal c = (rhs.negative ? -rhs.numerator : rhs.numerator);
		edecimal d = rhs.denominator;
		if (denominator == rhs.denominator) {
			edecimal num = a + c;
			negative = num.isneg();
			numerator = (negative ? -num : num);
		}
		else {
			edecimal e = a * d + b * c;
			edecimal f = b * d;
			negative = e.isneg();
			numerator = (negative ? -e : e);
			denominator = f;
		}
		normalize();
		return *this;
	}
	erational& operator-=(const erational& rhs) {
		edecimal a = (negative ? -numerator : numerator);
		edecimal b = denominator;
		edecimal c = (rhs.negative ? -rhs.numerator : rhs.numerator);
		edecimal d = rhs.denominator;
		if (denominator == rhs.denominator) {
			edecimal num = a - c;
			negative = num.isneg();
			numerator = (negative ? -num : num);
		}
		else {
			edecimal e = a * d - b * c;
			edecimal f = b * d;
			negative = e.isneg();
			numerator = (negative ? -e : e);
			denominator = f;
		}
		normalize();
		return *this;
	}
	erational& operator*=(const erational& rhs) {
		numerator *= rhs.numerator;
		denominator *= rhs.denominator;
		negative = !((negative && rhs.negative) || (!negative && !rhs.negative));
		normalize();
		return *this;
	}
	erational& operator/=(const erational& rhs) {
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) {
			throw erational_divide_by_zero();
		}
#else
		std::cerr << "erational_divide_by_zero\n";
#endif
		negative = !((negative && rhs.negative) || (!negative && !rhs.negative));
		numerator *= rhs.denominator;
		denominator *= rhs.numerator;
		normalize();
		return *this;
	}

	// conversion operators 
	explicit operator unsigned short() const     { return to_unsigned<unsigned short>(); }
	explicit operator unsigned int() const       { return to_unsigned<unsigned int>(); }
	explicit operator unsigned long() const      { return to_unsigned<unsigned long>(); }
	explicit operator unsigned long long() const { return to_unsigned<unsigned long long>(); }
	explicit operator short() const              { return to_signed<short>(); }
	explicit operator int() const                { return to_signed<int>(); }
	explicit operator long() const               { return to_signed<long>(); }
	explicit operator long long() const          { return to_signed<long long>(); }
	explicit operator float() const              { return to_ieee754<float>(); }
	explicit operator double() const             { return to_ieee754<double>(); }
	explicit operator long double() const        { return to_ieee754<long double>(); }

	// selectors
	inline bool iszero() const {
		return numerator.iszero();
	}
	inline bool sign() const { return negative; }
	inline bool isneg() const { return negative; }   // <  0
	inline bool ispos() const { return !negative; }  // >= 0
	inline edecimal top() const { return numerator; }
	inline edecimal bottom() const { return denominator; }
	// modifiers
	inline void setzero() { 
		negative    = false;
		numerator   = 0;
		denominator = 1;
	}
	inline void setsign(bool sign) { negative = sign; }
	inline void setneg() { negative = true; }
	inline void setpos() { negative = false; }
	inline void setnumerator(const edecimal& num) { numerator = num; }
	inline void setdenominator(const edecimal& denom) { denominator = denom; }
	inline void setbits(uint64_t v) { *this = v; } // API to be consistent with the other number systems

	// read a erational ASCII format and make a erational type out of it
	bool parse(const std::string& _digits) {
		bool bSuccess = false;
		std::string digits(_digits);
		trim(digits);
		// check if the txt is an erational form:[+-]*[0123456789]+
		std::regex erational_regex("[+-]*[0123456789]+");
		if (std::regex_match(digits, erational_regex)) {
			// found a erational representation
			numerator.clear();
			auto it = digits.begin();
			if (*it == '-') {
				setneg();
				++it;
			}
			else if (*it == '+') {
				++it;
			}
			for (; it != digits.end(); ++it) {
				uint8_t v;
				switch (*it) {
				case '0':
					v = 0;
					break;
				case '1':
					v = 1;
					break;
				case '2':
					v = 2;
					break;
				case '3':
					v = 3;
					break;
				case '4':
					v = 4;
					break;
				case '5':
					v = 5;
					break;
				case '6':
					v = 6;
					break;
				case '7':
					v = 7;
					break;
				case '8':
					v = 8;
					break;
				case '9':
					v = 9;
					break;
				default:
					v = 0;
				}
				numerator.push_back(v);
			}
			std::reverse(numerator.begin(), numerator.end());
			bSuccess = true;
		}
		return bSuccess;
	}

protected:
	// HELPER methods

	// remove greatest common divisor out of the numerator/denominator pair
	inline void normalize() {
		edecimal a, b, r;
		a = numerator; b = denominator;  // precondition is numerator and denominator are positive
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (b.iszero()) {
			throw erational_divide_by_zero();
		}
#else
		std::cerr << "erational_divide_by_zero\n";
#endif
		while (a % b > 0) {
			r = a % b;
			a = b;
			b = r;
		}
		numerator /= b;
		denominator /= b;
	}
	// conversion functions
	// convert to signed int: TODO, SFINEA
	template<typename SignedInt>
	inline SignedInt to_signed() const { return static_cast<SignedInt>(numerator / denominator); }
	// convert to unsigned int: TODO, SFINEA
	template<typename UnsignedInt>
	inline UnsignedInt to_unsigned() const { return static_cast<UnsignedInt>(numerator / denominator); }
	// convert to ieee-754: TODO, SFINEA
	template<typename Ty>
	inline Ty to_ieee754() const { return Ty(numerator) / Ty(denominator); }

	// from signed int: TODO, SFINEA
	template<typename UnsignedInt>
	erational& from_signed(UnsignedInt& rhs) {
		if (rhs < 0) {
			negative = true;
			numerator = -rhs;
		}
		else {
			negative = false;
			numerator = rhs;
		}
		denominator = 1;
		return *this;
	}
	// from signed int: TODO, SFINEA
	template<typename UnsignedInt>
	erational& from_unsigned(UnsignedInt& rhs) {
		negative  = false;
		numerator = rhs;
		denominator = 1;
		return *this;
	}
	// from ieee754: TODO, SFINEA
	template<typename Real>
	erational& from_ieee754(Real& rhs) {
		std::cerr << "TBD" << rhs << std::endl;
		return *this;
	}

private:
	// sign-magnitude number: indicate if number is positive or negative
	bool negative;
	edecimal numerator; // will be managed as a positive number
	edecimal denominator; // will be managed as a positive number

	friend std::ostream& operator<<(std::ostream& ostr, const erational& d);
	friend std::istream& operator>>(std::istream& istr, erational& d);

	// erational - erational logic operators
	friend bool operator==(const erational& lhs, const erational& rhs);
	friend bool operator!=(const erational& lhs, const erational& rhs);
	friend bool operator<(const erational& lhs, const erational& rhs);
	friend bool operator>(const erational& lhs, const erational& rhs);
	friend bool operator<=(const erational& lhs, const erational& rhs);
	friend bool operator>=(const erational& lhs, const erational& rhs);
};

////////////////// helper functions


////////////////// erational operators

/// stream operators

// generate an ASCII erational string
inline std::string to_string(const erational& d) {
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << "TBD";
	return str.str();
}

// generate an ASCII erational format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const erational& d) {
	// make certain that setw and left/right operators work properly
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << d.numerator << '/' << d.denominator;
	return ostr << str.str();
}

// read an ASCII erational format from an istream
inline std::istream& operator>>(std::istream& istr, erational& p) {
	std::string txt;
	istr >> txt;
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into a erational value\n";
	}
	return istr;
}

/// erational binary arithmetic operators

// binary addition of erational numbers
inline erational operator+(const erational& lhs, const erational& rhs) {
	erational sum = lhs;
	sum += rhs;
	return sum;
}
// binary subtraction of erational numbers
inline erational operator-(const erational& lhs, const erational& rhs) {
	erational diff = lhs;
	diff -= rhs;
	return diff;
}
// binary mulitplication of erational numbers
inline erational operator*(const erational& lhs, const erational& rhs) {
	erational mul = lhs;
	mul *= rhs;
	return mul;
}
// binary division of erational numbers
inline erational operator/(const erational& lhs, const erational& rhs) {
	erational ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////
/// logic operators

/// erational - erational logic operators

// equality test
bool operator==(const erational& lhs, const erational& rhs) {
	return lhs.numerator == rhs.numerator && lhs.denominator == rhs.denominator;
}
// inequality test
bool operator!=(const erational& lhs, const erational& rhs) {
	return !operator==(lhs, rhs);
}
// less-than test
bool operator<(const erational& lhs, const erational& rhs) {
	// a/b < c/d  => ad / bd < cb / bd => ad < cb
	edecimal ad = lhs.numerator * rhs.denominator;
	edecimal bc = lhs.denominator * rhs.numerator;
	return ad < bc;
}
// greater-than test
bool operator>(const erational& lhs, const erational& rhs) {
	return operator<(rhs, lhs);
}
// less-or-equal test
bool operator<=(const erational& lhs, const erational& rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}
// greater-or-equal test
bool operator>=(const erational& lhs, const erational& rhs) {
	return !operator<(lhs, rhs);
}

// erational - long logic operators
inline bool operator==(const erational& lhs, long rhs) {
	return lhs == erational(rhs);
}
inline bool operator!=(const erational& lhs, long rhs) {
	return !operator==(lhs, erational(rhs));
}
inline bool operator< (const erational& lhs, long rhs) {
	return operator<(lhs, erational(rhs));
}
inline bool operator> (const erational& lhs, long rhs) {
	return operator< (erational(rhs), lhs);
}
inline bool operator<=(const erational& lhs, long rhs) {
	return operator< (lhs, erational(rhs)) || operator==(lhs, erational(rhs));
}
inline bool operator>=(const erational& lhs, long rhs) {
	return !operator<(lhs, erational(rhs));
}

// long - erational logic operators
inline bool operator==(long lhs, const erational& rhs) {
	return erational(lhs) == rhs;
}
inline bool operator!=(long lhs, const erational& rhs) {
	return !operator==(erational(lhs), rhs);
}
inline bool operator< (long lhs, const erational& rhs) {
	return operator<(erational(lhs), rhs);
}
inline bool operator> (long lhs, const erational& rhs) {
	return operator< (erational(lhs), rhs);
}
inline bool operator<=(long lhs, const erational& rhs) {
	return operator< (erational(lhs), rhs) || operator==(erational(lhs), rhs);
}
inline bool operator>=(long lhs, const erational& rhs) {
	return !operator<(erational(lhs), rhs);
}

///////////////////////////////////////////////////////////////////////


// find largest multiplier of rhs being less or equal to lhs by subtraction; assumes 0*rhs <= lhs <= 9*rhs 
erational findLargestMultiple_(const erational& lhs, const erational& rhs) {
	erational multiplier;

	return multiplier;
}


///////////////////////
// erationalntdiv_t for erational to capture quotient and remainder during long division
struct erationalintdiv {
	erational quot; // quotient
	erational rem;  // remainder
};

// divide integer erational a and b and return result argument
erationalintdiv erational_divide(const erational& lhs, const erational& rhs) {
	if (rhs.iszero()) {
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
		throw erational_divide_by_zero{};
#else
		std::cerr << "erational_divide_by_zero\n";
#endif
	}

	// a/b / c/d => ad / bc
	erationalintdiv divresult;
	divresult.quot.setnumerator(lhs.top() * rhs.bottom());
	divresult.quot.setdenominator(lhs.bottom() * rhs.top());
	return divresult;
}

// return quotient of a erational integer division
erational quotient(const erational& _a, const erational& _b) {
	return erational_divide(_a, _b).quot;
}
// return remainder of a erational integer division
erational remainder(const erational& _a, const erational& _b) {
	return erational_divide(_a, _b).rem;
}

}} // namespace sw::universal

