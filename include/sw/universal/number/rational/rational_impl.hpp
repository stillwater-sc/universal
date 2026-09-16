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
			std::fprintf(stderr, "rational_divide_by_zero\n");
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
		// |rhs| = (a / b) * 2^exponent with b = 2^hb, the weight of a's leading bit. The algorithm
		// below is the one this function always used -- reduce by the common power of two, shift
		// both down when the numerator is wider than nbits, and shift the numerator (or the
		// denominator) up by the scale as far as nbits allows -- but on an integer wide enough for
		// nbits and for a long double's significand. It used to run in a uint64_t: past scale 64 it
		// gave up ("overflow: scale = 70" on stdout, rational<128> of 2^70 became maxpos, #1519),
		// and a long double brought only its leading 64 significand bits (#1517). It also returned
		// +1 for -1.0, maxpos for any negative overflow, and left a subnormal double unconverted.
		constexpr unsigned wideBits = 64u * (nbits / 64u + 3u);  // >= nbits + 64, and 2 words of headroom
		using Wide = blockbinary<wideBits, bt, BinaryNumberType::Signed>;
		Wide a, b;
		a.clear();
		b.clear();
		bool s{ false };
		int  exponent{ 0 };
		int  hb{ 0 };
#if LONG_DOUBLE_SUPPORT
		if constexpr (std::is_same_v<Real, long double>) {
			// the leading words of the significand: at least nbits + 64 bits, more than can survive
			// the shift to nbits below, which makes the rest irrelevant
			long_double_significand sig(rhs);
			s        = sig.negative();
			exponent = sig.scale();
			constexpr int words = static_cast<int>(wideBits / 64u) - 2;
			for (int w = 0; w < words; ++w) {
				a <<= 64;
				const uint64_t bits = sig.next();
				for (int k = 0; k < 64; ++k)
					if ((bits >> k) & 1ull) a.setbit(static_cast<unsigned>(k));
			}
			hb = 64 * words - 1;
		}
		else
