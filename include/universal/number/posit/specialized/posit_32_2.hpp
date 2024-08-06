#pragma once
// posit_32_2.hpp: specialized 32-bit posit using fast compute specialized for posit<32,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_32_2
#define POSIT_FAST_POSIT_32_2 0
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_32_2
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<32,2>")
//#else
//#warning("Fast specialization of posit<32,2>")
#endif

// fast specialized posit<32,2>
template<>
class posit<NBITS_IS_32, ES_IS_2> {
public:
	static constexpr unsigned nbits = NBITS_IS_32;
	static constexpr unsigned es = ES_IS_2;
	static constexpr unsigned sbits = 1;
	static constexpr unsigned rbits = nbits - sbits;
	static constexpr unsigned ebits = es;
	static constexpr unsigned fbits = nbits - 3 - es;
	static constexpr unsigned fhbits = fbits + 1;
	static constexpr uint32_t sign_mask = 0x80000000ul;  // 0x8000'0000ul;

	constexpr posit() : _bits(0) {}
	posit(const posit&) = default;
	posit(posit&&) = default;
	posit& operator=(const posit&) = default;
	posit& operator=(posit&&) = default;

	// specific value constructor
	constexpr posit(const SpecificValue code) : _bits(0) {
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
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:
			setnar();
			break;
		}
	}

	// initializers for native types
	explicit constexpr posit(signed char initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(short initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(int initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(long initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(long long initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(char initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(unsigned short initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(unsigned int initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(unsigned long initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(unsigned long long initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(float initial_value) : _bits(0) { *this = initial_value; }
	                   posit(double initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(long double initial_value) : _bits(0) { *this = initial_value; }

	// assignment operators for native types
	constexpr posit& operator=(signed char rhs) { return integer_assign((long)(rhs)); }
	constexpr posit& operator=(short rhs) { return integer_assign((long)(rhs)); }
	constexpr posit& operator=(int rhs) { return integer_assign((long)(rhs)); }
	constexpr posit& operator=(long rhs) { return integer_assign(rhs); }
	posit& operator=(long long rhs) { return float_assign((long double)(rhs)); }
	constexpr posit& operator=(char rhs) { return integer_assign((long)(rhs)); }
	constexpr posit& operator=(unsigned short rhs) { return integer_assign((long)(rhs)); }
	constexpr posit& operator=(unsigned int rhs) { return integer_assign((long)(rhs)); }
	          posit& operator=(unsigned long rhs) { return float_assign((long double)(rhs)); }
	          posit& operator=(unsigned long long rhs) { return float_assign((long double)(rhs)); }
	          posit& operator=(float rhs) { return float_assign((long double)rhs); }
	          posit& operator=(double rhs) { return float_assign((long double)rhs); }
	          posit& operator=(long double rhs) { return float_assign(rhs); }

	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator long() const { return to_long(); }
	explicit operator int() const { return to_int(); }
	explicit operator unsigned long long() const { return to_long_long(); }
	explicit operator unsigned long() const { return to_long(); }
	explicit operator unsigned int() const { return to_int(); }

	posit operator-() const {
		posit p;
		uint64_t raw = _bits;
		return p.setbits((~raw) + 1ull);
	}
	// arithmetic assignment operators
	posit& operator+=(const posit& b) {
		// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || b.isnar()) {
			throw posit_operand_is_nar{};
		}
#else
		if (isnar() || b.isnar()) {
			setnar();
			return *this;
		}
#endif
		if (b.iszero()) return *this;
		if (iszero()) { _bits = b._bits; return *this; }
		if (isneg() != b.isneg()) return *this -= b.twosComplement();

		uint32_t lhs = _bits;
		uint32_t rhs = b._bits;
		bool sign = bool(_bits & sign_mask);
		if (sign) {
			lhs = -int32_t(lhs) & 0xFFFFFFFF;
			rhs = -int32_t(rhs) & 0xFFFFFFFF;
		}
		if (lhs < rhs) std::swap(lhs, rhs);

		// decode the regime of lhs
		int32_t m = 0; // pattern length
		uint32_t remaining = 0;
		decode_regime(lhs, m, remaining);

		// extract the exponent
		uint32_t exp = (remaining >> 29);

		// extract the remaining fraction
		uint64_t frac64A{ 0 }, frac64B{ 0 };
		frac64A = ((0x4000'0000ull | uint64_t(remaining) << 1) & 0x7FFF'FFFFull) << 32;

		int32_t shiftRight = m;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		frac64B = ((0x4000'0000ull | uint64_t(remaining) << 1) & 0x7FFF'FFFFull) << 32;

		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 29);

		// Work-around CLANG (LLVM) compiler when shifting right more than number of bits
		frac64B = (shiftRight > 63) ? 0 : (frac64B >> shiftRight);

		frac64A += frac64B; // add the now aligned fractions

		bool rcarry = bool(0x8000'0000'0000'0000 & frac64A); // is MSB set   bool(0x8000'0000'0000'0000 & frac64A); 
		if (rcarry) {
			++exp;
			if (exp > 3) {
				++m;
				exp &= 0x3;
			}
			frac64A >>= 1;
		}

		_bits = round(m, exp, frac64A);
		if (sign) _bits = -int32_t(_bits) & 0xFFFF'FFFF;
		return *this;
	}
	posit& operator+=(double rhs) {
		return *this += posit<nbits, es>(rhs);
	}
	posit& operator-=(const posit& b) {
		// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || b.isnar()) {
			throw posit_operand_is_nar{};
		}
#else
		if (isnar() || b.isnar()) {
			setnar();
			return *this;
		}
#endif
		if (b.iszero()) return *this;
		if (iszero()) { _bits = -int32_t(b._bits) & 0xFFFFFFFF; return *this; }
		posit bComplement = b.twosComplement();
		if (isneg() != b.isneg()) return *this += bComplement;

		uint32_t lhs = _bits;
		uint32_t rhs = bComplement._bits;
		// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
		bool sign = bool(lhs & sign_mask);
		(sign) ? (lhs = (-int32_t(lhs) & 0xFFFFFFFF)) : (rhs = (-int32_t(rhs) & 0xFFFFFFFF));

		if (lhs == rhs) {
			_bits = 0;
			return *this;
		}
		if (lhs < rhs) {
			std::swap(lhs, rhs);
			sign = !sign;
		}

		// decode the regime of lhs
		int32_t m = 0; // pattern length
		uint32_t remaining = 0;
		decode_regime(lhs, m, remaining);

		// extract the exponent
		uint32_t exp = remaining >> 29;

		// extract the remaining fraction
		uint64_t frac64A = ((0x40000000ull | uint64_t(remaining) << 1) & 0x7FFFFFFFull) << 32;
		int32_t shiftRight = m;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		uint64_t frac64B = ((0x40000000ull | uint64_t(remaining) << 1ull) & 0x7FFFFFFFull) << 32;

		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 29);
		if (shiftRight > 63) {  // catastrophic cancellation case
			_bits = lhs;
			if (sign) _bits = -int32_t(_bits) & 0xFFFFFFFF;
			return *this;
		}
		else {
			frac64B >>= shiftRight;  // adjust the rhs fraction
		}
		frac64A -= frac64B;			// subtract the aligned fractions

		// adjust the results
		while ((frac64A >> 59) == 0) {
			--m;
			frac64A <<= 4;
		}
		bool ecarry = bool(0x4000000000000000 & frac64A);
		//bool ecarry = bool(0x4000'0000'0000'0000 & frac64A);
		while (!ecarry) {
			if (exp == 0) {
				--m;
				exp = 0x3;
			}
			else {
				exp--;
			}
			frac64A <<= 1;
			ecarry = bool(0x4000000000000000 & frac64A);
			//ecarry = bool(0x4000'0000'0000'0000 & frac64A);
		}

		_bits = round(m, exp, frac64A);
		if (sign) _bits = -int32_t(_bits) & 0xFFFFFFFF;
		return *this;
	}
	posit& operator-=(double rhs) {
		return *this -= posit<nbits, es>(rhs);
	}
	posit& operator*=(const posit& b) {
		// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || b.isnar()) {
			throw posit_operand_is_nar{};
		}
#else
		if (isnar() || b.isnar()) {
			setnar();
			return *this;
		}
#endif // POSIT_THROW_ARITHMETIC_EXCEPTION

		if (iszero() || b.iszero()) {
			_bits = 0;
			return *this;
		}
		uint32_t lhs = _bits;
		uint32_t rhs = b._bits;
		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = (lhs & sign_mask) ? -int32_t(lhs) : lhs;
		rhs = (rhs & sign_mask) ? -int32_t(rhs) : rhs;

		// decode the regime of lhs
		int32_t m = 0;
		uint32_t remaining = 0;
		decode_regime(lhs, m, remaining);
		uint32_t exp = remaining >> 29;  // lhs exponent
		uint32_t lhs_fraction = ((remaining << 1) | 0x4000'0000) & 0x7FFF'FFFF;;

		// adjust shift and extract fraction bits of rhs
		extractMultiplicand(rhs, m, remaining);
		uint32_t rhs_fraction = (((remaining << 1) | 0x4000'0000) & 0x7FFF'FFFF);
		uint64_t result_fraction = uint64_t(lhs_fraction) * uint64_t(rhs_fraction);
		exp += (remaining >> 29);  // product exp is the sum of lhs exp and rhs exp

		// adjust exponent if it has overflown
		if (exp > 3) {
			++m;
			exp &= 0x3;
		}

		bool rcarry = bool(result_fraction >> 61); // the 3rd bit of the 64-bit result fraction
		if (rcarry) {
			++exp;
			if (exp > 3) { // adjust again if we have overflown
				++m;
				exp &= 0x3;
			}
			result_fraction >>= 1;
		}

		// round
		_bits = round_mul(m, exp, result_fraction);
		if (sign) _bits = -int32_t(_bits) & 0xFFFF'FFFF;
		return *this;
	}
	posit& operator*=(double rhs) {
		return *this *= posit<nbits, es>(rhs);
	}
	posit& operator/=(const posit& b) {
		// since we are encoding error conditions as NaR (Not a Real), we need to process that condition first
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (b.iszero()) {
			throw posit_divide_by_zero{};    // not throwing is a quiet signalling NaR
		}
		if (b.isnar()) {
			throw posit_divide_by_nar{};
		}
		if (isnar()) {
			throw posit_numerator_is_nar{};
		}
#else
		if (isnar() || b.isnar() || b.iszero()) {
			setnar();
			return *this;
		}
#endif // POSIT_THROW_ARITHMETIC_EXCEPTION
		if (iszero()) {
			setzero();
			return *this;
		}

		uint32_t lhs = _bits;
		uint32_t rhs = b._bits;
		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = (lhs & sign_mask) ? -int32_t(lhs) : lhs;
		rhs = (rhs & sign_mask) ? -int32_t(rhs) : rhs;

		// decode the regime of lhs
		int32_t m = 0;
		uint32_t remaining = 0;
		decode_regime(lhs, m, remaining);

		// extract exponent
		int32_t exp = remaining >> 29;

		// extract the lhs fraction
		uint32_t lhs_fraction = ((remaining << 1) | 0x4000'0000) & 0x7FFF'FFFF;
		uint64_t lhs64 = uint64_t(lhs_fraction) << 30;

		// adjust shift and extract fraction bits of rhs
		extractDividand(rhs, m, remaining);
		// calculate exponent, exp = lhs_exp - rhs_exp
		exp -= (remaining >> 29);
		uint32_t rhs_fraction = ((remaining << 1) | 0x4000'0000) & 0x7FFF'FFFF;

		// execute the integer division of fractions
		lldiv_t result = lldiv(lhs64, uint64_t(rhs_fraction));
		uint64_t result_fraction = result.quot;
		uint64_t remainder = result.rem;

		// adjust exponent if underflowed
		if (exp < 0) {
			exp += 4;
			--m;
		}

		if (result_fraction != 0) {
			bool rcarry = result_fraction >> 30; // this is the hidden bit (31th bit), extreme right bit is bit 0
			if (!rcarry) {
				if (exp == 0) {
					--m;
					exp = 0x3u;
				}
				else {
					--exp;
				}
				result_fraction <<= 1;
			}
		}

		// round
		_bits = adjustAndRound(m, exp, result_fraction, remainder != 0);
		if (sign) _bits = -int32_t(_bits) & 0xFFFF'FFFF;
		return *this;
	}
	posit& operator/=(double rhs) {
		return *this /= posit<nbits, es>(rhs);
	}

	// prefix/postfix operators
	posit& operator++() {
		++_bits;
		return *this;
	}
	posit operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit& operator--() {
		--_bits;
		return *this;
	}
	posit operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}
	posit reciprocal() const {
		posit p = 1.0 / *this;
		return p;
	}
	posit abs() const {
		if (isneg()) {
			return posit(-*this);
		}
		return *this;
	}

	// Modifiers
	constexpr void clear() noexcept { _bits = 0x0; }
	constexpr void setzero() noexcept { clear(); }
	constexpr void setnar() noexcept { _bits = 0x80000000; }
	posit& setBitblock(const sw::universal::bitblock<NBITS_IS_32>& raw) noexcept {
		_bits = uint32_t(raw.to_ulong());
		return *this;
	}
	constexpr posit& setbits(uint64_t value) noexcept {
		_bits = uint32_t(value & 0xFFFF'FFFFul);
		return *this;
	}
	constexpr posit& setbit(unsigned bitIndex, bool value = true) noexcept {
		uint32_t bit_mask = (0x1u << bitIndex);
		if (value) {
			_bits |= bit_mask;
		}
		else {
			_bits &= ~bit_mask;
		}
		return *this;
	}
	posit& minpos() {
		clear();
		return ++(*this);
	}
	posit& maxpos() {
		setnar();
		return --(*this);
	}
	posit& zero() {
		clear();
		return *this;
	}
	posit& minneg() {
		clear();
		return --(*this);
	}
	posit& maxneg() {
		setnar();
		return ++(*this);
	}

	// Selectors
	constexpr bool sign() const       { return (_bits & 0x80000000u); }
	constexpr bool isnar() const      { return (_bits == 0x80000000u); }
	constexpr bool iszero() const     { return (_bits == 0x0); }
	constexpr bool isone() const      { return (_bits == 0x40000000u); } // pattern 010000...
	constexpr bool isminusone() const { return (_bits == 0xC0000000u); } // pattern 110000...
	constexpr bool isneg() const      { return (_bits & 0x80000000u); }
	constexpr bool ispos() const      { return !isneg(); }
	constexpr bool ispowerof2() const { return !(_bits & 0x1); }

	int sign_value() const { return (_bits & 0x8) ? -1 : 1; }

	bitblock<NBITS_IS_32> get() const { bitblock<NBITS_IS_32> bb; bb = long(_bits); return bb; }
	unsigned long long bits() const { return (unsigned long long)(_bits); }
	inline posit twosComplement() const {
		posit p;
		uint64_t raw = _bits;
		return p.setbits((~raw) + 1ull);
	}

#ifdef NEW_TO_VALUE
	int rscale() const { // scale of the regime
		return 1;
	}
	int escale() const { // scale of the exponent
		return 1;
	}
	bitblock<fbits> frac() const {
		bitblock<fbits> f;
		return f;
	}
	value<fbits> to_value() const {
		bool _sign = isneg();
		return value<fbits>(_sign, rscale() + escale(), frac(), iszero(), isnar());
	}
#else
	internal::value<fbits> to_value() const {
		bool		     	 _sign;
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		bitblock<nbits>		 _raw_bits;
		_raw_bits.reset();
		uint64_t mask = 1;
		for (unsigned i = 0; i < nbits; i++) {
			_raw_bits.set(i, (_bits & mask));
			mask <<= 1;
		}
		decode(_raw_bits, _sign, _regime, _exponent, _fraction);
		return internal::value<fbits>(_sign, _regime.scale() + _exponent.scale(), _fraction.get(), iszero(), isnar());
	}
#endif

private:
	uint32_t _bits;

	// Conversion functions
#if POSIT_THROW_ARITHMETIC_EXCEPTION
	int         to_int() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return int(to_float());
	}
	long        to_long() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return long(to_double());
	}
	long long   to_long_long() const {
		if (iszero()) return 0;
		if (isnar()) throw posit_nar{};
		return long(to_long_double());
	}
#else
	int         to_int() const {
		if (iszero()) return 0;
		if (isnar())  return int(INFINITY);
		return int(to_float());
	}
	long        to_long() const {
		if (iszero()) return 0;
		if (isnar())  return long(INFINITY);
		return long(to_double());
	}
	long long   to_long_long() const {
		if (iszero()) return 0;
		if (isnar())  return (long long)(INFINITY);
		return long(to_long_double());
	}
#endif
	float       to_float() const {
		return (float)to_double();
	}
	double      to_double() const {
		if (iszero())	return 0.0;
		if (isnar())	return NAN;
		bool		     	 _sign;
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		bitblock<nbits>		 _raw_bits;
		_raw_bits.reset();
		uint64_t mask = 1;
		for (unsigned i = 0; i < nbits; i++) {
			_raw_bits.set(i, (_bits & mask));
			mask <<= 1;
		}
		decode(_raw_bits, _sign, _regime, _exponent, _fraction);
		double s = (_sign ? -1.0 : 1.0);
		double r = _regime.value();
		double e = _exponent.value();
		double f = (1.0 + _fraction.value());
		return s * r * e * f;
	}
	long double to_long_double() const {
		if (iszero())  return 0.0;
		if (isnar())   return static_cast<long double>(NAN);
		bool		     	 _sign;
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		bitblock<nbits>		 _raw_bits;
		_raw_bits.reset();
		uint64_t mask = 1;
		for (unsigned i = 0; i < nbits; i++) {
			_raw_bits.set(i, (_bits & mask));
			mask <<= 1;
		}
		decode(_raw_bits, _sign, _regime, _exponent, _fraction);
		long double s = (_sign ? -1.0 : 1.0);
		long double r = _regime.value();
		long double e = _exponent.value();
		long double f = (1.0 + _fraction.value());
		return s * r * e * f;
	}

	// helper methods
	constexpr posit& integer_assign(long rhs) {
		// special case for speed as this is a common initialization
		if (rhs == 0) {
			_bits = 0x0;
			return *this;
		}

		bool sign = rhs < 0 ? true : false;
		uint32_t v = sign ? -rhs : rhs; // project to positive side of the projective reals
		int32_t raw = 0;          // we can use signed integer representation as we are taking care of the sign bit
		if (v == sign_mask) { // +-maxpos, 0x8000'0000 is special in int32 arithmetic as it is its own negation
			raw = 0x7FB00000;     // -2147483648  0x7FB0'0000; 
		}
//			else if (v > 0xFFFFFBFF) { //  4294966271        // this is for unsigned arguments
//				raw = 0x7FC00000;     //  4294967296
//			}
		else if (v < 0x2) {        // 0 and 1
			raw = (v << 30);
		}
		else {
			int8_t m = 31;
			uint32_t fraction_bits = v;
			while (!(fraction_bits & sign_mask)) {
				--m;
				fraction_bits <<= 1;
			}
			int8_t k = (m >> 2);
			uint32_t exponent_bits = (m & 0x3) << (27 - k);
			fraction_bits = (fraction_bits ^ sign_mask);
			raw = (0x7FFFFFFF ^ (0x3FFFFFFF >> k)) | exponent_bits | (fraction_bits >> (k + 4));

			uint32_t fraction_bit_mask = 0x8 << k; //bitNPlusOne
			if (fraction_bit_mask & fraction_bits) {
				if (((fraction_bit_mask - 1) & fraction_bits) | ((fraction_bit_mask << 1) & fraction_bits)) raw++;
			}
		}
		_bits = sign ? -raw : raw;
		return *this;
	}
	posit& float_assign(long double rhs) {
		constexpr int dfbits = std::numeric_limits<long double>::digits - 1;
		internal::value<dfbits> v(rhs);
		// special case processing
		if (v.iszero()) {
			setzero();
			return *this;
		}
		if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
			setnar();
			return *this;
		}

		internal::bitblock<NBITS_IS_32> ptt;
		convert_to_bb<NBITS_IS_32, ES_IS_2, dfbits>(v.sign(), v.scale(), v.fraction(), ptt); // TODO: needs to be faster
		_bits = uint32_t(ptt.to_ulong());
		return *this;
	}

	// decode_regime takes the raw bits of the posit, and returns the regime run-length, m, and the remaining fraction bits in remainder
	inline void decode_regime(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
		remaining = (bits << 2) & 0xFFFFFFFF;
		if (bits & 0x40000000) {  // positive regimes
			while (remaining >> 31) {
				++m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
		}
		else {              // negative regimes
			m = -1;
			while (!(remaining >> 31)) {
				--m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
			remaining &= 0x7FFFFFFF;
		}
	}
	inline void extractAddand(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
		remaining = (bits << 2) & 0xFFFFFFFF;
		if (bits & 0x40000000) {  // positive regimes
			while (remaining >> 31) {
				--m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
		}
		else {              // negative regimes
			++m;
			while (!(remaining >> 31)) {
				++m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
			remaining &= 0x7FFFFFFF;
		}
	}
	inline void extractMultiplicand(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
		remaining = (bits << 2) & 0xFFFFFFFF;
		if (bits & 0x40000000) {  // positive regimes
			while (remaining >> 31) {
				++m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
		}
		else {              // negative regimes
			--m;
			while (!(remaining >> 31)) {
				--m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
			remaining &= 0x7FFFFFFF;
		}
	}
	inline void extractDividand(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
		remaining = (bits << 2) & 0xFFFFFFFF;
		if (bits & 0x40000000) {  // positive regimes
			while (remaining >> 31) {
				--m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
		}
		else {              // negative regimes
			++m;
			while (!(remaining >> 31)) {
				++m;
				remaining = (remaining << 1) & 0xFFFFFFFF;
			}
			remaining &= 0x7FFFFFFF;
		}
	}

	inline uint32_t round(const int8_t m, uint32_t exp, uint64_t fraction) const {
		uint32_t scale, regime, bits;
		if (m < 0) {
			scale = -m;
			regime = 0x40000000 >> scale;
		}
		else {
			scale = m + 1;
			regime = 0x7FFFFFFF - (0x7FFFFFFF >> scale);
		}

		if (scale > 30) {
			bits = m<0 ? 0x1 : 0x7FFFFFFF;  // minpos and maxpos
		}
		else {
			fraction = (fraction & 0x3FFFFFFFFFFFFFFF) >> (scale + 2);
			//fraction = (fraction & 0x3FFF'FFFF'FFFF'FFFF) >> (scale + 2);
			uint32_t final_fbits = uint32_t(fraction >> 32);
			bool bitNPlusOne = false;
			if (scale <= 28) {
				bitNPlusOne = bool(0x80000000 & fraction);
				exp <<= (28 - scale);
			}
			else {
				if (scale == 30) {
					bitNPlusOne = bool(exp & 0x2);
					exp = 0;
				}
				else if (scale == 29) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (final_fbits > 0) {
					final_fbits = 0x0;
				}
			}
			bits = uint32_t(regime) + uint32_t(exp) + uint32_t(final_fbits);
			// n+1 frac bit is 1. Need to check if another bit is 1 too, if not round to even
			if (bitNPlusOne) {
				uint32_t moreBits = (0x7FFFFFFF & fraction) ? 0x1 : 0x0;
				bits += (bits & 0x0000001) | moreBits;
			}
		}
		return bits;
	}
	inline uint32_t round_mul(const int8_t m, uint32_t exp, uint64_t fraction) const {
		uint32_t scale, regime, bits;
		if (m < 0) {
			scale = -m;
			regime = 0x40000000 >> scale;
		}
		else {
			scale = m + 1;
			regime = 0x7FFFFFFF - (0x7FFFFFFF >> scale);
		}

		if (scale > 30) {
			bits = m<0 ? 0x1 : 0x7FFFFFFF;  // minpos and maxpos
		}
		else {
			//std::cout << "fracin = " << std::hex << fraction << std::dec << std::endl;
			fraction = (fraction & 0x0FFFFFFFFFFFFFFF) >> scale;
			//fraction = (fraction & 0x0FFF'FFFF'FFFF'FFFF) >> scale;
			//std::cout << "fracsh = " << std::hex << fraction << std::dec << std::endl;

			uint32_t final_fbits = uint32_t(fraction >> 32);
			bool bitNPlusOne = false;
			if (scale <= 28) {
				bitNPlusOne = bool(0x0000000080000000 & fraction);
				//bitNPlusOne = bool(0x0000'0000'8000'0000 & fraction);
				exp <<= (28 - scale);
			}
			else {
				if (scale == 30) {
					bitNPlusOne = bool(exp & 0x2);
					exp = 0;
				}
				else if (scale == 29) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (final_fbits > 0) {
					final_fbits = 0;
				}
			}
			// sign is set by the calling environment as +/- behaves differently compared to */div
			bits = uint32_t(regime) + uint32_t(exp) + uint32_t(final_fbits);

			//std::cout << "scale  = " << scale << std::endl;
			//std::cout << "frac64 = " << std::hex << fraction << std::dec << std::endl;
			//std::cout << std::hex ;
			//std::cout << "regime = " << regime << std::endl;
			//std::cout << "expone = " << exp << std::endl;
			//std::cout << "fracti = " << final_fbits << std::endl;
			//std::cout << std::dec ;

			// n+1 frac bit is 1. Need to check if another bit is 1 too, if not round to even
			if (bitNPlusOne) {
				uint32_t moreBits = (0x7FFFFFFF & fraction) ? 0x1 : 0x0;
				bits += (bits & 0x0000001) | moreBits;
			}
		}
		return bits;
	}
	inline uint32_t adjustAndRound(const int8_t k, uint32_t exp, uint64_t frac64, bool nonZeroRemainder) const {
		uint32_t reglen, regime, bits;
		if (k < 0) {
			reglen = -k;
			regime = 0x4000'0000 >> reglen;
		}
		else {
			reglen = k + 1;
			regime = 0x7FFF'FFFF - (0x7FFF'FFFF >> reglen);
		}

		if (reglen > 30) {
			bits = (k<0 ? 0x1 : 0x7FFF'FFFF);  // minpos and maxpos
		}
		else {
			// remove carry and rcarry bits and shift to correct position
			frac64 &= 0x3FFF'FFFF;
			uint32_t fraction = uint32_t((frac64) >> (reglen + 2));

			bool bitNPlusOne = false;
			uint32_t moreBits{ 0 };
			if (reglen <= 28) {
				bitNPlusOne = bool ((frac64 >> (reglen + 1)) & 0x1);
				exp <<= (28 - reglen);
				if (bitNPlusOne) moreBits = (((1ull << (reglen + 1)) - 1ull) & frac64) ? 0x1 : 0x0;
			}
			else {
				if (reglen == 30) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (reglen == 29) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (frac64 > 0) {
					fraction = 0;
					moreBits = 0x1;
				}
			}
			if (nonZeroRemainder) moreBits = 0x1;
			bits = uint32_t(regime) | uint32_t(exp) | uint32_t(fraction);
			if (bitNPlusOne) bits += (bits & 0x1) | moreBits;
#define TRACE_DIV_
#ifdef TRACE_DIV
			std::cout << "reglen         = " << reglen << std::endl;
			std::cout << "regime         = " << to_binary(regime) << std::endl;
			std::cout << "exponent       = " << exp << std::endl;
			std::cout << "fraction raw   = " << to_binary(frac64, true, 64) << std::endl;
			std::cout << "fraction final = " << to_binary(fraction, true, 32) << std::endl;
			std::cout << "posit bits     = " << to_binary(bits, true, 32) << std::endl;
#endif
		}
		return bits;
	}
	// I/O operators
	friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_32, ES_IS_2>& p);
	friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_32, ES_IS_2>& p);

	// posit - posit logic functions
	friend bool operator==(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs);
	friend bool operator!=(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs);
	friend bool operator< (const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs);
	friend bool operator> (const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs);
	friend bool operator<=(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs);
	friend bool operator>=(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs);

	friend bool operator< (const posit<NBITS_IS_32, ES_IS_2>& lhs, double rhs);
	friend bool operator< (double lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs);
};

// posit I/O operators
// generate a posit format ASCII format nbits.esxNN...NNp
inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_32, ES_IS_2>& p) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the posit into a string
	std::stringstream ss;
#if POSIT_ERROR_FREE_IO_FORMAT
	ss << NBITS_IS_32 << '.' << ES_IS_2 << 'x' << to_hex(p.get()) << 'p';
#else
	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << to_string(p, prec);  // TODO: we need a true native serialization function
#endif
	return ostr << ss.str();
}

// read an ASCII float or posit format: nbits.esxNN...NNp, for example: 32.2x80000000p
inline std::istream& operator>> (std::istream& istr, posit<NBITS_IS_32, ES_IS_2>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

// convert a posit value to a string using "nar" as designation of NaR
inline std::string to_string(const posit<NBITS_IS_32, ES_IS_2>& p, std::streamsize precision) {
	if (p.isnar()) {
		return std::string("nar");
	}
	std::stringstream ss;
	ss << std::setprecision(precision) << float(p);
	return ss.str();
}

inline bool twosComplementLessThan(std::uint32_t lhs, std::uint32_t rhs) {
	// comparison of the sign bit
	uint32_t mask = 0x8000'0000;
	if ((lhs & mask) == 0 && (rhs & mask) == mask)	return false;
	if ((lhs & mask) == mask && (rhs & mask) == 0) return true;
	// sign is equal, compare the remaining bits
	mask >>= 1;
	while (mask > 0) {
		if ((lhs & mask) == 0 && (rhs & mask) == mask)	return true;
		if ((lhs & mask) == mask && (rhs & mask) == 0) return false;
		mask >>= 1;
	}
	// numbers are equal
	return false;
}

// posit - posit binary logic operators
inline bool operator==(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return lhs._bits == rhs._bits;
}
inline bool operator!=(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return !operator==(lhs, rhs);
}
inline bool operator< (const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return int32_t(lhs._bits) < int32_t(rhs._bits);
}
inline bool operator> (const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return operator< (rhs, lhs);
}
inline bool operator<=(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
inline bool operator>=(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return !operator< (lhs, rhs);
}

// binary operator+() is provided by generic function
// binary operator-() is provided by generic function
// binary operator*() is provided by generic function
// binary operator/() is provided by generic function

#if POSIT_ENABLE_LITERALS
// posit - literal logic functions

// posit - int logic operators
inline bool operator==(const posit<NBITS_IS_32, ES_IS_2>& lhs, int rhs) {
	return operator==(lhs, posit<NBITS_IS_32, ES_IS_2>(rhs));
}
inline bool operator!=(const posit<NBITS_IS_32, ES_IS_2>& lhs, int rhs) {
	return !operator==(lhs, posit<NBITS_IS_32, ES_IS_2>(rhs));
}
inline bool operator< (const posit<NBITS_IS_32, ES_IS_2>& lhs, int rhs) {
	return operator<(lhs, posit<NBITS_IS_32, ES_IS_2>(rhs));
}
inline bool operator> (const posit<NBITS_IS_32, ES_IS_2>& lhs, int rhs) {
	return operator< (posit<NBITS_IS_32, ES_IS_2>(rhs), lhs);
}
inline bool operator<=(const posit<NBITS_IS_32, ES_IS_2>& lhs, int rhs) {
	return operator< (lhs, posit<NBITS_IS_32, ES_IS_2>(rhs)) || operator==(lhs, posit<NBITS_IS_32, ES_IS_2>(rhs));
}
inline bool operator>=(const posit<NBITS_IS_32, ES_IS_2>& lhs, int rhs) {
	return !operator<(lhs, posit<NBITS_IS_32, ES_IS_2>(rhs));
}

// int - posit logic operators
inline bool operator==(int lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return posit<NBITS_IS_32, ES_IS_2>(lhs) == rhs;
}
inline bool operator!=(int lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return !operator==(posit<NBITS_IS_32, ES_IS_2>(lhs), rhs);
}
inline bool operator< (int lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return operator<(posit<NBITS_IS_32, ES_IS_2>(lhs), rhs);
}
inline bool operator> (int lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return operator< (posit<NBITS_IS_32, ES_IS_2>(rhs), lhs);
}
inline bool operator<=(int lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return operator< (posit<NBITS_IS_32, ES_IS_2>(lhs), rhs) || operator==(posit<NBITS_IS_32, ES_IS_2>(lhs), rhs);
}
inline bool operator>=(int lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
	return !operator<(posit<NBITS_IS_32, ES_IS_2>(lhs), rhs);
}

inline bool operator< (const posit<NBITS_IS_32, ES_IS_2>& lhs, double rhs) {
	return twosComplementLessThan(lhs._bits, posit<NBITS_IS_32, ES_IS_2>(rhs)._bits);
}

#endif // POSIT_ENABLE_LITERALS

#endif // POSIT_FAST_POSIT_32_2

}} // namespace sw::universal
