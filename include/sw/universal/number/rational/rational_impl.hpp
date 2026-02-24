#pragma once
// rational_impl.hpp: definition of a multi-radix rational arithmetic type
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
#include <universal/internal/blockdigit/blockdigit.hpp>

// Forward definitions
#include <universal/number/rational/rational_fwd.hpp>

namespace sw {	namespace universal {

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// convert a floating-point value to a specific rational configuration. Semantically, p = v, return reference to p
template<unsigned nbits, typename Base, typename bt>
inline rational<nbits, Base, bt>& convert(const triple<nbits,bt>& v, rational<nbits, Base, bt>& p) {
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

template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt>& minpos(rational<nbits, Base, bt>& r) {
	return r.minpos();
}
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt>& maxpos(rational<nbits, Base, bt>& r) {
	return r.maxpos();
}
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt>& minneg(rational<nbits, Base, bt>& r) {
	return r.minneg();
}
template<unsigned nbits, typename Base, typename bt>
rational<nbits, Base, bt>& maxneg(rational<nbits, Base, bt>& r) {
	return r.maxneg();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Binary rational specialization: rational<nbits, base2, bt>
// Stores numerator and denominator as blockbinary<nbits, bt, Signed>
//
template<unsigned _nbits, typename bt>
class rational<_nbits, base2, bt> {
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
			break;
		case SpecificValue::minpos:
			break;
		case SpecificValue::zero:
		default:
			break;
		case SpecificValue::minneg:
			break;
		case SpecificValue::maxneg:
			break;
		case SpecificValue::infpos:
			break;
		case SpecificValue::infneg:
			break;
		case SpecificValue::nar:
		case SpecificValue::qnan:
		case SpecificValue::snan:
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
	// convenience: set numerator and denominator from integer values
	constexpr void set(long long _n, long long _d) noexcept {
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
		n.maxpos(); d = 1;
		return *this;
	}
	constexpr rational& minpos() noexcept {
		n = 1; d.maxpos();
		return *this;
	}
	constexpr rational& zero() noexcept {
		n = 0; d = 1;
		return *this;
	}
	constexpr rational& minneg() noexcept {
		n = -1; d.maxpos();
		return *this;
	}
	constexpr rational& maxneg() noexcept {
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
		a = (nsign ? -n : n); b = (dsign ? -d : d);

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
			n = -n; d = -d;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// conversion helpers

	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	SignedInt to_signed() const { return static_cast<SignedInt>(n / d); }
	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	UnsignedInt to_unsigned() const { return static_cast<UnsignedInt>(n / d); }
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
				if (exponent >= 0) {
					unsigned msb = find_msb(a);
					if (msb > nbits) {
						unsigned shift = 1u + msb - nbits;
						a >>= shift;
						b >>= shift;
					}
					msb = find_msb(a);
					uint64_t maxUpShift = (nbits - msb - 1u);
					uint64_t maxDownShift = find_msb(b);
					uint64_t scale = static_cast<uint64_t>(exponent);
					if (scale >= 64) {
						std::cerr << "overflow: scale = " << exponent << '\n';
						maxpos();
						return *this;
					}
					if (scale > maxUpShift) {
						if (scale > (maxUpShift + maxDownShift)) {
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
					unsigned msb = find_msb(b);
					if (msb > nbits) {
						unsigned shift = 1u + msb - nbits;
						a >>= shift;
						b >>= shift;
					}
					msb = find_msb(b);
					uint64_t maxUpShift = (nbits - msb - 1u);
					uint64_t maxDownShift = find_msb(a);
					uint64_t scale = static_cast<uint64_t>(-exponent);
					if (scale >= 64) {
						std::cerr << "underflow: scale = " << exponent << '\n';
						setzero();
						return *this;
					}
					if (scale > maxUpShift) {
						if (scale > (maxUpShift + maxDownShift)) {
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
			}
		}
		return *this;
	}

private:
	SignedBlockBinary n; // numerator
	SignedBlockBinary d; // denominator

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, typename nBase, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const rational<nnbits,nBase,nbt>& r);
	template<unsigned nnbits, typename nBase, typename nbt>
	friend std::istream& operator>> (std::istream& istr, rational<nnbits,nBase,nbt>& r);

	template<unsigned nnbits, typename nBase, typename nbt>
	friend bool operator==(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs);
	template<unsigned nnbits, typename nBase, typename nbt>
	friend bool operator!=(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs);
	template<unsigned nnbits, typename nBase, typename nbt>
	friend bool operator< (const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs);
	template<unsigned nnbits, typename nBase, typename nbt>
	friend bool operator> (const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs);
	template<unsigned nnbits, typename nBase, typename nbt>
	friend bool operator<=(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs);
	template<unsigned nnbits, typename nBase, typename nbt>
	friend bool operator>=(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Digit-based rational specializations: rational<ndigits, base8/base10/base16, bt>
// Stores numerator and denominator as blockdigit<ndigits, Base::radix>
//
template<unsigned _ndigits, typename bt>
class rational<_ndigits, base8, bt> {
public:
	static constexpr unsigned ndigits = _ndigits;
	using Component = blockdigit<ndigits, 8>;

	rational() = default;
	rational(const rational&) = default;
	rational(rational&&) = default;
	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	rational(signed char v)        { *this = static_cast<long long>(v); }
	rational(short v)              { *this = static_cast<long long>(v); }
	rational(int v)                { *this = static_cast<long long>(v); }
	rational(long v)               { *this = static_cast<long long>(v); }
	rational(long long v)          { n = v; d = 1; }
	rational(unsigned char v)      { *this = static_cast<unsigned long long>(v); }
	rational(unsigned short v)     { *this = static_cast<unsigned long long>(v); }
	rational(unsigned int v)       { *this = static_cast<unsigned long long>(v); }
	rational(unsigned long v)      { *this = static_cast<unsigned long long>(v); }
	rational(unsigned long long v) { n = v; d = 1; }
	rational(float v)              { n = static_cast<long long>(v); d = 1; }
	rational(double v)             { n = static_cast<long long>(v); d = 1; }

	rational& operator=(signed char rhs)        { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(short rhs)              { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(int rhs)                { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(long rhs)               { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(long long rhs)          { n = rhs; d = 1; return *this; }
	rational& operator=(unsigned char rhs)      { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned short rhs)     { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned int rhs)       { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned long rhs)      { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned long long rhs) { n = rhs; d = 1; return *this; }
	rational& operator=(float rhs)              { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(double rhs)             { n = static_cast<long long>(rhs); d = 1; return *this; }

	explicit operator int()       const noexcept { return static_cast<int>(to_double()); }
	explicit operator long()      const noexcept { return static_cast<long>(to_double()); }
	explicit operator long long() const noexcept { return static_cast<long long>(to_double()); }
	explicit operator float()     const noexcept { return static_cast<float>(to_double()); }
	explicit operator double()    const noexcept { return to_double(); }

	rational operator-() const { rational tmp; tmp.n = -n; tmp.d = d; return tmp; }

	rational& operator+=(const rational& rhs) { n = n * rhs.d + d * rhs.n; d = d * rhs.d; normalize(); return *this; }
	rational& operator-=(const rational& rhs) { n = n * rhs.d - d * rhs.n; d = d * rhs.d; normalize(); return *this; }
	rational& operator*=(const rational& rhs) { n *= rhs.n; d *= rhs.d; normalize(); return *this; }
	rational& operator/=(const rational& rhs) { n *= rhs.d; d *= rhs.n; normalize(); return *this; }

	void clear() { n = 0; d = 1; }
	void setzero() { n = 0; d = 1; }
	void setnan() { n = 0; d = 0; }
	void set(long long _n, long long _d) { n = _n; d = _d; normalize(); }
	void setbits(int64_t bits) { n = bits; d = 1; }

	rational& maxpos() { n = Component(); for (unsigned i = 0; i < ndigits; ++i) n.setdigit(i, 8 - 1); d = 1; return *this; }
	rational& minpos() { n = 1; d = Component(); for (unsigned i = 0; i < ndigits; ++i) d.setdigit(i, 8 - 1); return *this; }
	rational& zero() { n = 0; d = 1; return *this; }
	rational& minneg() { minpos(); n = -n; return *this; }
	rational& maxneg() { maxpos(); n = -n; return *this; }

	bool iszero() const { return n.iszero() && !d.iszero(); }
	bool isneg()  const { return n.isneg(); }
	bool isinf()  const { return false; }
	bool isnan()  const { return n.iszero() && d.iszero(); }
	bool sign()   const { return n.sign(); }
	int  scale()  const { double v = to_double(); return (v == 0.0) ? 0 : static_cast<int>(std::floor(std::log2(std::abs(v)))); }

	Component numerator() const { return n; }
	Component denominator() const { return d; }

private:
	Component n;
	Component d;

	void normalize() {
		if (d.iszero()) return;
		// ensure denominator is positive
		if (d.isneg()) { n = -n; d = -d; }
		// GCD normalization
		Component a = n.isneg() ? -n : n;
		Component b = d;
		Component zero_val(0);
		while (!(a % b == zero_val)) {
			Component r = a % b;
			a = b;
			b = r;
		}
		n /= b;
		d /= b;
	}

	double to_double() const { return static_cast<double>(n) / static_cast<double>(d); }
};

template<unsigned _ndigits, typename bt>
class rational<_ndigits, base10, bt> {
public:
	static constexpr unsigned ndigits = _ndigits;
	using Component = blockdigit<ndigits, 10>;

	rational() = default;
	rational(const rational&) = default;
	rational(rational&&) = default;
	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	rational(signed char v)        { *this = static_cast<long long>(v); }
	rational(short v)              { *this = static_cast<long long>(v); }
	rational(int v)                { *this = static_cast<long long>(v); }
	rational(long v)               { *this = static_cast<long long>(v); }
	rational(long long v)          { n = v; d = 1; }
	rational(unsigned char v)      { *this = static_cast<unsigned long long>(v); }
	rational(unsigned short v)     { *this = static_cast<unsigned long long>(v); }
	rational(unsigned int v)       { *this = static_cast<unsigned long long>(v); }
	rational(unsigned long v)      { *this = static_cast<unsigned long long>(v); }
	rational(unsigned long long v) { n = v; d = 1; }
	rational(float v)              { n = static_cast<long long>(v); d = 1; }
	rational(double v)             { n = static_cast<long long>(v); d = 1; }

	rational& operator=(signed char rhs)        { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(short rhs)              { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(int rhs)                { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(long rhs)               { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(long long rhs)          { n = rhs; d = 1; return *this; }
	rational& operator=(unsigned char rhs)      { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned short rhs)     { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned int rhs)       { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned long rhs)      { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned long long rhs) { n = rhs; d = 1; return *this; }
	rational& operator=(float rhs)              { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(double rhs)             { n = static_cast<long long>(rhs); d = 1; return *this; }

	explicit operator int()       const noexcept { return static_cast<int>(to_double()); }
	explicit operator long()      const noexcept { return static_cast<long>(to_double()); }
	explicit operator long long() const noexcept { return static_cast<long long>(to_double()); }
	explicit operator float()     const noexcept { return static_cast<float>(to_double()); }
	explicit operator double()    const noexcept { return to_double(); }

	rational operator-() const { rational tmp; tmp.n = -n; tmp.d = d; return tmp; }

	rational& operator+=(const rational& rhs) { n = n * rhs.d + d * rhs.n; d = d * rhs.d; normalize(); return *this; }
	rational& operator-=(const rational& rhs) { n = n * rhs.d - d * rhs.n; d = d * rhs.d; normalize(); return *this; }
	rational& operator*=(const rational& rhs) { n *= rhs.n; d *= rhs.d; normalize(); return *this; }
	rational& operator/=(const rational& rhs) { n *= rhs.d; d *= rhs.n; normalize(); return *this; }

	void clear() { n = 0; d = 1; }
	void setzero() { n = 0; d = 1; }
	void setnan() { n = 0; d = 0; }
	void set(long long _n, long long _d) { n = _n; d = _d; normalize(); }
	void setbits(int64_t bits) { n = bits; d = 1; }

	rational& maxpos() { n = Component(); for (unsigned i = 0; i < ndigits; ++i) n.setdigit(i, 9); d = 1; return *this; }
	rational& minpos() { n = 1; d = Component(); for (unsigned i = 0; i < ndigits; ++i) d.setdigit(i, 9); return *this; }
	rational& zero() { n = 0; d = 1; return *this; }
	rational& minneg() { minpos(); n = -n; return *this; }
	rational& maxneg() { maxpos(); n = -n; return *this; }

	bool iszero() const { return n.iszero() && !d.iszero(); }
	bool isneg()  const { return n.isneg(); }
	bool isinf()  const { return false; }
	bool isnan()  const { return n.iszero() && d.iszero(); }
	bool sign()   const { return n.sign(); }
	int  scale()  const { double v = to_double(); return (v == 0.0) ? 0 : static_cast<int>(std::floor(std::log2(std::abs(v)))); }

	Component numerator() const { return n; }
	Component denominator() const { return d; }

private:
	Component n;
	Component d;

	void normalize() {
		if (d.iszero()) return;
		if (d.isneg()) { n = -n; d = -d; }
		Component a = n.isneg() ? -n : n;
		Component b = d;
		Component zero_val(0);
		while (!(a % b == zero_val)) {
			Component r = a % b;
			a = b;
			b = r;
		}
		n /= b;
		d /= b;
	}

	double to_double() const { return static_cast<double>(n) / static_cast<double>(d); }
};

template<unsigned _ndigits, typename bt>
class rational<_ndigits, base16, bt> {
public:
	static constexpr unsigned ndigits = _ndigits;
	using Component = blockdigit<ndigits, 16>;

	rational() = default;
	rational(const rational&) = default;
	rational(rational&&) = default;
	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	rational(signed char v)        { *this = static_cast<long long>(v); }
	rational(short v)              { *this = static_cast<long long>(v); }
	rational(int v)                { *this = static_cast<long long>(v); }
	rational(long v)               { *this = static_cast<long long>(v); }
	rational(long long v)          { n = v; d = 1; }
	rational(unsigned char v)      { *this = static_cast<unsigned long long>(v); }
	rational(unsigned short v)     { *this = static_cast<unsigned long long>(v); }
	rational(unsigned int v)       { *this = static_cast<unsigned long long>(v); }
	rational(unsigned long v)      { *this = static_cast<unsigned long long>(v); }
	rational(unsigned long long v) { n = v; d = 1; }
	rational(float v)              { n = static_cast<long long>(v); d = 1; }
	rational(double v)             { n = static_cast<long long>(v); d = 1; }

	rational& operator=(signed char rhs)        { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(short rhs)              { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(int rhs)                { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(long rhs)               { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(long long rhs)          { n = rhs; d = 1; return *this; }
	rational& operator=(unsigned char rhs)      { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned short rhs)     { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned int rhs)       { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned long rhs)      { n = static_cast<unsigned long long>(rhs); d = 1; return *this; }
	rational& operator=(unsigned long long rhs) { n = rhs; d = 1; return *this; }
	rational& operator=(float rhs)              { n = static_cast<long long>(rhs); d = 1; return *this; }
	rational& operator=(double rhs)             { n = static_cast<long long>(rhs); d = 1; return *this; }

	explicit operator int()       const noexcept { return static_cast<int>(to_double()); }
	explicit operator long()      const noexcept { return static_cast<long>(to_double()); }
	explicit operator long long() const noexcept { return static_cast<long long>(to_double()); }
	explicit operator float()     const noexcept { return static_cast<float>(to_double()); }
	explicit operator double()    const noexcept { return to_double(); }

	rational operator-() const { rational tmp; tmp.n = -n; tmp.d = d; return tmp; }

	rational& operator+=(const rational& rhs) { n = n * rhs.d + d * rhs.n; d = d * rhs.d; normalize(); return *this; }
	rational& operator-=(const rational& rhs) { n = n * rhs.d - d * rhs.n; d = d * rhs.d; normalize(); return *this; }
	rational& operator*=(const rational& rhs) { n *= rhs.n; d *= rhs.d; normalize(); return *this; }
	rational& operator/=(const rational& rhs) { n *= rhs.d; d *= rhs.n; normalize(); return *this; }

	void clear() { n = 0; d = 1; }
	void setzero() { n = 0; d = 1; }
	void setnan() { n = 0; d = 0; }
	void set(long long _n, long long _d) { n = _n; d = _d; normalize(); }
	void setbits(int64_t bits) { n = bits; d = 1; }

	rational& maxpos() { n = Component(); for (unsigned i = 0; i < ndigits; ++i) n.setdigit(i, 15); d = 1; return *this; }
	rational& minpos() { n = 1; d = Component(); for (unsigned i = 0; i < ndigits; ++i) d.setdigit(i, 15); return *this; }
	rational& zero() { n = 0; d = 1; return *this; }
	rational& minneg() { minpos(); n = -n; return *this; }
	rational& maxneg() { maxpos(); n = -n; return *this; }

	bool iszero() const { return n.iszero() && !d.iszero(); }
	bool isneg()  const { return n.isneg(); }
	bool isinf()  const { return false; }
	bool isnan()  const { return n.iszero() && d.iszero(); }
	bool sign()   const { return n.sign(); }
	int  scale()  const { double v = to_double(); return (v == 0.0) ? 0 : static_cast<int>(std::floor(std::log2(std::abs(v)))); }

	Component numerator() const { return n; }
	Component denominator() const { return d; }

private:
	Component n;
	Component d;

	void normalize() {
		if (d.iszero()) return;
		if (d.isneg()) { n = -n; d = -d; }
		Component a = n.isneg() ? -n : n;
		Component b = d;
		Component zero_val(0);
		while (!(a % b == zero_val)) {
			Component r = a % b;
			a = b;
			b = r;
		}
		n /= b;
		d /= b;
	}

	double to_double() const { return static_cast<double>(n) / static_cast<double>(d); }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// stream operators

template<unsigned nnbits, typename nBase, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const rational<nnbits,nBase,nbt>& v) {
	return ostr << double(v);
}

template<unsigned nnbits, typename nBase, typename nbt>
inline std::istream& operator>>(std::istream& istr, const rational<nnbits,nBase,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nbits, typename Base, typename bt>
inline std::string to_binary(const rational<nbits,Base,bt>& v, bool nibbleMarker = true) {
	std::stringstream s;
	s << to_binary(v.numerator(), nibbleMarker)
		<< " / "
		<< to_binary(v.denominator(), nibbleMarker);
	return s.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// binary logic functions

template<unsigned nnbits, typename nBase, typename nbt>
inline bool operator==(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs) {
	return double(lhs) == double(rhs);
}
template<unsigned nnbits, typename nBase, typename nbt>
inline bool operator!=(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, typename nBase, typename nbt>
inline bool operator< (const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs) {
	return double(lhs) < double(rhs);
}
template<unsigned nnbits, typename nBase, typename nbt>
inline bool operator> (const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, typename nBase, typename nbt>
inline bool operator<=(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, typename nBase, typename nbt>
inline bool operator>=(const rational<nnbits,nBase,nbt>& lhs, const rational<nnbits,nBase,nbt>& rhs) { return !operator< (lhs, rhs); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
///  binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, typename Base, typename bt>
inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, const rational<nbits, Base, bt>& rhs) {
	rational<nbits,Base,bt> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, signed char rhs) { return lhs + rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, short rhs)       { return lhs + rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, int rhs)         { return lhs + rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, long rhs)        { return lhs + rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, long long rhs)   { return lhs + rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, float rhs)       { return lhs + rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(const rational<nbits, Base, bt>& lhs, double rhs)      { return lhs + rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(signed char lhs, const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) + rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(short lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) + rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(int lhs,         const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) + rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(long lhs,        const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) + rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(long long lhs,   const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) + rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(float lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) + rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator+(double lhs,      const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) + rhs; }

// BINARY SUBTRACTION
template<unsigned nbits, typename Base, typename bt>
inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, const rational<nbits, Base, bt>& rhs) {
	rational<nbits,Base,bt> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, signed char rhs) { return lhs - rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, short rhs)       { return lhs - rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, int rhs)         { return lhs - rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, long rhs)        { return lhs - rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, long long rhs)   { return lhs - rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, float rhs)       { return lhs - rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(const rational<nbits, Base, bt>& lhs, double rhs)      { return lhs - rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(signed char lhs, const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) - rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(short lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) - rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(int lhs,         const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) - rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(long lhs,        const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) - rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(long long lhs,   const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) - rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(float lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) - rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator-(double lhs,      const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) - rhs; }

// BINARY MULTIPLICATION
template<unsigned nbits, typename Base, typename bt>
inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, const rational<nbits, Base, bt>& rhs) {
	rational<nbits,Base,bt> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, signed char rhs) { return lhs * rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, short rhs)       { return lhs * rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, int rhs)         { return lhs * rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, long rhs)        { return lhs * rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, long long rhs)   { return lhs * rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, float rhs)       { return lhs * rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(const rational<nbits, Base, bt>& lhs, double rhs)      { return lhs * rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(signed char lhs, const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) * rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(short lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) * rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(int lhs,         const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) * rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(long lhs,        const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) * rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(long long lhs,   const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) * rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(float lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs)* rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator*(double lhs,      const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs)* rhs; }

// BINARY DIVISION
template<unsigned nbits, typename Base, typename bt>
inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, const rational<nbits, Base, bt>& rhs) {
	rational<nbits,Base,bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, signed char rhs) { return lhs / rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, short rhs)       { return lhs / rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, int rhs)         { return lhs / rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, long rhs)        { return lhs / rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, long long rhs)   { return lhs / rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, float rhs)       { return lhs / rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(const rational<nbits, Base, bt>& lhs, double rhs)      { return lhs / rational<nbits, Base, bt>(rhs); }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(signed char lhs, const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) / rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(short lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) / rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(int lhs,         const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) / rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(long lhs,        const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) / rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(long long lhs,   const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) / rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(float lhs,       const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) / rhs; }
template<unsigned nbits, typename Base, typename bt> inline rational<nbits, Base, bt> operator/(double lhs,      const rational<nbits, Base, bt>& rhs) { return rational<nbits, Base, bt>(lhs) / rhs; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// math functions

template<unsigned nbits, typename Base, typename bt>
rational<nbits,Base,bt> abs(const rational<nbits,Base,bt>& v) {
	return (v.isneg() ? -v : v);
}


}}  // namespace sw::universal
