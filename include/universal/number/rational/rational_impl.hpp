#pragma once
// rational_impl.hpp: implementation of adaptive precision rational arithmetic type
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
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
#include <universal/number/rational/exceptions.hpp>
#include <universal/number/decimal/decimal.hpp>

namespace sw { namespace universal {

/////////////////////////////////////////////
// Forward references
class rational;
rational quotient(const rational&, const rational&);

/// <summary>
/// Adaptive precision rational number system type
/// </summary>
/// The rational is comprised of two adaptive rationals representing the numerator and denominator.
/// The digits of both are managed as a vector with the digit for 10^0 stored at index 0, 10^1 stored at index 1, etc.
class rational {
public:
	rational() { setzero(); }

	rational(const rational&) = default;
	rational(rational&&) = default;

	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	// initializers for native types
	rational(char initial_value)               { *this = initial_value; }
	rational(short initial_value)              { *this = initial_value; }
	rational(int initial_value)                { *this = initial_value; }
	rational(long initial_value)               { *this = initial_value; }
	rational(long long initial_value)          { *this = initial_value; }
	rational(unsigned char initial_value)      { *this = initial_value; }
	rational(unsigned short initial_value)     { *this = initial_value; }
	rational(unsigned int initial_value)       { *this = initial_value; }
	rational(unsigned long initial_value)      { *this = initial_value; }
	rational(unsigned long long initial_value) { *this = initial_value; }
	rational(float initial_value)              { *this = initial_value; }
	rational(double initial_value)             { *this = initial_value; }


	// assignment operators for native types
	rational& operator=(const std::string& digits) {
		parse(digits);
		return *this;
	}
	rational& operator=(signed char rhs)        { return from_signed<signed char>(rhs); }
	rational& operator=(short rhs)              { return from_signed<short>(rhs); }
	rational& operator=(int rhs)                { return from_signed<int>(rhs); }
	rational& operator=(long rhs)               { return from_signed<long>(rhs); }
	rational& operator=(long long rhs)          { return from_signed<long long>(rhs); }
	rational& operator=(unsigned char rhs)      { return from_unsigned<unsigned char>(rhs); }
	rational& operator=(unsigned short rhs)     { return from_unsigned<unsigned short>(rhs); }
	rational& operator=(unsigned int rhs)       { return from_unsigned<unsigned int>(rhs); }
	rational& operator=(unsigned long rhs)      { return from_unsigned<unsigned long>(rhs); }
	rational& operator=(unsigned long long rhs) { return from_unsigned<unsigned long long>(rhs); }
	rational& operator=(float rhs)              { return from_ieee754<float>(rhs); }
	rational& operator=(double rhs)             { return from_ieee754<double>(rhs); }

#if LONG_DOUBLE_SUPPORT
	rational(long double initial_value)         { *this = initial_value; }
	rational& operator=(long double rhs)        { return from_ieee754<long double>(rhs); }
#endif

	// unitary operators
	rational operator-() const {
		rational tmp(*this);
		tmp.setsign(!tmp.sign());
		return tmp;
	}
	rational operator++(int) { // postfix
		rational tmp(*this);
		++numerator;
		return tmp;
	}
	rational& operator++() { // prefix
		++numerator;
		return *this;
	}
	rational operator--(int) { // postfix
		rational tmp(*this);
		--numerator;
		return tmp;
	}
	rational& operator--() { // prefix
		--numerator;
		return *this;
	}

