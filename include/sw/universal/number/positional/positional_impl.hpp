#pragma once
// positional_impl.hpp: definition of a sign-magnitude, multi-radix positional integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/internal/blockdigit/blockdigit.hpp>

// Forward definitions
#include <universal/number/positional/positional_fwd.hpp>

namespace sw { namespace universal {

// free function helpers
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix>& minpos(positional<ndigits, radix>& p) { return p.minpos(); }
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix>& maxpos(positional<ndigits, radix>& p) { return p.maxpos(); }
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix>& minneg(positional<ndigits, radix>& p) { return p.minneg(); }
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix>& maxneg(positional<ndigits, radix>& p) { return p.maxneg(); }

// positional: a sign-magnitude integer in an arbitrary radix, wrapping blockdigit
template<unsigned _ndigits, unsigned _radix>
class positional {
	static_assert(_ndigits > 0, "positional requires at least 1 digit");
	static_assert(_radix >= 2, "positional requires radix >= 2");
public:
	static constexpr unsigned ndigits = _ndigits;
	static constexpr unsigned radix   = _radix;
	using Storage = blockdigit<ndigits, radix>;

	// trivial default constructor: no in-class member initializers (triviality requirement)
	positional() = default;

	positional(const positional&) = default;
	positional(positional&&) = default;
	positional& operator=(const positional&) = default;
	positional& operator=(positional&&) = default;

	// specific value constructor
	constexpr positional(const SpecificValue code) noexcept : _value(code) {}

	// constructors from native types
	positional(signed char iv)        : _value(iv) {}
	positional(short iv)              : _value(iv) {}
	positional(int iv)                : _value(iv) {}
	positional(long iv)               : _value(iv) {}
	positional(long long iv)          : _value(iv) {}
	positional(unsigned char iv)      : _value(iv) {}
	positional(unsigned short iv)     : _value(iv) {}
	positional(unsigned int iv)       : _value(iv) {}
	positional(unsigned long iv)      : _value(iv) {}
	positional(unsigned long long iv) : _value(iv) {}
	positional(float iv)              : _value(iv) {}
	positional(double iv)             : _value(iv) {}

	// assignment operators for native types
	positional& operator=(signed char rhs)        { _value = rhs; return *this; }
	positional& operator=(short rhs)              { _value = rhs; return *this; }
	positional& operator=(int rhs)                { _value = rhs; return *this; }
	positional& operator=(long rhs)               { _value = rhs; return *this; }
	positional& operator=(long long rhs)          { _value = rhs; return *this; }
	positional& operator=(unsigned char rhs)      { _value = rhs; return *this; }
	positional& operator=(unsigned short rhs)     { _value = rhs; return *this; }
	positional& operator=(unsigned int rhs)       { _value = rhs; return *this; }
	positional& operator=(unsigned long rhs)      { _value = rhs; return *this; }
	positional& operator=(unsigned long long rhs) { _value = rhs; return *this; }
	positional& operator=(float rhs)              { _value = rhs; return *this; }
	positional& operator=(double rhs)             { _value = rhs; return *this; }

	// explicit conversion operators
	explicit operator long long() const noexcept { return static_cast<long long>(_value); }
	explicit operator unsigned long long() const noexcept { return static_cast<unsigned long long>(_value); }
	explicit operator int() const noexcept { return static_cast<int>(_value); }
	explicit operator long() const noexcept { return static_cast<long>(_value); }
	explicit operator unsigned int() const noexcept { return static_cast<unsigned int>(_value); }
	explicit operator unsigned long() const noexcept { return static_cast<unsigned long>(_value); }
	explicit operator float() const noexcept { return static_cast<float>(_value); }
	explicit operator double() const noexcept { return static_cast<double>(_value); }

	//////////////////////////////////////////////////////////////////////
	// arithmetic operators

	// unary negation
	positional operator-() const {
		positional tmp;
		tmp._value = -_value;
		return tmp;
	}

	// prefix increment
	positional& operator++() {
		++_value;
		return *this;
	}
	// postfix increment
	positional operator++(int) {
		positional tmp(*this);
		++(*this);
		return tmp;
	}
	// prefix decrement
	positional& operator--() {
		--_value;
		return *this;
	}
	// postfix decrement
	positional operator--(int) {
		positional tmp(*this);
		--(*this);
		return tmp;
	}

	// in-place arithmetic
	positional& operator+=(const positional& rhs) { _value += rhs._value; return *this; }
	positional& operator-=(const positional& rhs) { _value -= rhs._value; return *this; }
	positional& operator*=(const positional& rhs) { _value *= rhs._value; return *this; }
	positional& operator/=(const positional& rhs) {
#if POSITIONAL_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) throw positional_divide_by_zero();
#else
		if (rhs.iszero()) std::cerr << "positional: division by zero\n";
#endif
		_value /= rhs._value;
		return *this;
	}
	positional& operator%=(const positional& rhs) {
#if POSITIONAL_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) throw positional_divide_by_zero();
#else
		if (rhs.iszero()) std::cerr << "positional: modulo by zero\n";
#endif
		_value %= rhs._value;
		return *this;
	}

