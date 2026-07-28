#pragma once
// posit_4_0.hpp: specialized 4-bit posit using lookup table arithmetic
//
// Copyright (C) 2017 Supercomputing, Inc.
// SPDX - License - Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_4_0
#define POSIT_FAST_POSIT_4_0 0
#endif

#include <universal/utility/directives.hpp>

namespace sw { namespace universal {

		// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_4_0
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<4,0>")

constexpr uint8_t posit_4_0_addition_lookup[256] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
	1,2,3,4,4,6,6,7,8,9,10,12,13,14,15,0,
	2,3,4,4,5,6,6,7,8,9,11,12,14,15,0,1,
	3,4,4,5,6,6,6,7,8,9,12,13,15,0,1,2,
	4,4,5,6,6,6,6,7,8,10,12,14,0,1,2,3,
	5,6,6,6,6,6,7,7,8,10,14,0,2,3,4,4,
	6,6,6,6,6,7,7,7,8,10,0,2,4,4,5,6,
	7,7,7,7,7,7,7,7,8,0,6,6,6,7,7,7,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	9,9,9,9,10,10,10,0,8,9,9,9,9,9,9,9,
	10,10,11,12,12,14,0,6,8,9,9,9,10,10,10,10,
	11,12,12,13,14,0,2,6,8,9,9,10,10,10,10,10,
	12,13,14,15,0,2,4,6,8,9,10,10,10,10,11,12,
	13,14,15,0,1,3,4,7,8,9,10,10,10,11,12,12,
	14,15,0,1,2,4,5,7,8,9,10,10,11,12,12,13,
	15,0,1,2,3,4,6,7,8,9,10,10,12,12,13,14,
};

constexpr uint8_t posit_4_0_subtraction_lookup[256] = {
	0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,
	1,0,15,14,13,12,10,9,8,7,6,6,4,4,3,2,
	2,1,0,15,14,12,11,9,8,7,6,6,5,4,4,3,
	3,2,1,0,15,13,12,9,8,7,6,6,6,5,4,4,
	4,3,2,1,0,14,12,10,8,7,6,6,6,6,5,4,
	5,4,4,3,2,0,14,10,8,7,7,6,6,6,6,6,
	6,6,5,4,4,2,0,10,8,7,7,7,6,6,6,6,
	7,7,7,7,6,6,6,0,8,7,7,7,7,7,7,7,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	9,9,9,9,9,9,9,9,8,0,10,10,10,9,9,9,
	10,10,10,10,10,9,9,9,8,6,0,14,12,12,11,10,
	11,10,10,10,10,10,9,9,8,6,2,0,14,13,12,12,
	12,12,11,10,10,10,10,9,8,6,4,2,0,15,14,13,
	13,12,12,11,10,10,10,9,8,7,4,3,1,0,15,14,
	14,13,12,12,11,10,10,9,8,7,5,4,2,1,0,15,
	15,14,13,12,12,10,10,9,8,7,6,4,3,2,1,0,
};

constexpr uint8_t posit_4_0_multiplication_lookup[256] = {
	0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,
	0,1,1,1,1,2,2,4,8,12,14,14,15,15,15,15,
	0,1,1,2,2,3,4,6,8,10,12,13,14,14,15,15,
	0,1,2,2,3,4,5,6,8,10,11,12,13,14,14,15,
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
	0,2,3,4,5,6,6,7,8,9,10,10,11,12,13,14,
	0,2,4,5,6,6,7,7,8,9,9,10,10,11,12,14,
	0,4,6,6,7,7,7,7,8,9,9,9,9,10,10,12,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	0,12,10,10,9,9,9,9,8,7,7,7,7,6,6,4,
	0,14,12,11,10,10,9,9,8,7,7,6,6,5,4,2,
	0,14,13,12,11,10,10,9,8,7,6,6,5,4,3,2,
	0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,
	0,15,14,14,13,12,11,10,8,6,5,4,3,2,2,1,
	0,15,15,14,14,13,12,10,8,6,4,3,2,2,1,1,
	0,15,15,15,15,14,14,12,8,4,2,2,1,1,1,1,
};

constexpr uint8_t posit_4_0_division_lookup[256] = {
	8,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,
	8,4,2,1,1,1,1,1,8,15,15,15,15,15,14,12,
	8,6,4,3,2,1,1,1,8,15,15,15,14,13,12,10,
	8,6,5,4,3,2,2,1,8,15,14,14,13,12,11,10,
	8,7,6,5,4,3,2,1,8,15,14,13,12,11,10,9,
	8,7,6,6,5,4,3,2,8,14,13,12,11,10,10,9,
	8,7,7,6,6,5,4,2,8,14,12,11,10,10,9,9,
	8,7,7,7,7,6,6,4,8,12,10,10,9,9,9,9,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	8,9,9,9,9,10,10,12,8,4,6,6,7,7,7,7,
	8,9,9,10,10,11,12,14,8,2,4,5,6,6,7,7,
	8,9,10,10,11,12,13,14,8,2,3,4,5,6,6,7,
	8,9,10,11,12,13,14,15,8,1,2,3,4,5,6,7,
	8,10,11,12,13,14,14,15,8,1,2,2,3,4,5,6,
	8,10,12,13,14,15,15,15,8,1,1,1,2,3,4,6,
	8,12,14,15,15,15,15,15,8,1,1,1,1,1,2,4,
};

constexpr uint8_t posit_4_0_reciprocal_lookup[16] = {
	8,7,6,5,4,3,2,1,8,15,14,13,12,11,10,9,
};

template<>
class posit<NBITS_IS_4, ES_IS_0> {
public:
	static constexpr unsigned nbits = NBITS_IS_4;
	static constexpr unsigned es = ES_IS_0;
	static constexpr unsigned sbits = 1;
	static constexpr unsigned rbits = nbits - sbits;
	static constexpr unsigned ebits = es;
	static constexpr unsigned fbits = nbits - 3;
	static constexpr unsigned fhbits = fbits + 1;
	static constexpr uint8_t index_shift = 4;
	static constexpr uint8_t bit_mask = 0x0F;
	static constexpr uint8_t nar_encoding = 0x08;
	static constexpr uint8_t one_encoding = 0x04; // 0100
	static constexpr uint8_t minusone_encoding = 0x0C; // 1100

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

