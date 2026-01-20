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
rational<nbits, bt>& minpos(rational<nbits, bt>& r) {
	return r.minpos();
}
template<unsigned nbits, typename bt>
rational<nbits, bt>& maxpos(rational<nbits, bt>& r) {
	return r.maxpos();
}
template<unsigned nbits, typename bt>
rational<nbits, bt>& minneg(rational<nbits, bt>& r) {
	return r.minneg();
}
template<unsigned nbits, typename bt>
rational<nbits, bt>& maxneg(rational<nbits, bt>& r) {
	return r.maxneg();
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<unsigned _nbits, typename bt = uint8_t>
class rational {
public:
	static constexpr unsigned nbits = _nbits;
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
	constexpr rational(long initial_value)               { *this = initial_value; }
	constexpr rational(long long initial_value)          { *this = initial_value; }
	constexpr rational(unsigned char initial_value)      { *this = initial_value; }
	constexpr rational(unsigned short initial_value)     { *this = initial_value; }
	constexpr rational(unsigned int initial_value)       { *this = initial_value; }
	constexpr rational(unsigned long initial_value)      { *this = initial_value; }
	constexpr rational(unsigned long long initial_value) { *this = initial_value; }
	constexpr rational(float initial_value)              { *this = initial_value; }
	constexpr rational(double initial_value)             { *this = initial_value; }

	// assignment operators
	constexpr rational& operator=(signed char rhs)        { return convert_signed(rhs); }
	constexpr rational& operator=(short rhs)              { return convert_signed(rhs); }
	constexpr rational& operator=(int rhs)                { return convert_signed(rhs); }
	constexpr rational& operator=(long rhs)               { return convert_signed(rhs); }
	constexpr rational& operator=(long long rhs)          { return convert_signed(rhs); }
	constexpr rational& operator=(unsigned char rhs)      { return convert_unsigned(rhs); }
	constexpr rational& operator=(unsigned short rhs)     { return convert_unsigned(rhs); }
	constexpr rational& operator=(unsigned int rhs)       { return convert_unsigned(rhs); }
	constexpr rational& operator=(unsigned long rhs)      { return convert_unsigned(rhs); }
	constexpr rational& operator=(unsigned long long rhs) { return convert_unsigned(rhs); }
	constexpr rational& operator=(float rhs)              { return convert_ieee754(rhs); }
	constexpr rational& operator=(double rhs)             { return convert_ieee754(rhs); }

	// explicit conversion operators
	explicit operator char()               const noexcept { return to_unsigned<char>(); }
	explicit operator unsigned short()     const noexcept { return to_unsigned<unsigned short>(); }
	explicit operator unsigned int()       const noexcept { return to_unsigned<unsigned int>(); }
	explicit operator unsigned long()      const noexcept { return to_unsigned<unsigned long>(); }
	explicit operator unsigned long long() const noexcept { return to_unsigned<unsigned long long>(); }
	explicit operator signed char()        const noexcept { return to_signed<signed char>(); }
	explicit operator short()              const noexcept { return to_signed<short>(); }
	explicit operator int()                const noexcept { return to_signed<int>(); }
	explicit operator long()               const noexcept { return to_signed<long>(); }
	explicit operator long long()          const noexcept { return to_signed<long long>(); }
	explicit operator float()              const noexcept { return to_ieee754<float>(); }
	explicit operator double()             const noexcept { return to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	rational(long double initial_value) { *this = initial_value; }
	rational& operator=(long double rhs)        { return convert_ieee754(rhs); }
	explicit operator long double()        const noexcept { return to_ieee754<long double>(); }
#endif

	// arithmetic operators

	// unitary operators
	rational operator-() const {
		rational tmp(-n,d);
		return tmp;
	}
	// increment and decrement operators are not defined for rational

	// in-place arithmetic assignment operators
	
	// in-place addition
	rational& operator+=(const rational& rhs) {
		SignedBlockBinary x = n;
		SignedBlockBinary y = d;
		SignedBlockBinary v = rhs.n;
		SignedBlockBinary w = rhs.d;
		if (y == w) {
			SignedBlockBinary num = x + v;
			n = num;
		}
		else {
			SignedBlockBinary e = x * w + y * v;
			SignedBlockBinary f = y * w;
			n = e;
			d = f;
		}
		normalize();
		return *this;
	}
	rational& operator+=(unsigned short rhs)     { return *this += rational(rhs); }
	rational& operator+=(unsigned int rhs)       { return *this += rational(rhs); }
	rational& operator+=(unsigned long rhs)      { return *this += rational(rhs); }
	rational& operator+=(unsigned long long rhs) { return *this += rational(rhs); }
	rational& operator+=(short rhs)              { return *this += rational(rhs); }
	rational& operator+=(int rhs)                { return *this += rational(rhs); }
	rational& operator+=(long rhs)               { return *this += rational(rhs); }
	rational& operator+=(long long rhs)          { return *this += rational(rhs); }
	rational& operator+=(float rhs)              { return *this += rational(rhs); }
	rational& operator+=(double rhs)             { return *this += rational(rhs); }
	// in-place subtraction
	rational& operator-=(const rational& rhs) { 
		SignedBlockBinary x = n;
		SignedBlockBinary y = d;
		SignedBlockBinary v = rhs.n;
		SignedBlockBinary w = rhs.d;
		if (y == w) {
			SignedBlockBinary num = x - v;
			n = num;
		}
		else {
			SignedBlockBinary e = x * w - y * v;
			SignedBlockBinary f = y * w;
			n = e;
			d = f;
		}
		normalize();
		return *this; 
	}
	rational& operator-=(unsigned short rhs)     { return *this -= rational(rhs); }
	rational& operator-=(unsigned int rhs)       { return *this -= rational(rhs); }
	rational& operator-=(unsigned long rhs)      { return *this -= rational(rhs); }
	rational& operator-=(unsigned long long rhs) { return *this -= rational(rhs); }
	rational& operator-=(short rhs)              { return *this -= rational(rhs); }
	rational& operator-=(int rhs)                { return *this -= rational(rhs); }
	rational& operator-=(long rhs)               { return *this -= rational(rhs); }
	rational& operator-=(long long rhs)          { return *this -= rational(rhs); }
	rational& operator-=(float rhs)              { return *this -= rational(rhs); }
	rational& operator-=(double rhs)             { return *this -= rational(rhs); }
	// in-place multiplication
	rational& operator*=(const rational& rhs) {
		n *= rhs.n;
		d *= rhs.d;
		normalize();
		return *this;
	}
	rational& operator*=(unsigned short rhs)     { return *this *= rational(rhs); }
	rational& operator*=(unsigned int rhs)       { return *this *= rational(rhs); }
	rational& operator*=(unsigned long rhs)      { return *this *= rational(rhs); }
	rational& operator*=(unsigned long long rhs) { return *this *= rational(rhs); }
	rational& operator*=(short rhs)              { return *this *= rational(rhs); }
	rational& operator*=(int rhs)                { return *this *= rational(rhs); }
	rational& operator*=(long rhs)               { return *this *= rational(rhs); }
	rational& operator*=(long long rhs)          { return *this *= rational(rhs); }
	rational& operator*=(float rhs)              { return *this *= rational(rhs); }
	rational& operator*=(double rhs)             { return *this *= rational(rhs); }
	// in-place division
	rational& operator/=(const rational& rhs) {
		n *= rhs.d;
		d *= rhs.n;
		normalize(); 
		return *this; 
	}
	rational& operator/=(unsigned short rhs)     { return *this /= rational(rhs); }
	rational& operator/=(unsigned int rhs)       { return *this /= rational(rhs); }
	rational& operator/=(unsigned long rhs)      { return *this /= rational(rhs); }
	rational& operator/=(unsigned long long rhs) { return *this /= rational(rhs); }
	rational& operator/=(short rhs)              { return *this /= rational(rhs); }
	rational& operator/=(int rhs)                { return *this /= rational(rhs); }
	rational& operator/=(long rhs)               { return *this /= rational(rhs); }
	rational& operator/=(long long rhs)          { return *this /= rational(rhs); }
	rational& operator/=(float rhs)              { return *this /= rational(rhs); }
	rational& operator/=(double rhs)             { return *this /= rational(rhs); }

	// modifiers
	constexpr void clear()  noexcept { n = 0; d = 1; }
	constexpr void setzero() noexcept { n = 0; d = 1; }
	constexpr void setnan() noexcept { n = 0; d = 0; }
	constexpr void set(const SignedBlockBinary& _n, const SignedBlockBinary& _d) noexcept { 
		n = _n; d = _d;
		normalize();
	}
	constexpr void setbits(std::int64_t bits) noexcept { n = bits; d = 1; }
	constexpr void setnbit(unsigned index) noexcept { n.set(index); }
	constexpr void setdbit(unsigned index) noexcept { d.set(index); }
	constexpr void resetnbit(unsigned index) noexcept { n.reset(index); }
	constexpr void resetdbit(unsigned index) noexcept { d.reset(index); }

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
		n.maxneg(); d = 1;
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
		n /= b;
		d /= b;
		if (sign && dsign) {
			// move the sign to the numerator
			n = -n; d = -d;
		}

	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// conversion helpers

	// convert to signed int
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	SignedInt to_signed() const { return static_cast<SignedInt>(n / d); }
	// convert to unsigned int
	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	UnsignedInt to_unsigned() const { return static_cast<UnsignedInt>(n / d); }
	// convert to ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real to_ieee754() const { return Real(n) / Real(d); }

	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	rational& convert_signed(SignedInt& rhs) {
		n = rhs;
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
		if (std::isnan(rhs)) {
			n = 0; d = 0;
			return *this;
		}
		if (rhs == 0.0) {
			n = 0; d = 1;
			return *this;
		}
		// extract components, convert mantissa to fraction with denominator 2^fbits, adjust fraction using scale, normalize
		uint64_t bits{ 0 };
		uint64_t e{ 0 }, f{ 0 };
		bool s{ false };
		extractFields(rhs, s, e, f, bits);
		int exponent = static_cast<int>(e - ieee754_parameter<Real>::bias);
		if (e == 0) { // subnormal
		}
		else { // normal
			uint64_t a = f | ieee754_parameter<Real>::hmask;
			uint64_t b = ieee754_parameter<Real>::hmask;

			//std::cout << "exponent = " << exponent << '\n';
			//std::cout << "a        = " << to_binary(a) << '\n';
			//std::cout << "b        = " << to_binary(b) << '\n';

			// remove any redundancy in the representation
			uint64_t rr{ 0 }, aa{ a }, bb{ b };
			while (aa % bb > 0ull) {
				rr = aa % bb;
				aa = bb;
				bb = rr;
			}
			a /= bb;
			b /= bb;

			if (exponent == 0 && a == b) {
				n = 1;
				d = 1;
			}
			else {
				/*
				* two cases:
				* exponent > 0
				*     we need to scale the numerator
				*         0000 0010 0100 0010  numerator
				*         0000 0010 0000 0000  denominator
				* we can shift the numerator up maximally (nbits - msb - 1) 
				* and after that we need to shift the denominator down maximally till the msb is on bit 0
				* 
				* exponent < 0
				*     we need to scale the denominator
				*         0000 0010 0100 0010  numerator
				*         0000 0010 0000 0000  denominator
				* we can shift the denominator up maximally (nbits - msb - 1)
				* and after that we need to shift the numerator down maximally till the msb is on bit 0
				*/
				// TODO: do we need to round the value or is it ok if we just throw the lower bits away?
				if (exponent >= 0) {

					// find the msb of the numerator value and shift it to the msb of the numerator size of this rational
					unsigned msb = find_msb(a);
					if (msb > nbits) {
						unsigned shift = 1u + msb - nbits; // one extra slot as we are shifting into a 2's complement encoding
						a >>= shift;
						b >>= shift;
					}

					//std::cout << "a        = " << to_binary(a) << '\n';
					//std::cout << "b        = " << to_binary(b) << '\n';

					// and finally scale the ratio

					msb = find_msb(a);  // find the msb of the numerator
					uint64_t maxUpShift = (nbits - msb - 1u);  // this will be 0 if we had to scale the ratio down to fit
					// find the new msb of the denominator to direct how we need to scale while avoiding overflow
					uint64_t maxDownShift = find_msb(b);
					uint64_t scale = static_cast<uint64_t>(exponent);
					if (scale >= 64) {
						// overflow, saturate to maxpos
						std::cerr << "overflow: scale = " << exponent << '\n';
						maxpos();
						return *this;
					}
					if (scale > maxUpShift) {
						if (scale > (maxUpShift + maxDownShift)) {
							// overflow, saturate to maxpos
							std::cerr << "overflow: scale = " << exponent << '\n';
							maxpos();
							return *this;
						}
						else {
							a <<= maxUpShift;
							b >>= (scale - maxUpShift);
						}
					}
					else {
						a <<= scale;
					}
				}
				else {
					// find the msb of the denominator value and shift it to the msb of the denominator size of this rational
					unsigned msb = find_msb(b);
					if (msb > nbits) {
						unsigned shift = 1u + msb - nbits; // one extra slot as we are shifting into a 2's complement encoding
						a >>= shift;
						b >>= shift;
					}

					//std::cout << "a        = " << to_binary(a) << '\n';
					//std::cout << "b        = " << to_binary(b) << '\n';

					// and finally scale the ratio

					msb = find_msb(b);  // find the msb of the denominator
					uint64_t maxUpShift = (nbits - msb - 1u);  // this will be 0 if we had to scale the ratio down to fit
					// find the new msb of the numerator to direct how we need to scale while avoiding underflow
					uint64_t maxDownShift = find_msb(a);
					uint64_t scale = static_cast<uint64_t>(-exponent);
					if (scale >= 64) {
						// underflow, saturate to maxpos
						std::cerr << "underflow: scale = " << exponent << '\n';
						setzero();
						return *this;
					}
					if (scale > maxUpShift) {
						if (scale > (maxUpShift + maxDownShift)) {
							// underflow, saturate to maxpos
							std::cerr << "underflow: scale = " << exponent << '\n';
							setzero();
							return *this;
						}
						else {
							b <<= maxUpShift;
							a >>= (scale - maxUpShift);
						}
					}
					else {
						b <<= scale;
					}

				}
				n = (s ? -static_cast<int64_t>(a) : static_cast<int64_t>(a));
				d = static_cast<int64_t>(b);
				normalize();
//				std::cout << "n        = " << to_binary(n) << '\n';
//				std::cout << "d        = " << to_binary(d) << '\n';
			}
		}
		return *this;
	}

private:
	SignedBlockBinary n; // numerator
	SignedBlockBinary d; // denominator, always positive so that sign of numerator is sign of rational

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const rational<nnbits,nbt>& r);
	template<unsigned nnbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, rational<nnbits,nbt>& r);

	template<unsigned nnbits, typename nbt>
	friend bool operator==(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator!=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator< (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator> (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator<=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator>=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// stream opreators

template<unsigned nnbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const rational<nnbits,nbt>& v) {
	return ostr << double(v);
}

template<unsigned nnbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const rational<nnbits,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nbits, typename bt>
inline std::string to_binary(const rational<nbits,bt>& v, bool nibbleMarker = true) {
	std::stringstream s;
	s << to_binary(v.numerator(), nibbleMarker)
		<< " / "
		<< to_binary(v.denominator(), nibbleMarker);
	return s.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// binary logic functions

template<unsigned nnbits, typename nbt>
inline bool operator==(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { 
	return (lhs.d == rhs.d) && (lhs.n == rhs.n);
}
template<unsigned nnbits, typename nbt>
inline bool operator!=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, typename nbt>
inline bool operator< (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) {
	// a / b is less than c / d when ad < bc
	// problem is that the products ad and bc can overflow, thus destroying the logic structure
	// so better is to take the hit and reduce to double, this will fail with some values but
	// provides a better cover than evaluating (ad < bc)
	return double(lhs) < double(rhs); 
}
template<unsigned nnbits, typename nbt>
inline bool operator> (const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, typename nbt>
inline bool operator<=(const rational<nnbits,nbt>& lhs, const rational<nnbits,nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, typename nbt>
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
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, signed char rhs) { return lhs + rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, short rhs)       { return lhs + rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, int rhs)         { return lhs + rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, long rhs)        { return lhs + rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, long long rhs)   { return lhs + rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, float rhs)       { return lhs + rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(const rational<nbits, bt>& lhs, double rhs)      { return lhs + rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(signed char lhs, const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) + rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(short lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) + rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(int lhs,         const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) + rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(long lhs,        const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) + rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(long long lhs,   const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) + rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(float lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) + rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator+(double lhs,      const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) + rhs; }

// BINARY SUBTRACTION
template<unsigned nbits, typename bt>
inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, const rational<nbits, bt>& rhs) {
	rational<nbits,bt> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, signed char rhs) { return lhs - rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, short rhs)       { return lhs - rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, int rhs)         { return lhs - rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, long rhs)        { return lhs - rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, long long rhs)   { return lhs - rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, float rhs)       { return lhs - rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(const rational<nbits, bt>& lhs, double rhs)      { return lhs - rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(signed char lhs, const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) - rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(short lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) - rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(int lhs,         const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) - rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(long lhs,        const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) - rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(long long lhs,   const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) - rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(float lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) - rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator-(double lhs,      const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) - rhs; }

// BINARY MULTIPLICATION
template<unsigned nbits, typename bt>
inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, const rational<nbits, bt>& rhs) {
	rational<nbits,bt> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, signed char rhs) { return lhs * rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, short rhs)       { return lhs * rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, int rhs)         { return lhs * rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, long rhs)        { return lhs * rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, long long rhs)   { return lhs * rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, float rhs)       { return lhs * rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(const rational<nbits, bt>& lhs, double rhs)      { return lhs * rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(signed char lhs, const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) * rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(short lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) * rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(int lhs,         const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) * rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(long lhs,        const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) * rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(long long lhs,   const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) * rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(float lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs)* rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator*(double lhs,      const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs)* rhs; }

// BINARY DIVISION
template<unsigned nbits, typename bt>
inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, const rational<nbits, bt>& rhs) {
	rational<nbits,bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, signed char rhs) { return lhs / rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, short rhs)       { return lhs / rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, int rhs)         { return lhs / rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, long rhs)        { return lhs / rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, long long rhs)   { return lhs / rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, float rhs)       { return lhs / rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(const rational<nbits, bt>& lhs, double rhs)      { return lhs / rational<nbits, bt>(rhs); }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(signed char lhs, const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) / rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(short lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) / rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(int lhs,         const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) / rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(long lhs,        const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) / rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(long long lhs,   const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) / rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(float lhs,       const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) / rhs; }
template<unsigned nbits, typename bt> inline rational<nbits, bt> operator/(double lhs,      const rational<nbits, bt>& rhs) { return rational<nbits, bt>(lhs) / rhs; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// math functions

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, typename bt>
rational<nbits,bt> abs(const rational<nbits,bt>& v) {
	return rational<nbits,bt>();
}


}}  // namespace sw::universal
