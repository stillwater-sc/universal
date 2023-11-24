#pragma once
// posit_8_2.hpp: specialized 8-bit posit using fast implementation specialized for posit<8,2>
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/number/posit/posit.hpp>

#include <universal/native/integers.hpp>

#ifndef POSIT_FAST_POSIT_8_2
#define POSIT_FAST_POSIT_8_2 0
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_8_2
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<8,2>")
//#else   some compile time message that indicates that we are using a specialization for non MS compilers
//#warning("Fast specialization of posit<8,2>")
#endif

// fast specialized posit<8,2>
template<>
class posit<NBITS_IS_8, ES_IS_2> {
public:
	static constexpr unsigned nbits = NBITS_IS_8;
	static constexpr unsigned es = ES_IS_2;
	static constexpr unsigned sbits = 1;
	static constexpr unsigned rbits = nbits - sbits;
	static constexpr unsigned ebits = es;
	static constexpr unsigned fbits = nbits - 3 - es;
	static constexpr unsigned fhbits = fbits + 1;
	static constexpr uint8_t sign_mask = 0x80;

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
	constexpr explicit posit(signed char initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(short initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(int initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(long initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(long long initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(char initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(unsigned short initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(unsigned int initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(unsigned long initial_value) : _bits(0) { *this = initial_value; }
	constexpr explicit posit(unsigned long long initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(float initial_value) : _bits(0) { *this = initial_value; }
	posit(double initial_value) : _bits(0) { *this = initial_value; }
	explicit posit(long double initial_value) : _bits(0) { *this = initial_value; }

	// assignment operators for native types
	constexpr posit& operator=(signed char rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(short rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(int rhs) { return integer_assign(rhs); }
	constexpr posit& operator=(long rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(long long rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(char rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(unsigned short rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(unsigned int rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(unsigned long rhs) { return operator=((int)(rhs)); }
	constexpr posit& operator=(unsigned long long rhs) { return operator=((int)(rhs)); }
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

	posit& setBitblock(const sw::universal::bitblock<NBITS_IS_8>& raw) {
		_bits = uint8_t(raw.to_ulong());
		return *this;
	}
	constexpr posit& setbits(uint64_t value) {
		_bits = uint8_t(value & 0xffu);
		return *this;
	}
	constexpr posit operator-() const {
		posit p;
		return p.setbits((~_bits) + 1ul);
	}
	// arithmetic assignment operators
	posit& operator+=(const posit& b) {

		return *this;
	}
	posit& operator-=(const posit& b) {

		return *this;
	}
	posit& operator*=(const posit& b) {

		return *this;
	}
	posit& operator/=(const posit& b) {

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

	// Selelctors
	inline bool sign() const { return (_bits & sign_mask); }
	inline bool isnar() const { return (_bits == sign_mask); }
	inline bool iszero() const { return (_bits == 0x00); }
	inline bool isone() const { return (_bits == 0x40); } // pattern 010000...
	inline bool isminusone() const { return (_bits == 0xC0); } // pattern 110000...
	inline bool isneg() const { return (_bits & sign_mask); }
	inline bool ispos() const { return !isneg(); }
	inline bool ispowerof2() const { return !(_bits & 0x1); }

	inline int sign_value() const { return (_bits & sign_mask ? -1 : 1); }

	bitblock<NBITS_IS_8> get() const { bitblock<NBITS_IS_8> bb; bb = int(_bits); return bb; }
	unsigned long long encoding() const { return (unsigned long long)(_bits); }

	// Modifiers
	inline void clear() { _bits = 0; }
	inline void setzero() { clear(); }
	inline void setnar() { _bits = 0x80; }
	inline posit& minpos() {
		clear();
		return ++(*this);
	}
	inline posit& maxpos() {
		setnar();
		return --(*this);
	}
	inline posit& zero() {
		clear();
		return *this;
	}
	inline posit& minneg() {
		clear();
		return --(*this);
	}
	inline posit& maxneg() {
		setnar();
		return ++(*this);
	}
	inline posit twosComplement() const {
		posit<NBITS_IS_8, ES_IS_2> p;
		int8_t v = -*(int8_t*)&_bits;
		p.setbits(v);
		return p;
	}

private:
	uint8_t _bits;

	// decode takes the raw bits of the posit, 
	// returns the regime, m, and leaves the remaining bits in 'remainder'
	int8_t decode_regime(uint8_t bits, uint8_t* remaining) const {
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
		uint8_t ebits{ 0 };
		switch (m) {
		case -5: case 4:
			ebits = (*remaining >> 5);
			*remaining <<= 1;
			break;
		case -7: case -6: case 5: case 6:
			ebits = 0;
			*remaining = 0;
		default:
			ebits = (*remaining >> 5);
			*remaining <<= 2;
			break;
		}
		return ebits;
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
		if (isnar()) return NAN;   //  INFINITY is not semantically correct. NaR is Not a Real and thus is more closely related to a NAN, or Not a Number

		uint8_t bits = ((_bits & 0x80) ? -_bits : _bits);	
		uint8_t remaining = 0;
		int8_t m = decode_regime(bits, &remaining);
//		std::cout << to_binary(bits, 8) << " : " << to_binary(remaining, 8) << " : ";
		int regimeScale = (1 << es) * m;
		float s = (float)(sign_value());
		float r = (m > 0 ? (float)(1 << regimeScale) : (1.0f / (float)(1 << -regimeScale)));
		uint8_t ebits = extract_exponent(m, &remaining);
//		std::cout << to_binary(ebits, 2) << " : " << to_binary(remaining, 8) << '\n';
		float e = float((uint32_t(1) << ebits));
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
	posit& float_assign(float rhs) {
		bool sign = false;
		bool bitNPlusOne = 0, bitsMore = 0;
		constexpr float _minpos = 5.9604644775390625e-08f;
		constexpr float _maxpos = 16777216.0f;

		sign = (rhs < 0.0) ? true : false;

		constexpr int spfbits = std::numeric_limits<float>::digits - 1;
		internal::value<spfbits> v(rhs);
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
					uint8_t frac = (uint8_t)convertFraction(rhs, fracLength, &bitNPlusOne, &bitsMore);
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
				uint8_t frac = (uint8_t)convertFraction(rhs, fracLength, &bitNPlusOne, &bitsMore);
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
