#pragma once
// decimal.hpp: definition of arbitrary decimal integer configurations
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
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

#include <universal/native/ieee-754.hpp>
#include <universal/string/strmanip.hpp>
#include "./decimal_exceptions.hpp"

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

namespace sw { namespace unum {

/////////////////////////////////////////////
// Forward references
class decimal;
struct decintdiv;
decimal quotient(const decimal&, const decimal&);
decimal remainder(const decimal&, const decimal&);
int findMsd(const decimal&);
template<typename Ty> void convert_to_decimal(Ty, decimal&);

// Arbitrary precision decimal integer number
class decimal : public std::vector<uint8_t> {
public:
	decimal() { setzero(); }

	decimal(const decimal&) = default;
	decimal(decimal&&) = default;

	decimal& operator=(const decimal&) = default;
	decimal& operator=(decimal&&) = default;

	// initializers for native types
	decimal(char initial_value) { *this = initial_value; }
	decimal(short initial_value) { *this = initial_value; }
	decimal(int initial_value) { *this = initial_value; }
	decimal(long initial_value) { *this = initial_value; }
	decimal(long long initial_value) { *this = initial_value; }
	decimal(unsigned char initial_value) { *this = initial_value; }
	decimal(unsigned short initial_value) { *this = initial_value; }
	decimal(unsigned int initial_value) { *this = initial_value; }
	decimal(unsigned long initial_value) { *this = initial_value; }
	decimal(unsigned long long initial_value) { *this = initial_value; }
	decimal(float initial_value) { *this = initial_value; }
	decimal(double initial_value) { *this = initial_value; }
	decimal(long double initial_value) { *this = initial_value; }

	// assignment operators for native types
	decimal& operator=(const std::string& digits) {
		parse(digits);
		return *this;
	}
	decimal& operator=(char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(unsigned char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_to_decimal(rhs, *this);
		}
		return *this;
	}
	decimal& operator=(float rhs) {
		return float_assign(rhs);
	}
	constexpr decimal& operator=(double rhs) {
		return float_assign(rhs);
	}
	decimal& operator=(long double rhs) {
		return float_assign(rhs);
	}

	// arithmetic operators
	decimal& operator+=(const decimal& rhs) {
		decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
		if (negative != rhs.negative) {  // different signs
			_rhs.setsign(!rhs.sign());
			return operator-=(_rhs);
		}
		else {
			// same sign implies this->negative is invariant
		}
		size_t l = size();
		size_t r = _rhs.size();
		// zero pad the shorter decimal
		if (l < r) {
			insert(end(), r - l, 0);
		}
		else {
			_rhs.insert(_rhs.end(), l - r, 0);
		}
		decimal::iterator lit = begin();
		decimal::iterator rit = _rhs.begin();
		uint8_t carry = 0;
		for (; lit != end() || rit != _rhs.end(); ++lit, ++rit) {
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
	decimal& operator-=(const decimal& rhs) {
		decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
		bool sign = this->sign();
		if (negative != rhs.negative) {
			_rhs.setsign(!rhs.sign());
			return operator+=(_rhs);
		}
		// largest value must be subtracted from
		size_t l = size();
		size_t r = _rhs.size();
		// zero pad the shorter decimal
		if (l < r) {
			insert(end(), r - l, 0);
			std::swap(*this, _rhs);
			sign = !sign;
		}
		else if (r < l) {
			_rhs.insert(_rhs.end(), l - r, 0);
		}
		else {
			// the operands are the same size, thus we need to check their magnitude
			this->setpos();
			_rhs.setpos();
			if (*this < _rhs) {
				std::swap(*this, _rhs);
				sign = !sign;
			}
		}
		decimal::iterator lit = begin();
		decimal::iterator rit = _rhs.begin();
		uint8_t borrow = 0;
		for (; lit != end() || rit != _rhs.end(); ++lit, ++rit) {
			if (*rit > *lit - borrow) {
				*lit = 10 + *lit - borrow - *rit;
				borrow = 1;
			}
			else {
				*lit = *lit - borrow - *rit;
				borrow = 0;
			}
		}
		if (borrow) std::cout << "can this happen?" << std::endl;
		unpad();
		if (this->iszero()) { // special case of zero having positive sign
			this->setpos();
		}
		else {
			this->setsign(sign);
		}
		return *this;
	}
	decimal& operator*=(const decimal& rhs) {
		// special case
		if (iszero() || rhs.iszero()) {
			setzero();
			return *this;
		}
		bool signOfFinalResult = (negative != rhs.negative) ? true : false;
		decimal product;
		// find the smallest decimal to minimize the amount of work
		size_t l = size();
		size_t r = rhs.size();
		decimal::const_iterator sit, bit; // sit = smallest iterator, bit = biggest iterator
		if (l < r) {
			size_t position = 0;
			for (sit = begin(); sit != end(); ++sit) {
				decimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
				partial_sum.insert(partial_sum.end(), r + position, 0);
				decimal::iterator pit = partial_sum.begin() + position;
				uint8_t carry = 0;
				for (bit = rhs.begin(); bit != rhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
					uint8_t digit = *sit * *bit + carry;
					*pit = digit % 10;
					carry = digit / 10;
				}
				if (carry) partial_sum.push_back(carry);
				product += partial_sum;
				//				std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
				++position;
			}
		}
		else {
			size_t position = 0;
			for (sit = rhs.begin(); sit != rhs.end(); ++sit) {
				decimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
				partial_sum.insert(partial_sum.end(), l + position, 0);
				decimal::iterator pit = partial_sum.begin() + position;
				uint8_t carry = 0;
				for (bit = begin(); bit != end() && pit != partial_sum.end(); ++bit, ++pit) {
					uint8_t digit = *sit * *bit + carry;
					*pit = digit % 10;
					carry = digit / 10;
				}
				if (carry) partial_sum.push_back(carry);
				product += partial_sum;
				//				std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
				++position;
			}
		}
		product.unpad();
		*this = product;
		setsign(signOfFinalResult);
		return *this;
	}
	decimal& operator/=(const decimal& rhs) {
		*this = quotient(*this, rhs);
		return *this;
	}
	decimal& operator%=(const decimal& rhs) {
		*this = remainder(*this, rhs);
		return *this;
	}
	decimal& operator<<=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) {
			return operator>>=(-shift);
		}
		for (int i = 0; i < shift; ++i) {
			this->insert(this->begin(), 0);
		}
		return *this;
	}
	decimal& operator>>=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) {
			return operator<<=(-shift);
		}
		if (signed(size()) <= shift) {
			this->setzero();
		}
		else {
			for (int i = 0; i < shift; ++i) {
				this->erase(this->begin());
			}
		}
		return *this;
	}

