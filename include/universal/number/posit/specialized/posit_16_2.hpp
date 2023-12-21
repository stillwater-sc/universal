#pragma once
// posit_16_2.hpp: specialized 16-bit posit using fast implementation specialized for posit<16,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/integers.hpp>

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/number/posit/posit.hpp>

#ifndef POSIT_FAST_POSIT_16_2
#define POSIT_FAST_POSIT_16_2 0
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_16_2
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<16,2>")
//#else    some compile time message that indicates that we are using a specialization for non MS compilers
//#warning("Fast specialization of posit<16,2>")
#endif

// fast specialized posit<16,2>
template<>
class posit<NBITS_IS_16, ES_IS_2> {
public:
	static constexpr unsigned nbits = NBITS_IS_16;
	static constexpr unsigned es    = ES_IS_2;
	static constexpr unsigned sbits = 1;
	static constexpr unsigned rbits = nbits - sbits;
	static constexpr unsigned ebits = es;
	static constexpr unsigned fbits = nbits - 3 - es;
	static constexpr unsigned fhbits = fbits + 1;
	static constexpr uint16_t sign_mask = 0x8000u;

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
	explicit constexpr posit(long long initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(char initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(unsigned short initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(unsigned int initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(unsigned long initial_value) : _bits(0) { *this = initial_value; }
	explicit constexpr posit(unsigned long long initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(float initial_value) : _bits(0) { *this = initial_value; }
	posit(double initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(long double initial_value) : _bits(0) { *this = initial_value; }

	// assignment operators for native types
	constexpr posit& operator=(signed char rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(short rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(int rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(long rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(long long rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(char rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(unsigned short rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(unsigned int rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(unsigned long rhs) { return integer_assign((long)rhs); }
	constexpr posit& operator=(unsigned long long rhs) { return integer_assign((long)rhs); }
	posit& operator=(float rhs) { return float_assign(double(rhs)); }
	posit& operator=(double rhs) { return float_assign(rhs); }
	posit& operator=(long double rhs) { return float_assign(double(rhs)); }

	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator long() const { return to_long(); }
	explicit operator int() const { return to_int(); }
	explicit operator unsigned long long() const { return to_long_long(); }
	explicit operator unsigned long() const { return to_long(); }
	explicit operator unsigned int() const { return to_int(); }

	posit& setBitblock(const bitblock<NBITS_IS_16>& raw) {
		_bits = uint16_t(raw.to_ulong());
		return *this;
	}
	constexpr posit& setbits(uint64_t value) {
		_bits = uint16_t(value & 0xffffu);
		return *this;
	}

	// arithmetic assignment operators
	constexpr posit operator-() const {
		posit p;
		return p.setbits((~_bits) + 1ul);
	}
	posit& operator+=(const posit& b) {
		// process special cases
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

		uint16_t lhs = _bits;
		uint16_t rhs = b._bits;
		bool sign = bool(_bits & sign_mask);
		if (sign) {
			lhs = -lhs & 0xFFFF;
			rhs = -rhs & 0xFFFF;
		}
		if (lhs < rhs) std::swap(lhs, rhs);

		// decode the regime of lhs
		int8_t k; // regime numerical value
		uint16_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		uint16_t exp = remaining >> 13; // 16 - 1(sign) - 2(exponent)

		// extract remaining fraction bits
		uint32_t lhs_fraction = ((0x4000 | remaining << 1) & 0x7FFF) << 16; // 0x4000 is the hidden bit
		int8_t shiftRight = k;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		uint32_t rhs_fraction = ((0x4000 | remaining << 1) & 0x7FFF) << 16; // 0x4000 is the hidden bit

		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 13);

		if (shiftRight == 0) { // we can simplify the computation
			lhs_fraction += rhs_fraction;  // this will always produce a carry
			++exp;
			if (exp > 3) {
				++k;
				exp &= 0x3;
			}
			lhs_fraction >>= 1;
		}
		else {
			(shiftRight > 15) ? (rhs_fraction = 0) : (rhs_fraction >>= shiftRight); // frac16B >>= shiftRight
			lhs_fraction += rhs_fraction;

			bool rcarry = (0x8000'0000 & lhs_fraction) != 0; // first left bit
			if (rcarry) {
				++exp;
				if (exp > 3) {
					++k;
					exp &= 0x3;
				}
				lhs_fraction >>= 1;
			}
		}

		_bits = round(k, exp, lhs_fraction);
		if (sign) _bits = -_bits & 0xFFFF;
		return *this;
	}
	posit& operator-=(const posit& b) {
		// process special cases
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
		if (iszero()) { _bits = -int16_t(b._bits) & 0xFFFF; return *this; }
		posit bComplement = b.twosComplement();
		if (isneg() != b.isneg()) return *this += bComplement;

		uint16_t lhs = _bits;
		uint16_t rhs = bComplement._bits;
		// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
		bool sign = bool(lhs & sign_mask);
		(sign) ? (lhs = (-lhs & 0xFFFF)) : (rhs = (-rhs & 0xFFFF));

		if (lhs == rhs) {
			_bits = 0x0;
			return *this;
		}
		if (lhs < rhs) {
			std::swap(lhs, rhs);
			sign = !sign;
		}

		// decode the regime of lhs
		int8_t k;  // regime numerical value
		uint16_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		uint16_t exp = remaining >> 13;  // 16 - 1(sign) - 2(exponent)

		uint32_t lhs_fraction = ((0x4000 | remaining << 1) & 0x7FFF) << 16; // 0x4000 is the hidden bit
		int8_t shiftRight = k;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		uint32_t rhs_fraction = ((0x4000 | remaining << 1) & 0x7FFF) << 16; // 0x4000 is the hidden bit

		// align the fractions for subtraction
		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 13);
		if (shiftRight > 31) {
			_bits = lhs;
			if (sign) _bits = -_bits & 0xFFFF;
			return *this;
		}
		else {
			rhs_fraction >>= shiftRight;
		}

		lhs_fraction -= rhs_fraction;

		while ((lhs_fraction >> 27) == 0) {
			--k;
			lhs_fraction <<= 4;
		}

		bool ecarry = (0x4000'0000 & lhs_fraction) != 0;
		while (!ecarry) {
			if (exp == 0) {
				--k;
				exp = 3;
			}
			else {
				--exp;
			}
			lhs_fraction <<= 1;
			ecarry = (0x4000'0000 & lhs_fraction) != 0;
		}

		_bits = round(k, exp, lhs_fraction);
		if (sign) _bits = -_bits & 0xFFFF;
		return *this;
	}
	posit& operator*=(const posit& b) {
		// process special cases
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
		if (iszero() || b.iszero()) {
			_bits = 0x0000;
			return *this;
		}
		uint16_t lhs = _bits;
		uint16_t rhs = b._bits;
		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = lhs & sign_mask ? -lhs : lhs;
		rhs = rhs & sign_mask ? -rhs : rhs;

		// decode the regime of lhs
		int8_t k = 0;  // regime numerical value
		uint16_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		int32_t exp = remaining >> 13;  // 16 - 1(sign) - 2(exponent)

		// extract remaining fraction bits
		uint16_t lhs_fraction = (0x4000 | remaining << 1) & 0x7FFF; // 0x4000 is the hidden bit
		// adjust shift and extract fraction bits of rhs
		extractMultiplicand(rhs, k, remaining);
		exp += (remaining >> 13);
		uint16_t rhs_fraction = (0x4000 | remaining << 1) & 0x7FFF; // 0x4000 is the hidden bit
		uint32_t result_fraction = (uint32_t) lhs_fraction * rhs_fraction;

		if (exp > 3) {
			++k;
			exp &= 0x3;
		}
	
		bool rcarry = bool(result_fraction & 0x20000000);
		if (rcarry) {
			exp++;
			if (exp > 3) {
				++k;
				exp &= 0x3;
			}
			result_fraction >>= 1;
		}

		// round
		_bits = adjustAndRound(k, exp, result_fraction);
		if (sign) _bits = -_bits & 0xFFFF;
		return *this;
	}
	posit& operator/=(const posit& b) {
		// process special cases
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

		uint16_t lhs = _bits;
		uint16_t rhs = b._bits;
		if (iszero()) {
			_bits = 0x0000;
			return *this;
		}

		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = lhs & sign_mask ? -lhs : lhs;
		rhs = rhs & sign_mask ? -rhs : rhs;

		// decode the regime of lhs
		int8_t k; // regime numerical value
		uint16_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		int32_t exp = remaining >> 13; // 16 - 1(sign) - 2(exponent)

		// extract remaining fraction bits
		uint16_t lhs_fraction = (0x4000 | remaining << 1) & 0x7FFF; // 0x4000 is the hidden bit
		uint32_t fraction = (uint32_t) lhs_fraction << 14;

		// adjust shift and extract fraction bits of rhs
		extractDividand(rhs, k, remaining);
		exp -= remaining >> 13;
		uint16_t rhs_fraction = (0x4000 | remaining << 1) & 0x7FFF; // 0x4000 is the hidden bit

		div_t result = div(fraction, rhs_fraction);
		uint32_t result_fraction = result.quot;
		uint32_t remainder = result.rem;

		// adjust the exponent if needed
		if (exp < 0) {
			exp += 4;
			--k;
		}
		if (result_fraction != 0) {
			bool rcarry = result_fraction >> 14; // this is the hidden bit (14th bit), extreme right bit is bit 0
			if (!rcarry) {
				if (exp == 0) {
					--k;
					exp = 3;
				}
				else {
					--exp;
				}
				result_fraction <<= 1;
			}
		}

		// round
		_bits = divRound(k, exp, result_fraction, remainder != 0);
		if (sign) _bits = -_bits & 0xFFFF;

		return *this;
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

	posit reciprocate() const {
		posit p = 1.0 / *this;
		return p;
	}
	posit abs() const {
		if (isneg()) {
			return posit(-*this);
		}
		return *this;
	}

	// Selectors
	bool sign() const { return (_bits & sign_mask); }
	bool isnar() const { return (_bits == sign_mask); }
	bool iszero() const { return (_bits == 0x0); }
	bool isone() const { return (_bits == 0x4000); } // pattern 010000...
	bool isminusone() const { return (_bits == 0xC000); } // pattern 110000...
	bool isneg() const { return (_bits & sign_mask); }
	bool ispos() const { return !isneg(); }
	bool ispowerof2() const { return !(_bits & 0x1); }

	int sign_value() const { return (_bits & 0x8 ? -1 : 1); }

	bitblock<NBITS_IS_16> get() const { bitblock<NBITS_IS_16> bb; bb = int(_bits); return bb; }
	uint16_t bits() const noexcept { return _bits; }
	unsigned long long encoding() const { return (unsigned long long)(_bits); }

	// Modifiers
	void clear() { _bits = 0; }
	void setzero() { clear(); }
	void setnar() { _bits = sign_mask; }
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
	posit twosComplement() const {
		posit p;
		return p.setbits(~_bits + 1ul);
	}

	internal::value<fbits> to_value() const {
		bool		     	 _sign;
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
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

private:
	uint16_t _bits;

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
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
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
		if (isnar())   return NAN;
		bool		     	 _sign;
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
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

		bool sign = (rhs < 0);
		uint32_t v = sign ? -rhs : rhs; // project to positve side of the projective reals
		uint16_t raw = 0;
		if (v > 0x08000000) { // v > 134,217,728
			raw = 0x7FFFu;  // +-maxpos
		}
		else if (v > 0x02FFFFFF) { // 50,331,647 < v < 134,217,728
			raw = 0x7FFEu;  // 0.5 of maxpos
		}
		else if (v < 2) {  // v == 0 or v == 1
			raw = (v << 14); // generates 0x0000 if v is 0, or 0x4000 if 1
		}
		else {
			uint32_t mask = 0x02000000;
			int8_t scale = 25;
			uint32_t fraction_bits = v;
			while (!(fraction_bits & mask)) {
				--scale;
				fraction_bits <<= 1;
			}
			int8_t k = scale >> 1;
			uint16_t exp = (scale & 0x01) << (12 - k); // extract exponent and shift to correct location
			fraction_bits = (fraction_bits ^ mask);
			raw = (0x7FFF ^ (0x3FFF >> k)) | exp | (fraction_bits >> (k + 13));

			mask = 0x1000 << k; // bitNPlusOne
			if (mask & fraction_bits) {
				if (((mask - 1) & fraction_bits) | ((mask << 1) & fraction_bits)) raw++; // increment by 1
			}
		}
		_bits = sign ? -raw : raw;
		return *this;
	}

	// convert a double precision IEEE floating point to a posit<16,1>. You need to use at least doubles to capture
	// enough bits to correctly round mul/div and elementary function results. That is, if you use a single precision
	// float, you will inject errors in the validation suites.
	posit& float_assign(double rhs) {
		constexpr int dfbits = std::numeric_limits<double>::digits - 1;
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

		bitblock<NBITS_IS_16> ptt;
		convert_to_bb<NBITS_IS_16, ES_IS_2, dfbits>(v.sign(), v.scale(), v.fraction(), ptt); // TODO: needs to be faster
		_bits = uint16_t(ptt.to_ulong());
		return *this;
	}

	// decode_regime takes the raw bits of the posit, and returns the 
	// regime numerical meaning, k, and the remaining bits shifted to
	// the left in remaining, with a 0 appended to the left. I.e.,
	// remaining = 0<remaining_bits>0..0
	//
	// The regime numerical meaning is as follows: If m is the number of
	// identical bits in the regime, if the bits are 0s, then k = −m;
	// if they are 1s, then k = m − 1.
	inline void decode_regime(const uint16_t bits, int8_t& k, uint16_t& remaining) const {
		remaining = (bits << 2) & 0xFFFF;  // sign and first regime bit
		if (bits & 0x4000) {  // positive regimes
			k = 0;
			while (remaining >> 15) {
				++k;
				remaining = (remaining << 1) & 0xFFFF;
			}
		}
		else {              // negative regimes
			k = -1;
			while (!(remaining >> 15)) {
				--k;
				remaining = (remaining << 1) & 0xFFFF;
			}
			remaining &= 0x7FFF;
		}
	}
	inline void extractAddand(const uint16_t bits, int8_t& shift, uint16_t& remaining) const {
		remaining = (bits << 2) & 0xFFFF;
		if (bits & 0x4000) {  // positive regimes
			while (remaining >> 15) {
				--shift;
				remaining = (remaining << 1) & 0xFFFF;
			}
		}
		else {              // negative regimes
			++shift;
			while (!(remaining >> 15)) {
				++shift;
				remaining = (remaining << 1) & 0xFFFF;
			}
			remaining &= 0x7FFF;
		}
	}
	// TODO: This is the same as decode_regime except the initialization of k. Can we combine them?
	inline void extractMultiplicand(const uint16_t bits, int8_t& k, uint16_t& remaining) const {
		remaining = (bits << 2) & 0xFFFF;
		if (bits & 0x4000) {  // positive regimes
			while (remaining >> 15) {
				++k;
				remaining = (remaining << 1) & 0xFFFF;
			}
		}
		else {              // negative regimes
			--k;
			while (!(remaining >> 15)) {
				--k;
				remaining = (remaining << 1) & 0xFFFF;
			}
			remaining &= 0x7FFF;
		}
	}
	// TODO: This is the same as extractAddand. Can we combine them?
	inline void extractDividand(const uint16_t bits, int8_t& k, uint16_t& remaining) const {
		remaining = (bits << 2) & 0xFFFF;
		if (bits & 0x4000) {  // positive regimes
			while (remaining >> 15) {
				--k;
				remaining = (remaining << 1) & 0xFFFF;
			}
		}
		else {              // negative regimes
			++k;
			while (!(remaining >> 15)) {
				++k;
				remaining = (remaining << 1) & 0xFFFF;
			}
			remaining &= 0x7FFF;
		}
	}
	inline uint16_t round(const int8_t k, uint16_t exp, uint32_t fraction) const {
		int16_t scale;
		uint16_t regime, bits;
		if (k < 0) {
			scale = (-k & 0xFFFF);
			regime = 0x4000 >> scale;
		}
		else {
			scale = int16_t(k) + 1;
			regime = 0x7FFF - (0x7FFF >> scale);
		}

		if (scale > 14) {
			bits = k < 0 ? 0x0001 : 0x7FFF;  // minpos and maxpos. exp and frac do not matter
		}
		else {
			fraction = (fraction & 0x3FFFFFFF) >> (scale + 2); // remove both carry bits, 2 bits exp
			uint16_t final_fbits = uint16_t(fraction >> 16);
			bool bitNPlusOne = false;
			uint16_t moreBits = 0;
			if (scale <= 12) {
				bitNPlusOne = bool(0x8000 & fraction);
				exp <<= (12 - scale);
			}
			else {
				if (scale == 14) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (scale == 13) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (final_fbits > 0) {
					final_fbits = 0;
					moreBits = 1;
				}
			}

			bits = uint16_t(regime) + uint16_t(exp) + uint16_t(final_fbits);
			// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			if (bitNPlusOne) {
				if (0x7FFF & fraction) moreBits = 1;
				bits += (bits & 0x0001) | moreBits;
			}
		}
		return bits;
	}
	inline uint16_t divRound(const int8_t k, uint16_t exp, uint32_t fraction, bool nonZeroRemainder) const {
		int16_t scale;
		uint16_t regime, bits;
		if (k < 0) {
			scale = (-k & 0xFFFF);
			regime = 0x4000 >> scale;
		}
		else {
			scale = int16_t(k) + 1;
			regime = 0x7FFF - (0x7FFF >> scale);
		}

		if (scale > 14) {
			bits = k < 0 ? 0x0001 : 0x7FFF;  // minpos and maxpos. exp and frac do not matter 
		}
		else {
			fraction &= 0x3FFF; //remove carry and rcarry bits and shift to correct position
			uint16_t final_fbits = uint16_t(fraction >> (scale + 2));
			bool bitNPlusOne = false;
			uint16_t moreBits = 0;
			if (scale <= 12) {
				bitNPlusOne = bool((fraction >> (scale + 1)) & 0x1);
				exp <<= (12 - scale);
				if (bitNPlusOne && (((1 << (scale + 1)) - 1) & fraction)) {
					moreBits = 0x0001;
				}
			}
			else {
				if (scale == 14) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (scale == 13) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (final_fbits > 0) {
					final_fbits = 0;
					moreBits = 0x0001;
				}
			}

			bits = uint16_t(regime) + uint16_t(exp) + uint16_t(final_fbits);

			if (bitNPlusOne) {
				if (nonZeroRemainder) moreBits = 0x0001;
				bits += (bits & 0x0001) | moreBits;
			}
		}
		return bits;
	}
	inline uint16_t adjustAndRound(const int8_t k, uint16_t exp, uint32_t fraction) const {
		int16_t scale;
		uint16_t regime, bits;
		if (k < 0) {
			scale = (-k & 0xFFFF);
			regime = 0x4000 >> scale;
		}
		else {
			scale = int16_t(k) + 1;
			regime = 0x7FFF - (0x7FFF >> scale);
		}

		if (scale > 14) {
			bits = k < 0 ? 0x0001 : 0x7FFF;  // minpos and maxpos. exp and frac do not matter 
		}
		else {
			// remove carry and rcarry bits and shift to correct position
			fraction = (fraction & 0x0FFFFFFF) >> scale;
			uint16_t final_fbits = uint16_t(fraction >> 16);
			bool bitNPlusOne = false;
			uint16_t moreBits = 0;
			if (scale <= 12) {
				bitNPlusOne = bool(0x8000 & fraction);
				exp <<= (12 - scale);
			}
			else {
				if (scale == 14) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (scale == 13) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (final_fbits > 0) {
					final_fbits = 0;
					moreBits = 1;
				}
			}

			bits = uint16_t(regime) + uint16_t(exp) + uint16_t(final_fbits);
			// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			if (bitNPlusOne) {
				if (0x7FFF & fraction) moreBits = 1;
				bits += (bits & 0x0001) | moreBits;
			}
		}
		return bits;
	}
	// I/O operators
	friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_16, ES_IS_2>& p);
	friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_16, ES_IS_2>& p);

	// posit - posit logic functions
	friend bool operator==(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs);
	friend bool operator!=(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs);
	friend bool operator< (const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs);
	friend bool operator> (const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs);
	friend bool operator<=(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs);
	friend bool operator>=(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs);

	friend bool operator< (const posit<NBITS_IS_16, ES_IS_2>& lhs, double rhs);
};

// posit I/O operators
// generate a posit format ASCII format nbits.esxNN...NNp
inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_16, ES_IS_2>& p) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the posit into a string
	std::stringstream ss;
#if POSIT_ERROR_FREE_IO_FORMAT
	ss << NBITS_IS_16 << '.' << ES_IS_2 << 'x' << to_hex(p.get()) << 'p';
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
inline std::istream& operator>> (std::istream& istr, posit<NBITS_IS_16, ES_IS_2>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

// convert a posit value to a string using "nar" as designation of NaR
inline std::string to_string(const posit<NBITS_IS_16, ES_IS_2>& p, std::streamsize precision) {
	if (p.isnar()) {
		return std::string("nar");
	}
	std::stringstream ss;
	ss << std::setprecision(precision) << float(p);
	return ss.str();
}

// posit - posit binary logic operators
inline bool operator==(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return lhs._bits == rhs._bits;
}
inline bool operator!=(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return !operator==(lhs, rhs);
}
inline bool operator< (const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return int16_t(lhs._bits) < int16_t(rhs._bits);
}
inline bool operator> (const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return operator< (rhs, lhs);
}
inline bool operator<=(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
inline bool operator>=(const posit<NBITS_IS_16, ES_IS_2>& lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return !operator< (lhs, rhs);
}

// binary operator+() is provided by generic function
// binary operator-() is provided by generic function
// binary operator*() is provided by generic function
// binary operator/() is provided by generic function

#if POSIT_ENABLE_LITERALS
// posit - literal logic functions

// posit - int logic operators
inline bool operator==(const posit<NBITS_IS_16, ES_IS_2>& lhs, int rhs) {
	return operator==(lhs, posit<NBITS_IS_16, ES_IS_2>(rhs));
}
inline bool operator!=(const posit<NBITS_IS_16, ES_IS_2>& lhs, int rhs) {
	return !operator==(lhs, posit<NBITS_IS_16, ES_IS_2>(rhs));
}
inline bool operator< (const posit<NBITS_IS_16, ES_IS_2>& lhs, int rhs) {
	return operator<(lhs, posit<NBITS_IS_16, ES_IS_2>(rhs));
}
inline bool operator> (const posit<NBITS_IS_16, ES_IS_2>& lhs, int rhs) {
	return operator< (posit<NBITS_IS_16, ES_IS_2>(rhs), lhs);
}
inline bool operator<=(const posit<NBITS_IS_16, ES_IS_2>& lhs, int rhs) {
	return operator< (lhs, posit<NBITS_IS_16, ES_IS_2>(rhs)) || operator==(lhs, posit<NBITS_IS_16, ES_IS_2>(rhs));
}
inline bool operator>=(const posit<NBITS_IS_16, ES_IS_2>& lhs, int rhs) {
	return !operator<(lhs, posit<NBITS_IS_16, ES_IS_2>(rhs));
}

// int - posit logic operators
inline bool operator==(int lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return posit<NBITS_IS_16, ES_IS_2>(lhs) == rhs;
}
inline bool operator!=(int lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return !operator==(posit<NBITS_IS_16, ES_IS_2>(lhs), rhs);
}
inline bool operator< (int lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return operator<(posit<NBITS_IS_16, ES_IS_2>(lhs), rhs);
}
inline bool operator> (int lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return operator< (posit<NBITS_IS_16, ES_IS_2>(rhs), lhs);
}
inline bool operator<=(int lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return operator< (posit<NBITS_IS_16, ES_IS_2>(lhs), rhs) || operator==(posit<NBITS_IS_16, ES_IS_2>(lhs), rhs);
}
inline bool operator>=(int lhs, const posit<NBITS_IS_16, ES_IS_2>& rhs) {
	return !operator<(posit<NBITS_IS_16, ES_IS_2>(lhs), rhs);
}

inline bool operator< (const posit<NBITS_IS_16, ES_IS_2>& lhs, double rhs) {
	return int16_t(lhs._bits) < int16_t(posit<NBITS_IS_16, ES_IS_2>(rhs)._bits);
}

#endif // POSIT_ENABLE_LITERALS

#endif // POSIT_FAST_POSIT_16_2

}} // namespace sw::universal
