#pragma once
// posit8_1.h: specialized 8-bit posit<8,1> C implementation
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>  // for NAN and INFINITY

#include <universal/number/posit/positctypes.h>

static const uint8_t posit8_1_sign_mask = 0x80;

// characterization tests
inline bool posit8_1_isnar(posit8_1_t p) { return (p.v == 0x80); }
inline bool posit8_1_iszero(posit8_1_t p) { return (p.v == 0x00); }
inline bool posit8_1_isone(posit8_1_t p) { return (p.v == 0x40); }      // pattern 010000...
inline bool posit8_1_isminusone(posit8_1_t p) { return (p.v == 0xC0); } // pattern 110000...
inline bool posit8_1_isneg(posit8_1_t p) { return (p.v & 0x80); }
inline bool posit8_1_ispos(posit8_1_t p) { return !(p.v & 0x80); }
inline bool posit8_1_ispowerof2(posit8_1_t p) { return !(p.v & 0x1); }

// decode takes the raw bits of the posit, and returns the regime, m, and returns the fraction bits in 'remainder'
inline int8_t  posit8_1_decode_regime(const uint8_t bits, uint8_t* remaining) {
	int8_t m = 0;
	*remaining = (bits << 2) & 0xFF;
	if (bits & 0x40) {  // positive regimes
		while (*remaining >> 7) {
			++m;
			*remaining = (*remaining << 1) & 0xFF;
		}
	}
	else {              // negative regimes
		m = -1;
		while (!(*remaining >> 7)) {
			--m;
			*remaining = (*remaining << 1) & 0xFF;
		}
		*remaining &= 0x7F;
	}
	return m;
}
// rounding
inline uint8_t posit8_1_round(const int8_t m, uint16_t fraction) {
	uint8_t scale, regime, bits;
	if (m < 0) {
		scale = (-m & 0xFF);
		regime = 0x40 >> scale;
	}
	else {
		scale = m + 1;
		regime = 0x7F - (0x7F >> scale);
	}

	if (scale > 6) {
		bits = m<0 ? 0x1 : 0x7F;  // minpos and maxpos
	}
	else {
		fraction = (fraction & 0x3FFF) >> scale;
		uint8_t final_fbits = (uint8_t)(fraction >> 8);
		bool bitNPlusOne = (bool)(0x80 & fraction);
		bits = (uint8_t)regime + (uint8_t)final_fbits;
		// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			uint8_t moreBits = (0x7F & fraction) ? 0x01 : 0x00;
			bits += (bits & 0x01) | moreBits;
		}
	}
	return bits;
}
inline uint8_t posit8_1_roundDiv(const int8_t m, uint16_t fraction, bool nonZeroRemainder) {
	uint8_t scale, regime, bits;
	if (m < 0) {
		scale = (-m & 0xFF);
		regime = 0x40 >> scale;
	}
	else {
		scale = m + 1;
		regime = 0x7F - (0x7F >> scale);
	}

	if (scale > 6) {
		bits = m<0 ? 0x1 : 0x7F;  // minpos and maxpos
	}
	else {
		// remove carry and rcarry bits and shift to correct position
		fraction &= 0x7F;
		uint8_t final_fbits = fraction >> (scale + 1);
		bool bitNPlusOne = (bool)(0x1 & (fraction >> scale));
		bits = (uint8_t)regime + (uint8_t)final_fbits;
		if (bitNPlusOne) {
			uint8_t moreBits = (((1 << scale) - 1) & fraction) ? 0x01 : 0x00;
			if (nonZeroRemainder) moreBits = 0x01;
			// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			bits += (bits & 0x01) | moreBits;
		}
	}
	return bits;
}