	// unitary operators
	decimal operator-() {
		decimal tmp(*this);
		tmp.setsign(!tmp.sign());
		return tmp;
	}
	decimal operator++() {
		decimal tmp(*this);
		decimal plusOne;
		plusOne.setdigit(1);
		*this += plusOne;
		return tmp;
	}
	decimal& operator++(int d) {
		decimal plusOne;
		plusOne.setdigit(1);
		*this += plusOne;
		return *this;
	}
	decimal operator--() {
		decimal tmp(*this);
		decimal plusOne;
		plusOne.setdigit(1);
		*this -= plusOne;
		return tmp;
	}
	decimal& operator--(int d) {
		decimal plusOne;
		plusOne.setdigit(1);
		*this -= plusOne;
		return *this;
	}

	// conversion operators: Maybe remove explicit, MTL compiles, but we have lots of double computation then
	explicit operator unsigned short() const { return to_ushort(); }
	explicit operator unsigned int() const { return to_uint(); }
	explicit operator unsigned long() const { return to_ulong(); }
	explicit operator unsigned long long() const { return to_ulong_long(); }
	explicit operator short() const { return to_short(); }
	explicit operator int() const { return to_int(); }
	explicit operator long() const { return to_long(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator float() const { return to_float(); }
	explicit operator double() const { return to_double(); }
	explicit operator long double() const { return to_long_double(); }

	// selectors
	inline bool iszero() const {
		if (size() == 0) return true;
		return std::all_of(begin(), end(), [](uint8_t i) { return 0 == i; });
	}
	inline bool sign() const { return negative; }
	inline bool isneg() const { return negative; }   // <  0
	inline bool ispos() const { return !negative; }  // >= 0

	// modifiers
	inline void setzero() { clear(); push_back(0); negative = false; }
	inline void setsign(bool sign) { negative = sign; }
	inline void setneg() { negative = true; }
	inline void setpos() { negative = false; }
	inline void setdigit(uint8_t d, bool sign = false) {
		assert(d <= 9); // test argument assumption
		clear();
		push_back(d);
		negative = sign;
	}

	// remove any leading zeros from a decimal representation
	void unpad() {
		int n = (int)size();
		for (int i = n - 1; i > 0; --i) {
			if (operator[](i) == 0) {
				pop_back();
			}
			else {
				return;  // found the most significant digit
			}
		}
	}

	// read a decimal ASCII format and make a decimal type out of it
	bool parse(const std::string& _digits) {
		bool bSuccess = false;
		std::string digits(_digits);
		trim(digits);
		// check if the txt is an decimal form:[+-]*[0123456789]+
		std::regex decimal_regex("[+-]*[0123456789]+");
		if (std::regex_match(digits, decimal_regex)) {
			// found a decimal representation
			clear();
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
	// HELPER methods

	// conversion functions
	inline short to_short() const { return short(to_long_long()); }
	inline int to_int() const { return short(to_long_long()); }
	inline long to_long() const { return short(to_long_long()); }
	inline long long to_long_long() const {
		long long v = 0;
		decimal::const_iterator it;
		long long order;
		order = sign() ? -1 : 1;
		for (it = this->begin(); it != this->end(); ++it) {
			v += *it * order;
			order *= 10;
		}
		return v;
	}
	inline unsigned short to_ushort() const { return short(to_ulong_long()); }
	inline unsigned int to_uint() const { return short(to_ulong_long()); }
	inline unsigned long to_ulong() const { return short(to_ulong_long()); }
	inline unsigned long long to_ulong_long() const {
		return (unsigned long long)to_long_long();
	}
	inline float to_float() const {
		return 0.0f;
	}
	inline double to_double() const {
		return 0.0;
	}
	inline long double to_long_double() const {
		return 0.0l;
	}

	template<typename Ty>
	constexpr decimal& float_assign(Ty& rhs) {
		if (rhs < 0.5 && rhs > -0.5) {
			return *this = 0;
		}
		else {
			bool sign = false;
			if (rhs < 0.0) { sign = true; rhs = -rhs; }
			double_decoder decoder;
			decoder.d = rhs;
			int scale = int(decoder.parts.exponent) - 1023;
			constexpr uint64_t hidden_bit = (uint64_t(1) << 51);
			uint64_t bits = decoder.parts.fraction | hidden_bit;
			if (scale < 51) {
				bits >>= (51 - scale);
				*this = bits;
			}
			else {
				scale -= 51;
				*this = bits;
			}
		}
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

////////////////// helper functions

// find the order of the most significant digit, precondition decimal is unpadded
inline int findMsd(const decimal& v) {
	int msd = int(v.size()) - 1;
	if (msd == 0 && v == 0) return -1; // no significant digit found, all digits are zero
	assert(v.at(msd) != 0); // indicates the decimal wasn't unpadded
	return msd;
}

// Convert integer types to a decimal representation
// TODO: needs SFINAE enable_if to constrain it to native integer types
template<typename Ty>
void convert_to_decimal(Ty v, decimal& d) {
	using namespace std;
	//cout << numeric_limits<Ty>::digits << " max value " << numeric_limits<Ty>::max() << endl;
	bool sign = false;
	d.setzero(); // initialize the decimal value to 0
	if (v == 0) return;
	if (numeric_limits<Ty>::is_signed) {
		if (v < 0) {
			sign = true; // negative number
			// transform to sign-magnitude on positive side
			v *= -1;
		}
	}
	uint64_t mask = 0x1;
	// IMPORTANT: can't use initializer or assignment operator as it would yield 
	// an infinite loop calling convert_to_decimal. So we need to construct the
	// decimal from first principals
	decimal base; // can't use base(1) semantics here as it would cause an infinite loop
	base.clear();
	base.push_back(1);
	while (v) { // minimum loop iterations; exits when no bits left
		if (v & mask) {
			d += base;
		}
		base += base;
		v >>= 1;
	}
	// finally set the sign
	d.setsign(sign);
}


////////////////// DECIMAL operators

/// stream operators

// generate an ASCII decimal string
inline std::string to_string(const decimal& d) {
	std::stringstream ss;
	if (d.isneg()) ss << '-';
	for (decimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
}

// generate an ASCII decimal format and send to ostream
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

// read an ASCII decimal format from an istream
inline std::istream& operator>>(std::istream& istr, decimal& p) {
	std::string txt;
	istr >> txt;
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into a decimal value\n";
	}
	return istr;
}

/// decimal binary arithmetic operators

// binary addition of decimal numbers
inline decimal operator+(const decimal& lhs, const decimal& rhs) {
	decimal sum = lhs;
	sum += rhs;
	return sum;
}
// binary subtraction of decimal numbers
inline decimal operator-(const decimal& lhs, const decimal& rhs) {
	decimal diff = lhs;
	diff -= rhs;
	return diff;
}
// binary mulitplication of decimal numbers
inline decimal operator*(const decimal& lhs, const decimal& rhs) {
	decimal mul = lhs;
	mul *= rhs;
	return mul;
}
// binary division of decimal numbers
inline decimal operator/(const decimal& lhs, const decimal& rhs) {
	decimal ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// binary left shift
inline decimal operator<<(const decimal& lhs, int shift) {
	decimal d(lhs);
	return d <<= shift;
}
// binary right shift
inline decimal operator>>(const decimal& lhs, int shift) {
	decimal d(lhs);
	return d >>= shift;
}
/// logic operators

	// decimal - decimal logic operators
// equality test
bool operator==(const decimal& lhs, const decimal& rhs) {
	if (lhs.size() != rhs.size()) return false;
	bool areEqual = std::equal(lhs.begin(), lhs.end(), rhs.begin()) && lhs.sign() == rhs.sign();
	return areEqual;
}
// inequality test
bool operator!=(const decimal& lhs, const decimal& rhs) {
	return !operator==(lhs, rhs);
}
// less-than test
bool operator<(const decimal& lhs, const decimal& rhs) {
	if (lhs.sign() != rhs.sign()) {
		return lhs.sign() ? true : false;
	}

	// signs are the same
	// this logic assumes that there is no padding in the operands
	size_t l = lhs.size();
	size_t r = rhs.size();
	if (l < r) return lhs.sign() ? false : true;
	if (l > r) return lhs.sign() ? true : false;
	// numbers are the same size, need to compare magnitude
	decimal::const_reverse_iterator ritl = lhs.rbegin();
	decimal::const_reverse_iterator ritr = rhs.rbegin();
	for (; ritl != lhs.rend() || ritr != rhs.rend(); ++ritl, ++ritr) {
		if (*ritl < *ritr) return lhs.sign() ? false : true;
		if (*ritl > *ritr) return lhs.sign() ? true : false;
		// if the digits are equal we need to check the next set
	}
	// at this point we know the two operands are the same
	return false;
}
// greater-than test
bool operator>(const decimal& lhs, const decimal& rhs) {
	return operator<(rhs, lhs);
}
// less-or-equal test
bool operator<=(const decimal& lhs, const decimal& rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}
// greater-or-equal test
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
	return operator<(lhs, decimal(rhs));
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

///////////////////////////////////////////////////////////////////////
// 
// find largest multiplier of rhs being less or equal to lhs by subtraction; assumes 0*rhs <= lhs <= 9*rhs 
decimal findLargestMultiple(const decimal& lhs, const decimal& rhs) {
	// check argument assumption	assert(0 <= lhs && lhs >= 9 * rhs);
	decimal remainder = lhs;
	remainder.setpos();
	decimal multiplier;
	multiplier.setdigit(0);
	for (int i = 0; i <= 11; ++i) {  // function works for 9 into 99, just as an aside
		if (remainder > 0) {
			remainder -= rhs;
			++multiplier;
		}
		else {
			if (remainder < 0) {  // we went too far
				--multiplier;
			}
			// else implies remainder is 0										
			break;
		}
	}
	return multiplier;
}


///////////////////////
// decintdiv_t for decimal to capture quotient and remainder during long division
struct decintdiv {
	decimal quot; // quotient
	decimal rem;  // remainder
};

// divide integer decimal a and b and return result argument
decintdiv decint_divide(const decimal& _a, const decimal& _b) {
	if (_b == 0) {
#if DECIMAL_THROW_ARITHMETIC_EXCEPTION
		throw decimal_integer_divide_by_zero{};
#else
		std::cerr << "integer_divide_by_zero\n";
#endif // INTEGER_THROW_ARITHMETIC_EXCEPTION
	}
	// generate the absolute values to do long division 
	bool a_negative = _a.sign();
	bool b_negative = _b.sign();
	bool result_negative = (a_negative ^ b_negative);
	decimal a(_a); a.setpos();
	decimal b(_b); b.setpos();
	decintdiv divresult;
	if (a < b) {
		divresult.quot = 0;
		divresult.rem = _a; // a % b = a when a / b = 0
		return divresult; // a / b = 0 when b > a
	}
	// initialize the long division
	decimal accumulator = a;
	// prepare the subtractand
	decimal subtractand = b;
	int msd_b = findMsd(b);
	int msd_a = findMsd(a);
	int shift = msd_a - msd_b;
	subtractand <<= shift;
	// long division
	for (int i = shift; i >= 0; --i) {
		if (subtractand <= accumulator) {
			decimal multiple = findLargestMultiple(accumulator, subtractand);
			accumulator -= multiple * subtractand;
			uint8_t multiplier = int(multiple);
			divresult.quot.insert(divresult.quot.begin(), multiplier);
		}
		else {
			divresult.quot.insert(divresult.quot.begin(), 0);
		}
		subtractand >>= 1;
		if (subtractand == 0) break;
	}
	if (result_negative) {
		divresult.quot.setneg();
	}
	if (_a < 0) {
		divresult.rem = -accumulator;
	}
	else {
		divresult.rem = accumulator;
	}
	divresult.quot.unpad();
	divresult.rem.unpad();
	return divresult;
}

// return quotient of a decimal integer division
decimal quotient(const decimal& _a, const decimal& _b) {
	return decint_divide(_a, _b).quot;
}
// return remainder of a decimal integer division
decimal remainder(const decimal& _a, const decimal& _b) {
	return decint_divide(_a, _b).rem;
}

}} // namespace sw::unum

