#pragma once
// posit8.h: standard 8-bit posit C implementation
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace c_impl {

		static const uint8_t posit8_sign_mask = 0x80;

		// characterization tests
		inline bool posit8_isnar(uint8_t p) { return (p == 0x80); }
		inline bool posit8_iszero(uint8_t p) { return (p == 0x00); }
		inline bool posit8_isone(uint8_t p) { return (p == 0x40); } // pattern 010000...
		inline bool posit8_isminusone(uint8_t p) { return (p == 0xC0); } // pattern 110000...
		inline bool posit8_isneg(uint8_t p) { return (p & 0x80); }
		inline bool posit8_ispos(uint8_t p) { return !(p & 0x80); }
		inline bool posit8_ispowerof2(uint8_t p) { return !(p & 0x1); }

		inline int posit8_sign_value(uint8_t p) { return (p & 0x80 ? -1 : 1); }

		// decode and extraction

		// decode_regime takes the raw bits of the posit, and returns the regime run-length, m, and the remaining fraction bits in remainder
		inline void decode_regime(const uint8_t bits, int8_t& m, uint8_t& remaining) {
			remaining = (bits << 2) & 0xFF;
			if (bits & 0x40) {  // positive regimes
				while (remaining >> 7) {
					++m;
					remaining = (remaining << 1) & 0xFF;
				}
			}
			else {              // negative regimes
				m = -1;
				while (!(remaining >> 7)) {
					--m;
					remaining = (remaining << 1) & 0xFF;
				}
				remaining &= 0x7F;
			}
		}
		inline void extractAddand(const uint8_t bits, int8_t& m, uint8_t& remaining) {
			remaining = (bits << 2) & 0xFF;
			if (bits & 0x40) {  // positive regimes
				while (remaining >> 7) {
					--m;
					remaining = (remaining << 1) & 0xFF;
				}
			}
			else {              // negative regimes
				++m;
				while (!(remaining >> 7)) {
					++m;
					remaining = (remaining << 1) & 0xFF;
				}
				remaining &= 0x7F;
			}
		}
		inline void extractMultiplicand(const uint8_t bits, int8_t& m, uint8_t& remaining) {
			remaining = (bits << 2) & 0xFF;
			if (bits & 0x40) {  // positive regimes
				while (remaining >> 7) {
					++m;
					remaining = (remaining << 1) & 0xFF;
				}
			}
			else {              // negative regimes
				--m;
				while (!(remaining >> 7)) {
					--m;
					remaining = (remaining << 1) & 0xFF;
				}
				remaining &= 0x7F;
			}
		}
		inline void extractDividand(const uint8_t bits, int8_t& m, uint8_t& remaining) {
			remaining = (bits << 2) & 0xFF;
			if (bits & 0x40) {  // positive regimes
				while (remaining >> 7) {
					--m;
					remaining = (remaining << 1) & 0xFF;
				}
			}
			else {              // negative regimes
				++m;
				while (!(remaining >> 7)) {
					++m;
					remaining = (remaining << 1) & 0xFF;
				}
				remaining &= 0x7F;
			}
		}
		inline uint8_t round(const int8_t m, uint16_t fraction) {
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
				uint8_t final_fbits = uint8_t(fraction >> 8);
				bool bitNPlusOne = bool(0x80 & fraction);
				bits = uint8_t(regime) + uint8_t(final_fbits);
				// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
				if (bitNPlusOne) {
					uint8_t moreBits = (0x7F & fraction) ? 0x01 : 0x00;
					bits += (bits & 0x01) | moreBits;
				}
			}
			return bits;
		}
		inline uint8_t adjustAndRound(const int8_t k, uint16_t fraction, bool nonZeroRemainder) {
			uint8_t scale, regime, bits;
			if (k < 0) {
				scale = (-k & 0xFF);
				regime = 0x40 >> scale;
			}
			else {
				scale = k + 1;
				regime = 0x7F - (0x7F >> scale);
			}

			if (scale > 6) {
				bits = k<0 ? 0x1 : 0x7F;  // minpos and maxpos
			}
			else {
				//remove carry and rcarry bits and shift to correct position
				fraction &= 0x7F;
				uint8_t final_fbits = (uint_fast16_t)fraction >> (scale + 1);
				bool bitNPlusOne = (0x1 & (fraction >> scale));
				bits = uint8_t(regime) + uint8_t(final_fbits);
				if (bitNPlusOne) {
					uint8_t moreBits = (((1 << scale) - 1) & fraction) ? 0x01 : 0x00;
					if (nonZeroRemainder) moreBits = 0x01;
					// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
					bits += (bits & 0x01) | moreBits;
				}
			}
			return bits;
		}

		// assignment operators for native types
		uint8_t posit8_assign_int8(const signed char rhs) {
			// special case for speed as this is a common initialization
			if (rhs == 0) {
				return 0x00;
			}
			if (rhs == -128) {
				// 0x80 is special in int8 arithmetic as it is its own negation= 
				return 0x80; // NaR
			}
			bool sign = bool(rhs & posit8_sign_mask);
			int8_t v = sign ? -rhs : rhs; // project to positive side of the projective reals
			uint8_t raw;
			if (v > 48) { // +-maxpos
				raw = 0x7F;
			}
			else {
				uint8_t mask = 0x40;
				int8_t k = 6;
				uint8_t fraction_bits = v;
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
			return sign ? -raw : raw;
		}
		uint8_t posit8_assign_float32(const float rhs) {
			return 0x00;
		}

		uint8_t posit8_set_raw_bits(uint64_t value) {
			return uint8_t(value & 0xff);
		}

		uint8_t posit8_negate(uint8_t p) {
			return -p; // 0 and NaR are invariant under uint8 arithmetic
		}
		uint8_t posit8_add(uint8_t lhs, uint8_t rhs) {
			// process special cases
			if (posit8_isnar(lhs) || posit8_isnar(rhs)) {   // NaR
				return 0x80;
			}
			if (posit8_iszero(lhs) || posit8_iszero(rhs)) { // zero
				return lhs | rhs;
			}
			bool sign = bool(lhs & posit8_sign_mask);
			if (sign) {
				lhs = -lhs & 0xFF;
				rhs = -rhs & 0xFF;
			}
			if (lhs < rhs) std::swap(lhs, rhs);
					
			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint8_t remaining = 0;
			decode_regime(lhs, m, remaining);
			uint16_t frac16A = (0x80 | remaining) << 7;
			int8_t shiftRight = m;
			// adjust shift and extract fraction bits of rhs
			extractAddand(rhs, shiftRight, remaining);
			uint16_t frac16B = (0x80 | remaining) << 7;

			// Work-around CLANG (LLVM) compiler when shifting right more than number of bits
			(shiftRight>7) ? (frac16B = 0) : (frac16B >>= shiftRight); 

			frac16A += frac16B;

			bool rcarry = bool(0x8000 & frac16A); // is MSB set
			if (rcarry) {
				m++;
				frac16A >>= 1;
			}

			uint8_t raw = round(m, frac16A);
			return (sign ? -raw : raw);
		}
		uint8_t posit8_sub(uint8_t lhs, uint8_t rhs) {
			// process special cases
			if (posit8_isnar(lhs) || posit8_isnar(rhs)) {
				return 0x80;
			}
			if (posit8_iszero(lhs) || posit8_iszero(rhs)) {
				return lhs | rhs;
			}
			// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
			bool sign = bool(lhs & posit8_sign_mask);
			(sign) ? (lhs = (-lhs & 0xFF)) : (rhs = (-rhs & 0xFF));

			if (lhs == rhs) {
				return 0x00;
			}
			if (lhs < rhs) {
				std::swap(lhs, rhs);
				sign = !sign;
			}

			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint8_t remaining = 0;
			decode_regime(lhs, m, remaining);
			uint16_t frac16A = (0x80 | remaining) << 7;
			int8_t shiftRight = m;
			// adjust shift and extract fraction bits of rhs
			extractAddand(rhs, shiftRight, remaining);
			uint16_t frac16B = (0x80 | remaining) << 7;

			// do the subtraction of the fractions
			uint8_t raw = 0;
			if (shiftRight >= 14) {
				raw = lhs;
				return (sign ? -raw : raw);
			}
			else {
				frac16B >>= shiftRight;
			}
			frac16A -= frac16B;

			while ((frac16A >> 14) == 0) {
				m--;
				frac16A <<= 1;
			}
			bool ecarry = bool (0x4000 & frac16A);
			if (!ecarry) {
				m--;
				frac16A <<= 1;
			}

			raw = round(m, frac16A);
			return (sign ? -raw : raw);
		}
		uint8_t posit8_mul(uint8_t lhs, uint8_t rhs) {
			// process special cases
			if (posit8_isnar(lhs) || posit8_isnar(rhs)) {
				return 0x80;
			}
			if (posit8_iszero(lhs) || posit8_iszero(rhs)) {
				return 0x00;
			}

			// calculate the sign of the result
			bool sign = bool(lhs & 0x80) ^ bool(rhs & 0x80);
			lhs = lhs & 0x80 ? -lhs : lhs;
			rhs = rhs & 0x80 ? -rhs : rhs;

			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint8_t remaining = 0;
			decode_regime(lhs, m, remaining);
			uint8_t lhs_fraction = (0x80 | remaining);
			// adjust shift and extract fraction bits of rhs
			extractMultiplicand(rhs, m, remaining);
			uint8_t rhs_fraction = (0x80 | remaining);
			uint16_t result_fraction = uint16_t(lhs_fraction) * uint16_t(rhs_fraction);

			bool rcarry = bool(result_fraction & 0x8000);
			if (rcarry) {
				m++;
				result_fraction >>= 1;
			}

			// round
			uint8_t raw = round(m, result_fraction);
			return (sign ? -raw : raw);
		}
		uint8_t posit8_div(uint8_t lhs, uint8_t rhs) {
			// process special cases
			if (posit8_isnar(lhs) || posit8_isnar(rhs) || posit8_iszero(rhs)) {
				return 0x80;
			}
			if (posit8_iszero(lhs)) {
				return 0x00;
			}

			// calculate the sign of the result
			bool sign = bool(lhs & 0x80) ^ bool(rhs & 0x80);
			lhs = lhs & 0x80 ? -lhs : lhs;
			rhs = rhs & 0x80 ? -rhs : rhs;

			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint8_t remaining = 0;
			decode_regime(lhs, m, remaining);
			uint16_t lhs_fraction = (0x80 | remaining) << 7;
			// adjust shift and extract fraction bits of rhs
			extractDividand(rhs, m, remaining);
			uint8_t rhs_fraction = (0x80 | remaining);
			div_t result = div(lhs_fraction, uint16_t(rhs_fraction));
			uint16_t result_fraction = result.quot;
			uint16_t remainder = result.rem;

			if (result_fraction != 0) {
				bool rcarry = result_fraction >> 7; // this is the hidden bit (7th bit) , extreme right bit is bit 0
				if (!rcarry) {
					--m;
					result_fraction <<= 1;
				}
			}

			// round
			uint8_t raw = adjustAndRound(m, result_fraction, remainder != 0);
			return (sign ? -raw : raw);
		}

		uint8_t posit8_reciprocate(uint8_t rhs) {
			return posit8_div(0x40, rhs);
		}

		float posit8_to_float(uint8_t p) {
			if (p == 0)		return 0.0;
			if (p = 0x80)	return NAN;
			bool		     	 _sign;

			float s = (_sign ? -1.0 : 1.0);
			float r = 0.0; // _regime.value();
			float e = 1.0; // _exponent.value();
			float f = (1.0 + 0.0); // (1.0 + _fraction.value());
			return s * r * e * f;
		}

		int posit8_to_int(uint8_t p) {
			if (posit8_isnar(p))  return int(INFINITY);
			return int(posit8_to_float(p));
		}



		uint8_t float_assign(const float& rhs) {
			return 0;
		}


		// posit - posit binary logic functions
		bool posit8_equal(const uint8_t& lhs, const uint8_t& rhs)          { return lhs == rhs;  }
		bool posit8_notEqual(const uint8_t& lhs, const uint8_t& rhs)       { return lhs != rhs;  }
		bool posit8_lessThan(const uint8_t& lhs, const uint8_t& rhs)       { return lhs < rhs; }
		bool posit8_greaterThan(const uint8_t& lhs, const uint8_t& rhs)    { return lhs > rhs;  }
		bool posit8_lessOrEqual(const uint8_t& lhs, const uint8_t& rhs)    { return lhs <= rhs; }
		bool posit8_greaterOrEqual(const uint8_t& lhs, const uint8_t& rhs) { return lhs >= rhs; }

			// preprocessin for add and sub
			/*
			inline posit<NBITS_IS_8, ES_IS_0> operator+(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				posit<NBITS_IS_8, ES_IS_0> result = lhs;
				if (lhs.isneg() == rhs.isneg()) {  // are the posits the same sign?
					result += rhs;
				} 
				else {
					result -= rhs;
				}
				return result;
			}
			inline posit<NBITS_IS_8, ES_IS_0> operator-(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				posit<NBITS_IS_8, ES_IS_0> result = lhs;
				if (lhs.isneg() == rhs.isneg()) {  // are the posits the same sign?
					result -= rhs.twosComplement();
				}
				else {
					result += rhs.twosComplement();
				}
				return result;

			}
			// binary operator*() is provided by generic class
			*/

	}
}