// conversion functions
inline int  posit8_1_sign_value(posit8_1_t p) { return ((p.v & 0x80) ? -1 : 1); }
float       posit8_1_fraction_value(uint8_t fraction) {
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
void        posit8_1_checkExtraTwoBits(float f, float temp, bool* bitsNPlusOne, bool* bitsMore) {
	temp /= 2.0;
	if (temp <= f) {
		*bitsNPlusOne = 1;
		f -= temp;
	}
	if (f>0)
		*bitsMore = 1;
}
uint16_t    posit8_1_convertFraction(float f, uint8_t fracLength, bool* bitsNPlusOne, bool* bitsMore) {

	uint_fast8_t frac = 0;

	if (f == 0) return 0;
	else if (f == INFINITY) return 0x80;

	f -= 1; //remove hidden bit
	if (fracLength == 0)
		posit8_1_checkExtraTwoBits(f, 1.0, bitsNPlusOne, bitsMore);
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
					posit8_1_checkExtraTwoBits(f, temp, bitsNPlusOne, bitsMore);

					break;
				}
			}
			else {
				frac <<= 1; //shift in a zero
				fracLength--;
				if (fracLength == 0) {
					posit8_1_checkExtraTwoBits(f, temp, bitsNPlusOne, bitsMore);
					break;
				}
			}
		}
	}
	//printf("convertfloat: frac:%d bitsNPlusOne: %d, bitsMore: %d\n", frac, bitsNPlusOne, bitsMore);
	return frac;
}
// assignment operators for native types
posit8_1_t  posit8_1_fromsi(int rhs) {
	// special case for speed as this is a common initialization
	posit8_1_t p = { { 0x00} };
	if (rhs == 0) {
		return p;
	}
	bool sign = (rhs < 0 ? true : false);
	int v = sign ? -rhs : rhs; // project to positive side of the projective reals
	uint8_t raw; // 0x7C = 256, 0x7D = 512, 0x7E = 1024, 0x7F = 4096
	if (v > 2048) { // +-maxpos
		raw = 0x7F;
	}
	else if (v >= 768) {   
		raw = 0x7E;
	}
	else if (v < 2) {  // 0 or 1
		raw = (v << 6); 
	}
	else {
		uint8_t mask = 0x40;
		int8_t log2 = 12;
		uint16_t fraction_bits = v;
		while (!(fraction_bits & mask)) {
			log2--;
			fraction_bits <<= 1;
		}
		int8_t k = log2 >> 1;
		uint16_t exp_bits = (log2 & 0x1) << (12 - k);
		fraction_bits = (fraction_bits ^ mask);
		raw = (0x7F ^ (0x3F >> k)) | exp_bits | (fraction_bits >> (k + 1));

		mask = 0x1 << k; //bitNPlusOne
		if (mask & fraction_bits) {
			if (((mask - 1) & fraction_bits) | ((mask << 1) & fraction_bits)) raw++;
		}
	}

	p.v = (sign ? -raw : raw);
	return p;
}
posit8_1_t  posit8_1_fromf(float f) {
	posit8_1_t p;
	bool sign;
	bool bitNPlusOne = 0, bitsMore = 0;
	const float _minpos = 0.000244140625f;
	const float _maxpos = 4096.0f;

	sign = (f < 0 ? true : false);
	
	if (isinf(f) || isnan(f)) {
		p.v = 0x80;
	}
	else if (f == 0) {
		p.v = 0;
	}
	else if (f == 1) {
		p.v = 0x40;
	}
	else if (f == -1) {
		p.v = 0xC0;
	}
	else if (f >= _maxpos) {
		p.v = 0x7F;
	}
	else if (f <= -_maxpos) {
		p.v = 0x81;
	}
	else if (f <= _minpos && !sign) {
		p.v = 0x1;
	}
	else if (f >= -_minpos && sign) {
		p.v = 0xFF;
	}
	else if (f < -1 || f > 1) {
		if (sign) {		
			f = -f; // project to positive reals to simplify computation
		}
		
		if (f <= _minpos) {
			p.v = 0x01;
		}
		else { // determine the regime
			unsigned k = 1; // because k = m-1; we need to add back 1
			while (f >= 2) {
				f *= 0.5;
				k++;
			}
			// rounding off regime bits
			if (k > 6) {
				p.v = 0x7F;
			}
			else {
				int8_t fracLength = 6 - k;
				uint8_t frac = (uint8_t)posit8_1_convertFraction(f, fracLength, &bitNPlusOne, &bitsMore);
				uint_fast8_t regime = 0x7F - (0x7F >> k);
				p.v = (regime + frac);
				if (bitNPlusOne) p.v += ((p.v & 1) | bitsMore);
			}
			p.v = (sign ? -p.v : p.v);
		}
	}
	else if (f > -1 && f < 1) {
		if (sign) {
			f = -f;
		}
		unsigned k = 0;
		while (f<1) {
			f *= 2;
			k++;
		}
		// rounding off regime bits
		if (k > 6)
			p.v = 0x1;
		else {
			int8_t fracLength = 6 - k;
			uint8_t frac = (uint8_t)posit8_1_convertFraction(f, fracLength, &bitNPlusOne, &bitsMore);
			uint8_t regime = 0x40 >> k;
			p.v = (regime + frac);
			if (bitNPlusOne) p.v += ((p.v & 1) | bitsMore);
		}
		p.v = (sign ? -p.v : p.v);
	}
	else {	
		p = NAR8;   // NaR - for NaN, INF and all other combinations
	}
	return p;
}
posit8_1_t  posit8_1_fromd(double d) {
	return posit8_1_fromf((float)d);
}
posit8_1_t  posit8_1_fromld(long double ld) {
	return posit8_1_fromf((float)ld);
}
float       posit8_1_tof(posit8_1_t p) {
	if (p.v == 0) return 0.0f;
	if (p.v == 0x80) return NAN;   //  INFINITY is not semantically correct. NaR is Not a Real and thus is more closely related to a NAN, or Not a Number

	uint8_t bits = ((p.v & 0x80) ? -p.v : p.v);  // use 2's complement when negative	
	uint8_t fraction = 0;
	int8_t m = posit8_1_decode_regime(bits, &fraction);
	uint8_t xp = fraction >> 7;

	float s = (float)(posit8_1_sign_value(p));
	float r = 4.0f * (m > 0 ? (float)((uint32_t)(1) << m) : (1.0f / (float)((uint32_t)(1) << -m)));
	float e = 2.0f * xp;
	float f = 1.0f;
	f += posit8_1_fraction_value(fraction);

//	printf("sign = %f : m = %d : regime = %f : fraction = 0x%x : fraction_value %f\n", s, m, r, fraction, f);
	return s * r * e * f;
}
double      posit8_1_tod(posit8_1_t p) {
	return (double)posit8_1_tof(p);
}
long double posit8_1_told(posit8_1_t p) {
	return (long double)posit8_1_tof(p);
}
int         posit8_1_tosi(posit8_1_t p) {
	if (posit8_1_isnar(p)) return (int)NAN; // INFINITY;
	return (int)(posit8_1_tof(p));
}

