#pragma once
// posit_2_0.hpp: specialized 2-bit posit using lookup table arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_2_0
#define POSIT_FAST_POSIT_2_0 0
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_2_0
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<2,0>")
#else
#pragma message "Fast specialization of posit<2,0>"
#endif

/*  values of a posit<2,0>
00 +0
01 +1
10 nar
11 -1
*/
constexpr float posit_2_0_values_lookup[4] = {
	0.0f, 1.0f, -float(INFINITY), -1.0f,
};

constexpr uint8_t posit_2_0_addition_lookup[16] = {
	0,1,2,3,  // 0   + {0,1,NaR,-1}
	1,1,2,0,  // 1   + {0,1,NaR,-1}
	2,2,2,2,  // NaR + {0,1,NaR,-1}
	3,0,2,3,  // -1  + {0,1,NaR,-1}
};

constexpr uint8_t posit_2_0_subtraction_lookup[16] = {
	0,3,2,1,  // 0   - {0,1,NaR,-1}
	1,0,2,1,  // 1   - {0,1,NaR,-1}
	2,2,2,2,  // NaR - {0,1,NaR,-1}
	3,3,2,0,  // -1  - {0,1,NaR,-1}
};

constexpr uint8_t posit_2_0_multiplication_lookup[16] = {
	0,0,2,0,  // 0   * {0,1,NaR,-1}
	0,1,2,3,  // 1   * {0,1,NaR,-1}
	2,2,2,2,  // NaR * {0,1,NaR,-1}
	0,3,2,1,  // -1  * {0,1,NaR,-1}
};

constexpr uint8_t posit_2_0_division_lookup[16] = {
	2,0,2,0,  // 0   / {0,1,NaR,-1}
	2,1,2,3,  // 1   / {0,1,NaR,-1}
	2,2,2,2,  // NaR / {0,1,NaR,-1}
	2,3,2,1,  // -1  / {0,1,NaR,-1}
};

constexpr uint8_t posit_2_0_reciprocal_lookup[4] = {
	2,1,2,3,
};

template<>
class posit<NBITS_IS_2, ES_IS_0> {
public:
	static constexpr unsigned nbits = NBITS_IS_2;
	static constexpr unsigned es = ES_IS_0;
	static constexpr unsigned sbits = 1;
	static constexpr unsigned rbits = nbits - sbits;
	static constexpr unsigned ebits = es;
	static constexpr unsigned fbits = 0;
	static constexpr unsigned fhbits = fbits + 1;
	static constexpr uint8_t index_shift = NBITS_IS_2;
	static constexpr uint8_t bit_mask = 0x3;  // last two bits
	static constexpr uint8_t nar_encoding = 0x02;
	static constexpr uint8_t one_encoding = 0x01;
	static constexpr uint8_t minus_one_encoding = 0x03;

	posit() { _bits = 0; }
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

	posit(int initial_value) { *this = (long long)initial_value; }
	posit(long int initial_value) { *this = (long long)initial_value; }
	posit(long long initial_value) { *this = initial_value; }
	posit(float initial_value) {
		*this = float_assign(initial_value);
	}
	posit(double initial_value) {
		*this = float_assign(initial_value);
	}
	posit(long double initial_value) {
		*this = float_assign(initial_value);
	}
	// assignment operators for native types
	posit& operator=(int rhs) {
		return operator=((long long)(rhs));
	}
	posit& operator=(long int rhs) {
		return operator=((long long)(rhs));
	}
	posit& operator=(long long rhs) {
		// only valid integers are -1, 0, 1
		_bits = 0x00;
		if (rhs <= -1) {
			_bits = 0x3;   // value is -1, or -maxpos
		}
		else if (rhs == 0) {
			_bits = 0x0;   // value is 0
		}
		else if (1 <= rhs) {
			_bits = 0x1;   // value is 1, or maxpos
		}
		return *this;
	}
	posit& operator=(const float rhs) {
		return float_assign(rhs);
	}
	posit& operator=(const double rhs) {
		return float_assign(rhs);
	}
	posit& operator=(const long double rhs) {
		return float_assign(rhs);
	}

	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator long() const { return to_long(); }
	explicit operator int() const { return to_int(); }
	explicit operator unsigned long long() const { return to_long_long(); }
	explicit operator unsigned long() const { return to_long(); }
	explicit operator unsigned int() const { return to_int(); }