#endif
		{
			uint64_t bits{ 0 }, e{ 0 }, f{ 0 };
			extractFields(rhs, s, e, f, bits);
			constexpr int fbits = ieee754_parameter<Real>::fbits;
			if (e == 0) {  // subnormal: f * 2^(1 - bias - fbits), its leading bit at find_msb(f) - 1
				hb       = static_cast<int>(find_msb(f)) - 1;
				exponent = 1 - ieee754_parameter<Real>::bias - fbits + hb;
				a.setbits(f);
			}
			else {
				hb       = fbits;
				exponent = static_cast<int>(e) - ieee754_parameter<Real>::bias;
				a.setbits(f | ieee754_parameter<Real>::hmask);
			}
		}
		b.setbit(static_cast<unsigned>(hb));  // |rhs| = (a / b) * 2^exponent, b = 2^hb

		// |rhs| is the exact dyadic A / B. This used to shift A and B to fit nbits, which is not the
		// closest rational a nbits numerator and denominator can hold: rational<8> of 1/3's double
		// came out as 21/64, so r -> double -> r did not give r back for a non-dyadic r (#1523).
		// The continued fraction of A / B gives its convergents, each the closest rational of its
		// denominator or smaller; the last one that fits, or the better of it and the largest
		// semiconvergent that fits, is the closest rational this type can hold.
		Wide A, B;
		A.clear();
		B.clear();
		const int E = exponent - hb;  // |rhs| = a * 2^E
		Wide maxMagnitude;            // the largest numerator or denominator: 2^(nbits-1) - 1
		maxMagnitude.clear();
		for (unsigned i = 0; i + 1 < nbits; ++i) maxMagnitude.setbit(i);
		const int significandBits = a.msb() + 1;
		if (E >= 0) {
			if (significandBits + E > static_cast<int>(nbits)) {  // past 2^nbits: over the largest numerator
				if (s) maxneg(); else maxpos();
				return *this;
			}
			A = a;
			A <<= E;
			B.setbit(0);
		}
		else {
			if (-E - significandBits > static_cast<int>(nbits)) {  // below 2^-nbits: under half of 1 / maxMagnitude
				setzero();
				return *this;
			}
			A = a;
			B.setbit(static_cast<unsigned>(-E));
		}
		if ((A / B) > maxMagnitude) {  // the integer part alone does not fit
			if (s) maxneg(); else maxpos();
			return *this;
		}

		// the convergents h / k of the continued fraction of A / B
		Wide h, hPrev, k, kPrev, x, y, one;
		one.clear();
		one.setbit(0);
		h     = one;   // h(-1)
		hPrev.clear(); // h(-2)
		k.clear();     // k(-1)
		kPrev = one;   // k(-2)
		x     = A;
		y     = B;
		bool exact = false;
		Wide bestNumerator, bestDenominator;
		bestNumerator.clear();
		bestDenominator = one;
		while (!y.iszero()) {
			const Wide quotient  = x / y;
			const Wide remainder = x % y;
			// h = quotient * h + hPrev, k = quotient * k + kPrev, unless that no longer fits
			const bool fits = (quotient <= maxMagnitude) && ((quotient * h + hPrev) <= maxMagnitude) &&
			                  ((quotient * k + kPrev) <= maxMagnitude);
			if (!fits) {
				// the largest semiconvergent that fits: hPrev + t * h over kPrev + t * k
				Wide t = quotient;
				if (!h.iszero()) {
					const Wide room = (maxMagnitude - hPrev) / h;
					if (room < t) t = room;
				}
				if (!k.iszero()) {
					const Wide room = (maxMagnitude - kPrev) / k;
					if (room < t) t = room;
				}
				bestNumerator   = h;
				bestDenominator = k;
				if (!t.iszero()) {
					const Wide semiNumerator   = hPrev + t * h;
					const Wide semiDenominator = kPrev + t * k;
					// keep the closer of h / k and the semiconvergent: compare |A/B - p/q| by
					// |A q - p B| * q', the denominators being positive
					if (closerToTarget(A, B, semiNumerator, semiDenominator, h, k)) {
						bestNumerator   = semiNumerator;
						bestDenominator = semiDenominator;
					}
				}
				break;
			}
			const Wide nextH = quotient * h + hPrev;
			const Wide nextK = quotient * k + kPrev;
			hPrev = h;
			h     = nextH;
			kPrev = k;
			k     = nextK;
			x     = y;
			y     = remainder;
			if (y.iszero()) {  // A / B itself fits: exact
				bestNumerator   = h;
				bestDenominator = k;
				exact           = true;
			}
		}
		(void)exact;
		if (bestDenominator.iszero()) bestDenominator = one;  // a zero value: 0 / 1

		if (s) bestNumerator.twosComplement();
		n.clear();
		d.clear();
		for (unsigned i = 0; i < nbits; ++i) {
			if (bestNumerator.test(i)) n.setbit(i);
			if (bestDenominator.test(i)) d.setbit(i);
		}
		normalize();
		return *this;
	}

	// is p1 / q1 closer to A / B than p2 / q2? |A q1 - p1 B| q2 vs |A q2 - p2 B| q1, in twice the
	// width so the products cannot overflow. A tie keeps the smaller denominator, p2 / q2.
	template<typename Wide>
	static bool closerToTarget(const Wide& A, const Wide& B, const Wide& p1, const Wide& q1, const Wide& p2,
	                           const Wide& q2) noexcept {
		using Wider = blockbinary<2 * Wide::nbits, bt, BinaryNumberType::Signed>;
		auto widen  = [](const Wide& v) {
			Wider w;
			w.clear();
			for (unsigned i = 0; i < Wide::nbits; ++i)
				if (v.test(i)) w.setbit(i);
			return w;
		};
		const Wider a = widen(A), b = widen(B);
		auto distance = [&](const Wide& p, const Wide& q) {
			Wider e = a * widen(q) - widen(p) * b;
			if (e.sign()) e.twosComplement();  // |A q - p B|
			return e;
		};
		return (distance(p1, q1) * widen(q2)) < (distance(p2, q2) * widen(q1));
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
	rational& operator=(float rhs)              { return *this = static_cast<double>(rhs); }
	rational& operator=(double rhs)             { if (std::isfinite(rhs)) { n = static_cast<long long>(rhs); } else { n = 0; } d = 1; return *this; }

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
	rational& operator=(float rhs)              { return *this = static_cast<double>(rhs); }
	rational& operator=(double rhs)             { if (std::isfinite(rhs)) { n = static_cast<long long>(rhs); } else { n = 0; } d = 1; return *this; }

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
	rational& operator=(float rhs)              { return *this = static_cast<double>(rhs); }
	rational& operator=(double rhs)             { if (std::isfinite(rhs)) { n = static_cast<long long>(rhs); } else { n = 0; } d = 1; return *this; }

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
