#pragma once
// posit_16_2.hpp: specialized 16-bit posit using fast implementation specialized for posit<16,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
#else
#pragma message "Fast specialization of posit<16,2>"
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
	constexpr posit& operator=(signed char rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(short rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(int rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(long rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(long long rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(char rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(unsigned short rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(unsigned int rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(unsigned long rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(unsigned long long rhs) { return integer_assign(rhs); }
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

		uint16_t lhs = _bits;
		uint16_t rhs = b._bits;
		bool sign = (_bits & sign_mask) != 0;
		if (sign) {
			lhs = -lhs & 0xFFFFu;
			rhs = -rhs & 0xFFFFu;
		}
		if (lhs < rhs) std::swap(lhs, rhs);

		// decode the regime of lhs
		int8_t k; // regime numerical value
		uint16_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		uint16_t exp = remaining >> 13; // 16 - 1(sign) - 2(exponent)

		// extract remaining fraction bits
		uint32_t lhs_fraction = ((0x4000u | remaining << 1) & 0x7FFFu) << 16; // 0x4000 is the hidden bit
		int8_t shiftRight = k;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		uint32_t rhs_fraction = ((0x4000u | remaining << 1) & 0x7FFFu) << 16; // 0x4000 is the hidden bit

		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 13);

		if (shiftRight == 0) { // we can simplify the computation
			lhs_fraction += rhs_fraction;  // this will always produce a carry
			++exp;
			if (exp > 3) {
				++k;
				exp &= 0x3u;
			}
			lhs_fraction >>= 1;
		}
		else {
			(shiftRight > 15) ? (rhs_fraction = 0) : (rhs_fraction >>= shiftRight); // frac16B >>= shiftRight
			lhs_fraction += rhs_fraction;

			bool rcarry = (0x8000'0000u & lhs_fraction) != 0; // first left bit
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
		if (sign) _bits = -_bits & 0xFFFFu;
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
		if (iszero()) { _bits = -int16_t(b._bits) & 0xFFFFu; return *this; }
		posit bComplement = b.twosComplement();
		if (isneg() != b.isneg()) return *this += bComplement;

		uint16_t lhs = _bits;
		uint16_t rhs = bComplement._bits;
		// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
		bool sign = bool(lhs & sign_mask);
		(sign) ? (lhs = (-lhs & 0xFFFFu)) : (rhs = (-rhs & 0xFFFFu));

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
		uint16_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, k, remaining);

		// extract the exponent
		uint16_t exp = remaining >> 13;  // 16 - 1(sign) - 2(exponent)

		uint32_t lhs_fraction = ((0x4000u | remaining << 1) & 0x7FFFu) << 16; // 0x4000 is the hidden bit
		int8_t shiftRight = k;

		// adjust shift and extract fraction bits of rhs
		extractAddand(rhs, shiftRight, remaining);
		uint32_t rhs_fraction = ((0x4000u | remaining << 1) & 0x7FFFu) << 16; // 0x4000 is the hidden bit

		// align the fractions for subtraction
		// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
		shiftRight = (shiftRight << 2) + exp - (remaining >> 13);
		if (shiftRight > 31) {
			_bits = lhs;
			if (sign) _bits = -_bits & 0xFFFFu;
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

		bool ecarry = (0x4000'0000u & lhs_fraction) != 0;
		while (!ecarry) {
			if (exp == 0) {
				--k;
				exp = 3;
			}
			else {
				--exp;
			}
			lhs_fraction <<= 1;
			ecarry = (0x4000'0000u & lhs_fraction) != 0;
		}

		_bits = round(k, exp, lhs_fraction);
		if (sign) _bits = -_bits & 0xFFFFu;
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
		uint16_t lhs = _bits;
		uint16_t rhs = b._bits;
		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = lhs & sign_mask ? -lhs : lhs;
		rhs = rhs & sign_mask ? -rhs : rhs;

		// decode the regime of lhs
		int8_t m = 0;  // regime pattern length
		uint16_t remaining;  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, m, remaining);

		// extract the exponent
		int16_t exp = (remaining >> 13);  // 16 - 1(sign) - 2(exponent)

		// extract remaining fraction bits
		uint16_t lhs_fraction = (0x4000u | remaining << 1) & 0x7FFFu; // 0x4000 is the hidden bit

		// adjust shift and extract fraction bits of rhs
		extractMultiplicand(rhs, m, remaining);
		exp += (remaining >> 13);
		uint16_t rhs_fraction = (0x4000u | remaining << 1) & 0x7FFFu; // 0x4000 is the hidden bit
		uint32_t result_fraction = (uint32_t)lhs_fraction * rhs_fraction;

		if (exp > 3) {
			++m;
			exp &= 0x3;
		}
	
		bool rcarry = (result_fraction & 0x2000'0000u) != 0;
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
		if (sign) _bits = -_bits & 0xFFFFu;
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
			_bits = 0;
			return *this;
		}

		// calculate the sign of the result
		bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
		lhs = lhs & sign_mask ? -lhs : lhs;
		rhs = rhs & sign_mask ? -rhs : rhs;

		// decode the regime of lhs
		int8_t m{ 0 }; // regime pattern length
		uint16_t remaining{ 0 };  // Remaining bits after the regime: 0<remaining_bits>0..0
		decode_regime(lhs, m, remaining);

		//std::cout << "lhs m     : " << int(m) << '\n';

		// extract the exponent
		int16_t exp = (remaining >> 13); // 16 - 1(sign) - 2(exponent)
		//std::cout << "lhs exp   : " << exp << '\n';

		// extract remaining fraction bits
		uint16_t lhs_fraction = (0x4000u | remaining << 1) & 0x7FFFu; // 0x4000 is the hidden bit
		uint32_t fraction = (uint32_t) lhs_fraction << 14;
		//std::cout << "fraction  : " << to_binary(fraction, 32, true) << '\n';

		// adjust shift and extract fraction bits of rhs
		extractDividand(rhs, m, remaining);
		//uint16_t rhsExp = (remaining >> 13);
		exp -= (remaining >> 13);
		//std::cout << "result m  : " << int(m) << '\n';
		//std::cout << "rhs exp   : " << rhsExp << '\n';
		//std::cout << "final exp : " << exp << '\n';

		uint16_t rhs_fraction = (0x4000u | remaining << 1) & 0x7FFFu; // 0x4000 is the hidden bit
		//std::cout << "lhs frac  : " << to_binary(lhs_fraction, 16, true) << '\n';
		//std::cout << "rhs frac  : " << to_binary(rhs_fraction, 16, true) << '\n';

		div_t result = div(fraction, rhs_fraction);
		uint32_t result_fraction = result.quot;
		uint32_t remainder = result.rem;

		//std::cout << "result    : " << to_binary(result_fraction, 32, true) << '\n';

		// adjust the exponent if needed
		if (exp < 0) {
			exp += 4;
			--m;
		}
		if (result_fraction != 0) {
			bool rcarry = result_fraction >> 14; // this is the hidden bit (14th bit), extreme right bit is bit 0
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
		if (sign) _bits = -_bits & 0xFFFFu;

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

	posit reciprocal() const noexcept {
		posit p = 1.0 / *this;
		return p;
	}
	posit abs() const noexcept {
		if (isneg()) {
			return posit(-*this);
		}
		return *this;
	}

	// Selectors
	bool sign() const noexcept       { return (_bits & sign_mask); }
	bool isnar() const noexcept      { return (_bits == sign_mask); }
	bool isnan() const noexcept      { return isnar(); }
	bool iszero() const noexcept     { return (_bits == 0x0); }
	bool isone() const noexcept      { return (_bits == 0x4000u); } // pattern 010000...
	bool isminusone() const noexcept { return (_bits == 0xC000u); } // pattern 110000...
	bool isneg() const noexcept      { return (_bits & sign_mask); }
	bool ispos() const noexcept      { return !isneg(); }
	bool ispowerof2() const noexcept { return !(_bits & 0x1u); }

	int sign_value() const noexcept  { return (_bits & sign_mask ? -1 : 1); }

	bitblock<NBITS_IS_16> get() const noexcept { bitblock<NBITS_IS_16> bb; bb = int(_bits); return bb; }
	uint16_t bits() const noexcept { return _bits; }
	unsigned long long encoding() const noexcept { return (unsigned long long)(_bits); }

	// Modifiers
	void clear() noexcept { _bits = 0; }
	void setzero() noexcept { clear(); }
	void setnar() noexcept { _bits = sign_mask; }
	posit& setBitblock(const bitblock<NBITS_IS_16>& raw) noexcept {
		_bits = uint16_t(raw.to_ulong());
		return *this;
	}
	constexpr posit& setbits(uint64_t value) noexcept {
		_bits = uint16_t(value & 0xFFFFu);
		return *this;
	}
	constexpr posit& setbit(unsigned bitIndex, bool value = true) noexcept {
		uint16_t bit_mask = (0x1u << bitIndex);
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

	uint16_t decode_posit(const uint16_t bits, uint16_t& exp, uint16_t& fraction) const noexcept {
		int16_t m{ 0 };
		// posit is s.rrrr.ee.fffff
		fraction = (bits << 2u) & 0xFFFF;  // remove sign and first regime bit
		if (bits & 0x4000) {  // positive regimes
			m = 0;
			while (fraction >> 15) {
				++m;
				fraction <<= 1u;
			}
		}
		else {              // negative regimes
			m = -1;
			while (!(fraction >> 15)) {
				--m;
				fraction <<= 1u;
			}
			fraction &= 0x7FFF;
		}	
		exp = (fraction >> 13); // extract the exponent
		// finalize the fraction bits as in 0b0hffff...ff00, so we have MSB = 0, hidden bit realized at 0x2000, and two extra bits at the bottom
		fraction &= 0x9FFF; // null the exponent bits
		fraction |= 0x2000; // set the hidden bit
		return m;
	}

	internal::value<fbits> to_value() const noexcept {
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
		return (long long)(to_long_double());
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
		return (long long)(to_long_double());
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
		return (long double)to_double();
	}

	// helper methods
	constexpr posit& integer_assign(long long rhs) {
		// special case for speed as this is a common initialization
		if (rhs == 0) {
			_bits = 0x0;
			return *this;
		}

		// geometric range of the posit<16,2>
		// maxpos        = 72,057,594,037,927,936   0b0111'1111'1111'1111
		// maxpos / 2    = 36,028,797,018,963,968   0b0111'1111'1111'1110
		// maxpos / 3/8  = 27,021,597,764,222,976   0b0111'1111'1111'1101
		// maxpos / 4    = 18,014,398,509,481,984   0b0111'1111'1111'1100
		bool sign = (rhs < 0);
		uint64_t v = sign ? -rhs : rhs; // project to positve side of the projective reals
		uint16_t raw = 0;
		if (v > 0x0080'0000'0000'0000) { // v > 36,028,797,018,963,968
			raw = 0x7FFFu;  // +-maxpos
		}
		else if (v > 0x005F'FFFF'FFFF'FFFF) { // 27,021,597,764,222,976 < v < 36,028,797,018,963,968
			raw = 0x7FFEu;  // 0.5 of maxpos is the final value
		}
		else if (v == 1) {
			raw = 0x4000u;
		}
		else {
			// the scale of 0.5 * maxpos = 2^55, so we can filter out all bits above that
			uint64_t mask = 0x0040'0000'0000'0000;
			int8_t scale = 54;
			uint64_t fraction_bits = v;
			while (!(fraction_bits & mask)) {
				--scale;
				fraction_bits <<= 1;
			}
			int8_t k = scale >> 2;
			uint16_t exp = (scale & 0x3) << (11 - k); // extract exponent and shift to correct location
			fraction_bits = (fraction_bits ^ mask); // remove the leading 1
			//uint16_t reg = (0x7FFF ^ (0x3FFF >> k));
			//std::cout << "fra    : " << to_binary(fraction_bits, 64, true) << '\n';
			//uint64_t fraa = (fraction_bits >> (k + 43));
			//std::cout << "fraa   : " << to_binary(fraa, 64, true) << '\n';
			//uint16_t fra = uint16_t(fraction_bits >> (k + 43));

			//std::cout << "scale  : " << int(scale) << '\n';
			//std::cout << "k      : " << int(k) << '\n';
			//std::cout << "reg    : " << to_binary(reg, 16, true) << '\n';
			//std::cout << "exp    : " << to_binary(exp, 16, true) << '\n';
			//std::cout << "fra    : " << to_binary(fra, 16, true) << '\n';

			raw = (0x7FFFu ^ (0x3FFFu >> k)) | exp | (fraction_bits >> (k + 43));

			mask = 0x1000ul << k; // bitNPlusOne
			if (mask & fraction_bits) {
				std::cerr << "TBD: bitNPlusOne condition is triggered in posit<16,2>::integer_assign\n";
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
	void decode_regime(const uint16_t bits, int8_t& m, uint16_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFFFu;  // sign and first regime bit
		if (bits & 0x4000u) {  // positive regimes
			m = 0;
			while (remaining >> 15) {
				++m;
				remaining = (remaining << 1) & 0xFFFFu;
			}
		}
		else {              // negative regimes
			m = -1;
			while (!(remaining >> 15)) {
				--m;
				remaining = (remaining << 1) & 0xFFFFu;
			}
			remaining &= 0x7FFFu;
		}
	}
	void extractAddand(const uint16_t bits, int8_t& shift, uint16_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFFFu;
		if (bits & 0x4000u) {  // positive regimes
			while (remaining >> 15) {
				--shift;
				remaining = (remaining << 1) & 0xFFFFu;
			}
		}
		else {              // negative regimes
			++shift;
			while (!(remaining >> 15)) {
				++shift;
				remaining = (remaining << 1) & 0xFFFFu;
			}
			remaining &= 0x7FFFu;
		}
	}
	void extractMultiplicand(const uint16_t bits, int8_t& m, uint16_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFFFu;
		if (bits & 0x4000u) {  // positive regimes
			while (remaining >> 15) {
				++m;
				remaining = (remaining << 1) & 0xFFFFu;
			}
		}
		else {              // negative regimes
			--m;
			while (!(remaining >> 15)) {
				--m;
				remaining = (remaining << 1) & 0xFFFFu;
			}
			remaining &= 0x7FFFu;
		}
	}
	void extractDividand(const uint16_t bits, int8_t& m, uint16_t& remaining) const noexcept {
		remaining = (bits << 2) & 0xFFFFu;
		if (bits & 0x4000u) {  // positive regimes
			while (remaining >> 15) {
				--m;
				remaining = (remaining << 1) & 0xFFFFu;
			}
		}
		else {              // negative regimes
			++m;
			while (!(remaining >> 15)) {
				++m;
				remaining = (remaining << 1) & 0xFFFFu;
			}
			remaining &= 0x7FFFu;
		}
	}
	uint16_t round(const int8_t m, uint16_t exp, uint32_t frac32) const noexcept {
		uint16_t reglen, regime, bits;
		if (m < 0) {
			reglen = (-m & 0xFFFFu);
			regime = 0x4000u >> reglen;
		}
		else {
			reglen = int16_t(m) + 1;
			regime = 0x7FFFu - (0x7FFFu >> reglen);
		}

		if (reglen > 14) {
			bits = (m<0 ? 0x0001u : 0x7FFFu);  // minpos and maxpos. exp and frac do not matter
		}
		else {
			frac32 = (frac32 & 0x3FFF'FFFFu) >> (reglen + 2); // remove both carry bits, 2 bits exp
			uint16_t fraction = uint16_t(frac32 >> 16);
			bool bitNPlusOne = false;
			uint16_t moreBits = 0;
			if (reglen <= 12) {
				bitNPlusOne = (0x8000u & frac32) != 0;
				exp <<= (12 - reglen);
			}
			else {
				if (reglen == 14) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (reglen == 13) {
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
				if (0x7FFFu & frac32) moreBits = 1;
				bits += (bits & 0x0001u) | moreBits;
			}
		}
		return bits;
	}
	uint16_t divRound(const int8_t m, uint16_t exp, uint32_t frac32, bool nonZeroRemainder) const noexcept {
		uint16_t reglen, regime, bits;
		if (m < 0) {
			reglen = (-m & 0xFFFF);
			regime = 0x4000 >> reglen;
		}
		else {
			reglen = m + 1;
			regime = 0x7FFF - (0x7FFF >> reglen);
		}

		if (reglen > 14) {
			bits = (m<0 ? 0x0001 : 0x7FFF);  // minpos and maxpos
		}
		else {
			frac32 &= 0x3FFF; // remove both carry bits
			uint16_t fraction = uint16_t(frac32 >> (reglen + 2));

			bool bitNPlusOne = false;
			uint16_t moreBits{ 0 };
			if (reglen <= 12) {
				bitNPlusOne = bool((frac32 >> (reglen + 1)) & 0x1);
				exp <<= (12 - reglen);
				if (bitNPlusOne) moreBits = (((1ull << (reglen + 1)) - 1ull) & frac32) ? 0x1 : 0x0;
			}
			else {
				if (reglen == 14) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (reglen == 13) {
					bitNPlusOne = bool(exp & 0x1);
					exp >>= 1;
				}
				if (frac32 > 0) {
					fraction = 0;
					moreBits = 0x1;
				}
			}
			if (nonZeroRemainder) moreBits = 0x1;
			bits = regime | exp | uint16_t(fraction);
			if (bitNPlusOne) bits += (bits & 0x1) | moreBits;
		}
		return bits;
	}
	inline uint16_t adjustAndRound(const int8_t m, uint16_t exp, uint32_t frac32) const noexcept {
		uint16_t reglen, regime, bits;
		if (m < 0) {
			reglen = (-m & 0xFFFF);
			regime = 0x4000 >> reglen;
		}
		else {
			reglen = int16_t(m) + 1;
			regime = 0x7FFF - (0x7FFF >> reglen);
		}

		if (reglen > 14) {
			bits = m < 0 ? 0x0001 : 0x7FFF;  // minpos and maxpos. exp and frac do not matter 
		}
		else {
			// remove carry and rcarry bits and shift to correct position
			frac32 = (frac32 & 0x0FFF'FFFFu) >> reglen;
			uint16_t fraction = uint16_t(frac32 >> 16);
			bool bitNPlusOne = false;
			uint16_t moreBits = 0;
			if (reglen <= 12) {
				bitNPlusOne = bool(0x8000 & frac32);
				exp <<= (12 - reglen);
			}
			else {
				if (reglen == 14) {
					bitNPlusOne = bool(exp & 0x2);
					moreBits = exp & 0x1;
					exp = 0;
				}
				else if (reglen == 13) {
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
				if (0x7FFF & frac32) moreBits = 1;
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