	explicit posit(signed char initial_value) { *this = (long long)initial_value; }
	explicit posit(short initial_value) { *this = (long long)initial_value; }
	explicit posit(int initial_value) { *this = (long long)initial_value; }
	explicit posit(long int initial_value) { *this = (long long)initial_value; }
				posit(long long initial_value) { *this = (long long)initial_value; }
	explicit posit(char initial_value) { *this = (long long)initial_value; }
	explicit posit(unsigned short initial_value) { *this = (long long)initial_value; }
	explicit posit(unsigned int initial_value) { *this = (long long)initial_value; }
	explicit posit(unsigned long int initial_value) { *this = (long long)initial_value; }
	explicit posit(unsigned long long initial_value) { *this = (long long)initial_value; }
	explicit posit(float initial_value) { *this = initial_value; }
	explicit posit(double initial_value) { *this = initial_value; }
	explicit posit(long double initial_value) { *this = initial_value; }

	// assignment operators for native types
	posit& operator=(int rhs) {
		return operator=((long long)(rhs));
	}
	posit& operator=(long int rhs) {
		return operator=((long long)(rhs));
	}
	posit& operator=(long long rhs) {
		// only valid integers are -4, -2, -1, 0, 1, 2, 4
		_bits = uint8_t(0);
		if (rhs <= -4) {
			_bits = 0x9;   // value is -4, or -maxpos
		}
		else if (-4 < rhs && rhs <= -2) {
			_bits = 0xA;   // value is -2
		}
		else if (-2 < rhs && rhs <= -1) {
			_bits = 0xC;   // value is -1
		}
		else if (-1 < rhs && rhs < 1) {
			_bits = 0x0;   // value is 0
		}
		else if (1 <= rhs && rhs < 2) {
			_bits = 0x4;   // value is 1
		}
		else if (2 <= rhs && rhs < 4) {
			_bits = 0x6;   // value is 2
		}
		else if (4 <= rhs) {
			_bits = 0x7;   // value is 4, or maxpos
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

	posit& setBitblock(sw::universal::bitblock<NBITS_IS_4>& raw) {
		_bits = uint8_t(raw.to_ulong());
		return *this;
	}
	posit& setbits(uint64_t value) {
		_bits = uint8_t(value & bit_mask);
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
		return p.setbits((~_bits) + 1);
	}
	posit& operator+=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_4_0_addition_lookup[index];
		return *this;
	}
	posit& operator-=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_4_0_subtraction_lookup[index];
		return *this;
	}
	posit& operator*=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_4_0_multiplication_lookup[index];
		return *this;
	}
	posit& operator/=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_4_0_division_lookup[index];
		return *this;
	}
	posit& operator++() {
		_bits = (_bits + 1) & bit_mask;
		return *this;
	}
	posit operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit& operator--() {
		_bits = (_bits - 1) & bit_mask;
		return *this;
	}
	posit operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}
	posit reciprocal() const {
		posit p;
		p.setbits(posit_4_0_reciprocal_lookup[_bits]);
		return p;
	}
	// SELECTORS
	inline bool sign()   const { return bool(_bits & 0x08u); }
	inline bool isnar()  const { return (_bits == nar_encoding); }
	inline bool iszero() const { return (_bits == 0); }
	inline bool isone() const { // pattern 0100....
		return (_bits == one_encoding);
	}
	inline bool isminusone() const { // pattern 1100...
		return (_bits == minusone_encoding);
	}
	inline bool isneg()      const { return bool(_bits & 0x08u); }
	inline bool ispos()      const { return !isneg(); }
	inline bool ispowerof2() const { return !(_bits & 0x1u); }

	inline int sign_value() const { return (_bits & 0x08u ? -1 : 1); }

	bitblock<NBITS_IS_4> get() const { bitblock<NBITS_IS_4> bb; bb = int(_bits & bit_mask); return bb; }
	unsigned int bits() const { return (unsigned int)(_bits & bit_mask); }

	inline void clear() { _bits = 0; }
	inline void setzero() { clear(); }
	inline void setnar() { _bits = nar_encoding; }
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
		if (iszero())  return 0.0;
		if (isnar())   return NAN;
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
		long double s = (_sign ? -1.0 : 1.0);
		long double r = _regime.value();
		long double e = _exponent.value();
		long double f = (1.0 + _fraction.value());
		return s * r * e * f;
	}

	template <typename T>
	posit& float_assign(const T& rhs) {
		constexpr int dfbits = std::numeric_limits<T>::digits - 1;
		internal::value<dfbits> v((T)rhs);

		// special case processing
		if (v.iszero()) {
			setzero();
			return *this;
		}
		if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
			setnar();
			return *this;
		}
		bitblock<NBITS_IS_4> ptt;
		convert_to_bb<NBITS_IS_4, ES_IS_0, dfbits>(v.sign(), v.scale(), v.fraction(), ptt); // TODO: needs to be faster
		_bits = uint8_t(ptt.to_ulong());
		return *this;
	}

	// I/O operators
	friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_4, ES_IS_0>& p);
	friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_4, ES_IS_0>& p);

	// posit - posit logic functions
	friend bool operator==(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs);
	friend bool operator!=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs);
	friend bool operator< (const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs);
	friend bool operator> (const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs);
	friend bool operator<=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs);
	friend bool operator>=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs);

};