// arithmetic operators
posit8_1_t posit8_1_negate(posit8_1_t p) {
	p.v = -p.v; // 0 and NaR are invariant under uint8 arithmetic
	return p;
}
posit8_1_t posit8_1_addMagnitude(posit8_1_t lhs, posit8_1_t rhs) {
	//	printf("lhs = 0x%02x  rhs = 0x%02x\n", (uint8_t)(lhs.v), (uint8_t)(rhs.v));
		posit8_1_t p = { { 0x80 } };
	// process special cases
	if (posit8_1_isnar(lhs) || posit8_1_isnar(rhs)) {   // NaR
		return p;
	}
	if (posit8_1_iszero(lhs) || posit8_1_iszero(rhs)) { // zero
		p.v = (uint8_t)(lhs.v | rhs.v);
		return p;
	}
	bool sign = (bool)(lhs.v & posit8_1_sign_mask);
	if (sign) {
		lhs.v = -lhs.v & 0xFF;
		rhs.v = -rhs.v & 0xFF;
	}
	if (lhs.v < rhs.v) {
		uint8_t tmp = lhs.v;
		lhs.v = rhs.v;
		rhs.v = tmp;
	}

	// decode the regimes and extract the fractions of the operands
	uint8_t remaining = 0;
	int8_t mA = posit8_1_decode_regime(lhs.v, &remaining);
	uint16_t lhs_fraction = (0x80 | remaining) << 7;
	int8_t mB = posit8_1_decode_regime(rhs.v, &remaining);
	uint16_t rhs_fraction = (0x80 | remaining) << 7;
	int8_t shiftRight = mA - mB; // calculate the shift to normalize the fractions
	
	if (shiftRight > 7) {  // catastrophic cancellation case
		rhs_fraction = 0;
	}
	else {
		rhs_fraction >>= shiftRight; // align the rhs fraction
	}
	uint16_t result_fraction = lhs_fraction + rhs_fraction; // add

	bool rcarry = (bool)(0x8000 & result_fraction); // is MSB set
	if (rcarry) {
		mA++;
		result_fraction >>= 1;
	}

	uint8_t raw = posit8_1_round(mA, result_fraction);
	p.v = (sign ? -raw : raw);
	return p;
}
posit8_1_t posit8_1_subMagnitude(posit8_1_t lhs, posit8_1_t rhs) {
	// process special cases
	posit8_1_t p = { { 0x80} };
	if (posit8_1_isnar(lhs) || posit8_1_isnar(rhs)) {
		return p;
	}
	if (posit8_1_iszero(lhs) || posit8_1_iszero(rhs)) {
		p.v = (uint8_t)(lhs.v | rhs.v);
		return p;
	}

	// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
	bool sign = (bool)(lhs.v & posit8_1_sign_mask);
	if (sign) {
		lhs.v = -lhs.v;
	}
	else {
		rhs.v = -rhs.v;
	}

	if (lhs.v == rhs.v) {
		p.v = 0;
		return p;
	}
	if (lhs.v < rhs.v) {
		uint8_t tmp = lhs.v;
		lhs.v = rhs.v;
		rhs.v = tmp;
		sign = !sign;
	}

	// decode the regimes and extract the fractions of the operands
	uint8_t remaining = 0;
	int8_t mA = posit8_1_decode_regime(lhs.v, &remaining);
	uint16_t lhs_fraction = (0x80 | remaining) << 7;
	int8_t mB = posit8_1_decode_regime(rhs.v, &remaining);
	uint16_t rhs_fraction = (0x80 | remaining) << 7;	
	int8_t shiftRight = mA - mB;  // calculate the shift to normalize the fractions

	if (shiftRight >= 14) { // catastrophic cancellation case
		p.v = (sign ? -lhs.v : lhs.v);
		return p;
	}
	else {
		rhs_fraction >>= shiftRight;	// align the rhs fraction
	}
	uint16_t result_fraction = lhs_fraction - rhs_fraction;

	while ((result_fraction >> 14) == 0) {
		mA--;
		result_fraction <<= 1;
	}
	bool ecarry = (bool)(0x4000 & result_fraction);
	if (!ecarry) {
		mA--;
		result_fraction <<= 1;
	}

	uint8_t raw = posit8_1_round(mA, result_fraction);
	p.v = (sign ? -raw : raw);
	return p;
}
posit8_1_t posit8_1_addp8(posit8_1_t lhs, posit8_1_t rhs) {
	posit8_1_t p;
	if ((lhs.v^rhs.v) >> 7) {
		p = posit8_1_subMagnitude(lhs, rhs);
	}
	else {
		p = posit8_1_addMagnitude(lhs, rhs);
	}
	return p;
}
posit8_1_t posit8_1_subp8(posit8_1_t lhs, posit8_1_t rhs) {
	posit8_1_t p;
	bool differentSign = (bool)((lhs.v^rhs.v) >> 7);
	rhs.v = -rhs.v;
	if (differentSign) {
		p = posit8_1_addMagnitude(lhs, rhs);
	}
	else {
		p = posit8_1_subMagnitude(lhs, rhs);
	}
	return p;
}
posit8_1_t posit8_1_mulp8(posit8_1_t lhs, posit8_1_t rhs) {
	// process special cases
	posit8_1_t p = { { 0x80} };
	if (posit8_1_isnar(lhs) || posit8_1_isnar(rhs)) {
		return p;
	}
	if (posit8_1_iszero(lhs) || posit8_1_iszero(rhs)) {
		p.v = 0;
		return p;
	}

	// calculate the sign of the result
	bool sign = (bool)(lhs.v & 0x80) ^ (bool)(rhs.v & 0x80);
	lhs.v = (lhs.v & 0x80) ? -lhs.v : lhs.v;
	rhs.v = (rhs.v & 0x80) ? -rhs.v : rhs.v;

	// decode the regimes and extract the fractions of the operands
	uint8_t remaining = 0;
	int8_t mA = posit8_1_decode_regime(lhs.v, &remaining);
	uint16_t lhs_fraction = (0x80 | remaining);
	int8_t mB = posit8_1_decode_regime(rhs.v, &remaining);
	uint16_t rhs_fraction = (0x80 | remaining);
	uint16_t result_fraction = (lhs_fraction * rhs_fraction);
	int8_t scale = mA + mB;

	bool rcarry = (bool)(result_fraction & 0x8000);
	if (rcarry) {
		scale++;
		result_fraction >>= 1;
	}

	// round
	uint8_t raw = posit8_1_round(scale, result_fraction);
	p.v = (sign ? -raw : raw);
	return p;
}
posit8_1_t posit8_1_divp8(posit8_1_t lhs, posit8_1_t rhs) {
	// process special cases
	posit8_1_t p = { { 0x80 } };
	if (posit8_1_isnar(lhs) || posit8_1_isnar(rhs) || posit8_1_iszero(rhs)) {
		return p;
	}
	if (posit8_1_iszero(lhs)) {
		p.v = 0;
		return p;
	}

	// calculate the sign of the result
	bool sign = (bool)(lhs.v & 0x80) ^ (bool)(rhs.v & 0x80);
	lhs.v = (lhs.v & 0x80) ? -lhs.v : lhs.v;
	rhs.v = (rhs.v & 0x80) ? -rhs.v : rhs.v;

	// decode the regimes and extract the fractions of the operands
	uint8_t remaining = 0;
	int8_t mA = posit8_1_decode_regime(lhs.v, &remaining);
	uint16_t lhs_fraction = (0x80 | remaining) << 7;
	int8_t mB = posit8_1_decode_regime(rhs.v, &remaining);
	uint16_t rhs_fraction = (0x80 | remaining);
	div_t result = div(lhs_fraction, rhs_fraction);
	uint16_t result_fraction = result.quot;
	uint16_t remainder = result.rem;
	int8_t scale = mA - mB;

	if (result_fraction != 0) {
		bool rcarry = result_fraction >> 7; // this is the hidden bit (7th bit) , extreme right bit is bit 0
		if (!rcarry) {
			--scale;
			result_fraction <<= 1;
		}
	}

	// round
	uint8_t raw = posit8_1_roundDiv(scale, result_fraction, remainder != 0);
	p.v = (sign ? -raw : raw);
	return p;
}
posit8_1_t posit8_1_reciprocate(posit8_1_t rhs) {
	posit8_1_t one = { { 0x40 } };
	return posit8_1_divp8(one, rhs);
}

// posit - posit binary logic functions
bool posit8_1_equal(posit8_1_t lhs, posit8_1_t rhs)          { return lhs.v == rhs.v;  }
bool posit8_1_notEqual(posit8_1_t lhs, posit8_1_t rhs)       { return lhs.v != rhs.v;  }
bool posit8_1_lessThan(posit8_1_t lhs, posit8_1_t rhs)       { return lhs.v < rhs.v; }
bool posit8_1_greaterThan(posit8_1_t lhs, posit8_1_t rhs)    { return lhs.v > rhs.v;  }
bool posit8_1_lessOrEqual(posit8_1_t lhs, posit8_1_t rhs)    { return lhs.v <= rhs.v; }
bool posit8_1_greaterOrEqual(posit8_1_t lhs, posit8_1_t rhs) { return lhs.v >= rhs.v; }