	// arithmetic operators
	rational& operator+=(const rational& rhs) {
		decimal a = (negative ? -numerator : numerator);
		decimal b = denominator;
		decimal c = (rhs.negative ? -rhs.numerator : rhs.numerator);
		decimal d = rhs.denominator;
		if (denominator == rhs.denominator) {
			decimal num = a + c;
			negative = num.isneg();
			numerator = (negative ? -num : num);
		}
		else {
			decimal e = a * d + b * c;
			decimal f = b * d;
			negative = e.isneg();
			numerator = (negative ? -e : e);
			denominator = f;
		}
		normalize();
		return *this;
	}
	rational& operator-=(const rational& rhs) {
		decimal a = (negative ? -numerator : numerator);
		decimal b = denominator;
		decimal c = (rhs.negative ? -rhs.numerator : rhs.numerator);
		decimal d = rhs.denominator;
		if (denominator == rhs.denominator) {
			decimal num = a - c;
			negative = num.isneg();
			numerator = (negative ? -num : num);
		}
		else {
			decimal e = a * d - b * c;
			decimal f = b * d;
			negative = e.isneg();
			numerator = (negative ? -e : e);
			denominator = f;
		}
		normalize();
		return *this;
	}
	rational& operator*=(const rational& rhs) {
		numerator *= rhs.numerator;
		denominator *= rhs.denominator;
		negative = !((negative && rhs.negative) || (!negative && !rhs.negative));
		normalize();
		return *this;
	}
	rational& operator/=(const rational& rhs) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) {
			throw rational_divide_by_zero();
		}
#else
		std::cerr << "rational_divide_by_zero\n";
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
	inline decimal top() const { return numerator; }
	inline decimal bottom() const { return denominator; }
	// modifiers
	inline void setzero() { 
		negative    = false;
		numerator   = 0;
		denominator = 1;
	}
	inline void setsign(bool sign) { negative = sign; }
	inline void setneg() { negative = true; }
	inline void setpos() { negative = false; }
	inline void setnumerator(const decimal& num) { numerator = num; }
	inline void setdenominator(const decimal& denom) { denominator = denom; }
	inline void setbits(uint64_t v) { *this = v; } // API to be consistent with the other number systems

