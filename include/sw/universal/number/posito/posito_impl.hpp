#pragma once
// posito_impl.hpp: implementation of arbitrary configuration fixed-size posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>
#include <regex>
#include <type_traits>

#if POSITO_THROW_ARITHMETIC_EXCEPTION
// propagate this behavior down to constituent classes
#ifndef BITBLOCK_THROW_ARITHMETIC_EXCEPTION
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 1
#endif
#endif

// calling environment should define behavioral flags
// typically set in the library aggregation include file <posit>
// but can be set by individual programs when including posit.hpp
// For example:
// - define to non-zero if you want to enable arithmetic and logic literals
// #define POSITO_ENABLE_LITERALS 1
// - define to non-zero if you want to throw exceptions on arithmetic errors
// #define POSITO_THROW_ARITHMETIC_EXCEPTION 1

#if POSITO_THROW_ARITHMETIC_EXCEPTION
// Posits encode error conditions as NaR (Not a Real)
// propagating the error through arithmetic operations is preferred
#include "exceptions.hpp"
#endif // POSITO_THROW_ARITHMETIC_EXCEPTION

// TODO: these need to be redesigned to enable constexpr and improve performance: roadmap V3 Q1 2021
#include <universal/internal/bitblock/bitblock.hpp>
#include <universal/internal/value/value.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/algorithm/trace_constants.hpp>
// posit environment
#include <universal/number/posito/posito_fwd.hpp>
#include <universal/number/posit1/positFraction.hpp>
#include <universal/number/posit1/positExponent.hpp>
#include <universal/number/posit1/positRegime.hpp>
#include <universal/number/posito/attributes.hpp>

