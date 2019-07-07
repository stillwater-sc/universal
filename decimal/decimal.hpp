#pragma once
// decimal.hpp: definition of arbitrary decimal integer configurations
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>
#include <regex>
#include <algorithm>

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw {
namespace unum {

	// Forward references
class decimal;

template<typename Ty>
void convert_to_decimal(Ty v, decimal& d) {
	using namespace std;
	//cout << numeric_limits<Ty>::digits << " max value " << numeric_limits<Ty>::max() << endl;
	d.setzero();
	bool sign = false;
	if (v == 0) return;
	if (numeric_limits<Ty>::is_signed) {
		if (v < 0) {
			sign = true; // negative number
			// transform to sign-magnitude on positive side
			v *= -1;
		}
	}
	int msb = numeric_limits<Ty>::digits;
	uint64_t mask = 0x1;
	// can't use assignment operator as it would yield an infinite loop calling convert
	d.push_back(0); // initialize the decimal value to 0
	decimal base;
	base.push_back(1); // set to the value of 1, and double it each iteration
	while (v) {
		if (v & mask) {
			d += base;
		}
		base += base;
		v >>= 1;
	}
	// finally set the sign
	d.setsign(sign);
}

std::string& ltrim(std::string& s)
{
	auto it = std::find_if(s.begin(), s.end(),
		[](char c) {
		return !std::isspace<char>(c, std::locale::classic());
	});
	s.erase(s.begin(), it);
	return s;
}

std::string& rtrim(std::string& s)
{
	auto it = std::find_if(s.rbegin(), s.rend(),
		[](char c) {
		return !std::isspace<char>(c, std::locale::classic());
	});
	s.erase(it.base(), s.end());
	return s;
}

std::string& trim(std::string& s)
{
	return ltrim(rtrim(s));
}

// Arbitrary precision decimal number
class decimal : public std::vector<char> {
public:
	decimal() { setzero(); }

	decimal(const decimal&) = default;
	decimal(decimal&&) = default;

	decimal& operator=(const decimal&) = default;
	decimal& operator=(decimal&&) = default;

	// initializers for native types
	decimal(const char initial_value) { *this = initial_value; }
	decimal(const short initial_value) { *this = initial_value; }
	decimal(const int initial_value) { *this = initial_value; }
	decimal(const long initial_value) { *this = initial_value; }
	decimal(const long long initial_value) { *this = initial_value; }
	decimal(const unsigned char initial_value) { *this = initial_value; }
	decimal(const unsigned short initial_value) { *this = initial_value; }
	decimal(const unsigned int initial_value) { *this = initial_value; }
	decimal(const unsigned long initial_value) { *this = initial_value; }
	decimal(const unsigned long long initial_value) { *this = initial_value; }
	decimal(const float initial_value) { *this = initial_value; }
	decimal(const double initial_value) { *this = initial_value; }
	decimal(const long double initial_value) { *this = initial_value; }

	// assignment operators for native types
	decimal& operator=(const std::string& digits) {
		parse(digits);
		return *this;
	}
	decimal& operator=(const char rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const short rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const int rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const long rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const long long rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned char rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			push_back(0);
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(const float rhs) {
		return float_assign(rhs);
	}
	decimal& operator=(const double rhs) {
		return float_assign(rhs);
	}
	decimal& operator=(const long double rhs) {
		return float_assign(rhs);
	}

	// arithmetic operators
	decimal& operator+=(const decimal& d) {
		if (negative != d.negative) {  // different signs
			decimal _d(d);
			_d.setsign(!d.sign());
			return operator-=(_d);
		}
		else {
			// same sign implies this->negative is invariant
		}
		size_t l = size();
		size_t r = d.size();
		decimal _d(d);
		// zero pad the shorter decimal
		if (l < r) {
			insert(end(), r-l, 0);
		}
		else {
			_d.insert(_d.end(), l - r, 0);
		}
		decimal::iterator lit = begin();
		decimal::iterator rit = _d.begin();
		char carry = 0;
		for (; lit != end() || rit != _d.end(); ++lit, ++rit) {
			*lit += *rit + carry;
			if (*lit > 9) {
				carry = 1;
				*lit -= 10;
			} 
			else {
				carry = 0;
			}
		}
		if (carry) push_back(1);
		return *this;
	}
	decimal& operator-=(const decimal& d) {
		if (negative != d.negative) {
			decimal _d(d);
			_d.setsign(!d.sign());
			return operator+=(_d);
		}
		// largest value will be subtracted from
		size_t l = size();
		size_t r = d.size();
		decimal _d(d);
		// zero pad the shorter decimal
		if (l < r) {
			insert(end(), r - l, 0);
			std::swap(*this, _d);
		}
		else if (r < l) {
			_d.insert(_d.end(), l - r, 0);
		}
		else {
			// the are the same size, so need to check their magnitude
			if (*this < _d) std::swap(*this, _d);
		}
		decimal::iterator lit = begin();
		decimal::iterator rit = _d.begin();
		char borrow = 0;
		for (; lit != end() || rit != _d.end(); ++lit, ++rit) {			
			if (*rit > *lit - borrow) {
				*lit = 10 + *lit - borrow - *rit;
				borrow = 1;
			}
			else {
				*lit = *lit - borrow - *rit;
				borrow = 0;
			}
		}
		if (borrow) std::cout << "can this happen" << std::endl;
		return *this;
	}
	decimal& operator*=(const decimal& d) {
		return *this;
	}
	decimal& operator/=(const decimal& d) {
		return *this;
	}
	// selectors
	inline bool iszero() const {
		if (size() == 0) return true;
		return std::all_of(begin(), end(), [](uint8_t i) { return 0 == i; });
	}
	inline bool sign() const { return negative; }
	inline bool isneg() const { return negative; }
	inline bool ispos() const { return !negative; }

	// modifiers
	inline void setzero() { clear(); negative = false; }
	inline void setsign(bool sign) { negative = sign; }
	inline void setneg() { negative = true; }
	inline void setpos() { negative = false; }

	// read a decimal ASCII format and make a decimal type out of it
	bool parse(std::string digits) {
		bool bSuccess = false;
		trim(digits);
		// check if the txt is an decimal form:[+-]*[0123456789]+
		std::regex decimal_regex("[+-]*[0123456789]+");
		if (std::regex_match(digits, decimal_regex)) {
			// found a decimal representation
			setzero();
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
				push_back(v);
			}
			std::reverse(begin(), end());
			bSuccess = true;
		}
		return bSuccess;
	}

protected:

	template<typename Ty>
	decimal& float_assign(Ty& rhs) {
		return *this;
	}

private:
	// sign-magnitude number: indicate if number is positive or negative
	bool negative;

	friend std::ostream& operator<<(std::ostream& ostr, const decimal& d);
	friend std::istream& operator>>(std::istream& istr, decimal& d);

	// decimal - decimal logic operators
	friend bool operator==(const decimal& lhs, const decimal& rhs);
	friend bool operator!=(const decimal& lhs, const decimal& rhs);
	friend bool operator<(const decimal& lhs, const decimal& rhs);
	friend bool operator>(const decimal& lhs, const decimal& rhs);
	friend bool operator<=(const decimal& lhs, const decimal& rhs);
	friend bool operator>=(const decimal& lhs, const decimal& rhs);
};

////////////////// DECIMAL operators

/// stream operators

// generate an integer format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const decimal& d) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	//std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	if (d.isneg()) ss << '-';
	for (decimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ostr << ss.str();
}

// read an ASCII integer format
inline std::istream& operator>> (std::istream& istr, decimal& p) {
	std::string txt;
	istr >> txt;
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into a decimal value\n";
	}
	return istr;
}

/// decimal binary arithmetic operators
// BINARY ADDITION
inline decimal operator+(const decimal& lhs, const decimal& rhs) {
	decimal sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
inline decimal operator-(const decimal& lhs, const decimal& rhs) {
	decimal diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
inline decimal operator*(const decimal& lhs, const decimal& rhs) {
	decimal mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
inline decimal operator/(const decimal& lhs, const decimal& rhs) {
	decimal ratio = lhs;
	ratio /= rhs;
	return ratio;
}

/// logic operators

	// decimal - decimal logic operators
bool operator==(const decimal& lhs, const decimal& rhs) {
	bool areEqual = std::equal(lhs.begin(), lhs.end(), rhs.begin());
	return areEqual;
}
bool operator!=(const decimal& lhs, const decimal& rhs) {
	return !operator==(lhs, rhs);
}
bool operator<(const decimal& lhs, const decimal& rhs) {
	size_t l = lhs.size();
	size_t r = rhs.size();
	if (l < r) return true;
	if (l > r) return false;
	// numbers are the same size
	decimal::const_reverse_iterator ritl = lhs.rbegin();
	decimal::const_reverse_iterator ritr = rhs.rbegin();
	for (; ritl != lhs.rend() || ritr != rhs.rend(); ++ritl, ++ritr) {
		if (*ritl > *ritr) return false;
	}
	if (lhs == rhs) return false;
	return true;
}
bool operator>(const decimal& lhs, const decimal& rhs) {
	return operator<(rhs, lhs);
}
bool operator<=(const decimal& lhs, const decimal& rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}
bool operator>=(const decimal& lhs, const decimal& rhs) {
	return !operator<(lhs, rhs);
}

// decimal - long logic operators
inline bool operator==(const decimal& lhs, long rhs) {
	return lhs == decimal(rhs);
}
inline bool operator!=(const decimal& lhs, long rhs) {
	return !operator==(lhs, decimal(rhs));
}
inline bool operator< (const decimal& lhs, long rhs) {
	return false;
}
inline bool operator> (const decimal& lhs, long rhs) {
	return operator< (decimal(rhs), lhs);
}
inline bool operator<=(decimal& lhs, long rhs) {
	return operator< (lhs, decimal(rhs)) || operator==(lhs, decimal(rhs));
}
inline bool operator>=(const decimal& lhs, long rhs) {
	return !operator<(lhs, decimal(rhs));
}

// long - decimal logic operators
inline bool operator==(long lhs, const decimal& rhs) {
	return decimal(lhs) == rhs;
}
inline bool operator!=(long lhs, const decimal& rhs) {
	return !operator==(decimal(lhs), rhs);
}
inline bool operator< (long lhs, const decimal& rhs) {
	return false;
}
inline bool operator> (long lhs, const decimal& rhs) {
	return operator< (decimal(lhs), rhs);
}
inline bool operator<=(long lhs, const decimal& rhs) {
	return operator< (decimal(lhs), rhs) || operator==(decimal(lhs), rhs);
}
inline bool operator>=(long lhs, const decimal& rhs) {
	return !operator<(decimal(lhs), rhs);
}
} // namespace unum
} // namespace sw
