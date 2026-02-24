#pragma once
// blockdigit.hpp: parameterized blocked digit number system for fixed-size multi-radix integer arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cassert>
#include <limits>
#include <algorithm>
#include <type_traits>

namespace sw { namespace universal {

// blockdigit: a fixed-size, sign-magnitude, multi-radix integer type
// _ndigits : number of digits in the given radix
// _radix   : the base of the digit system (8 = octal, 10 = decimal, 16 = hexadecimal)
// DigitType: storage type for each digit (one digit per element)
template<unsigned _ndigits, unsigned _radix = 10, typename DigitType = uint8_t>
class blockdigit {
	static_assert(_ndigits > 0, "blockdigit requires at least 1 digit");
	static_assert(_radix >= 2, "blockdigit requires radix >= 2");
	static_assert(_radix <= 256, "blockdigit radix must fit in a uint8_t digit");
public:
	static constexpr unsigned ndigits = _ndigits;
	static constexpr unsigned radix   = _radix;
	using Digit = DigitType;

	// trivial default constructor: no in-class member initializers (triviality requirement)
	blockdigit() = default;

	blockdigit(const blockdigit&) = default;
	blockdigit(blockdigit&&) = default;
	blockdigit& operator=(const blockdigit&) = default;
	blockdigit& operator=(blockdigit&&) = default;

	// constructors from native types
	blockdigit(signed char initial_value)        { *this = static_cast<long long>(initial_value); }
	blockdigit(short initial_value)              { *this = static_cast<long long>(initial_value); }
	blockdigit(int initial_value)                { *this = static_cast<long long>(initial_value); }
	blockdigit(long initial_value)               { *this = static_cast<long long>(initial_value); }
	blockdigit(long long initial_value)          { *this = initial_value; }
	blockdigit(unsigned char initial_value)      { *this = static_cast<unsigned long long>(initial_value); }
	blockdigit(unsigned short initial_value)     { *this = static_cast<unsigned long long>(initial_value); }
	blockdigit(unsigned int initial_value)       { *this = static_cast<unsigned long long>(initial_value); }
	blockdigit(unsigned long initial_value)      { *this = static_cast<unsigned long long>(initial_value); }
	blockdigit(unsigned long long initial_value) { *this = initial_value; }
	blockdigit(float initial_value)              { *this = static_cast<long long>(initial_value); }
	blockdigit(double initial_value)             { *this = static_cast<long long>(initial_value); }

	// assignment operators for native types
	blockdigit& operator=(signed char rhs)        { return convert_signed(static_cast<long long>(rhs)); }
	blockdigit& operator=(short rhs)              { return convert_signed(static_cast<long long>(rhs)); }
	blockdigit& operator=(int rhs)                { return convert_signed(static_cast<long long>(rhs)); }
	blockdigit& operator=(long rhs)               { return convert_signed(static_cast<long long>(rhs)); }
	blockdigit& operator=(long long rhs)          { return convert_signed(rhs); }
	blockdigit& operator=(unsigned char rhs)      { return convert_unsigned(static_cast<unsigned long long>(rhs)); }
	blockdigit& operator=(unsigned short rhs)     { return convert_unsigned(static_cast<unsigned long long>(rhs)); }
	blockdigit& operator=(unsigned int rhs)       { return convert_unsigned(static_cast<unsigned long long>(rhs)); }
	blockdigit& operator=(unsigned long rhs)      { return convert_unsigned(static_cast<unsigned long long>(rhs)); }
	blockdigit& operator=(unsigned long long rhs) { return convert_unsigned(rhs); }
	blockdigit& operator=(float rhs)              { return convert_signed(static_cast<long long>(rhs)); }
	blockdigit& operator=(double rhs)             { return convert_signed(static_cast<long long>(rhs)); }

	// explicit conversion operators
	explicit operator long long() const noexcept { return to_long_long(); }
	explicit operator unsigned long long() const noexcept { return static_cast<unsigned long long>(to_long_long()); }
	explicit operator int() const noexcept { return static_cast<int>(to_long_long()); }
	explicit operator long() const noexcept { return static_cast<long>(to_long_long()); }
	explicit operator unsigned int() const noexcept { return static_cast<unsigned int>(to_long_long()); }
	explicit operator unsigned long() const noexcept { return static_cast<unsigned long>(to_long_long()); }
	explicit operator float() const noexcept { return to_float(); }
	explicit operator double() const noexcept { return to_double(); }

