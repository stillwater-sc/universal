#pragma once
// posit_8_0.hpp: specialized 8-bit posit using fast compute specialized for posit<8,0>
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_8_0
#define POSIT_FAST_POSIT_8_0 0
#endif

namespace sw { namespace unum {

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

		constexpr posit() : _bits(0) {}
		posit(const posit&) = default;
		posit(posit&&) = default;
		posit& operator=(const posit&) = default;
		posit& operator=(posit&&) = default;

		// initializers for native types
		constexpr explicit posit(signed char initial_value) : _bits(0)        { *this = initial_value; }
		constexpr explicit posit(short initial_value) : _bits(0)              { *this = initial_value; }
		constexpr explicit posit(int initial_value) : _bits(0)                { *this = initial_value; }
		constexpr explicit posit(long initial_value) : _bits(0)               { *this = initial_value; }
		constexpr explicit posit(long long initial_value) : _bits(0)          { *this = initial_value; }
		constexpr explicit posit(char initial_value) : _bits(0)               { *this = initial_value; }
		constexpr explicit posit(unsigned short initial_value) : _bits(0)     { *this = initial_value; }
		constexpr explicit posit(unsigned int initial_value) : _bits(0)       { *this = initial_value; }
		constexpr explicit posit(unsigned long initial_value) : _bits(0)      { *this = initial_value; }
		constexpr explicit posit(unsigned long long initial_value) : _bits(0) { *this = initial_value; }
		constexpr explicit posit(float initial_value) : _bits(0)              { *this = initial_value; }
		constexpr          posit(double initial_value) : _bits(0)             { *this = initial_value; }
		constexpr explicit posit(long double initial_value) : _bits(0)        { *this = initial_value; }

		// assignment operators for native types
		constexpr posit& operator=(signed char rhs)             { return operator=((int)(rhs)); }
		constexpr posit& operator=(short rhs)                   { return operator=((int)(rhs)); }
		constexpr posit& operator=(int rhs)                     { return integer_assign(rhs); }
		constexpr posit& operator=(long rhs)                    { return operator=((int)(rhs)); }
		constexpr posit& operator=(long long rhs)               { return operator=((int)(rhs)); }
		constexpr posit& operator=(char rhs)                    { return operator=((int)(rhs)); }
		constexpr posit& operator=(unsigned short rhs)          { return operator=((int)(rhs)); }
		constexpr posit& operator=(unsigned int rhs)            { return operator=((int)(rhs)); }
		constexpr posit& operator=(unsigned long rhs)           { return operator=((int)(rhs)); }
		constexpr posit& operator=(unsigned long long rhs)      { return operator=((int)(rhs)); }
		constexpr posit& operator=(float rhs)                   { return float_assign(rhs); }
		constexpr posit& operator=(double rhs)                  { return float_assign(float(rhs)); }
		constexpr posit& operator=(long double rhs)             { return float_assign(float(rhs)); }

		explicit operator long double() const { return to_long_double(); }
		explicit operator double() const { return to_double(); }
		explicit operator float() const { return to_float(); }
		explicit operator long long() const { return to_long_long(); }
		explicit operator long() const { return to_long(); }
		explicit operator int() const { return to_int(); }
		explicit operator unsigned long long() const { return to_long_long(); }
		explicit operator unsigned long() const { return to_long(); }
		explicit operator unsigned int() const { return to_int(); }

		posit& set(const sw::unum::bitblock<NBITS_IS_8>& raw) {
			_bits = uint8_t(raw.to_ulong());
			return *this;
		}
		constexpr posit& set_raw_bits(uint64_t value) {
			_bits = uint8_t(value & 0xff);
			return *this;
		}
		constexpr posit operator-() const {
			posit p;
			return p.set_raw_bits((~_bits) + 1);
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


		// helper methods			
		constexpr posit& integer_assign(int rhs) {
			// special case for speed as this is a common initialization
			if (rhs == 0) {
				_bits = 0x0;
				return *this;
			}
			bool sign = (rhs < 0) ? true : false;
			int v = sign ? -rhs : rhs; // project to positive side of the projective reals
			uint8_t raw = 0;
			if (v > 48) { // +-maxpos
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
		constexpr posit& float_assign(float rhs) {
			bool sign = false;
			bool bitNPlusOne = 0, bitsMore = 0;
			constexpr float _minpos = 0.015625f;
			constexpr float _maxpos = 64.0f;

			sign = (rhs < 0.0) ? true : false;

			constexpr int spfbits = std::numeric_limits<float>::digits - 1;
			value<spfbits> v(rhs);
			if (v.isinf() || v.isnan()) {
				_bits = 0x80;
			}
			else if (rhs == 0) {
				_bits = 0;
			}
			else if (rhs == 1.0f) {
				_bits = 0x40;
			}
			else if (rhs == -1.0f) {
				_bits = 0xC0;
			}
			else if (rhs >= _maxpos) {
				_bits = 0x7F;
			}
			else if (rhs <= -_maxpos) {
				_bits = 0x81;
			}
			else if (rhs <= _minpos && !sign) {
				_bits = 0x01;
			}
			else if (rhs >= -_minpos && sign) {
				_bits = 0xFF;
			}
			else if (rhs < -1 || rhs > 1) {
				if (sign) {
					rhs = -rhs; // project to positive reals to simplify computation
				}

				if (rhs <= _minpos) {
					_bits = 0x01;
				}
				else { // determine the regime	
					unsigned k = 1; //because k = m-1, we need to add back 1
					while (rhs >= 2) {
						rhs *= 0.5;
						k++;
					}

					// rounding off regime bits
					if (k > 6) {
						_bits = 0x7F;
					}
					else {
						int8_t fracLength = 6 - k;
						uint8_t frac = (uint8_t)posit8_convertFraction(rhs, fracLength, &bitNPlusOne, &bitsMore);
						uint_fast8_t regime = 0x7F - (0x7F >> k);
						_bits = (regime + frac);
						if (bitNPlusOne) _bits += ((_bits & 0x01) | bitsMore);
					}
					_bits = sign ? -_bits : _bits;
				}
			}
			else if (rhs > -1 && rhs < 1) {
				if (sign) {
					rhs = -rhs;
				}
				unsigned k = 0;
				while (rhs < 1) {
					rhs *= 2;
					k++;
				}
				// rounding off regime bits
				if (k > 6)
					_bits = 0x1;
				else {
					int8_t fracLength = 6 - k;
					uint8_t frac = (uint8_t)posit8_convertFraction(rhs, fracLength, &bitNPlusOne, &bitsMore);
					uint8_t regime = 0x40 >> k;
					_bits = (regime + frac);
					if (bitNPlusOne) _bits += ((_bits & 0x01) | bitsMore);
				}
				_bits = sign ? -_bits : _bits;
			}
			else {
				//NaR - for NaN, INF and all other combinations
				_bits = 0x80;
			}
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
		ss << std::setw(width) << std::setprecision(prec) << to_string(p, prec);  // TODO: we need a true native serialization function
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
		return int8_t(lhs._bits) < int8_t(rhs._bits);
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

#endif // POSIT_FAST_POSIT_8_0

}} // namespace sw::unum