	posit& setBitblock(sw::universal::bitblock<NBITS_IS_2>& raw) {
		_bits = uint8_t(raw.to_ulong() & bit_mask);
		return *this;
	}
	posit& setbits(uint64_t value) {
		_bits = uint8_t(value & bit_mask);
		return *this;
	}
	posit operator-() const {
		posit p;
		switch (_bits) {
		case 0x00:
			p.setbits(0x00);
			break;
		case 0x01:
			p.setbits(0x03);
			break;
		case 0x02:
			p.setbits(0x02);
			break;
		case 0x03:
			p.setbits(0x01);
			break;
		default:
			p.setbits(0x02);
		}
		return p;
	}
	posit& operator+=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_2_0_addition_lookup[index];
		return *this;
	}
	posit& operator-=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_2_0_subtraction_lookup[index];
		return *this;
	}
	posit& operator*=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_2_0_multiplication_lookup[index];
		return *this;
	}
	posit& operator/=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_2_0_division_lookup[index];
		return *this;
	}
	posit& operator++() {
		_bits = (_bits + 1) & 0x03;
		return *this;
	}
	posit operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit& operator--() {
		_bits = (_bits - 1) & 0x03;
		return *this;
	}
	posit operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}
	posit reciprocal() const {
		posit p;
		p.setbits(posit_2_0_reciprocal_lookup[_bits]);
		return p;
	}
				
	// SELECTORS
	inline bool sign() const   { return (_bits & 0x2u); }
	inline bool isnar() const  { return (_bits == nar_encoding); }
	inline bool iszero() const { return (_bits == 0); }
	inline bool isone() const { // pattern 010000....
		return (_bits == one_encoding);
	}
	inline bool isminusone() const { // pattern 110000...
		return (_bits == minus_one_encoding);
	}
	inline bool isneg() const      { return (_bits & 0x2u); }
	inline bool ispos() const      { return !isneg(); }
	inline bool ispowerof2() const { return !(_bits & 0x1u); }

	inline int sign_value() const { return (_bits & 0x2u ? -1 : 1); }

	bitblock<NBITS_IS_2> get() const { bitblock<NBITS_IS_2> bb; bb = int(_bits); return bb; }
	unsigned int bits() const { return (unsigned int)(_bits & bit_mask); }

	inline void clear()   { _bits = 0x00u; }
	inline void setzero() { _bits = 0x00u; }
	inline void setnar()  { _bits = nar_encoding; }
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

