#pragma once
// posit_32_2.hpp: specialized 32-bit posit using fast compute specialized for posit<32,2>
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_32_2
#pragma message("Fast specialization of posit<32,2>")

	// fast specialized posit<32,2>
	template<>
	class posit<NBITS_IS_32, ES_IS_2> {
	public:
		static constexpr size_t nbits = NBITS_IS_32;
		static constexpr size_t es = ES_IS_2;
		static constexpr size_t sbits = 1;
		static constexpr size_t rbits = nbits - sbits;
		static constexpr size_t ebits = es;
		static constexpr size_t fbits = nbits - 3 - es;
		static constexpr size_t fhbits = fbits + 1;
		static constexpr uint32_t sign_mask = 0x8000'0000ul;

		posit() { _bits = 0; }
		posit(const posit&) = default;
		posit(posit&&) = default;
		posit& operator=(const posit&) = default;
		posit& operator=(posit&&) = default;

		// initializers for native types
		posit(signed char initial_value)        { *this = initial_value; }
		posit(short initial_value)              { *this = initial_value; }
		posit(int initial_value)                { *this = initial_value; }
		posit(long initial_value)               { *this = initial_value; }
		posit(long long initial_value)          { *this = initial_value; }
		posit(char initial_value)               { *this = initial_value; }
		posit(unsigned short initial_value)     { *this = initial_value; }
		posit(unsigned int initial_value)       { *this = initial_value; }
		posit(unsigned long initial_value)      { *this = initial_value; }
		posit(unsigned long long initial_value) { *this = initial_value; }
		posit(float initial_value)              { *this = initial_value; }
		posit(double initial_value)             { *this = initial_value; }
		posit(long double initial_value)        { *this = initial_value; }

		// assignment operators for native types
		posit& operator=(signed char rhs)       { return integer_assign((long)(rhs)); }
		posit& operator=(short rhs)             { return integer_assign((long)(rhs)); }
		posit& operator=(int rhs)               { return integer_assign((long)(rhs)); }
		posit& operator=(long rhs)              { return integer_assign(rhs); }
		posit& operator=(long long rhs)         { return integer_assign((long)(rhs)); }
		posit& operator=(char rhs)              { return integer_assign((long)(rhs)); }
		posit& operator=(unsigned short rhs)    { return integer_assign((long)(rhs)); }
		posit& operator=(unsigned int rhs)      { return integer_assign((long)(rhs)); }
		posit& operator=(unsigned long rhs)     { return integer_assign((long)(rhs)); }
		posit& operator=(unsigned long long rhs){ return integer_assign((long)(rhs)); }
		posit& operator=(float rhs)             { return float_assign((long double)rhs); }
		posit& operator=(double rhs)            { return float_assign((long double)rhs); }
		posit& operator=(long double rhs)       { return float_assign(rhs); }

		explicit operator long double() const { return to_long_double(); }
		explicit operator double() const { return to_double(); }
		explicit operator float() const { return to_float(); }
		explicit operator long long() const { return to_long_long(); }
		explicit operator long() const { return to_long(); }
		explicit operator int() const { return to_int(); }
		explicit operator unsigned long long() const { return to_long_long(); }
		explicit operator unsigned long() const { return to_long(); }
		explicit operator unsigned int() const { return to_int(); }

		posit& set(sw::unum::bitblock<NBITS_IS_32>& raw) {
			_bits = uint32_t(raw.to_ulong());
			return *this;
		}
		posit& set_raw_bits(uint64_t value) {
			_bits = uint32_t(value & 0xFFFF'FFFF);
			return *this;
		}
		posit operator-() const {
			if (iszero()) {
				return *this;
			}
			if (isnar()) {
				return *this;
			}
			posit p;
			return p.set_raw_bits((~_bits) + 1);
		}
		posit& operator+=(const posit& b) { // derived from SoftPosit
			// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
			if (isnar() || b.isnar()) {
				throw operand_is_nar{};
			}
#else
			if (isnar() || b.isnar()) {
				setnar();
				return *this;
			}
#endif
			uint32_t lhs = _bits;
			uint32_t rhs = b._bits;
			if (iszero() || b.iszero()) { // zero
				_bits = lhs | rhs;
				return *this;
			}
			bool sign = bool(_bits & sign_mask);
			if (sign) {
				lhs = -int32_t(lhs) & 0xFFFF'FFFF;
				rhs = -int32_t(rhs) & 0xFFFF'FFFF;
			}
			if (lhs < rhs) std::swap(lhs, rhs);
			
			// decode the regime of lhs
			int32_t m = 0; // pattern length
			uint32_t remaining = 0;
			decode_regime(lhs, m, remaining);

			// extract the exponent
			uint32_t exp = remaining >> 29;

			// extract the remaining fraction
			uint64_t frac64A = ((0x4000'0000ull | remaining << 1) & 0x7FFF'FFFFull) << 32;
			int32_t shiftRight = m;

			// adjust shift and extract fraction bits of rhs
			extractAddand(rhs, shiftRight, remaining);
			uint64_t frac64B = ((0x4000'0000ull | remaining << 1) & 0x7FFF'FFFFull) << 32;
			// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
			shiftRight = (shiftRight << 2) + exp - (remaining >> 29);

			// Work-around CLANG (LLVM) compiler when shifting right more than number of bits
			frac64B = (shiftRight > 63) ? 0 : (frac64B >> shiftRight); 

			frac64A += frac64B; // add the now aligned fractions

			bool rcarry = bool(0x8000'0000'0000'0000 & frac64A); // is MSB set
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
		posit& operator-=(const posit& b) {  // derived from SoftPosit
			// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
			if (isnar() || b.isnar()) {
				throw operand_is_nar{};
			}
#else
			if (isnar() || b.isnar()) {
				setnar();
				return *this;
			}
#endif
			uint32_t lhs = _bits;
			uint32_t rhs = b._bits;
			if (iszero() || b.iszero()) {
				_bits = lhs | rhs;
				return *this;
			}
			// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
			bool sign = bool(lhs & sign_mask);
			(sign) ? (lhs = (-int32_t(lhs) & 0xFFFF'FFFF)) : (rhs = (-int32_t(rhs) & 0xFFFF'FFFF));

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
			uint64_t frac64A = ((0x4000'0000ull | remaining << 1) & 0x7FFF'FFFFull) << 32;
			int32_t shiftRight = m;

			// adjust shift and extract fraction bits of rhs
			extractAddand(rhs, shiftRight, remaining);
			uint64_t frac64B = ((0x4000'0000ull | remaining << 1) & 0x7FFF'FFFFull) << 32;

			// This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
			shiftRight = (shiftRight << 2) + exp - (remaining >> 29);
			if (shiftRight > 63) {  // catastrophic cancellation case
				_bits = lhs;
				if (sign) _bits = -int32_t(_bits) & 0xFFFF'FFFF;
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
			bool ecarry = bool (0x4000'0000'0000'0000 & frac64A);
			while (!ecarry) {
				if (exp == 0) {
					--m;
					exp = 0x3;
				}
				else {
					exp--;
				}
				frac64A <<= 1;
				ecarry = bool(0x4000'0000'0000'0000 & frac64A);
			}

			_bits = round(m, exp, frac64A);
			if (sign) _bits = -int32_t(_bits) & 0xFFFF'FFFF;
			return *this;
		}
		posit& operator-=(double rhs) {
			return *this -= posit<nbits, es>(rhs);
		}
		posit& operator*=(const posit& b) {
			// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
			if (isnar() || b.isnar()) {
				throw operand_is_nar{};
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
			lhs = lhs & sign_mask ? -int32_t(lhs) : lhs;
			rhs = rhs & sign_mask ? -int32_t(rhs) : rhs;

			// decode the regime of lhs
			int32_t m = 0;
			uint32_t remaining = 0;
			decode_regime(lhs, m, remaining);
			uint32_t exp = remaining >> 29;  // lhs exponent
			uint32_t lhs_fraction = ((remaining << 1) | 0x40000000) & 0x7FFFFFFF;;

			// adjust shift and extract fraction bits of rhs
			extractMultiplicand(rhs, m, remaining);
			uint32_t rhs_fraction = (((remaining << 1) | 0x40000000) & 0x7FFFFFFF);
			uint64_t result_fraction = uint64_t(lhs_fraction) * uint64_t(rhs_fraction);
			exp += remaining >> 29;  // product exp is the sum of lhs exp and rhs exp

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
				throw divide_by_zero{};    // not throwing is a quiet signalling NaR
			}
			if (b.isnar()) {
				throw divide_by_nar{};
			}
			if (isnar()) {
				throw numerator_is_nar{};
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
			lhs = lhs & sign_mask ? -int32_t(lhs) : lhs;
			rhs = rhs & sign_mask ? -int32_t(rhs) : rhs;

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
			exp -= remaining >> 29;
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
						exp = 0x3;
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
		// SELECTORS
		inline bool isnar() const      { return (_bits == 0x8000'0000); }
		inline bool iszero() const     { return (_bits == 0x0); }
		inline bool isone() const      { return (_bits == 0x4000'0000); } // pattern 010000...
		inline bool isminusone() const { return (_bits == 0xC000'0000); } // pattern 110000...
		inline bool isneg() const      { return (_bits & 0x8000'0000); }
		inline bool ispos() const      { return !isneg(); }
		inline bool ispowerof2() const { return !(_bits & 0x1); }

		inline int sign_value() const  { return (_bits & 0x8 ? -1 : 1); }

		bitblock<NBITS_IS_32> get() const { bitblock<NBITS_IS_32> bb; bb = long(_bits); return bb; }
		unsigned long long encoding() const { return (unsigned long long)(_bits); }

		inline void clear() { _bits = 0x0; }
		inline void setzero() { clear(); }
		inline void setnar() { _bits = 0x8000'0000; }
		inline posit twosComplement() const {
			posit<NBITS_IS_32, ES_IS_2> p;
			int32_t v = -(int32_t)_bits;
			p.set_raw_bits(v);
			return p;
		}
	private:
		uint32_t _bits;

		// Conversion functions
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		int         to_int() const {
			if (iszero()) return 0;
			if (isnar()) throw not_a_real{};
			return int(to_float());
		}
		long        to_long() const {
			if (iszero()) return 0;
			if (isnar()) throw not_a_real{};
			return long(to_double());
		}
		long long   to_long_long() const {
			if (iszero()) return 0;
			if (isnar()) throw not_a_real{};
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
			for (size_t i = 0; i < nbits; i++) {
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
			for (size_t i = 0; i < nbits; i++) {
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
		posit& integer_assign(long rhs) {
			// special case for speed as this is a common initialization
			if (rhs == 0) {
				_bits = 0x0;
				return *this;
			}

			bool sign = bool(rhs & sign_mask);
			uint32_t v = sign ? -rhs : rhs; // project to positive side of the projective reals
			uint32_t raw;
			if (v == sign_mask) { // +-maxpos, 0x8000'0000 is special in int32 arithmetic as it is its own negation
				raw = sign_mask;
			}
			else if (v > 0xFFFFFBFF) { // 4294966271
				raw = 0x7FC0'0000;     // 4294967296
			}
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
				raw = (0x7FFF'FFFF ^ (0x3FFF'FFFF >> k)) | exponent_bits | (fraction_bits >> (k + 4));

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
			value<dfbits> v((long double)rhs);

			// special case processing
			if (v.iszero()) {
				setzero();
				return *this;
			}
			if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
				setnar();
				return *this;
			}

			bitblock<NBITS_IS_32> ptt;
			convert_to_bb<NBITS_IS_32, ES_IS_2, dfbits>(v.sign(), v.scale(), v.fraction(), ptt); // TODO: needs to be faster
			_bits = uint32_t(ptt.to_ulong());
			return *this;
		}

		// decode_regime takes the raw bits of the posit, and returns the regime run-length, m, and the remaining fraction bits in remainder
		inline void decode_regime(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF'FFFF;
			if (bits & 0x4000'0000) {  // positive regimes
				while (remaining >> 31) {
					++m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
			}
			else {              // negative regimes
				m = -1;
				while (!(remaining >> 31)) {
					--m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
				remaining &= 0x7FFF'FFFF;
			}
		}
		inline void extractAddand(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF'FFFF;
			if (bits & 0x4000'0000) {  // positive regimes
				while (remaining >> 31) {
					--m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
			}
			else {              // negative regimes
				++m;
				while (!(remaining >> 31)) {
					++m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
				remaining &= 0x7FFF'FFFF;
			}
		}
		inline void extractMultiplicand(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF'FFFF;
			if (bits & 0x4000'0000) {  // positive regimes
				while (remaining >> 31) {
					++m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
			}
			else {              // negative regimes
				--m;
				while (!(remaining >> 31)) {
					--m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
				remaining &= 0x7FFF'FFFF;
			}
		}
		inline void extractDividand(const uint32_t bits, int32_t& m, uint32_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF'FFFF;
			if (bits & 0x4000'0000) {  // positive regimes
				while (remaining >> 31) {
					--m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
			}
			else {              // negative regimes
				++m;
				while (!(remaining >> 31)) {
					++m;
					remaining = (remaining << 1) & 0xFFFF'FFFF;
				}
				remaining &= 0x7FFF'FFFF;
			}
		}

		inline uint32_t round(const int8_t m, uint32_t exp, uint64_t fraction) const {
			uint32_t scale, regime, bits;
			if (m < 0) {
				scale = -m;
				regime = 0x4000'0000 >> scale;
			}
			else {
				scale = m + 1;
				regime = 0x7FFF'FFFF - (0x7FFF'FFFF >> scale);
			}

			if (scale > 30) {
				bits = m<0 ? 0x1 : 0x7FFF'FFFF;  // minpos and maxpos
			}
			else {
				fraction = (fraction & 0x3FFF'FFFF'FFFF'FFFF) >> (scale + 2);
				uint32_t final_fbits = uint32_t(fraction >> 32);
				bool bitNPlusOne = false;
				if (scale <= 28) {
					bitNPlusOne = bool(0x8000'0000 & fraction);
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
					uint32_t moreBits = (0x7FFF'FFFF & fraction) ? 0x1 : 0x0;
					bits += (bits & 0x000'0001) | moreBits;
				}
			}
			return bits;
		}
		inline uint32_t round_mul(const int8_t m, uint32_t exp, uint64_t fraction) const {
			uint32_t scale, regime, bits;
			if (m < 0) {
				scale = -m;
				regime = 0x4000'0000 >> scale;
			}
			else {
				scale = m + 1;
				regime = 0x7FFF'FFFF - (0x7FFF'FFFF >> scale);
			}

			if (scale > 30) {
				bits = m<0 ? 0x1 : 0x7FFF'FFFF;  // minpos and maxpos
			}
			else {
				//std::cout << "fracin = " << std::hex << fraction << std::dec << std::endl;
				fraction = (fraction & 0x0FFF'FFFF'FFFF'FFFF) >> scale;
				//std::cout << "fracsh = " << std::hex << fraction << std::dec << std::endl;

				uint32_t final_fbits = uint32_t(fraction >> 32);
				bool bitNPlusOne = false;
				if (scale <= 28) {
					bitNPlusOne = bool(0x0000'0000'8000'0000 & fraction);
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
					uint32_t moreBits = (0x7FFF'FFFF & fraction) ? 0x1 : 0x0;
					bits += (bits & 0x000'0001) | moreBits;
				}
			}
			return bits;
		}
		inline uint32_t adjustAndRound(const int8_t k, uint32_t exp, uint64_t frac64, bool nonZeroRemainder) const {
			uint32_t scale, regime, bits;
			if (k < 0) {
				scale = -k;
				regime = 0x4000'0000 >> scale;
			}
			else {
				scale = k + 1;
				regime = 0x7FFF'FFFF - (0x7FFF'FFFF >> scale);
			}

			if (scale > 30) {
				bits = k<0 ? 0x1 : 0x7FFF'FFFF;  // minpos and maxpos
			}
			else {
				//remove carry and rcarry bits and shift to correct position
				frac64 &= 0x3FFF'FFFF;
				uint32_t fraction = uint32_t(frac64) >> (scale + 2);

				bool bitNPlusOne = false;
				uint32_t moreBits = false;
				if (scale <= 28) {
					bitNPlusOne = bool (frac64 >> ((scale + 1)) & 0x1);
					exp <<= (28 - scale);
					if (bitNPlusOne) moreBits = (((1 << (scale + 1)) - 1) & frac64) ? 0x1 : 0x0;
				}
				else {
					if (scale == 30) {
						bitNPlusOne = bool(exp & 0x2);
						moreBits = exp & 0x1;
						exp = 0;
					}
					else if (scale == 29) {
						bitNPlusOne = bool(exp & 0x1);
						exp >>= 1;
					}
					if (frac64 > 0) {
						fraction = 0;
						moreBits = 0x1;
					}
				}
				if (nonZeroRemainder) moreBits = 0x1;
				bits = uint32_t(regime) + uint32_t(exp) + uint32_t(fraction);
				if (bitNPlusOne) bits += (bits & 0x1) | moreBits;
#define TRACE_DIV_
#ifdef TRACE_DIV
				std::cout << "universal\n";
				std::cout << "scale          = " << scale << std::endl;
				std::cout << std::hex;
				std::cout << "regime         = " << regime << std::endl;
				std::cout << "exponent       = " << exp << std::endl;
				std::cout << "fraction raw   = " << frac64 << std::endl;
				std::cout << "fraction final = " << fraction << std::endl;
				std::cout << "posit bits     = " << bits << std::endl;
				std::cout << std::dec;
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

	};

	// posit I/O operators
	// generate a posit format ASCII format nbits.esxNN...NNp
	inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_32, ES_IS_2>& p) {
		// to make certain that setw and left/right operators work properly
		// we need to transform the posit into a string
		std::stringstream ss;
#if POSIT_ROUNDING_ERROR_FREE_IO_FORMAT
		ss << NBITS_IS_32 << '.' << ES_IS_2 << 'x' << to_hex(p.get()) << 'p';
#else
		std::streamsize prec = ostr.precision();
		std::streamsize width = ostr.width();
		std::ios_base::fmtflags ff;
		ff = ostr.flags();
		ss.flags(ff);
		ss << std::showpos << std::setw(width) << std::setprecision(prec) << (long double)p;
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
	std::string to_string(const posit<NBITS_IS_32, ES_IS_2>& p, std::streamsize precision) {
		if (p.isnar()) {
			return std::string("nar");
		}
		std::stringstream ss;
		ss << std::setprecision(precision) << float(p);
		return ss.str();
	}

	// posit - posit binary logic operators
	inline bool operator==(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
		return lhs._bits == rhs._bits;
	}
	inline bool operator!=(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
		return !operator==(lhs, rhs);
	}
	inline bool operator< (const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
		return long(lhs._bits) < long(rhs._bits);
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

	inline posit<NBITS_IS_32, ES_IS_2> operator+(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
		posit<NBITS_IS_32, ES_IS_2> result = lhs;
		if (lhs.isneg() == rhs.isneg()) {  // are the posits the same sign?
			result += rhs;
		} 
		else {
			result -= rhs;
		}
		return result;
	}
	inline posit<NBITS_IS_32, ES_IS_2> operator-(const posit<NBITS_IS_32, ES_IS_2>& lhs, const posit<NBITS_IS_32, ES_IS_2>& rhs) {
		posit<NBITS_IS_32, ES_IS_2> result = lhs;
		if (lhs.isneg() == rhs.isneg()) {  // are the posits the same sign?
			result -= rhs.twosComplement();
		}
		else {
			result += rhs.twosComplement();
		}
		return result;

	}
	// binary operator*() is provided by generic class
	// binary operator/() is provided by generic class

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

#endif // POSIT_ENABLE_LITERALS

#else  // POSIT_FAST_POSIT_32_2
// too verbose #pragma message("Standard posit<32,2>")
#	define POSIT_FAST_POSIT_32_2 0
#endif // POSIT_FAST_POSIT_32_2

  }

}
