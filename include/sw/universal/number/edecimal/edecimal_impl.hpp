#pragma once
// edecimal_impl.hpp: definition of adaptive precision decimal integer data type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// ALPHA feature: occurrence is NOT an official API for any of the Universal number systems
#if !defined(EDECIMAL_OPERATIONS_COUNT)
#define EDECIMAL_OPERATIONS_COUNT 0
#endif
#if EDECIMAL_OPERATIONS_COUNT
#include <universal/utility/occurrence.hpp>
#endif

namespace sw { namespace universal {

/// <summary>
/// Adaptive precision decimal integer number type
/// </summary>
/// The digits are managed as a vector with the digit for 10^0 stored at index 0, 10^1 stored at index 1, etc.
class edecimal : public std::vector<uint8_t> {
#if EDECIMAL_OPERATIONS_COUNT
	static bool enableAdd;
	static occurrence<edecimal> ops;
#endif
public:
	edecimal() { setzero(); }

	edecimal(const edecimal&) = default;
	edecimal(edecimal&&) = default;

	edecimal& operator=(const edecimal&) = default;
	edecimal& operator=(edecimal&&) = default;

	// initializers for native types
	edecimal(char initial_value)                { *this = initial_value; }
	edecimal(short initial_value)               { *this = initial_value; }
	edecimal(int initial_value)                 { *this = initial_value; }
	edecimal(long initial_value)                { *this = initial_value; }
	edecimal(long long initial_value)           { *this = initial_value; }
	edecimal(unsigned char initial_value)       { *this = initial_value; }
	edecimal(unsigned short initial_value)      { *this = initial_value; }
	edecimal(unsigned int initial_value)        { *this = initial_value; }
	edecimal(unsigned long initial_value)       { *this = initial_value; }
	edecimal(unsigned long long initial_value)  { *this = initial_value; }
	edecimal(float initial_value)               { *this = initial_value; }
	edecimal(double initial_value)              { *this = initial_value; }


	// assignment operators for native types
	edecimal& operator=(const std::string& digits) {
		parse(digits);
		return *this;
	}
	edecimal& operator=(char rhs)               { return convert_integer(rhs);}
	edecimal& operator=(short rhs)              { return convert_integer(rhs);}
	edecimal& operator=(int rhs)                { return convert_integer(rhs);}
	edecimal& operator=(long rhs)               { return convert_integer(rhs);}
	edecimal& operator=(long long rhs)          { return convert_integer(rhs);}
	edecimal& operator=(unsigned char rhs)      { return convert_integer(rhs);}
	edecimal& operator=(unsigned short rhs)     { return convert_integer(rhs);}
	edecimal& operator=(unsigned int rhs)       { return convert_integer(rhs);}
	edecimal& operator=(unsigned long rhs)      { return convert_integer(rhs);}
	edecimal& operator=(unsigned long long rhs) { return convert_integer(rhs);}
	edecimal& operator=(float rhs)              { return convert_ieee754(rhs);}
	edecimal& operator=(double rhs)             { return convert_ieee754(rhs);}

#if LONG_DOUBLE_SUPPORT
	edecimal(long double initial_value)         { *this = initial_value; }
	edecimal& operator=(long double rhs)        { return convert_ieee754(rhs); }
#endif