namespace sw { namespace universal {

// inject internal namespace
using namespace sw::universal::internal;

// needed to avoid double rounding situations during arithmetic: TODO: does that mean the condensed version below should be removed?
template<unsigned nbits, unsigned es, unsigned fbits>
inline posito<nbits, es>& convert_(bool _sign, int _scale, const bitblock<fbits>& fraction_in, posito<nbits, es>& p) {
	if constexpr (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
	if constexpr (_trace_conversion) std::cout << "sign " << (_sign ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << fraction_in << std::endl;

	p.clear();
	// construct the posito
	// interpolation rule checks
	if (check_inward_projection_range<nbits, es>(_scale)) {    // regime dominated
		if constexpr (_trace_conversion) std::cout << "inward projection" << std::endl;
		// we are projecting to minpos/maxpos
		int k = calculate_unconstrained_k<nbits, es>(_scale);
		k < 0 ? p.setBitblock(minpos_pattern<nbits, es>(_sign)) : p.setBitblock(maxpos_pattern<nbits, es>(_sign));
		// we are done
		if constexpr (_trace_rounding) std::cout << "projection  rounding ";
	}
	else {
		constexpr unsigned pt_len = nbits + 3 + es;
		bitblock<pt_len> pt_bits;
		bitblock<pt_len> regime;
		bitblock<pt_len> exponent;
		bitblock<pt_len> fraction;
		bitblock<pt_len> sticky_bit;

		bool s = _sign;
		int e  = _scale;
		bool r = (e >= 0);

		unsigned run = unsigned(r ? 1 + (e >> es) : -(e >> es));
		regime.set(0, 1 ^ r);
		for (unsigned i = 1; i <= run; i++) regime.set(i, r);

		unsigned esval = e % (uint32_t(1) << es);
		exponent = convert_to_bitblock<pt_len>(esval);
		int nbits_plus_one = static_cast<int>(nbits) + 1;
		int sign_regime_es = 2 + int(run) + static_cast<int>(es);
		unsigned nf = (unsigned)std::max<int>(0, (nbits_plus_one - sign_regime_es));
		//unsigned nf = (unsigned)std::max<int>(0, (static_cast<int>(nbits + 1) - (2 + run + static_cast<int>(es))));
		// TODO: what needs to be done if nf > fbits?
		//assert(nf <= input_fbits);
		// copy the most significant nf fraction bits into fraction
		unsigned lsb = nf <= fbits ? 0 : nf - fbits;
		for (unsigned i = lsb; i < nf; ++i) fraction[i] = fraction_in[static_cast<uint64_t>(fbits) - nf + i];

		bool sb = anyAfter(fraction_in, static_cast<int64_t>(fbits) - 1ll - static_cast<int64_t>(nf));

		// construct the untruncated posito
		// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
		regime <<= (es + nf + 1ull);
		exponent <<= (nf + 1ull);
		fraction <<= 1;
		sticky_bit.set(0, sb);

		pt_bits |= regime;
		pt_bits |= exponent;
		pt_bits |= fraction;
		pt_bits |= sticky_bit;

		unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
		bool blast = pt_bits.test(len - nbits);
		bool bafter = pt_bits.test(len - nbits - 1);
		bool bsticky = anyAfter(pt_bits, int(len) - static_cast<int>(nbits) - 1 - 1);

		bool rb = (blast & bafter) | (bafter & bsticky);

		bitblock<nbits> ptt;
		pt_bits <<= pt_len - len;
		truncate(pt_bits, ptt);
		if (rb) increment_bitset(ptt);
		if (s) ptt = twos_complement(ptt);
		p.setBitblock(ptt);
	}
	return p;
}

// convert a floating point value to a specific posito configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned es, unsigned fbits>
inline posito<nbits, es>& convert(const internal::value<fbits>& v, posito<nbits, es>& p) {
	if constexpr (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
	if constexpr (_trace_conversion) std::cout << "sign " << (v.sign() ? "-1 " : " 1 ") << "scale " << std::setw(3) << v.scale() << " fraction " << v.fraction() << std::endl;

	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnar();
		return p;
	}
	return convert_<nbits, es, fbits>(v.sign(), v.scale(), v.fraction(), p);
}
	
// quadrant returns a two character string indicating the quadrant of the projective reals the posito resides: from 0, SE, NE, NaR, NW, SW
template<unsigned nbits, unsigned es>
std::string quadrant(const posito<nbits,es>& p) {
	posito<nbits, es> pOne(1), pMinusOne(-1);
	if (p.isneg()) {
		// west
		if (p > pMinusOne) {
			return "SW";
		}
		else {
			return "NW";
		}
	}
	else {
		// east
		if (p < pOne) {
			return "SE";
		}
		else {
			return "NE";
		}
	}
}

// Construct posito from its components
template<unsigned nbits, unsigned es, unsigned fbits>
posito<nbits, es>& construct(bool s, const positRegime<nbits, es>& r, const positExponent<nbits, es>& e, const positFraction<fbits>& f, posito<nbits,es>& p) {
	// generate raw bit representation
	bitblock<nbits> raw_bits = s ? twos_complement(collect(s, r, e, f)) : collect(s, r, e, f);
	raw_bits.set(nbits - 1, s);
	p.set(raw_bits);
	return p;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class posito represents posit numbers of arbitrary configuration and their basic arithmetic operations (add/sub, mul/div)
template<unsigned _nbits, unsigned _es>
class posito {

//	static_assert(sizeof(long double) == 16, "Posit library requires compiler support for 128 bit long double.");
//	static_assert((sizeof(long double) == 16) && (std::numeric_limits<long double>::digits < 113), "C++ math library for long double does not support 128-bit quad precision floats.");
  
public:
	static constexpr unsigned nbits   = _nbits;
	static constexpr unsigned es      = _es;
	static constexpr unsigned sbits   = 1;                          // number of sign bits:     specified
	static constexpr unsigned rbits   = nbits - sbits;              // maximum number of regime bits:   derived
	static constexpr unsigned ebits   = es;                         // maximum number of exponent bits: specified
	static constexpr unsigned fbits   = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived
	static constexpr unsigned fhbits  = fbits + 1;                  // maximum number of fraction + one hidden bit

	static constexpr unsigned abits   = fhbits + 3;                 // size of the addend
	static constexpr unsigned mbits   = 2 * fhbits;                 // size of the multiplier output
	static constexpr unsigned divbits = 3 * fhbits + 4;             // size of the divider output

	// constexpr posito() { setzero();  }
	constexpr posito() : _bits{} {}
	
	constexpr posito(const posito&) = default;
	constexpr posito(posito&&) = default;
	
	posito& operator=(const posito&) = default;
	posito& operator=(posito&&) = default;

	/// Construct posito from another posito
	template<unsigned nnbits, unsigned ees>
	posito(const posito<nnbits, ees>& a) noexcept {
		*this = a.to_value();
	}

	// specific value constructor
	constexpr posito(const SpecificValue code) noexcept {
		switch (code) {
		case SpecificValue::infpos:
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::infneg:
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::snan:
		case SpecificValue::qnan:
		case SpecificValue::nar:
			setnar();
			break;
		}
	}

	// initializers for native types, allow for implicit conversion (Peter)
	constexpr posito(signed char initial_value)        noexcept { *this = initial_value; }
	constexpr posito(short initial_value)              noexcept { *this = initial_value; }
	constexpr posito(int initial_value)                noexcept { *this = initial_value; }
	constexpr posito(long initial_value)               noexcept { *this = initial_value; }
	constexpr posito(long long initial_value)          noexcept { *this = initial_value; }
	constexpr posito(char initial_value)               noexcept { *this = initial_value; }
	constexpr posito(unsigned short initial_value)     noexcept { *this = initial_value; }
	constexpr posito(unsigned int initial_value)       noexcept { *this = initial_value; }
	constexpr posito(unsigned long initial_value)      noexcept { *this = initial_value; }
	constexpr posito(unsigned long long initial_value) noexcept { *this = initial_value; }
	constexpr posito(float initial_value)              noexcept { *this = initial_value; }
	constexpr posito(double initial_value)             noexcept { *this = initial_value; }
	constexpr posito(long double initial_value)        noexcept { *this = initial_value; }

	// assignment operators for native types
	posito& operator=(signed char rhs) noexcept {
		internal::value<8*sizeof(signed char)-1> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(short rhs) noexcept {
		internal::value<8*sizeof(short)-1> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(int rhs) noexcept {
		internal::value<8*sizeof(int)-1> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(long rhs) noexcept {
		internal::value<8*sizeof(long)> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(long long rhs) noexcept {
		internal::value<8*sizeof(long long)-1> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(char rhs) noexcept {
		internal::value<8*sizeof(char)> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(unsigned short rhs) noexcept {
		internal::value<8*sizeof(unsigned short)> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(unsigned int rhs) noexcept {
		internal::value<8*sizeof(unsigned int)> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(unsigned long rhs) noexcept {
		internal::value<8*sizeof(unsigned long)> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(unsigned long long rhs) noexcept {
		internal::value<8*sizeof(unsigned long long)> v(rhs);
		if (v.iszero()) {
			setzero();
			return *this;
		}
		else {
			convert(v, *this);
		}
		return *this;
	}
	posito& operator=(float rhs) noexcept {
		return convert_ieee754(rhs);
	}
	constexpr posito& operator=(double rhs) noexcept {
            convert_ieee754(rhs);
            return *this; 
	}
	posito& operator=(long double rhs) noexcept {
       	return convert_ieee754(rhs);
	}

#ifdef ADAPTER_POSITO_AND_INTEGER
	// convenience assignment operator
	template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
	posito& operator=(const integer<nbits, BlockType, NumberType>& rhs) {
		convert_i2p(rhs, *this);
		return *this;
	}
#endif

	// assignment for value type
	template<unsigned vbits>
	posito& operator=(const internal::value<vbits>& rhs) {
		clear();
		convert(rhs, *this);
		return *this;
	}
	
	// negation operator
	posito operator-() const {
		if (iszero()) {
			return *this;
		}
		if (isnar()) {
			return *this;
		}
		posito<nbits, es> negated(0);  // TODO: artificial initialization to pass -Wmaybe-uninitialized
		bitblock<nbits> raw_bits = twos_complement(_bits);
		negated.setBitblock(raw_bits);
		return negated;
	}
	// prefix/postfix operators
	posito& operator++() noexcept {
		increment_posit();
		return *this;
	}
	posito operator++(int) noexcept {
		posito tmp(*this);
		operator++();
		return tmp;
	}
	posito& operator--() noexcept {
		decrement_posit();
		return *this;
	}
	posito operator--(int) noexcept {
		posito tmp(*this);
		operator--();
		return tmp;
	}

	// we model a hw pipeline with register assignments, functional block, and conversion
	posito& operator+=(const posito& rhs) {
		if constexpr (_trace_add) std::cout << "---------------------- ADD -------------------" << std::endl;
		// special case handling of the inputs
#if POSITO_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || rhs.isnar()) {
			throw posito{};
		}
#else
		if (isnar() || rhs.isnar()) {
			setnar();
			return *this;
		}
#endif
		if (iszero()) {
			*this = rhs;
			return *this;
		}
		if (rhs.iszero()) return *this;

		// arithmetic operation
		internal::value<abits + 1> sum;
		internal::value<fbits> a, b;
		// transform the inputs into (sign,scale,fraction) triples
		normalize(a);
		rhs.normalize(b);
		module_add<fbits,abits>(a, b, sum);		// add the two inputs

		// special case handling of the result
		if (sum.iszero()) {
			setzero();
		}
		else if (sum.isinf()) {
			setnar();
		}
		else {
			convert(sum, *this);
		}
		return *this;                
	}
	posito& operator+=(double rhs) {
		return *this += posito<nbits, es>(rhs);
	}
	posito& operator-=(const posito& rhs) {
		if constexpr (_trace_sub) std::cout << "---------------------- SUB -------------------" << std::endl;
		// special case handling of the inputs
#if POSITO_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || rhs.isnar()) {
			throw posito{};
		}
#else
		if (isnar() || rhs.isnar()) {
			setnar();
			return *this;
		}
#endif
		if (iszero()) {
			*this = -rhs;
			return *this;
		}
		if (rhs.iszero()) return *this;

		// arithmetic operation
		internal::value<abits + 1> difference;
		internal::value<fbits> a, b;
		// transform the inputs into (sign,scale,fraction) triples
		normalize(a);
		rhs.normalize(b);
		module_subtract<fbits, abits>(a, b, difference);	// add the two inputs

		// special case handling of the result
		if (difference.iszero()) {
			setzero();
		}
		else if (difference.isinf()) {
			setnar();
		}
		else {
			convert(difference, *this);
		}
		return *this;
	}
	posito& operator-=(double rhs) {
		return *this -= posito<nbits, es>(rhs);
	}
	posito& operator*=(const posito& rhs) {
		static_assert(fhbits > 0, "posito configuration does not support multiplication");
		if constexpr (_trace_mul) std::cout << "---------------------- MUL -------------------" << std::endl;
		// special case handling of the inputs
#if POSITO_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || rhs.isnar()) {
			throw posito{};
		}
#else
		if (isnar() || rhs.isnar()) {
			setnar();
			return *this;
		}
#endif
		if (iszero() || rhs.iszero()) {
			setzero();
			return *this;
		}

		// arithmetic operation
		internal::value<mbits> product;
		internal::value<fbits> a, b;
		// transform the inputs into (sign,scale,fraction) triples
		normalize(a);
		rhs.normalize(b);

		module_multiply(a, b, product);    // multiply the two inputs

		// special case handling on the output
		if (product.iszero()) {
			setzero();
		}
		else if (product.isinf()) {
			setnar();
		}
		else {
			convert(product, *this);
		}
		return *this;
	}
	posito& operator*=(double rhs) {
		return *this *= posito<nbits, es>(rhs);
	}
	posito& operator/=(const posito& rhs) {
		if constexpr (_trace_div) std::cout << "---------------------- DIV -------------------" << std::endl;
#if POSITO_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) {
			throw posito{};    // not throwing is a quiet signalling NaR
		}
		if (rhs.isnar()) {
			throw posito{};
		}
		if (isnar()) {
			throw posito{};
		}
		if (iszero() || isnar()) {
			return *this;
		}
#else
		// not throwing is a quiet signalling NaR
		if (rhs.iszero()) {
			setnar();
			return *this;
		}
		if (rhs.isnar()) {
			setnar();
			return *this;
		}
		if (iszero() || isnar()) {
			return *this;
		}
#endif
		internal::value<divbits> ratio;
		internal::value<fbits> a, b;
		// transform the inputs into (sign,scale,fraction) triples
		normalize(a);
		rhs.normalize(b);

		module_divide(a, b, ratio);

		// special case handling on the output
#if POSITO_THROW_ARITHMETIC_EXCEPTION
		if (ratio.iszero()) {
			throw posito{};
		}
		else if (ratio.isinf()) {
			throw posito{};
		}
		else {
			convert<nbits, es, divbits>(ratio, *this);
		}
#else
		if (ratio.iszero()) {
			setzero();  // this shouldn't happen as we should project back onto minpos
		}
		else if (ratio.isinf()) {
			setnar();  // this shouldn't happen as we should project back onto maxpos
		}
		else {
			convert<nbits, es, divbits>(ratio, *this);
		}
#endif

		return *this;
	}
	posito& operator/=(double rhs) {
		return *this /= posito<nbits, es>(rhs);
	}
	
	posito reciprocal() const {
		if constexpr (_trace_reciprocal) std::cout << "-------------------- RECIPROCAl ----------------" << std::endl;
		posito<nbits, es> p;
		// special case of NaR (Not a Real)
		if (isnar()) {
			p.setnar();
			return p;
		}
		if (iszero()) {
			p.setnar();
			return p;
		}
		// compute the reciprocal
		bool old_sign = _bits[nbits-1];
		internal::bitblock<nbits> raw_bits;
		if (ispowerof2()) {
			raw_bits = twos_complement(_bits);
			raw_bits.set(nbits-1, old_sign);
			p.setBitblock(raw_bits);
		}
		else {
			bool s{ false };
			positRegime<nbits, es> r;
			positExponent<nbits, es> e;
			positFraction<fbits> f;
			decode(_bits, s, r, e, f);

			constexpr unsigned operand_size = fhbits;
			internal::bitblock<operand_size> one;
			one.set(operand_size - 1, true);
			internal::bitblock<operand_size> frac;
			copy_into(f.get(), 0, frac);
			frac.set(operand_size - 1, true);
			constexpr unsigned reciprocal_size = 3 * fbits + 4;
			internal::bitblock<reciprocal_size> reciprocal;
			divide_with_fraction(one, frac, reciprocal);
			if constexpr (_trace_reciprocal) {
				std::cout << "one    " << one << std::endl;
				std::cout << "frac   " << frac << std::endl;
				std::cout << "recip  " << reciprocal << std::endl;
			}

			// radix point falls at operand size == reciprocal_size - operand_size - 1
			reciprocal <<= operand_size - 1;
			if constexpr (_trace_reciprocal) std::cout << "frac   " << reciprocal << std::endl;
			int new_scale = -scale(*this);
			int msb = findMostSignificantBit(reciprocal);
			if (msb > 0) {
				int shift = static_cast<int>(reciprocal_size - static_cast<unsigned>(msb));
				reciprocal <<= static_cast<unsigned>(shift);
				new_scale -= (shift-1);
				if constexpr (_trace_reciprocal) std::cout << "result " << reciprocal << std::endl;
			}
			//std::bitset<operand_size> tr;
			//truncate(reciprocal, tr);
			//std::cout << "tr     " << tr << std::endl;

			// the following is failing for some reason
			// value<reciprocal_size> v(old_sign, new_scale, reciprocal);
			// convert(v, p);
			// instead the following works
			convert_<nbits,es, reciprocal_size>(old_sign, new_scale, reciprocal, p);
		}
		return p;
	}
	// absolute value is simply the 2's complement when negative
	posito abs() const {
		posito p;
		if (isneg()) {
			p.setBitblock(twos_complement(_bits));
		}
		else {
			p.setBitblock(_bits);
		}
		return p;
	}

	// conversion operators
	// Maybe remove explicit, MTL compiles, but we have lots of double computation then
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

	// Selectors
	bool sign() const noexcept { return _bits[nbits - 1]; }
	bool isnar() const noexcept {
		if (_bits[nbits - 1] == false) return false;
		bitblock<nbits> tmp(_bits);			
		tmp.reset(nbits - 1);
		return tmp.none() ? true : false;
	}
	bool isnan() const noexcept { return isnar(); }
	bool isinf() const noexcept { return false; }
	bool iszero() const noexcept { return _bits.none() ? true : false; }
	bool isone() const noexcept { // pattern 010000....
		bitblock<nbits> tmp(_bits);
		tmp.set(nbits - 2, false);
		return _bits[nbits - 2] & tmp.none();
	}
	bool isminusone() const noexcept { // pattern 110000...
		bitblock<nbits> tmp(_bits);
		tmp.set(nbits - 1, false);
		tmp.set(nbits - 2, false);
		return _bits[nbits - 1] & _bits[nbits - 2] & tmp.none();
	}
	bool isneg() const noexcept { return _bits[nbits - 1]; }
	bool ispos() const noexcept { return !_bits[nbits - 1]; }
	bool ispowerof2() const noexcept {
		bool s{ false };
		positRegime<nbits, es> r;
		positExponent<nbits, es> e;
		positFraction<fbits> f;
		decode(_bits, s, r, e, f);
		return f.none();
	}
	bool isinteger() const noexcept { return true; } // return (floor(*this) == *this) ? true : false; }

	bitblock<nbits>    get() const noexcept { return _bits; }
	unsigned long long bits() const noexcept { return _bits.to_ullong(); }
	constexpr bool test(unsigned bitIndex) const noexcept {
		return (bitIndex < nbits ? _bits[bitIndex] : false);
	}
	constexpr bool at(unsigned bitIndex) const noexcept {
		return (bitIndex < nbits ? _bits[bitIndex] : false);
	}
	constexpr uint8_t nibble(unsigned n) const noexcept {
		uint8_t nibbleBits{ 0 };
		if (n < (1 + ((nbits - 1) >> 2))) {
			unsigned baseNibbleIndex = 4 * n;
			unsigned mask = 0x1;
			for (unsigned i = baseNibbleIndex; i < nbits && i < baseNibbleIndex + 4; ++i) {
				nibbleBits |= (test(i) ? mask : 0);
				mask <<= 1;
			}
		}
		return nibbleBits;
	}
	// Modifiers
	constexpr void clear() noexcept { _bits.reset(); }
	constexpr void setzero() noexcept { clear(); }
	constexpr void setnar() noexcept {
		_bits.reset();
		_bits.set(nbits - 1, true);
	}
	constexpr void setnan(bool sign) noexcept { setnar(); }
	posito& minpos() noexcept { clear(); return ++(*this); }
	posito& maxpos() noexcept { setnar(); return --(*this); }
	posito& zero()   noexcept { clear(); return *this; }
	posito& minneg() noexcept { clear(); return --(*this); }
	posito& maxneg() noexcept { setnar(); return ++(*this); }

	// set the posito bits explicitely
	constexpr posito<nbits, es>& setBitblock(const bitblock<nbits>& raw_bits) {
		_bits = raw_bits;
		return *this;
	}
	// Set the raw bits of the posito given an unsigned value starting from the lsb. Handy for enumerating a posito state space
	constexpr posito<nbits, es>& setbits(uint64_t value) {
		clear();
		bitblock<nbits> raw_bits;
		uint64_t mask = 1;
		for ( unsigned i = 0; i < nbits; i++ ) {
			raw_bits.set(i,(value & mask));
			mask <<= 1;
		}
		_bits = raw_bits;
		return *this;
	}

	// currently, size is tied to fbits size of posito config. Is there a need for a case that captures a user-defined sized fraction?
	internal::value<fbits> to_value() const {
		using namespace sw::universal::internal;
		bool		     	 _sign{ false };
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		decode(_bits, _sign, _regime, _exponent, _fraction);
		return internal::value<fbits>(_sign, _regime.scale() + _exponent.scale(), _fraction.get(), iszero(), isnar());
	}
	void normalize(internal::value<fbits>& v) const {
		using namespace sw::universal::internal;
		bool		     	 _sign{ false };
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		decode(_bits, _sign, _regime, _exponent, _fraction);
		v.set(_sign, _regime.scale() + _exponent.scale(), _fraction.get(), iszero(), isnar());
	}
	template<unsigned tgt_fbits>
	void normalize_to(internal::value<tgt_fbits>& v) const {
		using namespace sw::universal::internal;
		bool		     	 _sign{ false };
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		decode(_bits, _sign, _regime, _exponent, _fraction);
		bitblock<tgt_fbits> _fr;
		bitblock<fbits> _src = _fraction.get();
		int tgt, src;
		for (tgt = int(tgt_fbits) - 1, src = int(fbits) - 1; tgt >= 0 && src >= 0; tgt--, src--) _fr[tgt] = _src[src];
		v.set(_sign, _regime.scale() + _exponent.scale(), _fr, iszero(), isnar());
	}
	
	// step up to the next posito in a lexicographical order
	void increment_posit() {
		increment_bitset(_bits);
	}
	// step down to the previous posito in a lexicographical order
	void decrement_posit() {
		decrement_bitset(_bits);
	}
	
	// return human readable type configuration for this posito
	inline std::string cfg() {
		std::stringstream ss;
		ss << "posito<" << nbits << ", " << es << ">";
		return ss.str();
	}

private:
	internal::bitblock<nbits>      _bits;	// raw bit representation

	// HELPER methods

	// Conversion functions
#if POSITO_THROW_ARITHMETIC_EXCEPTION
	short to_short() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return short(to_float());
	}
	int to_int() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return int(to_double());
	}
	long to_long() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return long(to_long_double());
	}
	long long to_long_long() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return (long long)(to_long_double());
	}
	unsigned short to_ushort() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return (unsigned short)(to_float());
	}
	unsigned int to_uint() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return (unsigned int)(to_double());
	}
	unsigned long to_ulong() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return (unsigned long)(to_long_double());
	}
	unsigned long long to_ulong_long() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return (unsigned long long)(to_long_double());
	}
#else
	short to_short() const                   { return short(to_float()); }
	int to_int() const                       { return int(to_double()); }
	long to_long() const                     { return long(to_long_double()); }
	long long to_long_long() const           { return (long long)(to_long_double()); }
	unsigned short to_ushort() const         { return (unsigned short)(to_float()); }
	unsigned int to_uint() const             { return (unsigned int)(to_double()); }
	unsigned long to_ulong() const           { return (unsigned long)(to_long_double()); }
	unsigned long long to_ulong_long() const { return (unsigned long long)(to_long_double()); }
#endif
	float to_float() const {
		return (float)to_double();
	}
	double to_double() const {
		if (iszero())	return 0.0;
		if (isnar())	return std::numeric_limits<double>::quiet_NaN();
		bool		     	 _sign{ false };
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		decode(_bits, _sign, _regime, _exponent, _fraction);
		double s = (_sign ? -1.0 : 1.0);
		double r = double(_regime.value());
		double e = double(_exponent.value());
		double f = (1.0 + double(_fraction.value()));
		return s * r * e * f;
	}
	long double to_long_double() const {
		if (iszero())  return 0.0l;
		if (isnar())   return std::numeric_limits<double>::quiet_NaN();;
		bool		     	 _sign{ false };
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		decode(_bits, _sign, _regime, _exponent, _fraction);
		long double s = (_sign ? -1.0l : 1.0l);
		long double r = _regime.value();
		long double e = _exponent.value();
		long double f = (1.0l + _fraction.value());
		return s * r * e * f;
	}
	template <typename T>
	constexpr posito<nbits, es>& convert_ieee754(const T& rhs) {
		constexpr int dfbits = std::numeric_limits<T>::digits - 1;
		internal::value<dfbits> v(static_cast<T>(rhs));

		// special case processing
		if (v.iszero()) {
			setzero();
			return *this;
		}
		if (v.isinf() || v.isnan()) {  // posito encode for FP_INFINITE and NaN as NaR (Not a Real)
			setnar();
			return *this;
		}

		convert(v, *this);
		return *this;
	}

	// friend functions
	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned ees>
	friend std::ostream& operator<< (std::ostream& ostr, const posito<nnbits, ees>& p);
	template<unsigned nnbits, unsigned ees>
	friend std::istream& operator>> (std::istream& istr, posito<nnbits, ees>& p);

	// posito - posito logic functions
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, const posito<nnbits, ees>& rhs);

#if POSITO_ENABLE_LITERALS
	// posito - literal logic functions

