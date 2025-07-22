// posit_3_0.hpp: specialized 3-bit posit using lookup table arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_3_0
#define POSIT_FAST_POSIT_3_0 0
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_3_0
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<3,0>")
#else
#pragma message "Fast specialization of posit<3,0>"
#endif

/*  values of a posit<3,0>
000 +0
001 +0.5
010 +1
011 +2
100 nar
101 -2
110 -1
111 -0.5
*/
constexpr float posit_3_0_values_lookup[8] = {
	0.0f, 0.5f, 1.0f, 2.0f, -float(INFINITY), -2.0f, -1.0f, -0.5f,
};

constexpr uint8_t posit_3_0_addition_lookup[64] = {
	0,1,2,3,4,5,6,7,
	1,2,2,3,4,6,7,0,
	2,2,3,3,4,6,0,1,
	3,3,3,3,4,0,2,2,
	4,4,4,4,4,4,4,4,
	5,6,6,0,4,5,5,5,
	6,7,0,2,4,5,5,6,
	7,0,1,2,4,5,6,6,
};
constexpr uint8_t posit_3_0_subtraction_lookup[64] = {
	0,7,6,5,4,3,2,1,
	1,0,7,6,4,3,2,2,
	2,1,0,6,4,3,3,2,
	3,2,2,0,4,3,3,3,
	4,4,4,4,4,4,4,4,
	5,5,5,5,4,0,6,6,
	6,6,5,5,4,2,0,7,
	7,6,6,5,4,2,1,0,
};
constexpr uint8_t posit_3_0_multiplication_lookup[64] = {
	0,0,0,0,4,0,0,0,
	0,1,1,2,4,6,7,7,
	0,1,2,3,4,5,6,7,
	0,2,3,3,4,5,5,6,
	4,4,4,4,4,4,4,4,
	0,6,5,5,4,3,3,2,
	0,7,6,5,4,3,2,1,
	0,7,7,6,4,2,1,1,
};
constexpr uint8_t posit_3_0_division_lookup[64] = {
	4,0,0,0,4,0,0,0,
	4,2,1,1,4,7,7,6,
	4,3,2,1,4,7,6,5,
	4,3,3,2,4,6,5,5,
	4,4,4,4,4,4,4,4,
	4,5,5,6,4,2,3,3,
	4,5,6,7,4,1,2,3,
	4,6,7,7,4,1,1,2,
};

constexpr uint8_t posit_3_0_reciprocal_lookup[8] = {
	4,3,2,1,4,7,6,5,  
};

constexpr bool posit_3_0_less_than_lookup[64] = {
	0,1,1,1,0,0,0,0,
	0,0,1,1,0,0,0,0,
	0,0,0,1,0,0,0,0,
	0,0,0,0,0,0,0,0,
	1,1,1,1,0,1,1,1,
	1,1,1,1,0,0,1,1,
	1,1,1,1,0,0,0,1,
	1,1,1,1,0,0,0,0,
};

// specialized posit<3,0>
template<>
class posit<NBITS_IS_3, ES_IS_0> {
public:
	static constexpr unsigned nbits = NBITS_IS_3;
	static constexpr unsigned es = ES_IS_0;
	static constexpr unsigned sbits = 1;
	static constexpr unsigned rbits = nbits - sbits;
	static constexpr unsigned ebits = es;
	static constexpr unsigned fbits = 0;
	static constexpr unsigned fhbits = fbits + 1;
	static constexpr uint8_t index_shift = NBITS_IS_3;
	static constexpr uint8_t bit_mask = 0x07;  // last three bits
	static constexpr uint8_t nar_encoding = 0x04;
	static constexpr uint8_t one_encoding = 0x02;
	static constexpr uint8_t minus_one_encoding = 0x06;

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

	posit(int initial_value)         { *this = initial_value; }
	posit(long long initial_value)   { *this = (int)initial_value; }
	posit(float initial_value)       { *this = float_assign(initial_value); }
	posit(double initial_value)      { *this = float_assign(initial_value); }
	posit(long double initial_value) { *this = float_assign(initial_value); }

	// assignment operators for native types
	posit& operator=(int rhs) noexcept {
		// only valid integers are -2, -1, 0, 1, 2
		_bits = 0x00;
		if (rhs <= -2) {
			_bits = 0x05;   // value is -2, or -maxpos
		}
		else if (rhs == -1) {
			_bits = 0x06;  // value is -1
		}
		else if (rhs == 0) {
			_bits = 0x0;   // value is 0
		}
		else if (1 == rhs) {
			_bits = 0x02;   // value is 1
		}
		else if (2 <= rhs) {
			_bits = 0x03;  // value is 2, or maxpos
		}
		return *this;
	}
	posit& operator=(long long rhs) noexcept     { return operator=((int)rhs); }
	posit& operator=(float rhs) noexcept         { return float_assign(rhs); }
	posit& operator=(double rhs) noexcept        { return float_assign(rhs); }
	posit& operator=(long double rhs) noexcept   { return float_assign(rhs);  }