	// arithmetic operators
	edecimal& operator+=(const edecimal& rhs) {
		edecimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
		if (negative != rhs.negative) {  // different signs
			_rhs.setsign(!rhs.sign());
			return operator-=(_rhs);
		}
		else {
			// same sign implies this->negative is invariant
		}
		size_t l = size();
		size_t r = _rhs.size();
		// zero pad the shorter edecimal
		if (l < r) {
			insert(end(), r - l, 0);
		}
		else {
			_rhs.insert(_rhs.end(), l - r, 0);
		}
		edecimal::iterator lit = begin();
		edecimal::iterator rit = _rhs.begin();
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
#if EDECIMAL_OPERATIONS_COUNT
		if (enableAdd) ++ops.add;
#endif
		return *this;
	}
	edecimal& operator-=(const edecimal& rhs) {
		edecimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
		bool sign = this->sign();
		if (negative != rhs.negative) {
			_rhs.setsign(!rhs.sign());
			return operator+=(_rhs);
		}
		// largest value must be subtracted from
		size_t l = size();
		size_t r = _rhs.size();
		// zero pad the shorter edecimal
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
		edecimal::iterator lit = begin();
		edecimal::iterator rit = _rhs.begin();
		uint8_t borrow = 0;
		for (; lit != end() || rit != _rhs.end(); ++lit, ++rit) {
			if (*rit > *lit - borrow) {
				*lit = static_cast<uint8_t>(10 + *lit - borrow - *rit);
				borrow = 1;
			}
			else {
				*lit = static_cast<uint8_t>(*lit - borrow - *rit);
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
#if EDECIMAL_OPERATIONS_COUNT
		++ops.sub;
#endif
		return *this;
	}
	edecimal& operator*=(const edecimal& rhs) {
		// special case
		if (iszero() || rhs.iszero()) {
			setzero();
#if EDECIMAL_OPERATIONS_COUNT
			++ops.mul;
#endif
			return *this;
		}
		bool signOfFinalResult = (negative != rhs.negative) ? true : false;
		edecimal product;
#if EDECIMAL_OPERATIONS_COUNT
		enableAdd = false;
#endif
		// find the smallest edecimal to minimize the amount of work
		size_t l = size();
		size_t r = rhs.size();
		edecimal::const_iterator sit, bit; // sit = smallest iterator, bit = biggest iterator
		if (l < r) {
			int64_t position = 0;
			for (sit = begin(); sit != end(); ++sit) {
				edecimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
				partial_sum.insert(partial_sum.end(), r + position, 0);
				edecimal::iterator pit = partial_sum.begin() + position;
				uint8_t carry = 0;
				for (bit = rhs.begin(); bit != rhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
					uint8_t digit = static_cast<uint8_t>(*sit * *bit + carry);
					*pit = static_cast<uint8_t>(digit % 10);
					carry = static_cast<uint8_t>(digit / 10);
				}
				if (carry) partial_sum.push_back(carry);
				product += partial_sum;
				//				std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
				++position;
			}
		}
		else {
			int64_t position = 0;
			for (sit = rhs.begin(); sit != rhs.end(); ++sit) {
				edecimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
				partial_sum.insert(partial_sum.end(), l + position, 0);
				edecimal::iterator pit = partial_sum.begin() + position;
				uint8_t carry = 0;
				for (bit = begin(); bit != end() && pit != partial_sum.end(); ++bit, ++pit) {
					uint8_t digit = static_cast<uint8_t>(*sit * *bit + carry);
					*pit = static_cast<uint8_t>(digit % 10);
					carry = static_cast<uint8_t>(digit / 10);
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
#if EDECIMAL_OPERATIONS_COUNT
		enableAdd = true;
		++ops.mul;
#endif
		return *this;
	}
	edecimal& operator/=(const edecimal& rhs) {
		*this = quotient(*this, rhs);
#if EDECIMAL_OPERATIONS_COUNT
		++ops.div;
#endif
		return *this;
	}
	edecimal& operator%=(const edecimal& rhs) {
		*this = remainder(*this, rhs);
#if EDECIMAL_OPERATIONS_COUNT
		++ops.rem;
#endif
		return *this;
	}
	edecimal& operator<<=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) {
			return operator>>=(-shift);
		}
		for (int i = 0; i < shift; ++i) {
			this->insert(this->begin(), 0);
		}
		return *this;
	}
	edecimal& operator>>=(int shift) {
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
	edecimal operator-() const {
		edecimal tmp(*this);
		tmp.setsign(!tmp.sign());
		return tmp;
	}
	edecimal operator++(int) { // postfix
		edecimal tmp(*this);
		edecimal one;
		one.setdigit(1);
		*this += one;
		return tmp;
	}
	edecimal& operator++() { // prefix
		edecimal one;
		one.setdigit(1);
		*this += one;
		return *this;
	}
	edecimal operator--(int) { // postfix
		edecimal tmp(*this);
		edecimal one;
		one.setdigit(1);
		*this -= one;
		return tmp;
	}
	edecimal& operator--() { // prefix
		edecimal one;
		one.setdigit(1);
		*this -= one;
		return *this;
	}

	// conversion operators: Maybe remove explicit, MTL compiles, but we have lots of double computation then
	explicit operator unsigned short()     const noexcept { return to_ushort(); }
	explicit operator unsigned int()       const noexcept { return to_uint(); }
	explicit operator unsigned long()      const noexcept { return to_ulong(); }
	explicit operator unsigned long long() const noexcept { return to_ulong_long(); }
	explicit operator short()              const noexcept { return to_short(); }
	explicit operator int()                const noexcept { return to_int(); }
	explicit operator long()               const noexcept { return to_long(); }
	explicit operator long long()          const noexcept { return to_long_long(); }
	explicit operator float()              const noexcept { return to_float(); }
	explicit operator double()             const noexcept { return to_double(); }
	explicit operator long double()        const noexcept { return to_long_double(); }

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
	inline void setbits(uint64_t v) { *this = v; } // API to be consistent with the other number systems

	// remove any leading zeros from a edecimal representation
	void unpad() {
		int n = (int)size();
		for (int i = n - 1; i > 0; --i) {
			if (operator[](static_cast<size_t>(i)) == 0) {
				pop_back();
			}
			else {
				return;  // found the most significant digit
			}
		}
	}

	// read a edecimal ASCII format and make a edecimal type out of it
	bool parse(const std::string& _digits) {
		bool bSuccess = false;
		std::string digits(_digits);
		trim(digits);
		// check if the txt is an edecimal form:[+-]*[0123456789]+
		std::regex edecimal_regex("[+-]*[0123456789]+");
		if (std::regex_match(digits, edecimal_regex)) {
			// found a edecimal representation
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

#if EDECIMAL_OPERATIONS_COUNT
	// reset the operation statistics
	void resetStats() {
		ops.reset();
	}
	void printStats(std::ostream& ostr) {
		ops.report(ostr);
	}
#endif

protected:
	// HELPER methods

	// conversion functions
	inline short              to_short()       const noexcept { return short(to_long_long()); }
	inline int                to_int()         const noexcept { return short(to_long_long()); }
	inline long               to_long()        const noexcept { return short(to_long_long()); }
	inline long long          to_long_long()   const noexcept {
		// Horner's method: accumulate from most-significant digit
		long long v = 0;
		for (edecimal::const_reverse_iterator rit = this->rbegin(); rit != this->rend(); ++rit) {
			v = v * 10 + *rit;
		}
		return sign() ? -v : v;
	}
	inline unsigned short     to_ushort()      const noexcept { return static_cast<unsigned short>(to_ulong_long()); }
	inline unsigned int       to_uint()        const noexcept { return static_cast<unsigned int>(to_ulong_long()); }
	inline unsigned long      to_ulong()       const noexcept { return static_cast<unsigned long>(to_ulong_long()); }
	inline unsigned long long to_ulong_long()  const noexcept { return static_cast<unsigned long long>(to_long_long()); }
	inline float              to_float()       const noexcept {
		return static_cast<float>(to_double());
	}
	inline double             to_double()      const noexcept {
		// Accumulate via long double Horner's method to minimize
		// floating-point rounding error for large integers (fixes issue #205)
		return static_cast<double>(to_long_double());
	}
	inline long double        to_long_double() const noexcept {
		// Horner's method: accumulate from most-significant digit
		long double ld = 0.0l;
		for (edecimal::const_reverse_iterator rit = this->rbegin(); rit != this->rend(); ++rit) {
			ld = ld * 10.0l + *rit;
		}
		return sign() ? -ld : ld;
	}

	// Convert integer types to a edecimal representation
// TODO: needs SFINAE enable_if to constrain it to native integer types
	template<typename Ty>
	edecimal& convert_integer(Ty v) {
		using namespace std;
		//cout << numeric_limits<Ty>::digits << " max value " << numeric_limits<Ty>::max() << endl;
		bool sign = false;
		setzero(); // initialize the edecimal value to 0
		if (v == 0) return *this;
		if (numeric_limits<Ty>::is_signed) {
			if (v < 0) {
				sign = true; // negative number
				// transform to sign-magnitude on positive side
				v *= Ty(-1);
			}
		}
		uint64_t mask = 0x1;
		// IMPORTANT: can't use initializer or assignment operator as it would yield 
		// an infinite loop calling convert_integer. So we need to construct the
		// edecimal from first principals
		edecimal base; // can't use base(1) semantics here as it would cause an infinite loop
		base.clear();
		base.push_back(1);
		while (v) { // minimum loop iterations; exits when no bits left
			if (v & mask) {
				*this += base;
			}
			base += base;
			v >>= 1;
		}
		// lastly, set the sign
		setsign(sign);
		return *this;
	}
	template<typename Ty>
	edecimal& convert_ieee754(Ty rhs) {
		clear();
		if (rhs <= 0.5 && rhs >= -0.5) {
			return *this = 0;
		}
		else {
			if (rhs < -0.5) negative = true; else negative = false;

			bool s{ false };
			uint64_t unbiasedExponent{ 0 };
			uint64_t fraction{ 0 };
			uint64_t bits{ 0 };
			extractFields(rhs, s, unbiasedExponent, fraction, bits);
			// TODO: subnormals

			fraction |= (1ull << ieee754_parameter<Ty>::fbits); // add in the hidden bit
			// scale up by fbits, convert, and then scale back
			uint64_t mask = 0x1;
			edecimal base; // can't use base(1) semantics here as it would cause an infinite loop
			base[0] = 1;
//			int i = 0;
			while (fraction) { // minimum loop iterations; exits when no bits left
//				std::cout << std::setw(3) << i++ << "  base : " << base << '\n';
				if (fraction & mask) {
					*this += base;
				}
				base += base;
				fraction >>= 1;
			}
//			std::cout << "upconversion : " << *this << " : final bit value: " << base << '\n';
			int scale = static_cast<int>(unbiasedExponent) - ieee754_parameter<Ty>::bias; // original scale of the number
			int upScale = ieee754_parameter<Ty>::fbits;
			int correction = upScale - scale;
//			std::cout << "correction = " << correction << " scale = " << scale << " upscale = " << upScale << '\n';
			if (correction >= 0) {
				edecimal downConvert;
				downConvert[0] = 1;
				for (int i = 0; i < correction; ++i) downConvert += downConvert;
//				std::cout << "downConvert : " << downConvert << '\n';
				// divide the factor out
				*this /= downConvert;
			}
			else {
				edecimal upConvert;
				upConvert[0] = 1;
				for (int i = 0; i < -correction; ++i) upConvert += upConvert;
//				std::cout << "upConvert : " << upConvert << '\n';
				// multiply to add the missing factor
				*this *= upConvert;
			}
		}
		return *this;
	}

private:
	// sign-magnitude number: indicate if number is positive or negative
	bool negative;

	friend std::ostream& operator<<(std::ostream& ostr, const edecimal& d);
	friend std::istream& operator>>(std::istream& istr, edecimal& d);

	// edecimal - edecimal logic operators
	friend bool operator==(const edecimal& lhs, const edecimal& rhs);
	friend bool operator!=(const edecimal& lhs, const edecimal& rhs);
	friend bool operator<(const edecimal& lhs, const edecimal& rhs);
	friend bool operator>(const edecimal& lhs, const edecimal& rhs);
	friend bool operator<=(const edecimal& lhs, const edecimal& rhs);
	friend bool operator>=(const edecimal& lhs, const edecimal& rhs);
};

////////////////// helper functions

// find the order of the most significant digit, precondition edecimal is unpadded
inline int findMsd(const edecimal& v) {
	int msd = int(v.size()) - 1;
	if (msd == 0 && v == 0) return -1; // no significant digit found, all digits are zero
	assert(v.at(static_cast<size_t>(msd)) != 0); // indicates the edecimal wasn't unpadded
	return msd;
}


////////////////// DECIMAL operators

/// stream operators

inline std::string to_binary(const edecimal& d) {
	std::stringstream s;
	if (d.isneg()) s << '-';
	for (edecimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		s << (int)*rit;
	}
	return s.str();
}

// generate an ASCII edecimal string
inline std::string to_string(const edecimal& d) {
	std::stringstream s;
	if (d.isneg()) s << '-';
	for (edecimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		s << (int)*rit;
	}
	return s.str();
}

// generate an ASCII edecimal format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const edecimal& d) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	//std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	if (d.isneg()) ss << '-';
	for (edecimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ostr << ss.str();
}

// read an ASCII edecimal format from an istream
inline std::istream& operator>>(std::istream& istr, edecimal& p) {
	std::string txt;
	istr >> txt;
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into a edecimal value\n";
	}
	return istr;
}

/// edecimal binary arithmetic operators

// binary addition of edecimal numbers
inline edecimal operator+(const edecimal& lhs, const edecimal& rhs) {
	edecimal sum = lhs;
	sum += rhs;
	return sum;
}
// binary subtraction of edecimal numbers
inline edecimal operator-(const edecimal& lhs, const edecimal& rhs) {
	edecimal diff = lhs;
	diff -= rhs;
	return diff;
}
// binary mulitplication of edecimal numbers
inline edecimal operator*(const edecimal& lhs, const edecimal& rhs) {
	edecimal mul = lhs;
	mul *= rhs;
	return mul;
}
// binary division of edecimal numbers
inline edecimal operator/(const edecimal& lhs, const edecimal& rhs) {
	edecimal ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// binary remainder of edecimal numbers
inline edecimal operator%(const edecimal& lhs, const edecimal& rhs) {
	edecimal remainder = lhs;
	remainder %= rhs;
	return remainder;
}
// binary left shift
inline edecimal operator<<(const edecimal& lhs, int shift) {
	edecimal d(lhs);
	return d <<= shift;
}
// binary right shift
inline edecimal operator>>(const edecimal& lhs, int shift) {
	edecimal d(lhs);
	return d >>= shift;
}
/// logic operators

	// edecimal - edecimal logic operators
// equality test
bool operator==(const edecimal& lhs, const edecimal& rhs) {
	if (lhs.size() != rhs.size()) return false;
	bool areEqual = std::equal(lhs.begin(), lhs.end(), rhs.begin()) && lhs.sign() == rhs.sign();
	return areEqual;
}
// inequality test
bool operator!=(const edecimal& lhs, const edecimal& rhs) {
	return !operator==(lhs, rhs);
}
// less-than test
bool operator<(const edecimal& lhs, const edecimal& rhs) {
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
	edecimal::const_reverse_iterator ritl = lhs.rbegin();
	edecimal::const_reverse_iterator ritr = rhs.rbegin();
	for (; ritl != lhs.rend() || ritr != rhs.rend(); ++ritl, ++ritr) {
		if (*ritl < *ritr) return lhs.sign() ? false : true;
		if (*ritl > *ritr) return lhs.sign() ? true : false;
		// if the digits are equal we need to check the next set
	}
	// at this point we know the two operands are the same
	return false;
}
// greater-than test
bool operator>(const edecimal& lhs, const edecimal& rhs) {
	return operator<(rhs, lhs);
}
// less-or-equal test
bool operator<=(const edecimal& lhs, const edecimal& rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}
// greater-or-equal test
bool operator>=(const edecimal& lhs, const edecimal& rhs) {
	return !operator<(lhs, rhs);
}

// edecimal - long logic operators
inline bool operator==(const edecimal& lhs, long rhs) {
	return lhs == edecimal(rhs);
}
inline bool operator!=(const edecimal& lhs, long rhs) {
	return !operator==(lhs, edecimal(rhs));
}
inline bool operator< (const edecimal& lhs, long rhs) {
	return operator<(lhs, edecimal(rhs));
}
inline bool operator> (const edecimal& lhs, long rhs) {
	return operator< (edecimal(rhs), lhs);
}
inline bool operator<=(const edecimal& lhs, long rhs) {
	return operator< (lhs, edecimal(rhs)) || operator==(lhs, edecimal(rhs));
}
inline bool operator>=(const edecimal& lhs, long rhs) {
	return !operator<(lhs, edecimal(rhs));
}

// long - edecimal logic operators
inline bool operator==(long lhs, const edecimal& rhs) {
	return edecimal(lhs) == rhs;
}
inline bool operator!=(long lhs, const edecimal& rhs) {
	return !operator==(edecimal(lhs), rhs);
}
inline bool operator< (long lhs, const edecimal& rhs) {
	return operator<(edecimal(lhs), rhs);
}
inline bool operator> (long lhs, const edecimal& rhs) {
	return operator< (edecimal(lhs), rhs);
}
inline bool operator<=(long lhs, const edecimal& rhs) {
	return operator< (edecimal(lhs), rhs) || operator==(edecimal(lhs), rhs);
}
inline bool operator>=(long lhs, const edecimal& rhs) {
	return !operator<(edecimal(lhs), rhs);
}

///////////////////////////////////////////////////////////////////////
// 
// find largest multiplier of rhs being less or equal to lhs by subtraction; assumes 0*rhs <= lhs <= 9*rhs 
edecimal findLargestMultiple(const edecimal& lhs, const edecimal& rhs) {
	// check argument assumption	assert(0 <= lhs && lhs >= 9 * rhs);
	edecimal remainder = lhs;
	remainder.setpos();
	edecimal multiplier;
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
// decintdiv_t for edecimal to capture quotient and remainder during long division
struct decintdiv {
	edecimal quot; // quotient
	edecimal rem;  // remainder
};

// divide integer edecimal a and b and return result argument
decintdiv decint_divide(const edecimal& _a, const edecimal& _b) {
	if (_b.iszero()) {
#if EDECIMAL_THROW_ARITHMETIC_EXCEPTION
		throw edecimal_integer_divide_by_zero{};
#else
		std::cerr << "integer_divide_by_zero\n";
#endif // EDECIMAL_THROW_ARITHMETIC_EXCEPTION
	}
	// generate the absolute values to do long division 
	bool a_negative = _a.sign();
	bool b_negative = _b.sign();
	bool result_negative = (a_negative ^ b_negative);
	edecimal a(_a); a.setpos();
	edecimal b(_b); b.setpos();
	decintdiv divresult;
	if (a < b) {
		divresult.quot = 0;
		divresult.rem = _a; // a % b = a when a / b = 0
		return divresult; // a / b = 0 when b > a
	}
	// initialize the long division
	edecimal accumulator = a;
	// prepare the subtractand
	edecimal subtractand = b;
	int msd_b = findMsd(b);
	int msd_a = findMsd(a);
	int shift = msd_a - msd_b;
	subtractand <<= shift;
	// long division
	for (int i = shift; i >= 0; --i) {
		if (subtractand <= accumulator) {
			edecimal multiple = findLargestMultiple(accumulator, subtractand);
			accumulator -= multiple * subtractand;
			uint8_t multiplier = static_cast<uint8_t>(int(multiple)); // TODO: fix the ugly
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

// return quotient of a edecimal integer division
edecimal quotient(const edecimal& _a, const edecimal& _b) {
	return decint_divide(_a, _b).quot;
}
// return remainder of a edecimal integer division
edecimal remainder(const edecimal& _a, const edecimal& _b) {
	return decint_divide(_a, _b).rem;
}

}} // namespace sw::universal