	// read a rational ASCII format and make a rational type out of it
	bool parse(const std::string& _digits) {
		bool bSuccess = false;
		std::string digits(_digits);
		trim(digits);
		// check if the txt is an rational form:[+-]*[0123456789]+
		std::regex rational_regex("[+-]*[0123456789]+");
		if (std::regex_match(digits, rational_regex)) {
			// found a rational representation
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
		decimal a, b, r;
		a = numerator; b = denominator;  // precondition is numerator and denominator are positive
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (b.iszero()) {
			throw rational_divide_by_zero();
		}
#else
		std::cerr << "rational_divide_by_zero\n";
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
	rational& from_signed(UnsignedInt& rhs) {
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
	rational& from_unsigned(UnsignedInt& rhs) {
		negative  = false;
		numerator = rhs;
		denominator = 1;
		return *this;
	}
	// from ieee754: TODO, SFINEA
	template<typename Real>
	rational& from_ieee754(Real& rhs) {
		std::cerr << "TBD" << rhs << std::endl;
		return *this;
	}

private:
	// sign-magnitude number: indicate if number is positive or negative
	bool negative;
	decimal numerator; // will be managed as a positive number
	decimal denominator; // will be managed as a positive number

	friend std::ostream& operator<<(std::ostream& ostr, const rational& d);
	friend std::istream& operator>>(std::istream& istr, rational& d);

	// rational - rational logic operators
	friend bool operator==(const rational& lhs, const rational& rhs);
	friend bool operator!=(const rational& lhs, const rational& rhs);
	friend bool operator<(const rational& lhs, const rational& rhs);
	friend bool operator>(const rational& lhs, const rational& rhs);
	friend bool operator<=(const rational& lhs, const rational& rhs);
	friend bool operator>=(const rational& lhs, const rational& rhs);
};

////////////////// helper functions


////////////////// rational operators

/// stream operators

// generate an ASCII rational string
inline std::string to_string(const rational& d) {
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << "TBD";
	return str.str();
}

// generate an ASCII rational format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const rational& d) {
	// make certain that setw and left/right operators work properly
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << d.numerator << '/' << d.denominator;
	return ostr << str.str();
}

// read an ASCII rational format from an istream
inline std::istream& operator>>(std::istream& istr, rational& p) {
	std::string txt;
	istr >> txt;
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into a rational value\n";
	}
	return istr;
}

/// rational binary arithmetic operators

// binary addition of rational numbers
inline rational operator+(const rational& lhs, const rational& rhs) {
	rational sum = lhs;
	sum += rhs;
	return sum;
}
// binary subtraction of rational numbers
inline rational operator-(const rational& lhs, const rational& rhs) {
	rational diff = lhs;
	diff -= rhs;
	return diff;
}
// binary mulitplication of rational numbers
inline rational operator*(const rational& lhs, const rational& rhs) {
	rational mul = lhs;
	mul *= rhs;
	return mul;
}
// binary division of rational numbers
inline rational operator/(const rational& lhs, const rational& rhs) {
	rational ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////
/// logic operators

/// rational - rational logic operators

// equality test
bool operator==(const rational& lhs, const rational& rhs) {
	return lhs.numerator == rhs.numerator && lhs.denominator == rhs.denominator;
}
// inequality test
bool operator!=(const rational& lhs, const rational& rhs) {
	return !operator==(lhs, rhs);
}
// less-than test
bool operator<(const rational& lhs, const rational& rhs) {
	// a/b < c/d  => ad / bd < cb / bd => ad < cb
	decimal ad = lhs.numerator * rhs.denominator;
	decimal bc = lhs.denominator * rhs.numerator;
	return ad < bc;
}
// greater-than test
bool operator>(const rational& lhs, const rational& rhs) {
	return operator<(rhs, lhs);
}
// less-or-equal test
bool operator<=(const rational& lhs, const rational& rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}
// greater-or-equal test
bool operator>=(const rational& lhs, const rational& rhs) {
	return !operator<(lhs, rhs);
}

// rational - long logic operators
inline bool operator==(const rational& lhs, long rhs) {
	return lhs == rational(rhs);
}
inline bool operator!=(const rational& lhs, long rhs) {
	return !operator==(lhs, rational(rhs));
}
inline bool operator< (const rational& lhs, long rhs) {
	return operator<(lhs, rational(rhs));
}
inline bool operator> (const rational& lhs, long rhs) {
	return operator< (rational(rhs), lhs);
}
inline bool operator<=(const rational& lhs, long rhs) {
	return operator< (lhs, rational(rhs)) || operator==(lhs, rational(rhs));
}
inline bool operator>=(const rational& lhs, long rhs) {
	return !operator<(lhs, rational(rhs));
}

// long - rational logic operators
inline bool operator==(long lhs, const rational& rhs) {
	return rational(lhs) == rhs;
}
inline bool operator!=(long lhs, const rational& rhs) {
	return !operator==(rational(lhs), rhs);
}
inline bool operator< (long lhs, const rational& rhs) {
	return operator<(rational(lhs), rhs);
}
inline bool operator> (long lhs, const rational& rhs) {
	return operator< (rational(lhs), rhs);
}
inline bool operator<=(long lhs, const rational& rhs) {
	return operator< (rational(lhs), rhs) || operator==(rational(lhs), rhs);
}
inline bool operator>=(long lhs, const rational& rhs) {
	return !operator<(rational(lhs), rhs);
}

///////////////////////////////////////////////////////////////////////


// find largest multiplier of rhs being less or equal to lhs by subtraction; assumes 0*rhs <= lhs <= 9*rhs 
rational findLargestMultiple_(const rational& lhs, const rational& rhs) {
	rational multiplier;

	return multiplier;
}


///////////////////////
// decintdiv_t for rational to capture quotient and remainder during long division
struct rationalintdiv {
	rational quot; // quotient
	rational rem;  // remainder
};

// divide integer rational a and b and return result argument
rationalintdiv rational_divide(const rational& lhs, const rational& rhs) {
	if (rhs.iszero()) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
		throw rational_divide_by_zero{};
#else
		std::cerr << "rational_divide_by_zero\n";
#endif // RATIONAL_THROW_ARITHMETIC_EXCEPTION
	}

	// a/b / c/d => ad / bc
	rationalintdiv divresult;
	divresult.quot.setnumerator(lhs.top() * rhs.bottom());
	divresult.quot.setdenominator(lhs.bottom() * rhs.top());
	return divresult;
}

// return quotient of a rational integer division
rational quotient(const rational& _a, const rational& _b) {
	return rational_divide(_a, _b).quot;
}
// return remainder of a rational integer division
rational remainder(const rational& _a, const rational& _b) {
	return rational_divide(_a, _b).rem;
}

}} // namespace sw::universal

