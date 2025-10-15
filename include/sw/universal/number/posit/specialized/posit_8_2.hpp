#pragma once
// posit_8_2.hpp: specialized 8-bit posit using fast implementation specialized for posit<8,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/number/posit/posit.hpp>
#include <universal/native/integers.hpp>
#include <universal/native/ieee754_float.hpp>

#ifndef POSIT_FAST_POSIT_8_2
#define POSIT_FAST_POSIT_8_2 0
#endif

#include <universal/utility/directives.hpp>

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_8_2
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<8,2>")

// fast specialized posit<8,2>
template<>
class posit<NBITS_IS_8, ES_IS_2> {
public:
	static constexpr unsigned nbits = NBITS_IS_8;
	static constexpr unsigned es    = ES_IS_2;
	static constexpr unsigned sbits = 1;
	static constexpr unsigned rbits = nbits - sbits;
	static constexpr unsigned ebits = es;
	static constexpr unsigned fbits = nbits - 3 - es;
	static constexpr unsigned fhbits = fbits + 1;
	static constexpr uint8_t sign_mask = 0x80u;

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
	explicit posit(signed char initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(short initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(int initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(long initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(long long initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(char initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(unsigned short initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(unsigned int initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(unsigned long initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(unsigned long long initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(float initial_value) : _bits(0) { *this = initial_value; }
	                   posit(double initial_value) : _bits(0) { *this = initial_value; }
	explicit           posit(long double initial_value) : _bits(0) { *this = initial_value; }

	// assignment operators for native types
	posit& operator=(signed char rhs) { return operator=((long long)(rhs)); }
	posit& operator=(short rhs) { return operator=((long long)(rhs)); }
	posit& operator=(int rhs) { return operator=((long long)rhs); }
	posit& operator=(long rhs) { return operator=((long long)(rhs)); }
	posit& operator=(long long rhs) { return integer_assign(rhs); }
	posit& operator=(char rhs) { return operator=((long long)(rhs)); }
	posit& operator=(unsigned short rhs) { return operator=((long long)(rhs)); }
	posit& operator=(unsigned int rhs) { return operator=((long long)(rhs)); }
	posit& operator=(unsigned long rhs) { return operator=((long long)(rhs)); }
	posit& operator=(unsigned long long rhs) { return operator=((long long)(rhs)); }
	posit& operator=(float rhs) { return float_assign(rhs); }
	posit& operator=(double rhs) { return float_assign(float(rhs)); }
	posit& operator=(long double rhs) { return float_assign(float(rhs)); }

	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator long() const { return to_long(); }
	explicit operator int() const { return to_int(); }
	explicit operator unsigned long long() const { return to_long_long(); }
	explicit operator unsigned long() const { return to_long(); }
	explicit operator unsigned int() const { return to_int(); }

	constexpr posit operator-() const {
		posit p;
		return p.setbits((~_bits) + 1ul);
	}

	////////////////////////////////////////////////////////////////
	// arithmetic assignment operators
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

		uint8_t lhs = _bits;
		uint8_t rhs = b._bits;
		bool sign = (_bits & sign_mask) != 0;
		if (sign) {
			lhs = -lhs & 0xFFu;
			rhs = -rhs & 0xFFu;
		}
		if (lhs < rhs) std::swap(lhs, rhs);

		// decode the regime of lhs
		int8_t k; // regime numerical value
		uint8_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		uint8_t exp = remaining >> 5; // 8 - 1(sign) - 2(exponent)

		// extract remaining fraction bits
		uint16_t lhs_fraction = ((0x40u | remaining << 1) & 0x7Fu) << 8; // 0x40 is the hidden bit
		int8_t shiftRight = k;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		uint16_t rhs_fraction = ((0x40u | remaining << 1) & 0x7Fu) << 8; // 0x40 is the hidden bit

		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 5);

		if (shiftRight == 0) { // we can simplify the computation
			lhs_fraction += rhs_fraction;  // this will always produce a carry
			++exp;
			if (exp > 3) {
				++k;
				exp &= 3;
			}
			lhs_fraction >>= 1;
		}
		else {
			(shiftRight > 7) ? (rhs_fraction = 0) : (rhs_fraction >>= shiftRight); // frac16B >>= shiftRight
			lhs_fraction += rhs_fraction;

			bool rcarry = (0x8000 & lhs_fraction) != 0; // first left bit
			if (rcarry) {
				++exp;
				if (exp > 3) {
					++k;
					exp &= 3;
				}
				lhs_fraction >>= 1;
			}
		}

		_bits = round(k, exp, lhs_fraction);
		if (sign) _bits = -_bits & 0xFFu;
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
		if (iszero()) { _bits = -(b._bits) & 0xFFu; return *this; }
		posit bComplement = b.twosComplement();
		if (isneg() != b.isneg()) return *this += bComplement;

		uint8_t lhs = _bits;
		uint8_t rhs = bComplement._bits;
		// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
		bool sign = (lhs & sign_mask) != 0;
		(sign) ? (lhs = (-lhs & 0xFFu)) : (rhs = (-rhs & 0xFFu));

		if (lhs == rhs) {
			_bits = 0;
			return *this;
		}
		if (lhs < rhs) {
			std::swap(lhs, rhs);
			sign = !sign;
		}

		// decode the regime of lhs
		int8_t k;  // regime numerical value
		uint8_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		uint8_t exp = remaining >> 5;  // 8 - 1(sign) - 2(exponent)

		uint16_t lhs_fraction = ((0x40u | remaining << 1) & 0x7Fu) << 8; // 0x40 is the hidden bit
		int8_t shiftRight = k;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		uint16_t rhs_fraction = ((0x40u | remaining << 1) & 0x7Fu) << 8; // 0x40 is the hidden bit

		// align the fractions for subtraction
		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 5);
		if (shiftRight > 15) {
			_bits = lhs;
			if (sign) _bits = -_bits & 0xFFu;
			return *this;
		}
		else {
			rhs_fraction >>= shiftRight;
		}

		lhs_fraction -= rhs_fraction;

		while ((lhs_fraction >> 11) == 0) {
			--k;
			lhs_fraction <<= 4;
		}

		bool ecarry = (0x4000u & lhs_fraction) != 0;
		while (!ecarry) {
			if (exp == 0) {
				--k;
				exp = 3;
			}
			else {
				--exp;
			}
			lhs_fraction <<= 1;
			ecarry = (0x4000u & lhs_fraction) != 0;
		}

		_bits = round(k, exp, lhs_fraction);
		if (sign) _bits = -_bits & 0xFFu;
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
			_bits = 0;
			return *this;
		}
		uint8_t lhs = _bits;
		uint8_t rhs = b._bits;
		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = lhs & sign_mask ? -lhs : lhs;
		rhs = rhs & sign_mask ? -rhs : rhs;

		// decode the regime of lhs
		int8_t m = 0;  // regime pattern length
		uint8_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, m, remaining);

		// extract the exponent
		int8_t exp = (remaining >> 5);  // 8 - 1(sign) - 2(exponent)

		// extract remaining fraction bits
		uint8_t lhs_fraction = (0x40u | remaining << 1) & 0x7Fu; // 0x40 is the hidden bit

		// adjust shift and extract fraction bits of rhs
		extractMultiplicand(rhs, m, remaining);
		exp += (remaining >> 5);
		uint8_t rhs_fraction = (0x40u | remaining << 1) & 0x7Fu; // 0x40 is the hidden bit
		uint16_t result_fraction = (uint16_t)lhs_fraction * rhs_fraction;

		if (exp > 3) {
			++m;
			exp &= 3;
		}

		bool rcarry = (result_fraction & 0x2000u) != 0;
		if (rcarry) {
			//std::cerr << "fraction carry processing commensing\n";
			//std::cerr << to_binary(*this, true) << " * " << to_binary(b, true) << '\n';
			exp++;
			if (exp > 3) {
				++m;
				exp &= 3;
			}
			result_fraction >>= 1;
		}

		// round
		_bits = adjustAndRound(m, exp, result_fraction);
		if (sign) _bits = -_bits & 0xFFu;
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

		uint8_t lhs = _bits;
		uint8_t rhs = b._bits;
		if (iszero()) {
			_bits = 0;
			return *this;
		}

		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = lhs & sign_mask ? -lhs : lhs;
		rhs = rhs & sign_mask ? -rhs : rhs;

		// decode the regime of lhs
		int8_t m{ 0 }; // regime pattern length
		uint8_t remaining{ 0 };  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, m, remaining);

		//std::cout << "lhs m     : " << int(m) << '\n';

		// extract the exponent
		int8_t exp = (remaining >> 5); // 8 - 1(sign) - 2(exponent)
		//std::cout << "lhs exp   : " << exp << '\n';

		// extract remaining fraction bits
		uint8_t lhs_fraction = (0x40u | remaining << 1) & 0x7Fu; // 0x40 is the hidden bit
		uint16_t fraction = (uint16_t)lhs_fraction << 6;
		//std::cout << "fraction  : " << to_binary(fraction, 32, true) << '\n';

		// adjust shift and extract fraction bits of rhs
		extractDividand(rhs, m, remaining);
		//uint16_t rhsExp = (remaining >> 5);
		exp -= (remaining >> 5);
		//std::cout << "result m  : " << int(m) << '\n';
		//std::cout << "rhs exp   : " << rhsExp << '\n';
		//std::cout << "final exp : " << exp << '\n';

		uint8_t rhs_fraction = (0x40u | remaining << 1) & 0x7Fu; // 0x40 is the hidden bit
		//std::cout << "lhs frac  : " << to_binary(lhs_fraction, 16, true) << '\n';
		//std::cout << "rhs frac  : " << to_binary(rhs_fraction, 16, true) << '\n';

		div_t result = div(fraction, rhs_fraction);
		uint16_t result_fraction = result.quot;
		uint16_t remainder = result.rem;

		//std::cout << "result    : " << to_binary(result_fraction, 32, true) << '\n';

		// adjust the exponent if needed
		if (exp < 0) {
			exp += 4;
			--m;
		}
		if (result_fraction != 0) {
			bool rcarry = result_fraction >> 6; // this is the hidden bit (6th bit), extreme right bit is bit 0
			if (!rcarry) {
				if (exp == 0) {
					--m;
					exp = 3;
				}
				else {
					--exp;
				}
				result_fraction <<= 1;
			}
		}

		// round
		_bits = divRound(m, exp, result_fraction, remainder != 0);
		if (sign) _bits = -_bits & 0xFFu;
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

	// Selectors
	bool sign() const noexcept       { return (_bits & sign_mask); }
	bool isnar() const noexcept      { return (_bits == sign_mask); }
	bool iszero() const noexcept     { return (_bits == 0x00u); }
	bool isone() const noexcept      { return (_bits == 0x40u); } // pattern 010000...
	bool isminusone() const noexcept { return (_bits == 0xC0u); } // pattern 110000...
	bool isneg() const noexcept      { return (_bits & sign_mask); }
	bool ispos() const noexcept      { return !isneg(); }
	bool ispowerof2() const noexcept { return !(_bits & 0x1u); }

	int sign_value() const noexcept  { return (_bits & sign_mask ? -1 : 1); }

	bitblock<NBITS_IS_8> get() const noexcept { bitblock<NBITS_IS_8> bb; bb = int(_bits); return bb; }
	uint8_t bits() const noexcept { return _bits; }
	unsigned long long encoding() const noexcept { return (unsigned long long)(_bits); }

	// Modifiers
	constexpr void clear() noexcept { _bits = 0; }
	constexpr void setzero() noexcept { clear(); }
	constexpr void setnar() noexcept { _bits = sign_mask; }
	posit& setBitblock(const bitblock<NBITS_IS_8>& raw) noexcept {
		_bits = uint8_t(raw.to_ulong());
		return *this;
	}
	constexpr posit& setbits(uint64_t value) noexcept {
		_bits = uint8_t(value & 0xFFu);
		return *this;
	}
	constexpr posit& setbit(unsigned bitIndex, bool value = true) noexcept {
		uint8_t bit_mask = (0x1u << bitIndex);
		if (value) {
			_bits |= bit_mask;
		}
		else {
			_bits &= ~bit_mask;
		}
		return *this;
	}

	posit& minpos() noexcept {
		clear();
		return ++(*this);
	}
	posit& maxpos() noexcept {
		setnar();
		return --(*this);
	}
	posit& zero() noexcept {
		clear();
		return *this;
	}
	posit& minneg() noexcept {
		clear();
		return --(*this);
	}
	posit& maxneg() noexcept {
		setnar();
		return ++(*this);
	}
	posit twosComplement() const noexcept {
		posit p;
		return p.setbits(~_bits + 1ul);
	}

	uint16_t decode_posit(const uint8_t bits, uint8_t& exp, uint8_t& fraction) const noexcept {
		int16_t m{ 0 };
		// posit is s.rrrr.ee.fffff
		fraction = (bits << 2u) & 0xFFu;  // remove sign and first regime bit
		if (bits & 0x40u) {  // positive regimes
			m = 0;
			while (fraction >> 7) {
				++m;
				fraction <<= 1;
			}
		}
		else {              // negative regimes
			m = -1;
			while (!(fraction >> 7)) {
				--m;
				fraction <<= 1;
			}
			fraction &= 0x7Fu;
		}
		exp = (fraction >> 5); // extract the exponent
		// finalize the fraction bits as in 0b0hffff...ff00, so we have MSB = 0, hidden bit realized at 0x20, and two extra bits at the bottom
		fraction &= 0x9Fu; // null the exponent bits
		fraction |= 0x20u; // set the hidden bit
		return m;
	}

private:
	uint8_t _bits;

	// extract_exponent takes the regime, and the remaining bits
	// returns the exponent value, and updates remaining to hold just the fraction bits
	uint8_t extract_exponent(int8_t m, uint8_t* remaining) const {
		//                                 765 432 10
		// 0.01.00.000   m = -1  2 ebits   #00.000.--  >> 5
		// 0.001.00.00   m = -2  2 ebits   #00.00.---  >> 5
		// 0.0001.00.0   m = -3  2 ebits   #00.0.----  >> 5
		// 0.00001.00.   m = -4  2 ebits   #00.------  >> 5
		// 0.000001.0.   m = -5  1 ebit    #0-.------  >> 6
		// 0.0000001..   m = -6  0 ebits   #.          >> 7 = 0
		// 0.0000000..   m = -7  0 ebits   #.          >> 7 = 0

		// 0.10.00.000   m =  0  2 ebits   #00.000.--  >> 5
		// 0.110.00.00   m =  1  2 ebits   #00.00.---  >> 5
		// 0.1110.00.0   m =  2  2 ebits   #00.0.----  >> 5
		// 0.11110.00.   m =  3  2 ebits   #00..-----  >> 5
		// 0.111110.0.   m =  4  1 ebit    #0.-------  >> 6
		// 0.1111110..   m =  5  0 ebits   #.          >> 7 = 0
		// 0.1111111..   m =  6  0 ebits   #.          >> 7 = 0
		uint8_t exp{ 0 };
		switch (m) {
		case -5: case 4:
			exp = (*remaining >> 5);
			*remaining <<= 1;
			break;
		case -7: case -6: case 5: case 6:
			exp = 0;
			*remaining = 0;
			break;
		default:
			exp = (*remaining >> 5);
			*remaining <<= 2;
			break;
		}
		return exp;
	}

	float fraction_value(uint8_t fraction) const {
		float v = 0.0f;
		float scale = 1.0f;
		uint8_t mask = 0x80;
		for (int i = 5; i >= 0; i--) {
			if (fraction & mask) v += scale;
			scale *= 0.5f;
			mask >>= 1;
			if (scale == 0.0) break;
		}
		return v;
	}

	void checkExtraTwoBits(float f, float temp, bool* bitsNPlusOne, bool* bitsMore) {
		temp /= 2.0;
		if (temp <= f) {
			*bitsNPlusOne = 1;
			f -= temp;
		}
		if (f > 0)
			*bitsMore = 1;
	}
	uint16_t convertFraction(float f, uint8_t fracLength, bool* bitsNPlusOne, bool* bitsMore) {

		uint_fast8_t frac = 0;

		if (f == 0) return 0;
		else if (f == INFINITY) return 0x80;

		f -= 1; //remove hidden bit
		if (fracLength == 0) {
			checkExtraTwoBits(f, 1.0, bitsNPlusOne, bitsMore);
		} 
		else {
			float temp = 1;
			while (true) {
				temp /= 2;
				if (temp <= f) {
					f -= temp;
					fracLength--;
					frac = (frac << 1) + 1; //shift in one
					if (f == 0) {
						//put in the rest of the bits
						frac <<= (uint_fast8_t)fracLength;
						break;
					}

					if (fracLength == 0) {
						checkExtraTwoBits(f, temp, bitsNPlusOne, bitsMore);

						break;
					}
				}
				else {
					frac <<= 1; //shift in a zero
					fracLength--;
					if (fracLength == 0) {
						checkExtraTwoBits(f, temp, bitsNPlusOne, bitsMore);
						break;
					}
				}
			}
		}
		//printf("convertfloat: frac:%d bitsNPlusOne: %d, bitsMore: %d\n", frac, bitsNPlusOne, bitsMore);
		return frac;
	}

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
		if (iszero()) return 0.0f;
		if (isnar()) return NAN;

		uint8_t bits = ((_bits & 0x80u) ? -_bits : _bits);	
		uint8_t remaining = 0;
		int8_t m;
		decode_regime(bits,m, remaining);
//		std::cout << to_binary(bits, 8) << " : " << to_binary(remaining, 8) << " : ";
		int regimeScale = (1 << es) * m;
		float s = (float)(sign_value());
		float r = (m > 0 ? (float)(1 << regimeScale) : (1.0f / (float)(1 << -regimeScale)));
		uint8_t expbits = extract_exponent(m, &remaining);
//		std::cout << to_binary(expbits, 2) << " : " << to_binary(remaining, 8) << '\n';
		float e = float((uint32_t(1) << expbits));
		remaining |= 0x80; // set hidden bit
		float f = fraction_value(remaining);
//		std::cout << "regime value   : " << r << '\n';
//		std::cout << "exponent value : " << e << '\n';
//		std::cout << "fraction value : " << f << '\n';

		return s * r * e * f;
	}
	double      to_double() const {
		return (double)to_float();
	}
	long double to_long_double() const {
		return (long double)to_float();
	}

	// helper methods			
	 posit& integer_assign(long long rhs) noexcept {
		// special case for speed as this is a common initialization
		if (rhs == 0) {
			_bits = 0x0;
			return *this;
		}
		bool sign = (rhs < 0) ? true : false;
		long long v = sign ? -rhs : rhs; // project to positive side of the projective reals
		uint8_t raw = 0;
		if (v > 48 || v == rhs) { // +-maxpos
			raw = 0x7F;
		}
		else if (v < 2) {
			raw = (v << 6);
		}
		else {
			uint8_t mask = 0x40;
			int8_t k = 6;
			uint8_t fraction_bits = (v & 0xff);
			while (!(fraction_bits & mask)) {
				k--;
				fraction_bits <<= 1;
			}
			fraction_bits = (fraction_bits ^ mask);
			raw = (0x7F ^ (0x3F >> k)) | (fraction_bits >> (k + 1));

			mask = 0x1 << k; //bitNPlusOne
			if (mask & fraction_bits) {
				if (((mask - 1) & fraction_bits) | ((mask << 1) & fraction_bits)) raw++;
			}
		}

		_bits = sign ? -raw : raw;
		return *this;
	}
	posit& float_assign(float rhs) noexcept {
		// special case for speed as this is a common initialization
		if (std::fpclassify(rhs) == FP_NAN || std::fpclassify(rhs) == FP_INFINITE) {
			_bits = 0x80u;
			return *this;
		}
		else if (rhs == 0.0f) {
			_bits = 0;
			return *this;
		}

		bool sign = (rhs < 0.0);
		float v = (sign ? -rhs : rhs);
		float_decoder fd{ v };
		uint8_t raw{ 0 };
		if (v == 1.0f) {
			raw = 0x40u;
		}
		else if (v > 1) {
			// geometric mean = sqrt(a*b)
			// geometric range of the posit<8,2>
			// maxpos          = 16,777,216   0x0111'1111  2^(6*2^2) * 2^0
			// geo mean        =  4,194,304                2^(5*2^2) * 2^2
			// maxpos / 2^4    =  1,048,576   0x0111'1110  2^(5*2^2) * 2^0
			// geo mean        =    524,288                2^(5*2^2) * 2^1
			// maxpos / 2^6    =    262,144   0x0111'1101  2^(4*2^2) * 2^2
			// geo mean        =    131,072                2^(4*2^2) * 2^1
			// maxpos / 2^8    =     65,536   0x0111'1100  2^(4*2^2) * 2^0
			// maxpos / 2^9    =     32,768   0x0111'1011  2^(3*2^2) * 2^3
			// maxpos / 2^10   =     16,384   0x0111'1010  2^(3*2^2) * 2^2
			if (v > 4194304) {
				raw = 0x7Fu; // maxpos
			}
			else if (v > 524288) { 
				raw = 0x7Eu; // maxpos / 2^4
			}
			else if (v > 131072) { 
				raw = 0x7Du; // maxpos / 2^6
			}
			else {
				//std::cout << "value    : " << v << '\n';
				int scale = fd.parts.exponent - 127;
				//std::cout << "scale    : " << scale << '\n';
				unsigned reglen = 1u + (scale >> 2);
				//std::cout << "reglen   : " << reglen << '\n';
				uint8_t regime = 0x7Fu - (0x7Fu >> reglen);
				//std::cout << "regime   : " << to_binary(regime, 8, true) << '\n';
				uint8_t esval = (scale % 0x04u);

				int sign_regime_es = 1 + reglen + 1 + 2; // 1 sign, reglen+1 regime, 2 exponent bits
				int nf = std::max<int>(0, nbits - sign_regime_es);
				uint8_t exponent = (esval << nf);
				//std::cout << "exponent : " << to_binary(exponent, 8, true) << '\n';

				// copy most significant nf fraction bits into fraction
				//std::cout << "fracin   : " << to_binary(fd.parts.fraction, 23, true) << '\n';
				uint8_t fraction = fd.parts.fraction >> (23 - nf);
				//std::cout << "fraction : " << to_binary(fraction, 8, true) << '\n';

				// construct the untruncated posit
				raw = regime | exponent | fraction;

				// round
			}
		}
		else if (v < 1) {
			// geometric range of the posit<8,2>
			// minpos        = 1/16,777,216   0x0000'0001  2^(-6*2^2) * 2^0   5.9604644775390625e-08f
			// geo mean      = 1/ 4,194,304                                   2.384185791015625e-07f
			// minpos * 2^4  = 1/ 1,048,576   0x0000'0010  2^(-5*2^2) * 2^0   9.5367431640625e-07f
			// geo mean      = 1/   524,288                                   1.9073486328125e-06f
			// minpos * 2^6  = 1/   262,144   0x0000'0011  2^(-4*2^2) * 2^2   3.814697265625e-06f
			// geo mean      = 1/   131,072                                   7.62939453125e-06f
			// minpos * 2^8  = 1/    65,536   0x0000'0100  2^(-4*2^2) * 2^0   1.52587890625e-05f

			if (v < 2.384185791015625e-07f) {
				raw = 0x01u;
			}
			else if (v < 1.9073486328125e-06f) {
				raw = 0x02u;
			}
			else if (v < 7.62939453125e-06f) {
				raw = 0x03u;
			}
			else {
				//std::cout << "value    : " << v << '\n';
				
				int scale = fd.parts.exponent - 127;
				//std::cout << "scale    : " << scale << '\n';
				unsigned reglen = -(scale >> 2);
				//std::cout << "reglen   : " << reglen << '\n';
				uint8_t regime = 0x40u >> reglen;
				//std::cout << "regime   : " << to_binary(regime, 8, true) << '\n';
				uint8_t esval = (scale % 0x04u);

				int sign_regime_es = 1 + reglen + 1 + 2; // 1 sign, reglen+1 regime, 2 exponent bits
				int nf = std::max<int>(0, nbits - sign_regime_es);
				uint8_t exponent = (esval << nf);
				//std::cout << "exponent : " << to_binary(exponent, 8, true) << '\n';

				// copy most significant nf fraction bits into fraction
				//std::cout << "fracin   : " << to_binary(fd.parts.fraction, 23, true) << '\n';
				uint8_t fraction = fd.parts.fraction >> (23 - nf);
				//std::cout << "fraction : " << to_binary(fraction, 8, true) << '\n';

				// construct the untruncated posit
				raw = regime | exponent | fraction;

				// round
			}
			 
		}
		else {
			// std::cout << "NaN maps to NaR\n";
			_bits = 0x80u; // NaR
			return *this;
		}

		_bits = sign ? -raw : raw;
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
	void decode_regime(uint8_t bits, int8_t& m, uint8_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFu; // remove sign and first regime bit
		if (bits & 0x40u) {  // positive regimes
			m = 0;
			while (remaining >> 7) {
				++m;
				remaining = (remaining << 1) & 0xFFu;
			}
		}
		else {              // negative regimes
			m = -1;
			while (!(remaining >> 7)) {
				--m;
				remaining = (remaining << 1) & 0xFFu;
			}
			remaining &= 0x7Fu;
		}
	}
	void extractAddand(const uint8_t bits, int8_t& m, uint8_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFu;
		if (bits & 0x40u) {  // positive regimes
			while (remaining >> 7) {
				--m;
				remaining = (remaining << 1) & 0xFFu;
			}
		}
		else {              // negative regimes
			++m;
			while (!(remaining >> 7)) {
				++m;
				remaining = (remaining << 1) & 0xFFu;
			}
			remaining &= 0x7Fu;
		}
	}
	void extractMultiplicand(const uint8_t bits, int8_t& m, uint8_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFu;
		if (bits & 0x40u) {  // positive regimes
			while (remaining >> 7) {
				++m;
				remaining = (remaining << 1) & 0xFFu;
			}
		}
		else {              // negative regimes
			--m;
			while (!(remaining >> 7)) {
				--m;
				remaining = (remaining << 1) & 0xFFu;
			}
			remaining &= 0x7Fu;
		}
	}
	void extractDividand(const uint8_t bits, int8_t& m, uint8_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFu;
		if (bits & 0x40u) {  // positive regimes
			while (remaining >> 7) {
				--m;
				remaining = (remaining << 1) & 0xFFu;
			}
		}
		else {              // negative regimes
			++m;
			while (!(remaining >> 7)) {
				++m;
				remaining = (remaining << 1) & 0xFFu;
			}
			remaining &= 0x7Fu;
		}
	}
	uint16_t round(const int8_t m, uint8_t exp, uint16_t frac16) const noexcept {
		uint8_t reglen, regime, bits;
		if (m < 0) {
			reglen = (-m & 0xFFu);
			regime = 0x40u >> reglen;
		}
		else {
			reglen = m + 1;
			regime = 0x7Fu - (0x7Fu >> reglen);
		}

		if (reglen > 6) {
			bits = (m < 0 ? 0x01u : 0x7Fu);  // minpos and maxpos. exp and frac do not matter
		}
		else {
			frac16 = (frac16 & 0x3FFF) >> (reglen + 2); // remove both carry bits, 2 bits exp
			uint8_t fraction = uint8_t(frac16 >> 8);
			bool bitNPlusOne = false;
			uint8_t moreBits = 0;
			if (reglen <= 4) {
				bitNPlusOne = (0x80u & frac16) != 0;
				exp <<= (4 - reglen);
			}
			else {
				if (reglen == 6) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (reglen == 5) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (fraction > 0) {
					fraction = 0;
					moreBits = 1;
				}
			}

			bits = regime | exp | fraction;
			// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			if (bitNPlusOne) {
				if (0x7Fu & frac16) moreBits = 1;
				bits += (bits & 0x01u) | moreBits;
			}
		}
		return bits;
	}
	uint8_t divRound(const int8_t m, uint8_t exp, uint16_t frac16, bool nonZeroRemainder) const noexcept {
		uint8_t reglen, regime, bits;
		if (m < 0) {
			reglen = (-m & 0xFFu);
			regime = 0x40u >> reglen;
		}
		else {
			reglen = m + 1;
			regime = 0x7Fu - (0x7Fu >> reglen);
		}

		if (reglen > 6) {
			bits = (m < 0 ? 0x01u : 0x7Fu);  // minpos and maxpos
		}
		else {
			frac16 &= 0x3Fu; // remove both carry bits
			uint8_t fraction = uint8_t(frac16 >> (reglen + 2));

			bool bitNPlusOne = false;
			uint16_t moreBits{ 0 };
			if (reglen <= 4) {
				bitNPlusOne = bool((frac16 >> (reglen + 1)) & 0x1);
				exp <<= (4 - reglen);
				if (bitNPlusOne) moreBits = (((1ull << (reglen + 1)) - 1ull) & frac16) ? 0x1 : 0x0;
			}
			else {
				if (reglen == 6) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (reglen == 5) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (frac16 > 0) {
					fraction = 0;
					moreBits = 0x1;
				}
			}
			if (nonZeroRemainder) moreBits = 0x1;
			bits = regime | exp | uint8_t(fraction);
			if (bitNPlusOne) bits += (bits & 0x1) | moreBits;
		}
		return bits;
	}
	inline uint8_t adjustAndRound(const int8_t m, uint8_t exp, uint16_t frac16) const noexcept {
		uint8_t reglen, regime, bits;
		if (m < 0) {
			reglen = (-m & 0xFFu);
			regime = 0x40u >> reglen;
		}
		else {
			reglen = m + 1u;
			regime = 0x7Fu - (0x7Fu >> reglen);
		}

		if (reglen > 6) {
			bits = m < 0 ? 0x01u : 0x7Fu;  // minpos and maxpos. exp and frac do not matter 
		}
		else {
			// remove carry and rcarry bits and shift to correct position
			frac16 = (frac16 & 0x0FFFu) >> reglen;
			uint8_t fraction = uint8_t(frac16 >> 8);
			bool bitNPlusOne = false;
			uint8_t moreBits = 0;
			if (reglen <= 4) {
				bitNPlusOne = bool(0x80u & frac16);
				exp <<= (4 - reglen);
			}
			else {
				if (reglen == 6) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (reglen == 5) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (fraction > 0) {
					fraction = 0;
					moreBits = 1;
				}
			}

			bits = regime | exp | fraction;
			// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			if (bitNPlusOne) {
				if (0x7Fu & frac16) moreBits = 1;
				bits += (bits & 0x01u) | moreBits;
			}
		}
		return bits;
	}

	// I/O operators
	friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_8, ES_IS_2>& p);
	friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_8, ES_IS_2>& p);

	// posit - posit logic functions
	friend bool operator==(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs);
	friend bool operator!=(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs);
	friend bool operator< (const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs);
	friend bool operator> (const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs);
	friend bool operator<=(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs);
	friend bool operator>=(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs);

	friend bool operator< (const posit<NBITS_IS_8, ES_IS_2>& lhs, double rhs);
	friend bool operator< (double lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs);
};

// posit I/O operators
// generate a posit format ASCII format nbits.esxNN...NNp
inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_8, ES_IS_2>& p) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the posit into a string
	std::stringstream ss;