	//////////////////////////////////////////////////////////////////////
	// arithmetic operators

	// unary negation
	blockdigit operator-() const {
		blockdigit tmp(*this);
		if (!tmp.iszero()) tmp._negative = !tmp._negative;
		return tmp;
	}

	// increment/decrement
	blockdigit& operator++() { // prefix
		blockdigit one(1);
		*this += one;
		return *this;
	}
	blockdigit operator++(int) { // postfix
		blockdigit tmp(*this);
		++(*this);
		return tmp;
	}
	blockdigit& operator--() { // prefix
		blockdigit one(1);
		*this -= one;
		return *this;
	}
	blockdigit operator--(int) { // postfix
		blockdigit tmp(*this);
		--(*this);
		return tmp;
	}

	// in-place addition
	blockdigit& operator+=(const blockdigit& rhs) {
		if (_negative != rhs._negative) {
			// different signs: subtract magnitude of rhs
			blockdigit tmp(rhs);
			tmp._negative = !tmp._negative;
			return operator-=(tmp);
		}
		// same sign: add magnitudes, sign stays the same
		DigitType carry = 0;
		for (unsigned i = 0; i < ndigits; ++i) {
			unsigned sum = static_cast<unsigned>(_digit[i]) + static_cast<unsigned>(rhs._digit[i]) + carry;
			_digit[i] = static_cast<DigitType>(sum % radix);
			carry = static_cast<DigitType>(sum / radix);
		}
		// carry overflow silently truncated (modular arithmetic)
		return *this;
	}

	// in-place subtraction
	blockdigit& operator-=(const blockdigit& rhs) {
		if (_negative != rhs._negative) {
			// different signs: add magnitude of rhs
			blockdigit tmp(rhs);
			tmp._negative = !tmp._negative;
			return operator+=(tmp);
		}
		// same sign: subtract magnitudes
		// compare magnitudes to determine result sign
		int cmp = compare_magnitude(rhs);
		if (cmp == 0) {
			// equal magnitudes, result is zero
			clear();
			return *this;
		}
		if (cmp < 0) {
			// |this| < |rhs|: swap and flip sign
			blockdigit tmp(rhs);
			DigitType borrow = 0;
			for (unsigned i = 0; i < ndigits; ++i) {
				int diff = static_cast<int>(tmp._digit[i]) - static_cast<int>(_digit[i]) - borrow;
				if (diff < 0) {
					diff += radix;
					borrow = 1;
				}
				else {
					borrow = 0;
				}
				_digit[i] = static_cast<DigitType>(diff);
			}
			_negative = !_negative;
			if (iszero()) _negative = false;
			return *this;
		}
		// |this| > |rhs|: subtract rhs from this, sign unchanged
		DigitType borrow = 0;
		for (unsigned i = 0; i < ndigits; ++i) {
			int diff = static_cast<int>(_digit[i]) - static_cast<int>(rhs._digit[i]) - borrow;
			if (diff < 0) {
				diff += radix;
				borrow = 1;
			}
			else {
				borrow = 0;
			}
			_digit[i] = static_cast<DigitType>(diff);
		}
		if (iszero()) _negative = false;
		return *this;
	}

	// in-place multiplication (schoolbook)
	blockdigit& operator*=(const blockdigit& rhs) {
		if (iszero() || rhs.iszero()) {
			clear();
			return *this;
		}
		bool resultSign = (_negative != rhs._negative);
		blockdigit result;
		result.clear();
		for (unsigned i = 0; i < ndigits; ++i) {
			if (_digit[i] == 0) continue;
			DigitType carry = 0;
			for (unsigned j = 0; j < ndigits; ++j) {
				if (i + j >= ndigits) break; // overflow truncation
				unsigned prod = static_cast<unsigned>(result._digit[i + j])
					+ static_cast<unsigned>(_digit[i]) * static_cast<unsigned>(rhs._digit[j])
					+ carry;
				result._digit[i + j] = static_cast<DigitType>(prod % radix);
				carry = static_cast<DigitType>(prod / radix);
			}
		}
		result._negative = resultSign;
		if (result.iszero()) result._negative = false;
		*this = result;
		return *this;
	}