private:
	uint8_t _bits;

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
		return posit_2_0_values_lookup[bits()];
	}
	double      to_double() const {
		return (double)posit_2_0_values_lookup[bits()];
	}
	long double to_long_double() const {
		return (long double)posit_2_0_values_lookup[bits()];
	}

	template <typename T>
	posit& float_assign(const T& rhs) {
		constexpr int dfbits = std::numeric_limits<T>::digits - 1;
		internal::value<dfbits> v((T)rhs);

		// special case processing
		if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
			setnar();
			return *this;
		}

		if (rhs <= -0.25) {
			_bits = 0x03;   // value is -1, or -maxpos
		}
		else if (-0.25 < rhs && rhs < 0.25) {
			_bits = 0x00;   // value is 0
		}
		else if (rhs >= 0.25) {
			_bits = 0x01;   // value is 1, or maxpos
		}
		return *this;
	}

	// I/O operators
	friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_2, 0>& p);
	friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_2, 0>& p);

	// posit - posit logic functions
	friend bool operator==(const posit<NBITS_IS_2, 0>& lhs, const posit<NBITS_IS_2, 0>& rhs);
	friend bool operator!=(const posit<NBITS_IS_2, 0>& lhs, const posit<NBITS_IS_2, 0>& rhs);
	friend bool operator< (const posit<NBITS_IS_2, 0>& lhs, const posit<NBITS_IS_2, 0>& rhs);
	friend bool operator> (const posit<NBITS_IS_2, 0>& lhs, const posit<NBITS_IS_2, 0>& rhs);
	friend bool operator<=(const posit<NBITS_IS_2, 0>& lhs, const posit<NBITS_IS_2, 0>& rhs);
	friend bool operator>=(const posit<NBITS_IS_2, 0>& lhs, const posit<NBITS_IS_2, 0>& rhs);

};

			// posit I/O operators
			inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_2, ES_IS_0>& p) {
				// to make certain that setw and left/right operators work properly
				// we need to transform the posit into a string
				std::stringstream ss;
#if POSIT_ERROR_FREE_IO_FORMAT
				ss << NBITS_IS_2 << '.' << ES_IS_0 << 'x' << to_hex(p.get()) << 'p';
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

			// convert a posit value to a string using "nar" as designation of NaR
			inline std::string to_string(const posit<NBITS_IS_2, ES_IS_0>& p, std::streamsize precision) {
				if (p.isnar()) {
					return std::string("nar");
				}
				std::stringstream ss;
				ss << std::setprecision(precision) << float(p);
				return ss.str();
			}

			// posit - posit binary logic operators
			inline bool operator==(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return lhs._bits == rhs._bits;
			}
			inline bool operator!=(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return !operator==(lhs, rhs);
			}
			inline bool operator< (const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				bool posit_2_0_less_than_lookup[16] = {
					false, true, false, false,
					false, false, false, false,
					true, true, false, true,
					true, true, false, false,
				};
				uint16_t index = (uint16_t(lhs.bits()) << NBITS_IS_2) | uint16_t(rhs.bits());
				return posit_2_0_less_than_lookup[index];
			}
			inline bool operator< (int lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return posit<NBITS_IS_2, ES_IS_0>(lhs) < rhs;
			}
			inline bool operator< (const posit<NBITS_IS_2, ES_IS_0>& lhs, int rhs) {
				return lhs < posit<NBITS_IS_2, ES_IS_0>(rhs);
			}
			inline bool operator< (float lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return posit<NBITS_IS_2, ES_IS_0>(lhs) < rhs;
			}
			inline bool operator< (const posit<NBITS_IS_2, ES_IS_0>& lhs, float rhs) {
				return lhs < posit<NBITS_IS_2, ES_IS_0>(rhs);
			}
			inline bool operator< (double lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return posit<NBITS_IS_2, ES_IS_0>(lhs) < rhs;
			}
			inline bool operator< (const posit<NBITS_IS_2, ES_IS_0>& lhs, double rhs) {
				return lhs < posit<NBITS_IS_2, ES_IS_0>(rhs);
			}
			inline bool operator> (const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return operator< (rhs, lhs);
			}
			inline bool operator<=(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return operator< (lhs, rhs) || operator==(lhs, rhs);
			}
			inline bool operator>=(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				return !operator< (lhs, rhs);
			}

			inline posit<NBITS_IS_2, ES_IS_0> operator+(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				posit<NBITS_IS_2, ES_IS_0> sum = lhs;
				sum += rhs;
				return sum;
			}
			inline posit<NBITS_IS_2, ES_IS_0> operator-(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				posit<NBITS_IS_2, ES_IS_0> sub = lhs;
				sub -= rhs;
				return sub;
			}
			inline posit<NBITS_IS_2, ES_IS_0> operator*(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				posit<NBITS_IS_2, ES_IS_0> mul = lhs;
				mul *= rhs;
				return mul;
			}
			inline posit<NBITS_IS_2, ES_IS_0> operator/(const posit<NBITS_IS_2, ES_IS_0>& lhs, const posit<NBITS_IS_2, ES_IS_0>& rhs) {
				posit<NBITS_IS_2, ES_IS_0> div = lhs;
				div /= rhs;
				return div;
			}

#else  // POSIT_FAST_POSIT_2_0
// too verbose #pragma message("Standard posit<2,0>")
#	define POSIT_FAST_POSIT_2_0 0
#endif // POSIT_FAST_POSIT_2_0
	
}} // namespace sw::universal