#if POSIT_ERROR_FREE_IO_FORMAT
	ss << NBITS_IS_8 << '.' << ES_IS_2 << 'x' << to_hex(p.get()) << 'p';
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
inline std::istream& operator>> (std::istream& istr, posit<NBITS_IS_8, ES_IS_2>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

// convert a posit value to a string using "nar" as designation of NaR
inline std::string to_string(const posit<NBITS_IS_8, ES_IS_2>& p, std::streamsize precision) {
	if (p.isnar()) {
		return std::string("nar");
	}
	std::stringstream ss;
	ss << std::setprecision(precision) << float(p);
	return ss.str();
}

// posit - posit binary logic operators
inline bool operator==(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return lhs._bits == rhs._bits;
}
inline bool operator!=(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return !operator==(lhs, rhs);
}
inline bool operator< (const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return int8_t(lhs._bits) < int8_t(rhs._bits);
}
inline bool operator> (const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return operator< (rhs, lhs);
}
inline bool operator<=(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
inline bool operator>=(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return !operator< (lhs, rhs);
}

/* base class has these operators: no need to specialize */
inline posit<NBITS_IS_8, ES_IS_2> operator+(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	posit<NBITS_IS_8, ES_IS_2> result = lhs;
	return result += rhs;
}
inline posit<NBITS_IS_8, ES_IS_2> operator-(const posit<NBITS_IS_8, ES_IS_2>& lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	posit<NBITS_IS_8, ES_IS_2> result = lhs;
	return result -= rhs;
}

// binary operator*() is provided by generic class

#if POSIT_ENABLE_LITERALS
// posit - literal logic functions

// posit - int logic operators
inline bool operator==(const posit<NBITS_IS_8, ES_IS_2>& lhs, int rhs) {
	return operator==(lhs, posit<NBITS_IS_8, ES_IS_2>(rhs));
}
inline bool operator!=(const posit<NBITS_IS_8, ES_IS_2>& lhs, int rhs) {
	return !operator==(lhs, posit<NBITS_IS_8, ES_IS_2>(rhs));
}
inline bool operator< (const posit<NBITS_IS_8, ES_IS_2>& lhs, int rhs) {
	return operator<(lhs, posit<NBITS_IS_8, ES_IS_2>(rhs));
}
inline bool operator> (const posit<NBITS_IS_8, ES_IS_2>& lhs, int rhs) {
	return operator< (posit<NBITS_IS_8, ES_IS_2>(rhs), lhs);
}
inline bool operator<=(const posit<NBITS_IS_8, ES_IS_2>& lhs, int rhs) {
	return operator< (lhs, posit<NBITS_IS_8, ES_IS_2>(rhs)) || operator==(lhs, posit<NBITS_IS_8, ES_IS_2>(rhs));
}
inline bool operator>=(const posit<NBITS_IS_8, ES_IS_2>& lhs, int rhs) {
	return !operator<(lhs, posit<NBITS_IS_8, ES_IS_2>(rhs));
}

// int - posit logic operators
inline bool operator==(int lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return posit<NBITS_IS_8, ES_IS_2>(lhs) == rhs;
}
inline bool operator!=(int lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return !operator==(posit<NBITS_IS_8, ES_IS_2>(lhs), rhs);
}
inline bool operator< (int lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return operator<(posit<NBITS_IS_8, ES_IS_2>(lhs), rhs);
}
inline bool operator> (int lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return operator< (posit<NBITS_IS_8, ES_IS_2>(rhs), lhs);
}
inline bool operator<=(int lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return operator< (posit<NBITS_IS_8, ES_IS_2>(lhs), rhs) || operator==(posit<NBITS_IS_8, ES_IS_2>(lhs), rhs);
}
inline bool operator>=(int lhs, const posit<NBITS_IS_8, ES_IS_2>& rhs) {
	return !operator<(posit<NBITS_IS_8, ES_IS_2>(lhs), rhs);
}

inline bool operator< (const posit<NBITS_IS_8, ES_IS_2>& lhs, double rhs) {
	return int8_t(lhs._bits) < int8_t(posit<NBITS_IS_8, ES_IS_2>(rhs)._bits);
}

#endif // POSIT_ENABLE_LITERALS

#endif // POSIT_FAST_POSIT_8_2

}} // namespace sw::universal
