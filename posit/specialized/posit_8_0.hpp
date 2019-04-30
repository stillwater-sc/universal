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
#include "posit_8_0.h"

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
			posit& operator=(signed char rhs)             { return operator=((int)(rhs)); }
			posit& operator=(short rhs)                   { return operator=((int)(rhs)); }
			posit& operator=(int rhs)                     { return integer_assign(rhs); }
			posit& operator=(long rhs)                    { return operator=((int)(rhs)); }
			posit& operator=(long long rhs)               { return operator=((int)(rhs)); }
			posit& operator=(char rhs)                    { return operator=((int)(rhs)); }
			posit& operator=(unsigned short rhs)          { return operator=((int)(rhs)); }
			posit& operator=(unsigned int rhs)            { return operator=((int)(rhs)); }
			posit& operator=(unsigned long rhs)           { return operator=((int)(rhs)); }
			posit& operator=(unsigned long long rhs)      { return operator=((int)(rhs)); }
			posit& operator=(float rhs)                   { return float_assign(rhs); }
			posit& operator=(double rhs)                  { return operator=(float(rhs)); }
			posit& operator=(long double rhs)             { return operator=(float(rhs)); }

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
			inline bool isnar() const      { return (_bits == sign_mask); }
			inline bool iszero() const     { return (_bits == 0x00); }
			inline bool isone() const      { return (_bits == 0x40); } // pattern 010000...
			inline bool isminusone() const { return (_bits == 0xC0); } // pattern 110000...
			inline bool isneg() const      { return (_bits & sign_mask); }
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


			// helper method			
			posit& integer_assign(int rhs) {
				posit8_t p = posit8_fromsi(rhs);
				_bits = p.v;
				return *this;
			}
			posit& float_assign(float rhs) {
				posit8_t p = posit8_fromf(rhs);
				_bits = p.v;
				return *this;
			}

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