	// digit shift operators
	positional& operator<<=(int shift) { _value <<= shift; return *this; }
	positional& operator>>=(int shift) { _value >>= shift; return *this; }

	//////////////////////////////////////////////////////////////////////
	// modifiers

	void clear()    { _value.clear(); }
	void setzero()  { _value.setzero(); }
	void setdigit(unsigned index, uint8_t v) { _value.setdigit(index, v); }
	void setsign(bool s) { _value.setsign(s); }

	constexpr positional& minpos() noexcept { _value.minpos(); return *this; }
	constexpr positional& maxpos() noexcept { _value.maxpos(); return *this; }
	constexpr positional& zero()   noexcept { _value.zero();   return *this; }
	constexpr positional& minneg() noexcept { _value.minneg(); return *this; }
	constexpr positional& maxneg() noexcept { _value.maxneg(); return *this; }

	//////////////////////////////////////////////////////////////////////
	// selectors

	bool iszero() const { return _value.iszero(); }
	bool sign()   const { return _value.sign(); }
	bool isneg()  const { return _value.isneg(); }
	bool ispos()  const { return _value.ispos(); }
	uint8_t digit(unsigned index) const { return _value.digit(index); }

	int findMsd() const { return _value.findMsd(); }
	unsigned significant_digits() const { return _value.significant_digits(); }

	// scale: power-of-radix exponent of the most significant digit
	int scale() const {
		int msd = _value.findMsd();
		return (msd < 0) ? 0 : msd;
	}

	//////////////////////////////////////////////////////////////////////
	// string conversion

	std::string to_string() const { return _value.to_string(); }

	// access to underlying storage
	const Storage& value() const { return _value; }

private:
	Storage _value;  // single blockdigit, no in-class initializer (triviality)

	// friend declarations for free operators
	template<unsigned N, unsigned R>
	friend bool operator==(const positional<N, R>& lhs, const positional<N, R>& rhs);
	template<unsigned N, unsigned R>
	friend bool operator<(const positional<N, R>& lhs, const positional<N, R>& rhs);
	template<unsigned N, unsigned R>
	friend std::ostream& operator<<(std::ostream& ostr, const positional<N, R>& v);
};

//////////////////////////////////////////////////////////////////////
// logic operators

template<unsigned N, unsigned R>
inline bool operator==(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	return lhs._value == rhs._value;
}
template<unsigned N, unsigned R>
inline bool operator!=(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	return !(lhs == rhs);
}
template<unsigned N, unsigned R>
inline bool operator<(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	return lhs._value < rhs._value;
}
template<unsigned N, unsigned R>
inline bool operator>(const positional<N, R>& lhs, const positional<N, R>& rhs) { return rhs < lhs; }
template<unsigned N, unsigned R>
inline bool operator<=(const positional<N, R>& lhs, const positional<N, R>& rhs) { return !(rhs < lhs); }
template<unsigned N, unsigned R>
inline bool operator>=(const positional<N, R>& lhs, const positional<N, R>& rhs) { return !(lhs < rhs); }

//////////////////////////////////////////////////////////////////////
// binary arithmetic operators

template<unsigned N, unsigned R>
inline positional<N, R> operator+(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	positional<N, R> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned N, unsigned R>
inline positional<N, R> operator-(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	positional<N, R> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned N, unsigned R>
inline positional<N, R> operator*(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	positional<N, R> product(lhs);
	product *= rhs;
	return product;
}
template<unsigned N, unsigned R>
inline positional<N, R> operator/(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	positional<N, R> quotient(lhs);
	quotient /= rhs;
	return quotient;
}
template<unsigned N, unsigned R>
inline positional<N, R> operator%(const positional<N, R>& lhs, const positional<N, R>& rhs) {
	positional<N, R> remainder(lhs);
	remainder %= rhs;
	return remainder;
}

// digit shift operators
template<unsigned N, unsigned R>
inline positional<N, R> operator<<(const positional<N, R>& lhs, int shift) {
	positional<N, R> result(lhs);
	result <<= shift;
	return result;
}
template<unsigned N, unsigned R>
inline positional<N, R> operator>>(const positional<N, R>& lhs, int shift) {
	positional<N, R> result(lhs);
	result >>= shift;
	return result;
}

//////////////////////////////////////////////////////////////////////
// stream I/O

template<unsigned N, unsigned R>
inline std::ostream& operator<<(std::ostream& ostr, const positional<N, R>& v) {
	return ostr << v._value;
}

//////////////////////////////////////////////////////////////////////
// abs function

template<unsigned N, unsigned R>
inline positional<N, R> abs(const positional<N, R>& v) {
	return v.isneg() ? -v : v;
}

#if POSITIONAL_ENABLE_LITERALS
//////////////////////////////////////////////////////////////////////
// mixed-type operators with native types on the left

// addition
template<unsigned N, unsigned R>
inline positional<N, R> operator+(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) + rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator+(long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) + rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator+(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) + rhs; }

// subtraction
template<unsigned N, unsigned R>
inline positional<N, R> operator-(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) - rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator-(long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) - rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator-(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) - rhs; }

// multiplication
template<unsigned N, unsigned R>
inline positional<N, R> operator*(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) * rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator*(long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) * rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator*(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) * rhs; }

// division
template<unsigned N, unsigned R>
inline positional<N, R> operator/(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) / rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator/(long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) / rhs; }
template<unsigned N, unsigned R>
inline positional<N, R> operator/(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) / rhs; }