	explicit operator long double() const        { return to_long_double(); }
	explicit operator double() const             { return to_double(); }
	explicit operator float() const              { return to_float(); }
	explicit operator long long() const          { return to_long_long(); }
	explicit operator long() const               { return to_long(); }
	explicit operator int() const                { return to_int(); }
	explicit operator unsigned long long() const { return to_long_long(); }
	explicit operator unsigned long() const      { return to_long(); }
	explicit operator unsigned int() const       { return to_int(); }

	posit& setBitblock(sw::universal::bitblock<NBITS_IS_3>& raw) {
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
			p.setbits(0x07);
			break;
		case 0x02:
			p.setbits(0x06);
			break;
		case 0x03:
			p.setbits(0x05);
			break;					
		case 0x04:
			p.setbits(0x04);
			break;					
		case 0x05:
			p.setbits(0x03);
			break;					
		case 0x06:
			p.setbits(0x02);
			break;					
		case 0x07:
			p.setbits(0x01);
			break;
		default:
			p.setbits(0x04);
		}
		return p;
	}
	posit& operator+=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_3_0_addition_lookup[index];
		return *this;
	}
	posit& operator-=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_3_0_subtraction_lookup[index];
		return *this;
	}
	posit& operator*=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_3_0_multiplication_lookup[index];
		return *this;
	}
	posit& operator/=(const posit& b) {
		uint16_t index = (_bits << index_shift) | b._bits;
		_bits = posit_3_0_division_lookup[index];
		return *this;
	}
	posit& operator++() {
		_bits = (_bits + 1) & 0x07;
		return *this;
	}
	posit operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit& operator--() {
		_bits = (_bits - 1) & 0x07;
		return *this;
	}
	posit operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}
	posit reciprocal() const {
		posit p;
		p.setbits(posit_3_0_reciprocal_lookup[_bits]);
		return p;
	}
	
	// SELECTORS
	bool sign()   const { return (_bits & 0x4u); }
	bool isnar()  const { return (_bits == nar_encoding); }
	bool iszero() const { return (_bits == 0x0u); }
	bool isone() const { // pattern 010....
		return (_bits == one_encoding);
	}
	bool isminusone() const { // pattern 110...
		return (_bits == minus_one_encoding);
	}
	bool isneg()      const { return (_bits & 0x4u); }
	bool ispos()      const { return !isneg(); }
	bool ispowerof2() const { return !(_bits & 0x1u); }

	int sign_value() const { return (_bits & 0x4 ? -1 : 1); }

	bitblock<NBITS_IS_3> get() const { bitblock<NBITS_IS_3> bb; bb = int(_bits); return bb; }
	unsigned int bits() const { return (unsigned int)(_bits & bit_mask); }

	void clear()   { _bits = 0x00; }
	void setzero() { _bits = 0x00; }
	void setnar()  { _bits = nar_encoding; }
	posit& minpos() {
		clear();
		return ++(*this);
	}
	posit& maxpos() {
		setnar();
		return --(*this);
	}
	posit& zero() {
		clear();
		return *this;
	}
	posit& minneg() {
		clear();
		return --(*this);
	}
	posit& maxneg() {
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
		return posit_3_0_values_lookup[bits()];
	}
	double      to_double() const {
		return (double)posit_3_0_values_lookup[bits()];
	}
	long double to_long_double() const {
		return (long double)posit_3_0_values_lookup[bits()];
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
		if (v.iszero()) {
			setzero();
			return *this;
		}
		bool _sign = v.sign();
		int  _scale = v.scale();
		bitblock<dfbits> fraction_in = v.fraction();
		if (check_inward_projection_range<nbits, es>(_scale)) {    // regime dominated
			if (_trace_conversion) std::cout << "inward projection" << std::endl;
			// we are projecting to minpos/maxpos
			int k = calculate_unconstrained_k<nbits, es>(_scale);
			bitblock<NBITS_IS_3> bb = k < 0 ? minpos_pattern<nbits, es>(_sign) : maxpos_pattern<nbits, es>(_sign);
			_bits = uint8_t(bb.to_ulong());
			// we are done
			if (_trace_rounding) std::cout << "projection  rounding ";
		}
		else {
			const unsigned pt_len = nbits + 3 + es;
			bitblock<pt_len> pt_bits;
			bitblock<pt_len> regime;
			bitblock<pt_len> exponent;
			bitblock<pt_len> fraction;
			bitblock<pt_len> sticky_bit;

			bool s = _sign;
			int e = _scale;
			bool r = (e >= 0);

			unsigned run = (r ? 1 + (e >> es) : -(e >> es));
			regime.set(0, 1 ^ r);
			for (unsigned i = 1; i <= run; i++) regime.set(i, r);

			unsigned esval = e % (uint32_t(1) << es);
			exponent = convert_to_bitblock<pt_len>(esval);
			unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2 + run + es));
			// TODO: what needs to be done if nf > fbits?
			//assert(nf <= input_fbits);
			// copy the most significant nf fraction bits into fraction
			unsigned lsb = nf <= dfbits ? 0 : nf - dfbits;
			for (unsigned i = lsb; i < nf; i++) fraction[i] = fraction_in[dfbits - nf + i];

			bool sb = anyAfter(fraction_in, dfbits - 1 - nf);

			// construct the untruncated posit
			// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
			regime <<= es + nf + 1;
			exponent <<= nf + 1;
			fraction <<= 1;
			sticky_bit.set(0, sb);

			pt_bits |= regime;
			pt_bits |= exponent;
			pt_bits |= fraction;
			pt_bits |= sticky_bit;

			unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
			bool blast = pt_bits.test(len - nbits);
			bool bafter = pt_bits.test(len - nbits - 1);
			bool bsticky = anyAfter(pt_bits, len - nbits - 1 - 1);

			bool rb = (blast & bafter) | (bafter & bsticky);

			bitblock<nbits> ptt;
			pt_bits <<= pt_len - len;
			truncate(pt_bits, ptt);
			if (rb) increment_bitset(ptt);
			if (s) ptt = twos_complement(ptt);
			_bits = uint8_t(ptt.to_ulong());
		}
		return *this;
	}

	// I/O operators
	friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_3, 0>& p);
	friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_3, 0>& p);

	// posit - posit logic functions
	friend bool operator==(const posit<NBITS_IS_3, 0>& lhs, const posit<NBITS_IS_3, 0>& rhs);
	friend bool operator!=(const posit<NBITS_IS_3, 0>& lhs, const posit<NBITS_IS_3, 0>& rhs);
	friend bool operator< (const posit<NBITS_IS_3, 0>& lhs, const posit<NBITS_IS_3, 0>& rhs);
	friend bool operator> (const posit<NBITS_IS_3, 0>& lhs, const posit<NBITS_IS_3, 0>& rhs);
	friend bool operator<=(const posit<NBITS_IS_3, 0>& lhs, const posit<NBITS_IS_3, 0>& rhs);
	friend bool operator>=(const posit<NBITS_IS_3, 0>& lhs, const posit<NBITS_IS_3, 0>& rhs);

};

		// posit I/O operators
		inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_3, ES_IS_0>& p) {
			// to make certain that setw and left/right operators work properly
			// we need to transform the posit into a string
			std::stringstream ss;
#if POSIT_ERROR_FREE_IO_FORMAT
			ss << NBITS_IS_3 << '.' << ES_IS_0 << 'x' << to_hex(p.get()) << 'p';
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
		inline std::string to_string(const posit<NBITS_IS_3, ES_IS_0>& p, std::streamsize precision) {
			if (p.isnar()) {
				return std::string("nar");
			}
			std::stringstream ss;
			ss << std::setprecision(precision) << float(p);
			return ss.str();
		}

		// posit - posit binary logic operators
		inline bool operator==(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return lhs._bits == rhs._bits;
		}
		inline bool operator!=(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return !operator==(lhs, rhs);
		}
		inline bool operator< (const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			uint16_t index = (uint16_t(lhs.bits()) << NBITS_IS_3) | uint16_t(rhs.bits());
			return posit_3_0_less_than_lookup[index];
		}
		inline bool operator< (int lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return posit<NBITS_IS_3, ES_IS_0>(lhs) < rhs;
		}
		inline bool operator< (const posit<NBITS_IS_3, ES_IS_0>& lhs, int rhs) {
			return lhs < posit<NBITS_IS_3, ES_IS_0>(rhs);
		}
		inline bool operator< (float lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return posit<NBITS_IS_3, ES_IS_0>(lhs) < rhs;
		}
		inline bool operator< (const posit<NBITS_IS_3, ES_IS_0>& lhs, float rhs) {
			return lhs < posit<NBITS_IS_3, ES_IS_0>(rhs);
		}
		inline bool operator< (double lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return posit<NBITS_IS_3, ES_IS_0>(lhs) < rhs;
		}
		inline bool operator< (const posit<NBITS_IS_3, ES_IS_0>& lhs, double rhs) {
			return lhs < posit<NBITS_IS_3, ES_IS_0>(rhs);
		}
		inline bool operator> (const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return operator< (rhs, lhs);
		}
		inline bool operator<=(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return operator< (lhs, rhs) || operator==(lhs, rhs);
		}
		inline bool operator>=(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			return !operator< (lhs, rhs);
		}

		inline posit<NBITS_IS_3, ES_IS_0> operator+(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			posit<NBITS_IS_3, ES_IS_0> sum = lhs;
			sum += rhs;
			return sum;
		}
		inline posit<NBITS_IS_3, ES_IS_0> operator-(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			posit<NBITS_IS_3, ES_IS_0> sub = lhs;
			sub -= rhs;
			return sub;
		}
		inline posit<NBITS_IS_3, ES_IS_0> operator*(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			posit<NBITS_IS_3, ES_IS_0> mul = lhs;
			mul *= rhs;
			return mul;
		}
		inline posit<NBITS_IS_3, ES_IS_0> operator/(const posit<NBITS_IS_3, ES_IS_0>& lhs, const posit<NBITS_IS_3, ES_IS_0>& rhs) {
			posit<NBITS_IS_3, ES_IS_0> div = lhs;
			div /= rhs;
			return div;
		}

#endif // POSIT_FAST_POSIT_3_0
	
}} // namespace sw::universal