	// posito - signed char
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, signed char rhs);

	// posito - char
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, char rhs);

	// posito - short
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, short rhs);

	// posito - unsigned short
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, unsigned short rhs);

	// posito - int
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, int rhs);

	// posito - unsigned int
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, unsigned int rhs);

	// posito - long
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, long rhs);

	// posito - unsigned long long
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, unsigned long rhs);

	// posito - long long
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, long long rhs);

	// posito - unsigned long long
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, unsigned long long rhs);

	// posito - float
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, float rhs);

	// posito - double
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, double rhs);

	// posito - long double
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const posito<nnbits, ees>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const posito<nnbits, ees>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const posito<nnbits, ees>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const posito<nnbits, ees>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const posito<nnbits, ees>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const posito<nnbits, ees>& lhs, long double rhs);

	// literal - posito logic functions

	// signed char - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(signed char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(signed char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (signed char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (signed char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(signed char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(signed char lhs, const posito<nnbits, ees>& rhs);

	// char - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(char lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(char lhs, const posito<nnbits, ees>& rhs);

	// short - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(short lhs, const posito<nnbits, ees>& rhs);

	// unsigned short - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(unsigned short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(unsigned short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (unsigned short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (unsigned short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(unsigned short lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(unsigned short lhs, const posito<nnbits, ees>& rhs);

	// int - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(int lhs, const posito<nnbits, ees>& rhs);

	// unsigned int - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(unsigned int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(unsigned int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (unsigned int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (unsigned int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(unsigned int lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(unsigned int lhs, const posito<nnbits, ees>& rhs);

	// long - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(long lhs, const posito<nnbits, ees>& rhs);

	// unsigned long - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(unsigned long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(unsigned long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (unsigned long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (unsigned long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(unsigned long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(unsigned long lhs, const posito<nnbits, ees>& rhs);

	// long long - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(long long lhs, const posito<nnbits, ees>& rhs);

	// unsigned long long - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(unsigned long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(unsigned long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (unsigned long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (unsigned long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(unsigned long long lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(unsigned long long lhs, const posito<nnbits, ees>& rhs);

	// float - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(float lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(float lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (float lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (float lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(float lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(float lhs, const posito<nnbits, ees>& rhs);

	// double - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(double lhs, const posito<nnbits, ees>& rhs);

	// long double - posito
	template<unsigned nnbits, unsigned ees>
	friend bool operator==(long double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(long double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (long double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (long double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(long double lhs, const posito<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(long double lhs, const posito<nnbits, ees>& rhs);

#endif // POSITO_ENABLE_LITERALS

};

////////////////// convenience/shim functions

template<unsigned nbits, unsigned es>
inline bool isnar(const posito<nbits, es>& p) {
	return p.isnar();
}
template<unsigned nbits, unsigned es>
inline bool iszero(const posito<nbits, es>& p) {
	return p.iszero();
}
template<unsigned nbits, unsigned es>
inline bool ispos(const posito<nbits, es>& p) {
	return p.ispos();
}
template<unsigned nbits, unsigned es>
inline bool isneg(const posito<nbits, es>& p) {
	return p.isneg();
}
template<unsigned nbits, unsigned es>
inline bool isone(const posito<nbits, es>& p) {
	return p.isone();
}		
template<unsigned nbits, unsigned es>
inline bool isminusone(const posito<nbits, es>& p) {
	return p.isminusone();
}
template<unsigned nbits, unsigned es>
inline bool ispowerof2(const posito<nbits, es>& p) {
	return p.ispowerof2();
}

////////////////// POSITO operators

// stream operators

// generate a posito format ASCII format nbits.esxNN...NNp
template<unsigned nbits, unsigned es>
inline std::ostream& operator<<(std::ostream& ostr, const posito<nbits, es>& p) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the posito into a string
	std::stringstream ss;
#if POSITO_ERROR_FREE_IO_FORMAT
	ss << nbits << '.' << es << 'x' << to_hex(p.get()) << 'p';
#else
	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
//	ss << std::showpos << std::setw(width) << std::setprecision(prec) << (long double)p;
	// TODO: how do you react to fmtflags being set, such as hexfloat or showpos?
	// it appears that the fmtflags are opaque and not a user-visible feature
	ss << std::setw(width) << std::setprecision(prec);
	ss << to_string(p, prec);  // TODO: we need a true native serialization function
#endif
	return ostr << ss.str();
}

// read an ASCII float or posito format: nbits.esxNN...NNp, for example: 32.2x80000000p
template<unsigned nbits, unsigned es>
inline std::istream& operator>> (std::istream& istr, posito<nbits, es>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posito value\n";
	}
	return istr;
}

// generate a posito format ASCII format nbits.esxNN...NNp
template<unsigned nbits, unsigned es>
inline std::string hex_format(const posito<nbits, es>& p) {
	// we need to transform the posito into a string
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(p.get()) << 'p';
	return ss.str();
}

// convert a posito value to a string using "nar" as designation of NaR
template<unsigned nbits, unsigned es>
inline std::string to_string(const posito<nbits, es>& p, std::streamsize precision = 17) {
	if (p.isnar()) {
		return std::string("nar");
	}
	std::stringstream ss;
	ss << std::setprecision(precision) << (long double)p;
	return ss.str();
}

// binary representation of a posito with delimiters: i.e. 0.10.00.000000 => sign.regime.exp.fraction
template<unsigned nbits, unsigned es>
inline std::string to_binary(const posito<nbits, es>& number, bool nibbleMarker = false) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived
	bool s{ false };
	positRegime<nbits, es> r;
	positExponent<nbits, es> e;
	positFraction<fbits> f;
	bitblock<nbits> raw = number.get();
	std::stringstream ss;
	extract_fields(raw, s, r, e, f);

	ss << (s ? "0b1." : "0b0.");
	ss << to_string(r, false, nibbleMarker) << "."
		<< to_string(e, false, nibbleMarker) << "."
		<< to_string(f, false, nibbleMarker);

	return ss.str();
}

template<unsigned nbits, unsigned es>
inline std::string to_triple(const posito<nbits, es>& number, bool nibbleMarker = false) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived
	bool s{ false };
	positRegime<nbits, es> r;
	positExponent<nbits, es> e;
	positFraction<fbits> f;
	bitblock<nbits> raw = number.get();
	std::stringstream ss;
	extract_fields(raw, s, r, e, f);

	ss << (s ? "(-, " : "(+, ");
	ss << scale(number) 
	   << ", "
	   << to_string(f, false, nibbleMarker)
	   << ')';

	return ss.str();
}

