#pragma once
// decimal.hpp: a streamlined decimal representation to transform binary formats into decimal representations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>
#include <cassert>

namespace sw { namespace universal { namespace support {

// paired down implementation of a decimal type to generate decimal representations for fixpnt<nbits,rbits> types


// forward reference
class decimal;
void add(decimal& lhs, const decimal& rhs);
void sub(decimal& lhs, const decimal& rhs);

// decimal representation
class decimal : public std::vector<uint8_t> {
public:
	decimal() { setzero(); }
	decimal(const decimal&) = default;
	decimal(decimal&&) = default;
	decimal& operator=(const decimal&) = default;
	decimal& operator=(decimal&&) = default;

	inline bool sign()   const { return _sign; }
	inline bool iszero() const { return (size() == 1 && at(0) == 0) ? true : false; }
	inline bool ispos()  const { return (!iszero() && _sign == false) ? true : false;  }
	inline bool isneg()  const { return (!iszero() && _sign == true) ? true : false; }
	inline void setzero() {
		clear();
		push_back(0);
		_sign = false;
	}
	inline void setpos() { _sign = false; }
	inline void setneg() { _sign = true; }
	inline void setsign(bool sign) { _sign = sign; }
	inline void setdigit(const uint8_t d, bool negative = false) {
		clear();
		push_back(d);
		_sign = negative;
	}
	inline void setvalue(long long v) {
		setzero();
		uint64_t absValue = (v < 0) ? static_cast<uint64_t>(-v) : static_cast<uint64_t>(v);

		uint64_t mask = 1ull;
		support::decimal multiplier;
		multiplier.setdigit(1);
		for (size_t i = 0; i < 64; ++i) {
			if (absValue & mask) {
				add(*this, multiplier);
			}
			add(multiplier, multiplier);
			mask <<= 1;
		}
		_sign = (v < 0) ? true : false;
	}

	// remove any leading zeros from a decimal representation
	void unpad() {
		int n = (int)size();
		for (int i = n - 1; i > 0; --i) {
			if (operator[](size_t(i)) == 0) {
				pop_back();
			}
			else {
				return; // found the most significant digit
			}
		}
	}

	void shiftLeft(size_t orders) {
		for (size_t i = 0; i < orders; ++i) {
			this->insert(this->begin(), 0);
		}
	}
	void shiftRight(size_t orders) {
		if (size() <= orders) {
			this->setzero();
		}
		else {
			for (size_t i = 0; i < orders; ++i) {
				this->erase(this->begin());
			}
		}
	}