	// in-place division (long division)
	blockdigit& operator/=(const blockdigit& rhs) {
		*this = div_mod(*this, rhs).first;
		return *this;
	}

	// in-place modulo
	blockdigit& operator%=(const blockdigit& rhs) {
		*this = div_mod(*this, rhs).second;
		return *this;
	}

	// digit shift left (insert zeros at LSD end)
	blockdigit& operator<<=(int shift) {
		if (shift <= 0) {
			if (shift < 0) return operator>>=(-shift);
			return *this;
		}
		if (static_cast<unsigned>(shift) >= ndigits) {
			clear();
			return *this;
		}
		for (int i = static_cast<int>(ndigits) - 1; i >= shift; --i) {
			_digit[i] = _digit[i - shift];
		}
		for (int i = 0; i < shift; ++i) {
			_digit[i] = 0;
		}
		return *this;
	}

	// digit shift right (remove digits from LSD end)
	blockdigit& operator>>=(int shift) {
		if (shift <= 0) {
			if (shift < 0) return operator<<=(-shift);
			return *this;
		}
		if (static_cast<unsigned>(shift) >= ndigits) {
			clear();
			return *this;
		}
		for (unsigned i = 0; i + static_cast<unsigned>(shift) < ndigits; ++i) {
			_digit[i] = _digit[i + static_cast<unsigned>(shift)];
		}
		for (unsigned i = ndigits - static_cast<unsigned>(shift); i < ndigits; ++i) {
			_digit[i] = 0;
		}
		if (iszero()) _negative = false;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////
	// modifiers

	void clear() {
		_negative = false;
		for (unsigned i = 0; i < ndigits; ++i) _digit[i] = 0;
	}
	void setzero() { clear(); }
	void setdigit(unsigned index, DigitType value) {
		assert(index < ndigits);
		assert(value < radix);
		_digit[index] = value;
	}
	void setsign(bool s) { _negative = s; }
	void setneg() { _negative = true; }
	void setpos() { _negative = false; }
	void setbits(uint64_t v) { *this = v; }

	//////////////////////////////////////////////////////////////////////
	// selectors

	bool iszero() const {
		for (unsigned i = 0; i < ndigits; ++i) {
			if (_digit[i] != 0) return false;
		}
		return true;
	}
	bool sign() const { return _negative; }
	bool isneg() const { return _negative; }
	bool ispos() const { return !_negative; }
	DigitType digit(unsigned index) const {
		assert(index < ndigits);
		return _digit[index];
	}

	// find the index of the most significant non-zero digit, -1 if zero
	int findMsd() const {
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			if (_digit[i] != 0) return i;
		}
		return -1;
	}

	// number of significant digits
	unsigned significant_digits() const {
		int msd = findMsd();
		return (msd < 0) ? 1 : static_cast<unsigned>(msd + 1);
	}

	//////////////////////////////////////////////////////////////////////
	// string conversion

	std::string to_string() const {
		std::string s;
		if (_negative) s += '-';
		bool leadingZero = true;
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			DigitType d = _digit[i];
			if (d == 0 && leadingZero && i > 0) continue;
			leadingZero = false;
			if constexpr (radix <= 10) {
				s += static_cast<char>('0' + d);
			}
			else {
				if (d < 10) {
					s += static_cast<char>('0' + d);
				}
				else {
					s += static_cast<char>('A' + d - 10);
				}
			}
		}
		return s;
	}

private:
	bool     _negative;        // sign-magnitude (not 2's complement)
	DigitType _digit[ndigits]; // _digit[0] = least significant

	// conversion helpers
	blockdigit& convert_signed(long long rhs) {
		clear();
		if (rhs < 0) {
			_negative = true;
			if (rhs == std::numeric_limits<long long>::min()) {
				unsigned long long v = static_cast<unsigned long long>(-(rhs + 1)) + 1ull;
				for (unsigned i = 0; i < ndigits && v > 0; ++i) {
					_digit[i] = static_cast<DigitType>(v % radix);
					v /= radix;
				}
			}
			else {
				unsigned long long v = static_cast<unsigned long long>(-rhs);
				for (unsigned i = 0; i < ndigits && v > 0; ++i) {
					_digit[i] = static_cast<DigitType>(v % radix);
					v /= radix;
				}
			}
		}
		else {
			_negative = false;
			unsigned long long v = static_cast<unsigned long long>(rhs);
			for (unsigned i = 0; i < ndigits && v > 0; ++i) {
				_digit[i] = static_cast<DigitType>(v % radix);
				v /= radix;
			}
		}
		return *this;
	}
	blockdigit& convert_unsigned(unsigned long long rhs) {
		clear();
		_negative = false;
		for (unsigned i = 0; i < ndigits && rhs > 0; ++i) {
			_digit[i] = static_cast<DigitType>(rhs % radix);
			rhs /= radix;
		}
		return *this;
	}