// numerical helpers

template<unsigned nbits, unsigned es>
inline posito<nbits, es> ulp(const posito<nbits, es>& a) {
	posito<nbits, es> b(a);
	return ++b - a;
}

// binary exponent representation: i.e. 1.0101010e2^-37
template<unsigned nbits, unsigned es>
inline std::string to_base2_scientific(const posito<nbits, es>& number) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived
	bool s{ false };
	scale(number);
	positRegime<nbits, es> r;
	positExponent<nbits, es> e;
	positFraction<fbits> f;
	bitblock<nbits> raw = number.get();
	std::stringstream ss;
	extract_fields(raw, s, r, e, f);
	ss << (s ? "-" : "+") << "1." << to_string(f, true) << "e2^" << std::showpos << r.scale() + e.scale();
	return ss.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// posito - posito binary logic operators

template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	return lhs._bits == rhs._bits;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(lhs._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	return !operator< (lhs, rhs);
}

// posito - posito binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator+(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> sum = lhs;
	return sum += rhs;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator-(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> diff = lhs;
	return diff -= rhs;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator*(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> mul = lhs;
	return mul *= rhs;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator/(const posito<nbits, es>& lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> ratio(lhs);
	return ratio /= rhs;
}

#if POSITO_ENABLE_LITERALS

// posito - signed char logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, signed char rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, signed char rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, signed char rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, signed char rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, signed char rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, signed char rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// signed char - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(signed char lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(signed char lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (signed char lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (signed char lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(signed char lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(signed char lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - char logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, char rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, char rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, char rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, char rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, char rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, char rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// char - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(char lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(char lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (char lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (char lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(char lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(char lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - short logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, short rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, short rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, short rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, short rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, short rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, short rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// short - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(short lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(short lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (short lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (short lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(short lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(short lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - unsigned short logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, unsigned short rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, unsigned short rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, unsigned short rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, unsigned short rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, unsigned short rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, unsigned short rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// unsigned short - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned short lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned short lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned short lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned short lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned short lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned short lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - int logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, int rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, int rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, int rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, int rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, int rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, int rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// int - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(int lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(int lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (int lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (int lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(int lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(int lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - unsigned int logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, unsigned int rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, unsigned int rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, unsigned int rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, unsigned int rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, unsigned int rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, unsigned int rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// unsigned int - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned int lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned int lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned int lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned int lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned int lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned int lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, long rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, long rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, long rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, long rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, long rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, long rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// long - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(long lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(long lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (long lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (long lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(long lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(long lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - unsigned long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, unsigned long rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, unsigned long rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, unsigned long rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, unsigned long rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, unsigned long rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, unsigned long rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// unsigned long - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned long lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned long lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned long lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned long lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned long lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned long lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - unsigned long long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, unsigned long long rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, unsigned long long rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, unsigned long long rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, unsigned long long rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, unsigned long long rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, unsigned long long rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// unsigned long long - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned long long lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned long long lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned long long lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned long long lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned long long lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned long long lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - long long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, long long rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, long long rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, long long rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, long long rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, long long rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, long long rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// long long - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(long long lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(long long lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (long long lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (long long lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(long long lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(long long lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - float logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, float rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, float rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, float rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, float rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, float rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, float rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// float  - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(float lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(float lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (float lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (float lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(float lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(float lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - double logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, double rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, double rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, double rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, double rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, double rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, double rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// double  - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(double lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(double lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (double lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (double lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(double lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(double lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// posito - long double logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posito<nbits, es>& lhs, long double rhs) {
	return lhs == posito<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posito<nbits, es>& lhs, long double rhs) {
	return !operator==(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posito<nbits, es>& lhs, long double rhs) {
	return twosComplementLessThan(lhs._bits, posito<nbits, es>(rhs)._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posito<nbits, es>& lhs, long double rhs) {
	return operator< (posito<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posito<nbits, es>& lhs, long double rhs) {
	return !operator>(lhs, posito<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posito<nbits, es>& lhs, long double rhs) {
	return !operator<(lhs, posito<nbits, es>(rhs));
}

// long double  - posito logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(long double lhs, const posito<nbits, es>& rhs) {
	return posito<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(long double lhs, const posito<nbits, es>& rhs) {
	return !operator==(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (long double lhs, const posito<nbits, es>& rhs) {
	return twosComplementLessThan(posito<nbits, es>(lhs)._bits, rhs._bits);
}
template<unsigned nbits, unsigned es>
inline bool operator> (long double lhs, const posito<nbits, es>& rhs) {
	return operator< (posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(long double lhs, const posito<nbits, es>& rhs) {
	return !operator>(posito<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(long double lhs, const posito<nbits, es>& rhs) {
	return !operator<(posito<nbits, es>(lhs), rhs);
}

// BINARY ADDITION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator+(const posito<nbits, es>& lhs, double rhs) {
	posito<nbits, es> sum = lhs;
	sum += posito<nbits, es>(rhs);
	return sum;
}

// More generic alternative to avoid ambiguities with intrinsic +
template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posito<nbits, es> operator+(const posito<nbits, es>& lhs, Value rhs) {
	posito<nbits, es> sum = lhs;
	sum += posito<nbits, es>(rhs);
	return sum;
}

template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator+(double lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> sum(lhs);
	sum += rhs;
	return sum;
}

// BINARY SUBTRACTION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator-(double lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> diff(lhs);
	diff -= rhs;
	return diff;
}

// More generic alternative to avoid ambiguities with intrinsic +
template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posito<nbits, es> operator-(const posito<nbits, es>& lhs, Value rhs) {
	posito<nbits, es> diff = lhs;
	diff -= posito<nbits, es>(rhs);
	return diff;
}

template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator-(const posito<nbits, es>& lhs, double rhs) {
	posito<nbits, es> diff(lhs);
	diff -= posito<nbits, es>(rhs);
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator*(double lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> mul(lhs);
	mul *= rhs;
	return mul;
}

template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posito<nbits, es> operator*(Value lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> mul(lhs);
	mul *= rhs;
	return mul;
}
    
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator*(const posito<nbits, es>& lhs, double rhs) {
	posito<nbits, es> mul(lhs);
	mul *= posito<nbits, es>(rhs);
	return mul;
}

// BINARY DIVISION
template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator/(double lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posito<nbits, es> operator/(Value lhs, const posito<nbits, es>& rhs) {
	posito<nbits, es> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es>
inline posito<nbits, es> operator/(const posito<nbits, es>& lhs, double rhs) {
	posito<nbits, es> ratio(lhs);
	ratio /= posito<nbits, es>(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posito<nbits, es> operator/(const posito<nbits, es>& lhs, Value rhs) {
	posito<nbits, es> ratio(lhs);
	ratio /= posito<nbits, es>(rhs);
	return ratio;
}

#endif // POSITO_ENABLE_LITERALS

// Magnitude of a posito (expensive as we are creating a new posito).
template<unsigned nbits, unsigned es> 
posito<nbits, es> abs(const posito<nbits, es>& p) {
	return p.abs();
}
template<unsigned nbits, unsigned es>
posito<nbits, es> fabs(const posito<nbits, es>& v) {
	posito<nbits, es> p(v);
	return p.abs();
}

// Atomic fused operators

// FMA: fused multiply-add:  a*b + c
template<unsigned nbits, unsigned es>
internal::value<1 + 2 * (nbits - es)> fma(const posito<nbits, es>& a, const posito<nbits, es>& b, const posito<nbits, es>& c) {
	constexpr unsigned fbits = nbits - 3 - es;
	constexpr unsigned fhbits = fbits + 1;      // size of fraction + hidden bit
	constexpr unsigned mbits = 2 * fhbits;      // size of the multiplier output
	constexpr unsigned abits = mbits + 4;       // size of the addend

	internal::value<mbits> product;
	internal::value<abits + 1> sum;
	internal::value<fbits> va, vb, ctmp;

	// special case handling of input arguments
	if (a.isnar() || b.isnar() || c.isnar()) {
		sum.setnan();
		return sum;
	}

	if (a.iszero() || b.iszero()) {  // product will only become non-zero if neither a and b are zero
		if (c.iszero()) {
			sum.setzero();
		}
		else {
			ctmp.set(sign(c), scale(c), extract_fraction<nbits, es, fbits>(c), c.iszero(), c.isnar());
			sum.template right_extend<fbits, abits + 1>(ctmp); // right-extend the c input argument and assign to sum
		}
	}
	else { // else clause guarantees that the product is non-zero	
		// first, the multiply: transform the inputs into (sign,scale,fraction) triples
		va.set(sign(a), scale(a), extract_fraction<nbits, es, fbits>(a), a.iszero(), a.isnar());;
		vb.set(sign(b), scale(b), extract_fraction<nbits, es, fbits>(b), b.iszero(), b.isnar());;

		module_multiply(va, vb, product);    // multiply the two inputs

		// second, the add : at this point we are guaranteed that product is non-zero and non-nar
		if (c.iszero()) {				
			sum.template right_extend<mbits, abits + 1>(product);   // right-extend the product and assign to sum
		}
		else {
			ctmp.set(sign(c), scale(c), extract_fraction<nbits, es, fbits>(c), c.iszero(), c.isnar());
			internal::value<mbits> vc;
			vc.template right_extend<fbits, mbits>(ctmp); // right-extend the c argument and assign to adder input
			module_add<mbits, abits>(product, vc, sum);
		}
	}

	return sum;
}

// FAM: fused add-multiply: (a + b) * c
template<unsigned nbits, unsigned es>
internal::value<2 * (nbits - 2 - es)> fam(const posito<nbits, es>& a, const posito<nbits, es>& b, const posito<nbits, es>& c) {
	constexpr unsigned fbits = nbits - 3 - es;
	constexpr unsigned abits = fbits + 4;       // size of the addend
	constexpr unsigned fhbits = fbits + 1;      // size of fraction + hidden bit
	constexpr unsigned mbits = 2 * fhbits;      // size of the multiplier output

	internal::value<fbits> va, vb;
	internal::value<abits+1> sum, vc;
	internal::value<mbits> product;

	// special case
	if (c.iszero()) return product;

	// first the add
	if (!a.iszero() || !b.iszero()) {
		// transform the inputs into (sign,scale,fraction) triples
		va.set(sign(a), scale(a), extract_fraction<nbits, es, fbits>(a), a.iszero(), a.isnar());;
		vb.set(sign(b), scale(b), extract_fraction<nbits, es, fbits>(b), b.iszero(), b.isnar());;

		module_add(va, vb, sum);    // multiply the two inputs
		if (sum.iszero()) return product;  // product is still zero
	}
	// second, the multiply		
	vc.set(c.get_size(), scale(c), extract_fraction<nbits, es, fbits>(c), c.iszero(), c.isnar());
	module_multiply(sum, vc, product);
	return product;
}

// FMMA: fused multiply-multiply-add: (a * b) +/- (c * d)
template<unsigned nbits, unsigned es>
internal::value<nbits> fmma(const posito<nbits, es>& a, const posito<nbits, es>& b, const posito<nbits, es>& c, const posito<nbits, es>& d, bool opIsAdd = true)
{
	// todo: implement
	internal::value<nbits> result;
	return result;
}



// free functions forms of member functions

template<unsigned nbits, unsigned es>
posito<nbits, es>& minpos(posito<nbits, es>& p) {
	return p.minpos();
}

template<unsigned nbits, unsigned es>
posito<nbits, es>& maxpos(posito<nbits, es>& p) {
	return p.maxpos();
}

template<unsigned nbits, unsigned es>
posito<nbits, es>& minneg(posito<nbits, es>& p) {
	return p.minneg();
}

template<unsigned nbits, unsigned es>
posito<nbits, es>& maxneg(posito<nbits, es>& p) {
	return p.maxneg();
}

}} // namespace sw::universal