	// in-place power of 2 function 2^exponent, returns result
	void powerOf2(const size_t exponent) {
		clear();
		setdigit(1);
		for (size_t i = 0; i < exponent; ++i) {
			support::add(*this, *this);
		}
	}
private:
	bool _sign;
	friend std::ostream& operator<<(std::ostream& ostr, const decimal& d);
};


// find largest multiplier
inline decimal findLargestMultiple(const decimal& lhs, const decimal& rhs) {
	// check argument assumption	assert(0 <= lhs && lhs >= 9 * rhs);
	decimal one, remainder, multiplier;
	one.setdigit(1);
	remainder = lhs; remainder.setpos();
	multiplier.setdigit(0);
	for (int i = 0; i <= 11; ++i) {  // function works for 9 into 99, just as an aside
		if (remainder.ispos() && !remainder.iszero()) {  // test for proper > 0
			sub(remainder, rhs); //  remainder -= rhs;
			add(multiplier, one);  // ++multiplier
		}
		else {
			if (remainder.isneg()) {  // we went too far
				sub(multiplier, one); // --multiplier
			}
			// else implies remainder is 0										
			break;
		}
	}
	return multiplier;
}

// find the order of the most significant digit, precondition decimal is unpadded
inline int findMsd(const decimal& v) {
	int msd = int(v.size()) - 1;
	if (msd == 0 && v.iszero()) return -1; // no significant digit found, all digits are zero
	//assert(v.at(msd) != 0); // indicates the decimal wasn't unpadded
	return msd;
}

inline bool less(const decimal& lhs, const decimal& rhs) {
	// this logic assumes that there is no padding in the operands
	size_t l = lhs.size();
	size_t r = rhs.size();
	if (l < r) return true;
	if (l > r) return false;
	// numbers are the same size, need to compare magnitude
	decimal::const_reverse_iterator ritl = lhs.rbegin();
	decimal::const_reverse_iterator ritr = rhs.rbegin();
	for (; ritl != lhs.rend() || ritr != rhs.rend(); ++ritl, ++ritr) {
		if (*ritl < *ritr) return true;
		if (*ritl > *ritr) return false;
		// if the digits are equal we need to check the next set
	}
	// at this point we know the two operands are the same
	return false;
}

inline bool lessOrEqual(const decimal& lhs, const decimal& rhs) {
	// this logic assumes that there is no padding in the operands
	size_t l = lhs.size();
	size_t r = rhs.size();
	if (l < r) return true;
	if (l > r) return false;
	// numbers are the same size, need to compare magnitude
	decimal::const_reverse_iterator ritl = lhs.rbegin();
	decimal::const_reverse_iterator ritr = rhs.rbegin();
	for (; ritl != lhs.rend() || ritr != rhs.rend(); ++ritl, ++ritr) {
		if (*ritl < *ritr) return true;
		if (*ritl > *ritr) return false;
		// if the digits are equal we need to check the next set
	}
	// at this point we know the two operands are the same
	return true;
}

// in-place addition (equivalent to lhs += rhs)
inline void add(decimal& lhs, const decimal& rhs) {
	decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
	if (lhs.sign() != rhs.sign()) {  // different signs
		_rhs.setsign(!rhs.sign());
		sub(lhs, _rhs);
	}
	else {
		// same sign implies this->negative is invariant
	}
	size_t l = lhs.size();
	size_t r = _rhs.size();
	// zero pad the shorter decimal
	if (l < r) {
		lhs.insert(lhs.end(), r - l, 0);
	}
	else {
		_rhs.insert(_rhs.end(), l - r, 0);
	}
	decimal::iterator lit = lhs.begin();
	decimal::iterator rit = _rhs.begin();
	char carry = 0;
	for (; lit != lhs.end() || rit != _rhs.end(); ++lit, ++rit) {
		*lit += *rit + carry;
		if (*lit > 9) {
			carry = 1;
			*lit -= 10;
		}
		else {
			carry = 0;
		}
	}
	if (carry) lhs.push_back(1);
}
// in-place subtraction (equivalent to lhs -= rhs)
inline void sub(decimal& lhs, const decimal& rhs) {
	decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
	bool sign = lhs.sign();
	if (lhs.sign() != rhs.sign()) {
		_rhs.setsign(!rhs.sign());
		add(lhs, _rhs);
		return;
	}
	// largest value must be subtracted from
	size_t l = lhs.size();
	size_t r = _rhs.size();
	// zero pad the shorter decimal
	if (l < r) {
		lhs.insert(lhs.end(), r - l, 0);
		std::swap(lhs, _rhs);
		sign = !sign;
	}
	else if (r < l) {
		_rhs.insert(_rhs.end(), l - r, 0);
	}
	else {
		// the operands are the same size, thus we need to check their magnitude
		lhs.setpos();
		_rhs.setpos();
		if (less(lhs, _rhs)) {
			std::swap(lhs, _rhs);
			sign = !sign;
		}
	}
	decimal::iterator lit = lhs.begin();
	decimal::iterator rit = _rhs.begin();
	uint8_t borrow = 0;
	for (; lit != lhs.end() || rit != _rhs.end(); ++lit, ++rit) {
		if (*rit > *lit - borrow) {
			*lit = uint8_t(uint8_t(10) + *lit - borrow - *rit);
			borrow = 1;
		}
		else {
			*lit = uint8_t(*lit - borrow - *rit);
			borrow = 0;
		}
	}
	if (borrow) std::cout << "can this happen?" << std::endl;
	lhs.unpad();
	if (lhs.iszero()) { // special case of zero having positive sign
		lhs.setpos();
	}
	else {
		lhs.setsign(sign);
	}
}
// in-place multiplication (equivalent to lhs *= rhs)
inline void mul(decimal& lhs, const decimal& rhs) {
	// special case
	if (lhs.iszero() || rhs.iszero()) {
		lhs.setzero();
		return;
	}
	bool signOfFinalResult = (lhs.sign() != rhs.sign()) ? true : false;
	decimal product;
	// find the smallest decimal to minimize the amount of work
	size_t l = lhs.size();
	size_t r = rhs.size();
	decimal::const_iterator sit, bit; // sit = smallest iterator, bit = biggest iterator
	if (l < r) {
		size_t position = 0;
		for (sit = lhs.begin(); sit != lhs.end(); ++sit) {
			decimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
			partial_sum.insert(partial_sum.end(), r + position, 0);
			decimal::iterator pit = partial_sum.begin() + static_cast<const int64_t>(position);
			uint8_t carry = 0;
			for (bit = rhs.begin(); bit != rhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
				uint8_t digit = uint8_t(*sit * *bit + carry);
				*pit = uint8_t(digit % 10);
				carry = uint8_t(digit / 10);
			}
			if (carry) partial_sum.push_back(carry);
			add(product, partial_sum);
			//				std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
			++position;
		}
	}
	else {
		size_t position = 0;
		for (sit = rhs.begin(); sit != rhs.end(); ++sit) {
			decimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
			partial_sum.insert(partial_sum.end(), l + position, 0);
			decimal::iterator pit = partial_sum.begin() + static_cast<const int64_t>(position);
			uint8_t carry = 0;
			for (bit = lhs.begin(); bit != lhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
				uint8_t digit = uint8_t(*sit * *bit + carry);
				*pit = uint8_t(digit % 10);
				carry = uint8_t(digit / 10);
			}
			if (carry) partial_sum.push_back(carry);
			add(product, partial_sum);
			//				std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
			++position;
		}
	}
	product.unpad();
	lhs = product;
	lhs.setsign(signOfFinalResult);
}
// integer division of lhs / rhs, returns new decimal
inline decimal div(const decimal& lhs, const decimal& rhs) {
	if (rhs.iszero()) {
		throw "Divide by 0";
	}
	// generate the absolute values to do long division 
	bool a_negative = lhs.sign();
	bool b_negative = rhs.sign();
	bool result_negative = (a_negative ^ b_negative);
	decimal a(lhs); a.setpos();
	decimal b(rhs); b.setpos();
	decimal quotient; // zero
	if (less(a, b)) {
		return quotient; // a / b = 0 when b > a
	}
	// initialize the long division
	decimal accumulator = a;
	// prepare the subtractand
	decimal subtractand = b;
	int msd_b = findMsd(b);
	int msd_a = findMsd(a);
	int shift = msd_a - msd_b; // precondition is a >= b, shift >= 0
	subtractand.shiftLeft(size_t(shift));
	// long division
	for (int i = shift; i >= 0; --i) {
		if (lessOrEqual(subtractand, accumulator)) {
			decimal multiple = findLargestMultiple(accumulator, subtractand);
			// std::cout << accumulator << " / " << subtractand << " = " << multiple << std::endl;
			// accumulator -= multiple * subtractand;
			decimal partial;
			partial = subtractand;
			mul(partial, multiple);
			sub(accumulator, partial);
			uint8_t multiplier = multiple[0];
			quotient.insert(quotient.begin(), multiplier);
		}
		else {
			quotient.insert(quotient.begin(), 0);
		}
		subtractand.shiftRight(1);
		if (subtractand.iszero()) break;
	}
	if (result_negative) {
		quotient.setneg();
	}
	quotient.unpad();
	return quotient;
}

// convert a native long long to a decimal representation
inline void convert_to_decimal(long long v, decimal& d) {
	using namespace std;
	d.setzero();
	bool sign = false;
	if (v == 0) return;
	if (v < 0) {
		sign = true; // negative number
		// transform to sign-magnitude on positive side
		v *= -1;
	}
	uint64_t mask = 0x1;
	// IMPORTANT: can't use initializer or assignment operator as it would yield 
	// an infinite loop calling convert_to_decimal. So we need to construct the
	// decimal from first principals
	decimal base; // can't use base(1) semantics here as it would cause an infinite loop
	base.setdigit(1);
	while (v) { // minimum loop iterations; exits when no bits left
		if (v & mask) {
			add(d, base);
		}
		add(base, base);
		v >>= 1;
	}
	// finally set the sign
	d.setsign(sign);
}

// generate an ASCII decimal format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const decimal& d) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the fixpnt into a string
	std::stringstream ss;

	//std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	if (d.sign()) ss << '-';
	for (decimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ostr << ss.str();
}

}}} // namespace sw::universal::support
