#pragma once
// rational_impl.hpp: definition of a binary rational arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>

// Forward definitions
#include <universal/number/rational/rational_fwd.hpp>

namespace sw {	namespace universal {
		
// convert a floating-point value to a specific rational configuration. Semantically, p = v, return reference to p
template<unsigned nbits, typename bt>
inline rational<nbits, bt>& convert(const triple<nbits,bt>& v, rational<nbits,bt>& p) {
	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnan();
		return p;
	}
	return p;
}

template<unsigned nbits, typename bt>
rational<nbits, bt>& minpos(rational<nbits, bt>& lminpos) {
	return lminpos;
}
template<unsigned nbits, typename bt>
rational<nbits, bt>& maxpos(rational<nbits, bt>& lmaxpos) {
	return lmaxpos;
}
template<unsigned nbits, typename bt>
rational<nbits, bt>& minneg(rational<nbits, bt>& lminneg) {
	return lminneg;
}
template<unsigned nbits, typename bt>
rational<nbits, bt>& maxneg(rational<nbits, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<unsigned nbits, typename bt = uint8_t>
class rational {
public:
	typedef bt BlockType;
	using SignedBlockBinary = blockbinary<nbits, bt, BinaryNumberType::Signed>;

	rational() = default;

	constexpr rational(const rational&) = default;
	constexpr rational(rational&&) = default;

	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	// decorated constructor
	constexpr rational(const SignedBlockBinary& _n, const SignedBlockBinary& _d) : n{ _n }, d{ _d } {};

	// specific value constructor
	constexpr rational(const SpecificValue code) noexcept
		: n {}, d{} {
		switch (code) {
		case SpecificValue::maxpos:
			//maxpos();
			break;
		case SpecificValue::minpos:
			//minpos();
			break;
		case SpecificValue::zero:
		default:
			//zero();
			break;
		case SpecificValue::minneg:
			//minneg();
			break;
		case SpecificValue::maxneg:
			//maxneg();
			break;
		case SpecificValue::infpos:
			// rationals do not have an infinite
			//maxpos();
			break;
		case SpecificValue::infneg:
			// rationals do not have an infinite
			//maxneg();
			break;
		case SpecificValue::nar: // approximation as rational don't have a NaR
		case SpecificValue::qnan:
		case SpecificValue::snan:
			//setnan();
			break;
		}
	}

	constexpr rational(signed char initial_value)        { *this = initial_value; }
	constexpr rational(short initial_value)              { *this = initial_value; }
	constexpr rational(int initial_value)                { *this = initial_value; }
	constexpr rational(long long initial_value)          { *this = initial_value; }
	constexpr rational(unsigned long long initial_value) { *this = initial_value; }
	constexpr rational(float initial_value)              { *this = initial_value; }
	constexpr rational(double initial_value)             { *this = initial_value; }


	// assignment operators
	constexpr rational& operator=(signed char rhs)        { return convert_signed(rhs); }
	constexpr rational& operator=(short rhs)              { return convert_signed(rhs); }
	constexpr rational& operator=(int rhs)                { return convert_signed(rhs); }
	constexpr rational& operator=(long long rhs)          { return convert_signed(rhs); }
	constexpr rational& operator=(unsigned long long rhs) { return convert_unsigned(rhs); }
	constexpr rational& operator=(float rhs)              { return convert_ieee754(rhs); }
	constexpr rational& operator=(double rhs)             { return convert_ieee754(rhs); }

#if LONG_DOUBLE_SUPPORT
	rational(long double initial_value) { *this = initial_value; }
	rational& operator=(long double rhs)        { return convert_ieee754(rhs); }
#endif

	// arithmetic operators

	// unitary operators
	rational operator-() const {
		SignedBlockBinary a = n;
		SignedBlockBinary b = d;
		rational tmp(-n,d);
		return tmp;
	}
	rational operator++(int) { // postfix
		rational tmp(*this);
		++n;
		return tmp;
	}
	rational& operator++() { // prefix
		++n;
		return *this;
	}
	rational operator--(int) { // postfix
		rational tmp(*this);
		--n;
		return tmp;
	}
	rational& operator--() { // prefix
		--n;
		return *this;
	}

	// in-place arithmetic assignment operators
	rational& operator+=(const rational& rhs) { return *this; }
	rational& operator+=(double rhs) { return *this += rational(rhs); }
	rational& operator-=(const rational& rhs) { return *this; }
	rational& operator-=(double rhs) { return *this -= rational<nbits,bt>(rhs); }
	rational& operator*=(const rational& rhs) { return *this; }
	rational& operator*=(double rhs) { return *this *= rational<nbits,bt>(rhs); }
	rational& operator/=(const rational& rhs) { return *this; }
	rational& operator/=(double rhs) { return *this /= rational<nbits,bt>(rhs); }

	// modifiers
	constexpr void clear()  noexcept { n = 0; d = 1; }
	constexpr void setzero() noexcept { n = 0; d = 1; }
	constexpr void setnan() noexcept { n = 0; d = 0; }
	constexpr void set(const SignedBlockBinary& _n, const SignedBlockBinary& _d) noexcept { n = _n; d = _d; }
	constexpr void setbits(std::uint64_t bits) noexcept { n = bits; d = 1; }

	// create specific number system values of interest
	constexpr rational& maxpos() noexcept {
		// maximum positive value
		n.maxpos(); d = 1;
		return *this;
	}
	constexpr rational& minpos() noexcept {
		// minimum positive value
		n = 1; d.maxpos();
		return *this;
	}
	constexpr rational& zero() noexcept {
		n = 0; d = 1;
		return *this;
	}
	constexpr rational& minneg() noexcept {
		// minimum negative value
		n = -1; d.maxpos();
		return *this;
	}
	constexpr rational& maxneg() noexcept {
		// maximum negative value
		n.minneg(); d = 1;
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept { return (n.iszero() && !d.iszero()); }
	constexpr bool isneg()  const noexcept { return n.isneg(); }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool isnan()  const noexcept { return (n.iszero() && d.iszero()); }
	constexpr bool sign()   const noexcept { return n.sign(); }
	constexpr int  scale()  const noexcept { return sw::universal::scale(double(n) / double(d)); }

	SignedBlockBinary numerator() const noexcept { return n; }
	SignedBlockBinary denominator() const noexcept { return d; }

	long double to_long_double() const {
		return 0.0l;
	}
	double to_double() const {
		return 0.0;
	}
	float to_float() const {
		return 0.0f;
	}
	// Maybe remove explicit
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

protected:
	// HELPER methods

	// remove greatest common divisor out of the numerator/denominator pair
	void normalize() {
		bool nsign = n.sign();
		bool dsign = d.sign();
		bool sign = n.sign() ^ d.sign();
		SignedBlockBinary a, b, r;
		a = (nsign ? -n : n); b = (dsign ? -d : d);  // precondition for gcd loop is numerator and denominator are positive

		if (b.iszero()) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
			throw rational_divide_by_zero();
#else
			std::cerr << "rational_divide_by_zero\n";
			d = 0;
			n = 0;
#endif
		}
		SignedBlockBinary zero{ 0 };
		while (a % b > zero) {
			r = a % b;
			a = b;
			b = r;
		}
		n /= (sign ? -b : b);
		d /= (dsign ? -b : b);
	}

	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	rational& convert_signed(SignedInt& rhs) {
		if (rhs < 0) {
			n = -rhs;
		}
		else {
			n = rhs;
		}
		d = 1;
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type >
	rational& convert_unsigned(UnsignedInt& rhs) {
		n = rhs;
		d = 1;
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	rational& convert_ieee754(Real rhs) noexcept {
		// extract components, convert mantissa to fraction with denominator 2^fbits, adjust fraction using scale, normalize
		uint64_t bits{ 0 };
		uint64_t e{ 0 }, f{ 0 };
		bool s{ false };
		extractFields(rhs, s, e, f, bits);
		if (e == 0) { // subnormal
		}
		else { // normal
			n = f | ieee754_parameter<Real>::hmask;
			d = ieee754_parameter<Real>::hmask;
			n = (s ? -n : n);
		}
		normalize();
		return *this;
	}

private:
	SignedBlockBinary n; // numerator
	SignedBlockBinary d; // denominator, always positive so that sign of numerator is sign of rational

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const rational<nnbits,nbt>& r);
	template<size_t nnbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, rational<nnbits,nbt>& r);

	template<size_t nnbits, typename nbt>
	friend bool operator==(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator!=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator< (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator> (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator<=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<size_t nnbits, typename nbt>
	friend bool operator>=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// stream opreators

template<size_t nnbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const rational<nnbits,nbt>& v) {

	return ostr;
}

template<size_t nnbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const rational<nnbits,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nbits, typename bt>
inline std::string to_binary(const rational<nbits,bt>& v) {
	std::stringstream s;
	s << to_binary(v.numerator())
		<< " / "
		<< to_binary(v.denominator());
	return s.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// binary logic functions

template<size_t nnbits, typename nbt>
inline bool operator==(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return false; }
template<size_t nnbits, typename nbt>
inline bool operator!=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator< (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return false; }
template<size_t nnbits, typename nbt>
inline bool operator> (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, typename nbt>
inline bool operator<=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, typename nbt>
inline bool operator>=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return !operator< (lhs, rhs); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
///  binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, typename bt>
inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, const rational<nbits, bt>& rhs) {
	rational<nbits,bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, typename bt>
inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, const rational<nbits, bt>& rhs) {
	rational<nbits,bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, typename bt>
inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, const rational<nbits, bt>& rhs) {
	rational<nbits,bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, typename bt>
inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, const rational<nbits, bt>& rhs) {
	rational<nbits,bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// math functions

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, typename bt>
rational<nbits,bt> abs(const rational<nbits,bt>& v) {
	return rational<nbits,bt>();
}



}}  // namespace sw::universal
