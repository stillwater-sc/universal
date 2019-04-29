#pragma once
// posit_8_0.hpp: specialized 8-bit posit using fast compute specialized for posit<8,0>
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_8_0
#pragma message("Fast specialization of posit<8,0>")

		// injecting the C API into namespace sw::unum
#include "posit8_t.h"

		template<>
		class posit<NBITS_IS_8, ES_IS_0> {
		public:
			static constexpr size_t nbits = NBITS_IS_8;
			static constexpr size_t es = ES_IS_0;
			static constexpr size_t sbits = 1;
			static constexpr size_t rbits = nbits - sbits;
			static constexpr size_t ebits = es;
			static constexpr size_t fbits = nbits - 3 - es;
			static constexpr size_t fhbits = fbits + 1;
			static constexpr uint8_t sign_mask = 0x80;

			posit() { _bits = 0; }
			posit(const posit&) = default;
			posit(posit&&) = default;
			posit& operator=(const posit&) = default;
			posit& operator=(posit&&) = default;

			// initializers for native types
			posit(const signed char initial_value)        { *this = initial_value; }
			posit(const short initial_value)              { *this = initial_value; }
			posit(const int initial_value)                { *this = initial_value; }
			posit(const long initial_value)               { *this = initial_value; }
			posit(const long long initial_value)          { *this = initial_value; }
			posit(const char initial_value)               { *this = initial_value; }
			posit(const unsigned short initial_value)     { *this = initial_value; }
			posit(const unsigned int initial_value)       { *this = initial_value; }
			posit(const unsigned long initial_value)      { *this = initial_value; }
			posit(const unsigned long long initial_value) { *this = initial_value; }
			posit(const float initial_value)              { *this = initial_value; }
			posit(const double initial_value)             { *this = initial_value; }
			posit(const long double initial_value)        { *this = initial_value; }

			// assignment operators for native types
			posit& operator=(const signed char rhs)       { 
				// special case for speed as this is a common initialization
				if (rhs == 0) {
					_bits = 0x00;
					return *this;
				}
				if (rhs == -128) {
					// 0x80 is special in int8 arithmetic as it is its own negation
					_bits = 0x80; // NaR
					return *this;
				}
				bool sign = bool(rhs & sign_mask);
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
				_bits = sign ? -raw : raw;
				return *this;
			}
			posit& operator=(const short rhs)             { return operator=((signed char)(rhs)); }
			posit& operator=(const int rhs)               { return operator=((signed char)(rhs)); }
			posit& operator=(const long rhs)              { return operator=((signed char)(rhs)); }
			posit& operator=(const long long rhs)         { return operator=((signed char)(rhs)); }
			posit& operator=(const char rhs)              { return operator=((signed char)(rhs)); }
			posit& operator=(const unsigned short rhs)    { return operator=((signed char)(rhs)); }
			posit& operator=(const unsigned int rhs)      { return operator=((signed char)(rhs)); }
			posit& operator=(const unsigned long rhs)     { return operator=((signed char)(rhs)); }
			posit& operator=(const unsigned long long rhs){ return operator=((signed char)(rhs)); }
			posit& operator=(const float rhs)             { return float_assign(rhs); }
			posit& operator=(const double rhs)            { return float_assign(float(rhs)); }
			posit& operator=(const long double rhs)       { return float_assign(float(rhs)); }

			explicit operator long double() const { return to_long_double(); }
			explicit operator double() const { return to_double(); }
			explicit operator float() const { return to_float(); }
			explicit operator long long() const { return to_long_long(); }
			explicit operator long() const { return to_long(); }
			explicit operator int() const { return to_int(); }
			explicit operator unsigned long long() const { return to_long_long(); }
			explicit operator unsigned long() const { return to_long(); }
			explicit operator unsigned int() const { return to_int(); }

			posit& set(sw::unum::bitblock<NBITS_IS_8>& raw) {
				_bits = uint8_t(raw.to_ulong());
				return *this;
			}
			posit& set_raw_bits(uint64_t value) {
				_bits = uint8_t(value & 0xff);
				return *this;
			}
			posit operator-() const {
				posit negated;
				posit8_t b = { { _bits } };
				return negated.set_raw_bits(posit8_negate(b).v);
			}
			posit& operator+=(const posit& b) {
				posit8_t lhs = { { _bits } };
				posit8_t rhs = { { b._bits} };
				posit8_t add = posit8_addp8(lhs, rhs);
				_bits = add.v;
				return *this;
			}
			posit& operator-=(const posit& b) {
				posit8_t lhs = { { _bits } };
				posit8_t rhs = { { b._bits } };
				posit8_t sub = posit8_subp8(lhs, rhs);
				_bits = sub.v;
				return *this;
			}
			posit& operator*=(const posit& b) {
				posit8_t lhs = { { _bits } };
				posit8_t rhs = { { b._bits } };
				posit8_t mul = posit8_mulp8(lhs, rhs);
				_bits = mul.v;
				return *this;
			}
			posit& operator/=(const posit& b) {
				posit8_t lhs = { { _bits } };
				posit8_t rhs = { { b._bits } };
				posit8_t div = posit8_divp8(lhs, rhs);
				_bits = div.v;
				return *this;
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
			inline bool isnar() const      { return (_bits == 0x80); }
			inline bool iszero() const     { return (_bits == 0x00); }
			inline bool isone() const      { return (_bits == 0x40); } // pattern 010000...
			inline bool isminusone() const { return (_bits == 0xC0); } // pattern 110000...
			inline bool isneg() const      { return (_bits & 0x80); }
			inline bool ispos() const      { return !isneg(); }
			inline bool ispowerof2() const { return !(_bits & 0x1); }

			inline int sign_value() const  { return (_bits & 0x80 ? -1 : 1); }

			bitblock<NBITS_IS_8> get() const { bitblock<NBITS_IS_8> bb; bb = int(_bits); return bb; }
			unsigned long long encoding() const { return (unsigned long long)(_bits); }

			inline void clear() { _bits = 0; }
			inline void setzero() { clear(); }
			inline void setnar() { _bits = 0x80; }
			inline posit twosComplement() const {
				posit<NBITS_IS_8, ES_IS_0> p;
				int8_t v = -*(int8_t*)&_bits;
				p.set_raw_bits(v);
				return p;
			}
		private:
			uint8_t _bits;

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
				posit8_t p = { { _bits } };
				return posit8_tof(p);
			}
			double      to_double() const {
				return (double)to_float();
			}
			long double to_long_double() const {
				return (long double)to_float();
			}

			posit& float_assign(float rhs) {
				posit8_t p = posit8_fromf(rhs);
				_bits = p.v;
				return *this;
				/*
				constexpr int dfbits = std::numeric_limits<T>::digits - 1;
				value<dfbits> v((T)rhs);

				// special case processing
				if (v.iszero()) {
					setzero();
					return *this;
				}
				if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
					setnar();
					return *this;
				}

				bitblock<NBITS_IS_8> ptt;
				convert_to_bb<NBITS_IS_8, ES_IS_0, dfbits>(v.sign(), v.scale(), v.fraction(), ptt); // TODO: needs to be faster
				_bits = uint8_t(ptt.to_ulong());
				return *this;
				*/
			}

			// helper method
			/*
			// decode_regime takes the raw bits of the posit, and returns the regime run-length, m, and the remaining fraction bits in remainder
			inline void decode_regime(const uint8_t bits, int8_t& m, uint8_t& remaining) const {
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
			inline void extractAddand(const uint8_t bits, int8_t& m, uint8_t& remaining) const {
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
			inline void extractMultiplicand(const uint8_t bits, int8_t& m, uint8_t& remaining) const {
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
			inline void extractDividand(const uint8_t bits, int8_t& m, uint8_t& remaining) const {
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
			inline uint8_t round(const int8_t m, uint16_t fraction) const {
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
			inline uint8_t adjustAndRound(const int8_t k, uint16_t fraction, bool nonZeroRemainder) const {
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
#ifdef NOW
					std::cout << std::hex;
					std::cout << "fraction raw   = " << int(fraction) << std::endl;
					std::cout << "fraction final = " << int(final_fbits) << std::endl;
					std::cout << "posit bits     = " << int(bits) << std::endl;
					std::cout << std::dec;
#endif
					if (bitNPlusOne) {
						uint8_t moreBits = (((1 << scale) - 1) & fraction) ? 0x01 : 0x00;
						if (nonZeroRemainder) moreBits = 0x01;
						//std::cout << "bitsMore = " << (moreBits ? "true" : "false") << std::endl;
						// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
						bits += (bits & 0x01) | moreBits;
					}
				}
				return bits;
			}
			*/

			// I/O operators
			friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_8, ES_IS_0>& p);
			friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_8, ES_IS_0>& p);

			// posit - posit logic functions
			friend bool operator==(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs);
			friend bool operator!=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs);
			friend bool operator< (const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs);
			friend bool operator> (const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs);
			friend bool operator<=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs);
			friend bool operator>=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs);

		};

		// posit I/O operators
		// generate a posit format ASCII format nbits.esxNN...NNp
		inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_8, ES_IS_0>& p) {
			// to make certain that setw and left/right operators work properly
			// we need to transform the posit into a string
			std::stringstream ss;
#if POSIT_ROUNDING_ERROR_FREE_IO_FORMAT
			ss << NBITS_IS_8 << '.' << ES_IS_0 << 'x' << to_hex(p.get()) << 'p';
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
		inline std::istream& operator>> (std::istream& istr, posit<NBITS_IS_8, ES_IS_0>& p) {
			std::string txt;
			istr >> txt;
			if (!parse(txt, p)) {
				std::cerr << "unable to parse -" << txt << "- into a posit value\n";
			}
			return istr;
		}

		// convert a posit value to a string using "nar" as designation of NaR
		std::string to_string(const posit<NBITS_IS_8, ES_IS_0>& p, std::streamsize precision) {
			if (p.isnar()) {
				return std::string("nar");
			}
			std::stringstream ss;
			ss << std::setprecision(precision) << float(p);
			return ss.str();
		}

		// posit - posit binary logic operators
		inline bool operator==(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return lhs._bits == rhs._bits;
		}
		inline bool operator!=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return !operator==(lhs, rhs);
		}
		inline bool operator< (const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return (signed char)(lhs._bits) < (signed char)(rhs._bits);
		}
		inline bool operator> (const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return operator< (rhs, lhs);
		}
		inline bool operator<=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return operator< (lhs, rhs) || operator==(lhs, rhs);
		}
		inline bool operator>=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return !operator< (lhs, rhs);
		}

		/* base class has these operators: no need to specialize */
		inline posit<NBITS_IS_8, ES_IS_0> operator+(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {				
			posit<NBITS_IS_8, ES_IS_0> result = lhs;
			return result += rhs;
		}
		inline posit<NBITS_IS_8, ES_IS_0> operator-(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			posit<NBITS_IS_8, ES_IS_0> result = lhs;
			return result -= rhs;

		}
			
		// binary operator*() is provided by generic class

#if POSIT_ENABLE_LITERALS
		// posit - literal logic functions

		// posit - int logic operators
		inline bool operator==(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
			return operator==(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
		}
		inline bool operator!=(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
			return !operator==(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
		}
		inline bool operator< (const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
			return operator<(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
		}
		inline bool operator> (const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
			return operator< (posit<NBITS_IS_8, ES_IS_0>(rhs), lhs);
		}
		inline bool operator<=(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
			return operator< (lhs, posit<NBITS_IS_8, ES_IS_0>(rhs)) || operator==(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
		}
		inline bool operator>=(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
			return !operator<(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
		}

		// int - posit logic operators
		inline bool operator==(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return posit<NBITS_IS_8, ES_IS_0>(lhs) == rhs;
		}
		inline bool operator!=(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return !operator==(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
		}
		inline bool operator< (int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return operator<(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
		}
		inline bool operator> (int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return operator< (posit<NBITS_IS_8, ES_IS_0>(rhs), lhs);
		}
		inline bool operator<=(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return operator< (posit<NBITS_IS_8, ES_IS_0>(lhs), rhs) || operator==(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
		}
		inline bool operator>=(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
			return !operator<(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
		}

#endif // POSIT_ENABLE_LITERALS

#else  // POSIT_FAST_POSIT_8_0
// too verbose #pragma message("Standard posit<8,0>")
#	define POSIT_FAST_POSIT_8_0 0
#endif // POSIT_FAST_POSIT_8_0

	}
}