// posit I/O operators
// generate a posit format ASCII format nbits.esxNN...NNp
inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_4, ES_IS_0>& p) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the posit into a string
	std::stringstream ss;
#if POSIT_ERROR_FREE_IO_FORMAT
	ss << NBITS_IS_4 << '.' << ES_IS_0 << 'x' << to_hex(p.get()) << 'p';
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
inline std::istream& operator>> (std::istream& istr, posit<NBITS_IS_4, ES_IS_0>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

// convert a posit value to a string using "nar" as designation of NaR
inline std::string to_string(const posit<NBITS_IS_4, ES_IS_0>& p, std::streamsize precision) {
	if (p.isnar()) {
		return std::string("nar");
	}
	std::stringstream ss;
	ss << std::setprecision(precision) << float(p);
	return ss.str();
}

// posit - posit binary logic operators
inline bool operator==(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
	return lhs._bits == rhs._bits;
}
inline bool operator!=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
	return !operator==(lhs, rhs);
}
inline bool operator< (const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
	if (rhs.isnar()) {
		return false;
	}
	posit<NBITS_IS_4, ES_IS_0> r = lhs - rhs;  // else calculate the difference and check if negative
	return r.isneg();
}
inline bool operator> (const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
	return operator< (rhs, lhs);
}
inline bool operator<=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
inline bool operator>=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
	return !operator< (lhs, rhs);
}

inline posit<NBITS_IS_4, ES_IS_0> operator+(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
	posit<NBITS_IS_4, ES_IS_0> sum = lhs;
	sum += rhs;
	return sum;
}

#endif // POSIT_FAST_POSIT_4_0

}} // namespace sw::universal