//////////////////////////////////////////////////////////////////////
// mixed-type operators with native types on the right

// addition
template<unsigned N, unsigned R>
inline positional<N, R> operator+(const positional<N, R>& lhs, int rhs) { return lhs + positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator+(const positional<N, R>& lhs, long rhs) { return lhs + positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator+(const positional<N, R>& lhs, long long rhs) { return lhs + positional<N, R>(rhs); }

// subtraction
template<unsigned N, unsigned R>
inline positional<N, R> operator-(const positional<N, R>& lhs, int rhs) { return lhs - positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator-(const positional<N, R>& lhs, long rhs) { return lhs - positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator-(const positional<N, R>& lhs, long long rhs) { return lhs - positional<N, R>(rhs); }

// multiplication
template<unsigned N, unsigned R>
inline positional<N, R> operator*(const positional<N, R>& lhs, int rhs) { return lhs * positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator*(const positional<N, R>& lhs, long rhs) { return lhs * positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator*(const positional<N, R>& lhs, long long rhs) { return lhs * positional<N, R>(rhs); }

// division
template<unsigned N, unsigned R>
inline positional<N, R> operator/(const positional<N, R>& lhs, int rhs) { return lhs / positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator/(const positional<N, R>& lhs, long rhs) { return lhs / positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline positional<N, R> operator/(const positional<N, R>& lhs, long long rhs) { return lhs / positional<N, R>(rhs); }

//////////////////////////////////////////////////////////////////////
// comparison with native types (left-hand side)

template<unsigned N, unsigned R>
inline bool operator==(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) == rhs; }
template<unsigned N, unsigned R>
inline bool operator!=(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) != rhs; }
template<unsigned N, unsigned R>
inline bool operator<(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) < rhs; }
template<unsigned N, unsigned R>
inline bool operator>(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) > rhs; }
template<unsigned N, unsigned R>
inline bool operator<=(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) <= rhs; }
template<unsigned N, unsigned R>
inline bool operator>=(int lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) >= rhs; }

template<unsigned N, unsigned R>
inline bool operator==(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) == rhs; }
template<unsigned N, unsigned R>
inline bool operator!=(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) != rhs; }
template<unsigned N, unsigned R>
inline bool operator<(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) < rhs; }
template<unsigned N, unsigned R>
inline bool operator>(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) > rhs; }
template<unsigned N, unsigned R>
inline bool operator<=(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) <= rhs; }
template<unsigned N, unsigned R>
inline bool operator>=(long long lhs, const positional<N, R>& rhs) { return positional<N, R>(lhs) >= rhs; }

// comparison with native types (right-hand side)
template<unsigned N, unsigned R>
inline bool operator==(const positional<N, R>& lhs, int rhs) { return lhs == positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator!=(const positional<N, R>& lhs, int rhs) { return lhs != positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator<(const positional<N, R>& lhs, int rhs) { return lhs < positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator>(const positional<N, R>& lhs, int rhs) { return lhs > positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator<=(const positional<N, R>& lhs, int rhs) { return lhs <= positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator>=(const positional<N, R>& lhs, int rhs) { return lhs >= positional<N, R>(rhs); }

template<unsigned N, unsigned R>
inline bool operator==(const positional<N, R>& lhs, long long rhs) { return lhs == positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator!=(const positional<N, R>& lhs, long long rhs) { return lhs != positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator<(const positional<N, R>& lhs, long long rhs) { return lhs < positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator>(const positional<N, R>& lhs, long long rhs) { return lhs > positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator<=(const positional<N, R>& lhs, long long rhs) { return lhs <= positional<N, R>(rhs); }
template<unsigned N, unsigned R>
inline bool operator>=(const positional<N, R>& lhs, long long rhs) { return lhs >= positional<N, R>(rhs); }

#endif // POSITIONAL_ENABLE_LITERALS

}} // namespace sw::universal
