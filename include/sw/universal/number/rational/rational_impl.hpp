#pragma once
// RATIONAL_THROW_ARITHMETIC_EXCEPTION: throw specific exceptions on arithmetic errors, left to the
// application to enable. The default lives here rather than in the rational.hpp umbrella,
// so that including core.hpp alone defines it as well (#1436).
#if !defined(RATIONAL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define RATIONAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

#include <cstdint>      // std::int64_t
#include <type_traits>  // std::enable_if / std::is_floating_point
#include <iosfwd>       // std::ostream/std::istream in the friend declarations (#1334)
#include <cstdio>       // fprintf(stderr,...) for the diagnostics
// rational_impl.hpp: definition of a multi-radix rational arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <cmath>
#include <limits>

#include <universal/native/ieee754_core.hpp>          // extractFields/ieee754_parameter; the text half is not needed here (#1334)
#include <universal/native/manipulators_core.hpp>   // scale() and find_msb(), without the to_triple/to_hex text layer
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>
#include <universal/internal/blockdigit/blockdigit.hpp>
#include <universal/number/rational/best_rational.hpp>  // the closest rational p / q to a native value (#1523, #1526)

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
	// negate the value, which is not always the numerator's job: -2^(nbits-1) has no positive twin
	// in nbits, so for those the denominator carries the sign instead and -(-8/7) is -8/-7, the
	// value 8/7 exactly. Only a pair that is the signed minimum over the signed minimum has neither
	// field to give, and normalize() reduces that one away (#1525).
	rational operator-() const {
		SignedBlockBinary negated = -n;
		if (!n.iszero() && negated.sign() == n.sign()) {   // negating the numerator overflowed
			SignedBlockBinary flipped = -d;
			if (d.iszero() || flipped.sign() != d.sign()) return rational(n, flipped);
			// both fields are the signed minimum, so neither can carry the sign. Only the raw
			// constructor builds such a pair, since it does not normalize; reducing it takes both
			// fields off the minimum -- -8/-8 becomes 1/1 -- and then the numerator can be negated.
			rational reduced(n, d);
			reduced.normalize();
			return -reduced;
		}
		return rational(negated, d);
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
	// the sign lives in the pair, not in the numerator alone: normalize() keeps a negative
	// denominator whenever moving its sign would take a field out of range, so -8/-7 in a
	// rational<4> is the positive value 8/7 (#1525)
	constexpr bool isneg()  const noexcept { return (n.sign() != d.sign()) && !n.iszero(); }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool isnan()  const noexcept { return (n.iszero() && d.iszero()); }
	constexpr bool sign()   const noexcept { return isneg(); }
	constexpr int  scale()  const noexcept { return sw::universal::scale(double(n) / double(d)); }

	SignedBlockBinary numerator() const noexcept { return n; }
	SignedBlockBinary denominator() const noexcept { return d; }

protected:
	// HELPER methods

	// remove greatest common divisor out of the numerator/denominator pair
	//
	// The reduction runs one bit wider than the fields. In nbits the signed minimum -2^(nbits-1)
	// negates to itself, so taking |n| in place left the Euclidean loop with a negative operand and
	// it reduced the pair to nonsense: set(-8, 7) in a rational<4> came out as -1/1 (#1525). One
	// extra bit holds every magnitude, and the reduced pair always fits back into nbits, since
	// dividing by the gcd cannot grow either field.
	void normalize() {
		if (d.iszero()) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
			throw rational_divide_by_zero();
#else
			std::fprintf(stderr, "rational_divide_by_zero\n");
			d = 0;
			n = 0;
			return;
#endif
		}
		using WideBlockBinary = blockbinary<nbits + 1, bt, BinaryNumberType::Signed>;
		auto widen = [](const SignedBlockBinary& v) {            // sign extend into the extra bit
			WideBlockBinary w;
			w.clear();
			for (unsigned i = 0; i < nbits; ++i)
				if (v.test(i)) w.setbit(i);
			if (v.sign()) w.setbit(nbits);
			return w;
		};
		auto narrow = [](const WideBlockBinary& w, SignedBlockBinary& v) {
			v.clear();
			for (unsigned i = 0; i < nbits; ++i)
				if (w.test(i)) v.setbit(i);
		};

		WideBlockBinary wn = widen(n), wd = widen(d);
		WideBlockBinary a = wn, b = wd;
		if (a.sign()) a.twosComplement();
		if (b.sign()) b.twosComplement();
		WideBlockBinary zero{ 0 };
		while (a % b > zero) {                                   // Euclid: b ends up the gcd
			WideBlockBinary r = a % b;
			a = b;
			b = r;
		}
		wn /= b;
		wd /= b;
		// canonical form keeps the denominator positive, but negating the pair can take either field
		// out of range: a magnitude of 2^(nbits-1) is representable only as a negative value. When
		// the flipped pair does not fit, this one stays as it is; the value is the same either way.
		auto fits = [](const WideBlockBinary& w) {               // sign extension still consistent
			return w.test(nbits) == w.test(nbits - 1);
		};
		if (wd.sign()) {
			WideBlockBinary flippedN = wn, flippedD = wd;
			flippedN.twosComplement();
			flippedD.twosComplement();
			if (fits(flippedN) && fits(flippedD)) {
				wn = flippedN;
				wd = flippedD;
			}
		}
		narrow(wn, n);
		narrow(wd, d);
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
		// the closest rational this type can hold, from best_rational.hpp. The conversion used to
		// build the exact dyadic fraction and shift it to fit, which is a different, worse rational:
		// rational<8> of 1/3's double came out as 21/64 (#1523).
		constexpr unsigned wideBits = 64u * (nbits / 64u + 3u);  // the bound, a 113-bit significand, and room
		using Wide = blockbinary<wideBits, bt, BinaryNumberType::Signed>;
		Wide qBound;  // the largest denominator, and the largest positive numerator: 2^(nbits-1) - 1
		qBound.clear();
		for (unsigned i = 0; i + 1 < nbits; ++i) qBound.setbit(i);
		// two's complement reaches one further down, so a negative value gets -2^(nbits-1) as well:
		// rational<4> of -8/7 used to come back as -7/6 (#1525)
		Wide pBound{ qBound };
		if (std::signbit(rhs)) {
			pBound.clear();
			pBound.setbit(nbits - 1);
		}
		bool negative{ false };
		Wide p, q;
		switch (best_rational_from_native<Wide, Real>(rhs, pBound, qBound, negative, p, q)) {
		case rational_conversion::nan:
			n = 0;
			d = 0;
			return *this;
		case rational_conversion::infinite:
		case rational_conversion::overflow:
			if (negative) maxneg(); else maxpos();
			return *this;
		case rational_conversion::zero:
			setzero();
			return *this;
		case rational_conversion::finite:
		default:
			break;
		}
		// the low nbits bits, two's complement for a negative numerator
		if (negative) p.twosComplement();
		n.clear();
		d.clear();
		for (unsigned i = 0; i < nbits; ++i) {
			if (p.test(i)) n.setbit(i);
			if (q.test(i)) d.setbit(i);
		}
		normalize();
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
	static constexpr unsigned nbits   = _ndigits;  // alias to make generic code easier to write
	using Component = blockdigit<ndigits, 8>;

	rational() = default;
	rational(const rational&) = default;
	rational(rational&&) = default;
	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	// decorated constructor
	constexpr rational(const std::int64_t& _n, const std::int64_t& _d) : n{_n}, d{_d} {};
	constexpr rational(const Component& _n, const Component& _d) : n{_n}, d{_d} {};

	// specific value constructor
	constexpr rational(const SpecificValue code) noexcept : n{}, d{} {
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
	rational(float v)              { *this = static_cast<double>(v); }
	rational(double v)             { *this = v; }

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
	rational& operator=(float rhs)              { return convert_ieee754(rhs); }
	rational& operator=(double rhs)             { return convert_ieee754(rhs); }
#if LONG_DOUBLE_SUPPORT
	rational& operator=(long double rhs)        { return convert_ieee754(rhs); }
#endif

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

	rational& maxpos() { n = Component(); n.clear(); for (unsigned i = 0; i < ndigits; ++i) n.setdigit(i, 8 - 1); d = 1; return *this; }
	rational& minpos() { n = 1; d = Component(); d.clear(); for (unsigned i = 0; i < ndigits; ++i) d.setdigit(i, 8 - 1); return *this; }
	rational& zero() { n = 0; d = 1; return *this; }
	rational& minneg() { minpos(); n = -n; return *this; }
	rational& maxneg() { maxpos(); n = -n; return *this; }

	bool iszero() const { return n.iszero() && !d.iszero(); }
	// the sign lives in the pair: the raw two-component constructor can hand this type a negative
	// denominator, which normalize() has not seen (#1525)
	bool isneg()  const { return (n.sign() != d.sign()) && !n.iszero(); }
	bool isinf()  const { return false; }
	bool isnan()  const { return n.iszero() && d.iszero(); }
	bool sign()   const { return isneg(); }
	int  scale()  const { double v = to_double(); return (v == 0.0) ? 0 : static_cast<int>(std::floor(std::log2(std::abs(v)))); }

	Component numerator() const { return n; }
	Component denominator() const { return d; }

private:
	Component n;
	Component d;

	// The closest rational this type can hold, from best_rational.hpp. This used to truncate to an
	// integer -- n = (long long)rhs, d = 1 -- so 0.75 became 0/1 (#1526).
	template<typename Real>
	rational& convert_ieee754(Real rhs) noexcept {
		constexpr unsigned digitBits = (Component::radix <= 8u ? 3u : 4u);   // bits per digit
		constexpr unsigned wideBits  = 64u * ((ndigits * digitBits) / 64u + 4u);  // the bound, a 113-bit significand, and room
		using Wide = blockbinary<wideBits, bt, BinaryNumberType::Signed>;
		Wide bound, radixWide, one;
		bound.clear();
		radixWide.clear();
		one.clear();
		bound.setbit(0);
		one.setbit(0);
		radixWide.setbits(Component::radix);
		for (unsigned i = 0; i < ndigits; ++i) bound = bound * radixWide;  // radix^ndigits
		bound = bound - one;                                              // the largest component
		bool negative{ false };
		Wide p, q;
		switch (best_rational_from_native<Wide, Real>(rhs, bound, negative, p, q)) {
		case rational_conversion::nan:
			setnan();
			return *this;
		case rational_conversion::infinite:
		case rational_conversion::overflow:
			if (negative) maxneg(); else maxpos();
			return *this;
		case rational_conversion::zero:
			setzero();
			return *this;
		case rational_conversion::finite:
		default:
			break;
		}
		n = to_digit_component<Component>(p);
		if (negative) n = -n;
		d = to_digit_component<Component>(q);
		normalize();
		return *this;
	}

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
	static constexpr unsigned nbits   = _ndigits;  // alias to make generic code easier to write
	using Component = blockdigit<ndigits, 10>;

	rational() = default;
	rational(const rational&) = default;
	rational(rational&&) = default;
	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	// decorated constructor
	constexpr rational(const std::int64_t& _n, const std::int64_t& _d) : n{_n}, d{_d} {};
	constexpr rational(const Component& _n, const Component& _d) : n{_n}, d{_d} {};

	// specific value constructor
	constexpr rational(const SpecificValue code) noexcept : n{}, d{} {
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
	rational(float v)              { *this = static_cast<double>(v); }
	rational(double v)             { *this = v; }

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
	rational& operator=(float rhs)              { return convert_ieee754(rhs); }
	rational& operator=(double rhs)             { return convert_ieee754(rhs); }
#if LONG_DOUBLE_SUPPORT
	rational& operator=(long double rhs)        { return convert_ieee754(rhs); }
#endif

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

	rational& maxpos() { n = Component(); n.clear(); for (unsigned i = 0; i < ndigits; ++i) n.setdigit(i, 9); d = 1; return *this; }
	rational& minpos() { n = 1; d = Component(); d.clear(); for (unsigned i = 0; i < ndigits; ++i) d.setdigit(i, 9); return *this; }
	rational& zero() { n = 0; d = 1; return *this; }
	rational& minneg() { minpos(); n = -n; return *this; }
	rational& maxneg() { maxpos(); n = -n; return *this; }

	bool iszero() const { return n.iszero() && !d.iszero(); }
	// the sign lives in the pair: the raw two-component constructor can hand this type a negative
	// denominator, which normalize() has not seen (#1525)
	bool isneg()  const { return (n.sign() != d.sign()) && !n.iszero(); }
	bool isinf()  const { return false; }
	bool isnan()  const { return n.iszero() && d.iszero(); }
	bool sign()   const { return isneg(); }
	int  scale()  const { double v = to_double(); return (v == 0.0) ? 0 : static_cast<int>(std::floor(std::log2(std::abs(v)))); }

	Component numerator() const { return n; }
	Component denominator() const { return d; }

private:
	Component n;
	Component d;

	// The closest rational this type can hold, from best_rational.hpp. This used to truncate to an
	// integer -- n = (long long)rhs, d = 1 -- so 0.75 became 0/1 (#1526).
	template<typename Real>
	rational& convert_ieee754(Real rhs) noexcept {
		constexpr unsigned digitBits = (Component::radix <= 8u ? 3u : 4u);   // bits per digit
		constexpr unsigned wideBits  = 64u * ((ndigits * digitBits) / 64u + 4u);  // the bound, a 113-bit significand, and room
		using Wide = blockbinary<wideBits, bt, BinaryNumberType::Signed>;
		Wide bound, radixWide, one;
		bound.clear();
		radixWide.clear();
		one.clear();
		bound.setbit(0);
		one.setbit(0);
		radixWide.setbits(Component::radix);
		for (unsigned i = 0; i < ndigits; ++i) bound = bound * radixWide;  // radix^ndigits
		bound = bound - one;                                              // the largest component
		bool negative{ false };
		Wide p, q;
		switch (best_rational_from_native<Wide, Real>(rhs, bound, negative, p, q)) {
		case rational_conversion::nan:
			setnan();
			return *this;
		case rational_conversion::infinite:
		case rational_conversion::overflow:
			if (negative) maxneg(); else maxpos();
			return *this;
		case rational_conversion::zero:
			setzero();
			return *this;
		case rational_conversion::finite:
		default:
			break;
		}
		n = to_digit_component<Component>(p);
		if (negative) n = -n;
		d = to_digit_component<Component>(q);
		normalize();
		return *this;
	}

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
	static constexpr unsigned nbits   = _ndigits;  // alias to make generic code easier to write
	using Component = blockdigit<ndigits, 16>;

	rational() = default;
	rational(const rational&) = default;
	rational(rational&&) = default;
	rational& operator=(const rational&) = default;
	rational& operator=(rational&&) = default;

	// decorated constructor
	constexpr rational(const std::int64_t& _n, const std::int64_t& _d) : n{_n}, d{_d} {};
	constexpr rational(const Component& _n, const Component& _d) : n{_n}, d{_d} {};

	// specific value constructor
	constexpr rational(const SpecificValue code) noexcept : n{}, d{} {
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
	rational(float v)              { *this = static_cast<double>(v); }
	rational(double v)             { *this = v; }

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
	rational& operator=(float rhs)              { return convert_ieee754(rhs); }
	rational& operator=(double rhs)             { return convert_ieee754(rhs); }
#if LONG_DOUBLE_SUPPORT
	rational& operator=(long double rhs)        { return convert_ieee754(rhs); }
#endif

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

	rational& maxpos() { n = Component(); n.clear(); for (unsigned i = 0; i < ndigits; ++i) n.setdigit(i, 15); d = 1; return *this; }
	rational& minpos() { n = 1; d = Component(); d.clear(); for (unsigned i = 0; i < ndigits; ++i) d.setdigit(i, 15); return *this; }
	rational& zero() { n = 0; d = 1; return *this; }
	rational& minneg() { minpos(); n = -n; return *this; }
	rational& maxneg() { maxpos(); n = -n; return *this; }

	bool iszero() const { return n.iszero() && !d.iszero(); }
	// the sign lives in the pair: the raw two-component constructor can hand this type a negative
	// denominator, which normalize() has not seen (#1525)
	bool isneg()  const { return (n.sign() != d.sign()) && !n.iszero(); }
	bool isinf()  const { return false; }
	bool isnan()  const { return n.iszero() && d.iszero(); }
	bool sign()   const { return isneg(); }
	int  scale()  const { double v = to_double(); return (v == 0.0) ? 0 : static_cast<int>(std::floor(std::log2(std::abs(v)))); }

	Component numerator() const { return n; }
	Component denominator() const { return d; }

private:
	Component n;
	Component d;

	// The closest rational this type can hold, from best_rational.hpp. This used to truncate to an
	// integer -- n = (long long)rhs, d = 1 -- so 0.75 became 0/1 (#1526).
	template<typename Real>
	rational& convert_ieee754(Real rhs) noexcept {
		constexpr unsigned digitBits = (Component::radix <= 8u ? 3u : 4u);   // bits per digit
		constexpr unsigned wideBits  = 64u * ((ndigits * digitBits) / 64u + 4u);  // the bound, a 113-bit significand, and room
		using Wide = blockbinary<wideBits, bt, BinaryNumberType::Signed>;
		Wide bound, radixWide, one;
		bound.clear();
		radixWide.clear();
		one.clear();
		bound.setbit(0);
		one.setbit(0);
		radixWide.setbits(Component::radix);
		for (unsigned i = 0; i < ndigits; ++i) bound = bound * radixWide;  // radix^ndigits
		bound = bound - one;                                              // the largest component
		bool negative{ false };
		Wide p, q;
		switch (best_rational_from_native<Wide, Real>(rhs, bound, negative, p, q)) {
		case rational_conversion::nan:
			setnan();
			return *this;
		case rational_conversion::infinite:
		case rational_conversion::overflow:
			if (negative) maxneg(); else maxpos();
			return *this;
		case rational_conversion::zero:
			setzero();
			return *this;
		case rational_conversion::finite:
		default:
			break;
		}
		n = to_digit_component<Component>(p);
		if (negative) n = -n;
		d = to_digit_component<Component>(q);
		normalize();
		return *this;
	}

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
