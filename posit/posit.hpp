#pragma once

#include <cmath>
#include <iostream>
#include "../bitset/bitset_helpers.hpp"
#include "posit_regime_lookup.hpp"
#include "posit_helpers.hpp"

const uint8_t POSIT_ROUND_DOWN = 0;
const uint8_t POSIT_ROUND_TO_NEAREST = 1;

/*
 class posit represents arbitrary configuration posits and their arithmetic
 */
template<size_t nbits, size_t es> class posit {
public:
	posit<nbits, es>() {
		bRoundingMode = POSIT_ROUND_DOWN;
		reset();
		validate();
	}
	posit<nbits, es>(int64_t initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(const posit& p) {
		*this = p;
	}
	posit<nbits, es>& operator=(int8_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(int16_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(int32_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(int64_t rhs) {
		reset();
		if (rhs == 0) {
			return *this;
		}

		int msb;
		bool value_is_negative = false;
		if (rhs < 0) {
			rhs = -rhs;
			value_is_negative = true;
		}
		msb = findMostSignificantBit(rhs)-1;
		if (msb > maxpos_scale()) {
			// TODO: Can we make this a compile time evaluated function for literals?
			std::cerr << "msb = " << msb << " and maxpos_scale() = " << maxpos_scale() << std::endl;
			std::cerr << "Can't represent " << rhs << " with posit<" << nbits << "," << es << ">: maxpos = " << (1 << maxpos_scale()) << std::endl;
		}
		_Bits[nbits - 1] = false;
		unsigned int nr_of_regime_bits = assign_regime_pattern(msb >> es);
		std::cout << "Regime   " << to_binary<nbits>(_Bits) << std::endl;

		unsigned int nr_of_exp_bits = assign_exponent_bits(msb, nr_of_regime_bits);
		std::cout << "Exponent " << to_binary<nbits>(_Bits) << std::endl;

		unsigned int remainder_bits = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
		switch (bRoundingMode) {
		case POSIT_ROUND_DOWN:
		{
			if (remainder_bits > 0) {
				uint64_t mask = (1 << (msb-1));  // first bit is transformed into a hidden bit
				for (int i = 0; i < remainder_bits; i++) {
					_Bits[nbits - 2 - nr_of_regime_bits - nr_of_exp_bits - i] = rhs & mask;
					mask >>= 1;
				}
				std::cout << "Fraction " << to_binary<nbits>(_Bits) << std::endl;
			}
		}
			break;
		case POSIT_ROUND_TO_NEAREST:
			std::cerr << "ROUND_TO_NEAREST not implemented yet" << std::endl;
			break;
		default:
			std::cerr << "Undefined rounding mode" << std::endl;
			break;
		}

		if (value_is_negative) {
			_Bits = twos_complement(_Bits);
			_Bits.set(nbits - 1);
		}
		decode();
		return *this;
	}

	posit<nbits, es>& operator=(const float rhs) {
            using namespace std;
		switch (fpclassify(rhs)) {
		case FP_INFINITE:
			_Bits.reset();
			_Bits.set(nbits - 1);
			break;
		case FP_NAN:
			std::cerr << "float is NAN" << std::endl;
			break;
		case FP_SUBNORMAL:
			_Bits.reset();
			std::cerr << "TODO: subnormal number" << std::endl;
			break;
		case FP_NORMAL:
			_Bits.reset();
			// 8 bits of exponent, 23 bits of mantissa
			extractIEEE754((uint64_t)rhs, 8, 23);
			break;
		}
		return *this;
	}
	posit<nbits, es>& operator=(const double rhs) {
		return *this;
	}
	posit<nbits, es>& operator=(const posit& rhs) {
		reset();
		_Bits = rhs._Bits;
		decode();
		return *this;
	}
	posit<nbits, es>& operator+=(const posit& rhs) {
		if (isZero()) {
			_Bits = rhs._Bits;
			return *this;
		}
		else {
			if (rhs.isZero()) {
				return *this;
			}
			else if (isInfinite()) {
				return *this;
			}
			else if (rhs.isInfinite()) {
				*this = rhs;
				return *this;
			}
		}
		std::bitset<nbits - 2> r1, r2; // fraction is at most nbits-3 bits, + 1 for the hidden bit
		int _scale;
		align_numbers(scale(), _Frac, rhs.scale(), rhs._Frac, _scale, r1, r2);

		cout << "lhs " << this->_Bits << " scale " << scale() << endl;
		cout << "rhs " << rhs._Bits <<   " scale " << rhs.scale() << endl;
		cout << "r1    " << r1 << endl;
		cout << "r2    " << r2 << endl;
		cout << "scale " << _scale << endl;


		std::bitset<nbits - 2> sum;
		bool carry = add_unsigned<nbits - 2>(r1, r2, sum);
		cout << "sum " << sum << " carry " << (carry ? "1" : "0") << endl;
		if (carry) {
			_scale++;
			sum >>= 1;
			sum.set(nbits - 3, carry);
		}
		cout << "scale " << _scale << endl;
		cout << "sum " << sum << endl;
		reset();
		convert_to_posit(_scale, sum);
		decode();
		return *this;
	}

	posit<nbits, es>& operator-=(const posit& rhs) {
		return *this;
	}
	posit<nbits, es>& operator*=(const posit& rhs) {
		return *this;
	}
	posit<nbits, es>& operator/=(const posit& rhs) {
		return *this;
	}
	posit<nbits, es>& operator++() {
		*this = *this + posit<nbits, es>(1);
		return *this;
	}
	posit<nbits, es> operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit<nbits, es>& operator--() {
		return *this;
	}
	posit<nbits, es> operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}

	// SELECTORS
	bool isInfinite() const {
		// +-infinite is a bit string of a sign bit of 1 followed by all 0s
		std::bitset<nbits> tmp(_Bits);
		tmp.reset(nbits - 1);
		return _Bits[nbits - 1] && tmp.none();
	}
	bool isZero() const {
		return _Bits.none();
	}
	bool isNegative() const {
		return _Bits[nbits - 1];
	}
	bool isPositive() const {
		return !_Bits[nbits - 1];
	}
	std::string RoundingMode() {
		switch (bRoundingMode) {
		case POSIT_ROUND_DOWN:
			return std::string("ROUND_DOWN");
			break;
		case POSIT_ROUND_TO_NEAREST:
			return std::string("ROUND_TO_NEAREST");
			break;
		default:
			return std::string("UNKNOWN");
		}
	}
	double maxpos() {
		return pow(double(useed()), double(nbits-2));
	}
	double minpos() {
		return pow(double(useed()), double(static_cast<int>(2-nbits)));
	}
	uint64_t useed() {
		return (1 << (1 << es));
	}
	unsigned int base_regime(int64_t rhs) {
		return (findMostSignificantBit(rhs) - 1) >> es;
	}
	unsigned int maxpos_scale() {
		return (nbits - 2) * (1 << es);
	}
	unsigned int minpos_scale() {
		return static_cast<int>(2 - nbits) * (1 << es);
	}

	int sign() const {
		return (_Bits[nbits - 1] ? -1 : 1);
	}
	double regime() const {
		double regime;
		int e2 = (1 << es) * k;
		if (e2 < -63 || e2 > 63) {
			regime = pow(2.0, e2);
		}
		else {
			if (e2 >= 0) {
				regime = (uint64_t(1) << e2);
			}
			else {
				regime = 1.0 / (uint64_t(1) << -e2);
			}
		}
		return regime;
	}
	double exponent() const {
		return double(1 << _Exp.to_ulong());
	}
	double fraction() const {
		return double(_Frac.to_ulong()) / double(1 << (nbits - 3));
	}
	uint64_t regime_int() const {
		if (k < 0) return 0;
		return (1 << k*(1 << es));
	}
	uint64_t exponent_int() const {
		return uint64_t(_Exp.to_ulong());
	}
	uint64_t fraction_int() const {
		uint64_t fraction;
		int nr_of_fraction_bits = 0;
		return _Frac.to_ullong();
	}

	// return the k-value of the regime: useed ^ k
	int regime_k() const {
		return k;
	}
	// return exponent bits
	std::bitset<es> exponent_bits() const {
		return _Exp;
	}
	// return fraction bits: nbits - 3
	std::bitset<nbits-3> fraction_bits() const {
		return _Frac;
	}
	// posit with nbits < 3 will fail due to zero-value fraction bits array
	void validate() throw(char*) {
		if (nbits < es + 3) {
			throw "Requested es is too large for nbits";
		}
	}

	// MODIFIERS
	void reset() {
		k = 0;
		bRoundingMode = POSIT_ROUND_DOWN;
		_Exp.reset();
		_Frac.reset();
		_Bits.reset();
	}
	void set(std::bitset<nbits> raw) {
		reset();
		_Bits = raw;
		decode();
	}
	std::bitset<nbits> get() const {
		return _Bits;
	}
	// Set the raw bits of the posit given a binary pattern
	posit<nbits,es>& set_raw_bits(uint64_t value) {
		reset();
		unsigned long mask = 1;
		for ( int i = 0; i < nbits; i++ ) {
			_Bits.set(i,(value & mask));
			mask = mask << 1;
		}
		// decode to cache the posit number interpretation
		decode();
		return *this;
	}

	// decode the segments: precondition: member vars reset with bits containing the value to decode
	int16_t decode() {
		if (isZero()) {  // special case = 0
			k = -int(nbits-1);
			return k;
		}
		if (isInfinite()) {	// special case = +-inf
			k = (nbits - 1);
			return k;
		}
		int m = 0;
		std::bitset<nbits> tmp(_Bits);
		if (tmp[nbits - 1]) {
			tmp = twos_complement(_Bits);
		}
		// let m be the number of identical bits in the regime
		if (tmp[nbits - 2] == 1) {   // run length of 1's
			m = 1;   // if a run of 1's k = m - 1
			for (int i = nbits - 3; i >= 0; --i) {
				if (tmp[i] == 1) {
					m++;
				}
				else {
					break;
				}
			}
			k = m - 1;
		}
		else {
			m = 1;  // if a run of 0's k = -m
			for (int i = nbits - 3; i >= 0; --i) {
				if (tmp[i] == 0) {
					m++;
				}
				else {
					break;
				}
			}
			k = -m;
		}	

		//                            cout << "k = " << int(k) << " m = " << m ;
		// get the exponent bits
		// start of exponent is nbits - (sign_bit + regime_bits)
		int32_t msb = nbits - (3 + m);
		//                             cout << " msb = " << msb << " ";
		int32_t size = 0;
		if (msb >= 0 && es > 0) {	
			size = (msb >= es - 1 ? es : msb + 1);
		//	                         cout << " size " << size << " msb " << msb << " ";
			for (int i = 0; i < size; i++) {
				_Exp[i] = tmp[msb - (size - 1) + i];
			}
		}

		//					std::cout << "fraction bits " << msb - size + 1 << std::endl;
		// finally, set the fraction bits
		// we do this so that the fraction is right extended with 0;
		// The max fraction is <nbits - 3 - es>, but we are setting it to <nbits - 3> and right-extent
		// The msb bit of the fraction represents 2^-1, the next 2^-2, etc.
		// If the fraction is empty, we have a fraction of nbits-3 0 bits
		// If the fraction is one bit, we have still have fraction of nbits-3, with the msb representing 2^-1, and the rest are right extended 0's
		msb = msb - size;
		size = (msb < 0 ? 0 : msb + 1);
		if (msb >= 0) {
			int f = 0;
			for (int i = msb; i >= 0; --i) {
				_Frac[nbits - 4 - f++] = tmp[i];
			}
		}
		return k;
	}

	int64_t to_int64() const {
		if (isZero()) return 0;
		if (isInfinite()) throw "inf";
		// returning the integer representation of a posit only works for [1,inf)
		int64_t value;
		int s = scale();
		if (s < 0) {
			value = ((fraction_int()) >> -s);
		}
		else {
			value = (fraction_int() << s);
		}	
		return value;
	}
	double to_double() const {
		if (isZero()) {
			return 0.0;
		}
		if (isInfinite()) {
			return INFINITY;
		}

		//double value = sign() * regime() * exponent() * fraction();

		double value = 0.0;
		double base = 0.0;
		int e = exponent_int();

		// scale = useed ^ k * 2^e -> 2^(k*2^es) * 2^e = 2^(k*2^es + e)
		int e2 = (k * (1 << es)) + e;
		if (e2 < -63 || e2 > 63) {
			base = pow(2.0, e2);
		}
		else {
			if (e2 >= 0) {			
				base = double(uint64_t(1) << e2);
			}
			else {
				base = 1.0 / double(uint64_t(1) << -e2);
			}
		}
		value = base + base * fraction();
		if (isNegative()) {	
			value = -value;
		} 
		return value;
	}


	// scale returns the shifts to normalize the number =  regime + exponent shifts
	int scale() const {
		// how many shifts represent the regime?
		// regime = useed ^ k = 2 ^ (k*(2 ^ e))
		// scale = useed ^ k * 2^e 
		return k*(1 << es) + _Exp.to_ulong();
	}
	void increment_scale() {
		if (es == 0) {
			k++;
		}
		else {
			if (this->_Exp.all()) {
				k++;
				_Exp.reset();
			}
			else {
				_Exp = assign_unsigned<es>(uint64_t(_Exp.to_ulong() + 1));
			}		
		}
	}
	// return the number of regime bits
	unsigned int assign_regime_pattern (int k) {
		unsigned int nr_of_regime_bits;
		if (k < 0) {
			k = -k - 1;
			uint64_t regime = REGIME_BITS[k];
			uint64_t mask = REGIME_BITS[0];
			nr_of_regime_bits = (k < nbits - 2 ? k + 2 : nbits - 1);
			for (int i = 0; i < nr_of_regime_bits; i++) {
				_Bits[nbits - 2 - i] = !(regime & mask);
				mask >>= 1;
			}
		}
		else {
			uint64_t regime = REGIME_BITS[k];
			uint64_t mask = REGIME_BITS[0];
			nr_of_regime_bits = (k < nbits - 2 ? k + 2 : nbits - 1);
			for (int i = 0; i < nr_of_regime_bits; i++) {
				_Bits[nbits - 2 - i] = regime & mask;
				mask >>= 1;
			}
		}
		return nr_of_regime_bits;
	}
	unsigned int assign_exponent_bits(unsigned int msb, unsigned int nr_of_regime_bits) {
		unsigned int nr_of_exp_bits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
		if (nr_of_exp_bits > 0) {
			unsigned int exponent = (es > 0 ? msb % (1 << es) : 0);
			uint64_t mask = (1 << (nr_of_exp_bits - 1));
			for (int i = 0; i < nr_of_exp_bits; i++) {
				_Bits[nbits - 2 - nr_of_regime_bits - i] = exponent & mask;
				mask >>= 1;
			}
		}
		return nr_of_exp_bits;
	}
	void assign_fraction(unsigned int remaining_bits, std::bitset<nbits - 2>& _fraction) {
		if (remaining_bits > 0) {
			for (int i = 0; i < remaining_bits; i++) {
				_Bits[remaining_bits - 1 - i] = _fraction[nbits - 3 - i];
			}
		}
	}
	void convert_to_posit(int _scale, std::bitset<nbits - 2>& _fraction) {
		_Bits.reset();
		unsigned int nr_of_regime_bits = assign_regime_pattern(_scale >> es);
		cout << "Regime   " << _Bits << "  regime bits " << nr_of_regime_bits << endl;
		unsigned int nr_of_exp_bits = assign_exponent_bits(_scale, nr_of_regime_bits);
		cout << "Exponent " << _Bits << "  exponent bits " << nr_of_exp_bits << endl;
		unsigned int remaining_bits = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
		cout << " remaining bits " << remaining_bits << " fraction " << _fraction << endl;
		assign_fraction(remaining_bits, _fraction);
		cout << "Posit    " << _Bits << endl;
	}
	// convert floats to posits
	void convert_to_posit(int _scale, uint32_t _23b_fraction_without_hidden_bit) {
		_Bits.reset();
		unsigned int nr_of_regime_bits = assign_regime_pattern(_scale >> es);
		cout << "Regime   " << _Bits << "  #regime bits " << nr_of_regime_bits << endl;
		unsigned int nr_of_exp_bits = assign_exponent_bits(_scale, nr_of_regime_bits);
		unsigned int remaining_bits = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
		cout << "Exponent " << _Bits << "  #exponent bits " << nr_of_exp_bits << " remaining bits " << remaining_bits << endl;
		cout << "         " << to_binary(_23b_fraction_without_hidden_bit) << " fraction " << endl;

		std::bitset<nbits - 2> _fraction = copy_float_fraction<nbits>(_23b_fraction_without_hidden_bit);
		assign_fraction(remaining_bits, _fraction);
		cout << "Fraction " << _fraction << endl;
		cout << "Posit    " << _Bits << endl;
	}
private:
	std::bitset<nbits> _Bits;
	std::bitset<es> _Exp;
	int8_t exponentLength;
	// fraction is max <nbits - 1 sign bit - minimum 2 regime bits - 1 or more exponent bits>
	// the conditional length of the exponent field creates a situation where we need to use the maximum size constant.
	// this is too big and not precise, but is an outcome of using a template specification that needs to be const
	// at time of compilation.
	std::bitset<nbits-3> _Frac; 
	int8_t fractionLength;
	int8_t k;
	int8_t bRoundingMode;

	// HELPER methods
	void align_numbers(int lhs_scale, const std::bitset<nbits - 3>& lhs, int rhs_scale, const std::bitset<nbits - 3>& rhs, int& scale, std::bitset<nbits - 2>& r1, std::bitset<nbits - 2>& r2) {
		int diff = lhs_scale - rhs_scale;
		if (diff < 0) {
			scale = rhs_scale;
			denormalize(lhs, diff, r1);
			normalize(rhs, r2);
		}
		else {
			scale = lhs_scale;
			normalize(lhs, r1);
			denormalize(rhs, diff, r2);
		}
	}
	void normalize(const std::bitset<nbits - 3>& fraction, std::bitset<nbits - 2>& number) {
		number.set(nbits - 3);
		for (int i = 0; i < nbits - 3; i++) {
			number.set(i, fraction[i]);
		}
	}
	void denormalize(const std::bitset<nbits - 3>& fraction, int shift, std::bitset<nbits - 2>& number) {
		number.set(nbits - 3);
		for (int i = 0; i < nbits - 3 - shift; i++) {
			number.set(i, fraction[shift + i]);
		}
	}

        // template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const posit<nnbits, ees>& p);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, posit<nnbits, ees>& p);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
};