	// compare magnitude (ignoring sign), returns -1, 0, +1
	int compare_magnitude(const blockdigit& rhs) const {
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			if (_digit[i] < rhs._digit[i]) return -1;
			if (_digit[i] > rhs._digit[i]) return +1;
		}
		return 0;
	}

	// long division: returns {quotient, remainder}
	static std::pair<blockdigit, blockdigit> div_mod(const blockdigit& _a, const blockdigit& _b) {
		if (_b.iszero()) {
			std::cerr << "blockdigit: division by zero\n";
			return { blockdigit(0), blockdigit(0) };
		}
		bool resultSign = (_a._negative != _b._negative);
		// work with positive magnitudes
		blockdigit a(_a); a._negative = false;
		blockdigit b(_b); b._negative = false;

		if (a.compare_magnitude(b) < 0) {
			// |a| < |b|: quotient is 0, remainder is a (with a's sign)
			blockdigit rem(_a);
			return { blockdigit(0), rem };
		}

		int msd_a = a.findMsd();
		int msd_b = b.findMsd();
		int shift = msd_a - msd_b;

		// prepare subtractand: b shifted up
		blockdigit subtractand(b);
		subtractand <<= shift;

		blockdigit quotient;
		quotient.clear();
		blockdigit accumulator(a);

		for (int i = shift; i >= 0; --i) {
			// find largest multiple k such that k * subtractand <= accumulator
			DigitType k = 0;
			blockdigit multiple;
			multiple.clear();
			for (unsigned m = 1; m < radix; ++m) {
				blockdigit candidate(multiple);
				candidate += subtractand;
				if (candidate.compare_magnitude(accumulator) <= 0) {
					multiple = candidate;
					k = static_cast<DigitType>(m);
				}
				else {
					break;
				}
			}
			quotient._digit[static_cast<unsigned>(i)] = k;
			accumulator -= multiple;
			subtractand >>= 1;
		}
		quotient._negative = resultSign;
		if (quotient.iszero()) quotient._negative = false;

		blockdigit remainder(accumulator);
		if (_a._negative) remainder._negative = true;
		if (remainder.iszero()) remainder._negative = false;
		return { quotient, remainder };
	}

	// conversion helpers
	long long to_long_long() const noexcept {
		long long v = 0;
		long long base = 1;
		for (unsigned i = 0; i < ndigits; ++i) {
			v += static_cast<long long>(_digit[i]) * base;
			base *= radix;
		}
		return _negative ? -v : v;
	}
	float to_float() const noexcept {
		return static_cast<float>(to_double());
	}
	double to_double() const noexcept {
		double v = 0.0;
		double base = 1.0;
		for (unsigned i = 0; i < ndigits; ++i) {
			v += static_cast<double>(_digit[i]) * base;
			base *= static_cast<double>(radix);
		}
		return _negative ? -v : v;
	}

	// friend declarations
	template<unsigned N, unsigned R, typename D>
	friend bool operator==(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs);
	template<unsigned N, unsigned R, typename D>
	friend bool operator<(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs);
	template<unsigned N, unsigned R, typename D>
	friend std::ostream& operator<<(std::ostream& ostr, const blockdigit<N, R, D>& v);
};

//////////////////////////////////////////////////////////////////////
// logic operators

template<unsigned N, unsigned R, typename D>
inline bool operator==(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	if (lhs.iszero() && rhs.iszero()) return true; // +0 == -0
	if (lhs._negative != rhs._negative) return false;
	for (unsigned i = 0; i < N; ++i) {
		if (lhs._digit[i] != rhs._digit[i]) return false;
	}
	return true;
}
template<unsigned N, unsigned R, typename D>
inline bool operator!=(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	return !(lhs == rhs);
}
template<unsigned N, unsigned R, typename D>
inline bool operator<(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	bool lzero = lhs.iszero();
	bool rzero = rhs.iszero();
	if (lzero && rzero) return false;
	if (lhs._negative && !rhs._negative) return !rzero || !lzero; // neg < pos unless both zero
	if (!lhs._negative && rhs._negative) return false; // pos !< neg
	// same sign
	if (!lhs._negative) {
		// both positive: compare magnitude
		return lhs.compare_magnitude(rhs) < 0;
	}
	else {
		// both negative: larger magnitude is smaller value
		return lhs.compare_magnitude(rhs) > 0;
	}
}
template<unsigned N, unsigned R, typename D>
inline bool operator>(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) { return rhs < lhs; }
template<unsigned N, unsigned R, typename D>
inline bool operator<=(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) { return !(rhs < lhs); }
template<unsigned N, unsigned R, typename D>
inline bool operator>=(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) { return !(lhs < rhs); }

//////////////////////////////////////////////////////////////////////
// binary arithmetic operators

template<unsigned N, unsigned R, typename D>
inline blockdigit<N, R, D> operator+(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	blockdigit<N, R, D> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned N, unsigned R, typename D>
inline blockdigit<N, R, D> operator-(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	blockdigit<N, R, D> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned N, unsigned R, typename D>
inline blockdigit<N, R, D> operator*(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	blockdigit<N, R, D> product(lhs);
	product *= rhs;
	return product;
}
template<unsigned N, unsigned R, typename D>
inline blockdigit<N, R, D> operator/(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	blockdigit<N, R, D> quotient(lhs);
	quotient /= rhs;
	return quotient;
}
template<unsigned N, unsigned R, typename D>
inline blockdigit<N, R, D> operator%(const blockdigit<N, R, D>& lhs, const blockdigit<N, R, D>& rhs) {
	blockdigit<N, R, D> remainder(lhs);
	remainder %= rhs;
	return remainder;
}

// digit shift operators
template<unsigned N, unsigned R, typename D>
inline blockdigit<N, R, D> operator<<(const blockdigit<N, R, D>& lhs, int shift) {
	blockdigit<N, R, D> result(lhs);
	result <<= shift;
	return result;
}
template<unsigned N, unsigned R, typename D>
inline blockdigit<N, R, D> operator>>(const blockdigit<N, R, D>& lhs, int shift) {
	blockdigit<N, R, D> result(lhs);
	result >>= shift;
	return result;
}

//////////////////////////////////////////////////////////////////////
// stream I/O

template<unsigned N, unsigned R, typename D>
inline std::ostream& operator<<(std::ostream& ostr, const blockdigit<N, R, D>& v) {
	return ostr << v.to_string();
}

//////////////////////////////////////////////////////////////////////
// manipulation functions

// Generate a type tag for blockdigit
template<unsigned N, unsigned R, typename D>
inline std::string type_tag(const blockdigit<N, R, D>& = {}) {
	std::stringstream s;
	if constexpr (R == 8) {
		s << "blockoctal<" << N << '>';
	}
	else if constexpr (R == 10) {
		s << "blockdecimal<" << N << '>';
	}
	else if constexpr (R == 16) {
		s << "blockhexadecimal<" << N << '>';
	}
	else {
		s << "blockdigit<" << N << ", " << R << '>';
	}
	return s.str();
}

// to_binary: show internal digit storage
template<unsigned N, unsigned R, typename D>
inline std::string to_binary(const blockdigit<N, R, D>& v) {
	std::stringstream s;
	s << (v.sign() ? '-' : '+') << "[ ";
	for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
		s << static_cast<int>(v.digit(static_cast<unsigned>(i)));
		if (i > 0) s << '.';
	}
	s << " ]";
	return s.str();
}

//////////////////////////////////////////////////////////////////////
// type aliases for common radixes

template<unsigned ndigits> using blockoctal        = blockdigit<ndigits, 8>;
template<unsigned ndigits> using blockdecimal_t    = blockdigit<ndigits, 10>;
template<unsigned ndigits> using blockhexadecimal  = blockdigit<ndigits, 16>;

}} // namespace sw::universal
