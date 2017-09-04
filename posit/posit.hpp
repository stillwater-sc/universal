#pragma once

#include <cmath>
#include <iostream>
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
	posit<nbits, es>(const posit& p) {
		*this = p;
	}
	posit<nbits, es>& operator=(const char rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(int rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(long rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(long long rhs) {
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
			cerr << "msb = " << msb << " and maxpos_scale() = " << maxpos_scale() << endl;
			cerr << "Can't represent " << rhs << " with posit<" << nbits << "," << es << ">: maxpos = " << (1 << maxpos_scale()) << endl;
		}
		_Bits[nbits - 1] = false;
		unsigned int nr_of_regime_bits = assign_regime_pattern(msb >> es);
		//cout << "Regime   " << to_binary<nbits>(bits) << endl;

		unsigned int nr_of_exp_bits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
		if (nr_of_exp_bits > 0) {
			unsigned int exponent = (es > 0 ? msb % (1 << es) : 0);
			uint64_t mask = (1 << (nr_of_exp_bits - 1));
			for (int i = 0; i < nr_of_exp_bits; i++) {
				_Bits[nbits - 2 - nr_of_regime_bits - i] = exponent & mask;
				mask >>= 1;
			}
			//cout << "Exponent " << to_binary<nbits>(bits) << endl;
		}

		switch (bRoundingMode) {
		case POSIT_ROUND_DOWN:
		{
			unsigned int remainder_bits = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
			if (remainder_bits > 0) {
				uint64_t mask = (1 << (msb-1));  // first bit is transformed into a hidden bit
				for (int i = 0; i < remainder_bits; i++) {
					_Bits[nbits - 2 - nr_of_regime_bits - nr_of_exp_bits - i] = rhs & mask;
					mask >>= 1;
				}
				//cout << "Fraction " << to_binary<nbits>(bits) << endl;
			}
		}
			break;
		case POSIT_ROUND_TO_NEAREST:
			cerr << "ROUND_TO_NEAREST not implemented yet" << endl;
			break;
		default:
			cerr << "Undefined rounding mode" << endl;
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
			bits.reset();
			bits.set(nbits - 1);
			break;
		case FP_NAN:
			cerr << "float is NAN" << endl;
			break;
		case FP_SUBNORMAL:
			bits.reset();
			cerr << "TODO: subnormal number" << endl;
			break;
		case FP_NORMAL:
			bits.reset();
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
		unsigned int msb;
		int lhs_scale = scale();
		int rhs_scale = rhs.scale();
		cout << "scales (lhs:rhs): " << lhs_scale << ":" << rhs_scale << endl;
		uint64_t lhs_fraction = _Frac.to_ullong();	// really only needs to be nbits-3 hardware
		uint64_t rhs_fraction = rhs._Frac.to_ullong();
		cout << "lhs fraction: 0x" << hex << lhs_fraction << endl;
		cout << "rhs fraction: 0x" << hex << rhs_fraction << endl;
		if (lhs_scale < rhs_scale) {
			lhs_fraction >>= (rhs_scale - lhs_scale);
			msb = findMostSignificantBit(rhs_fraction);
		}
		else {
			rhs_fraction >>= (lhs_scale - rhs_scale);
			msb = findMostSignificantBit(lhs_fraction);
		}
		uint64_t result = lhs_fraction + rhs_fraction;
		cout << "lhs fraction: 0x" << hex << lhs_fraction << endl;
		cout << "rhs fraction: 0x" << hex << rhs_fraction << endl;
		cout << "result      : 0x" << hex << result << endl;
		// see if we need to increment the scale
		if (findMostSignificantBit(result) > msb) {
			increment_scale();
		}
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
			return string("ROUND_DOWN");
			break;
		case POSIT_ROUND_TO_NEAREST:
			return string("ROUND_TO_NEAREST");
			break;
		default:
			return string("UNKNOWN");
		}
	}
	long double maxpos() {
		return pow(double(useed()), double(nbits-2));
	}
	long double minpos() {
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

		//							cout << "fraction bits " << msb - size + 1 << endl;
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
	long double to_double() const {
		if (isZero()) {
			return 0.0;
		}
		if (isInfinite()) {
			return INFINITY;
		}

		//double value = sign() * regime() * exponent() * fraction();

		long double value = 0.0;
		long double base = 0.0;
		int e = exponent_int();

		// scale = useed ^ k * 2^e -> 2^(k*2^es) * 2^e = 2^(k*2^es + e)
		int e2 = (k * (1 << es)) + e;
		if (e2 < -63 || e2 > 63) {
			base = pow(2.0, e2);
		}
		else {
			if (e2 >= 0) {			
				base = long double(uint64_t(1) << e2);
			}
			else {
				base = 1.0 / long double(uint64_t(1) << -e2);
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
				_Exp = convert_bits<es>(_Exp.to_ulong() + 1);
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
			//cout << "Regime   " << to_binary<nbits>(bits) << endl;
		}
		else {
			uint64_t regime = REGIME_BITS[k];
			uint64_t mask = REGIME_BITS[0];
			nr_of_regime_bits = (k < nbits - 2 ? k + 2 : nbits - 1);
			for (int i = 0; i < nr_of_regime_bits; i++) {
				_Bits[nbits - 2 - i] = regime & mask;
				mask >>= 1;
			}
			//cout << "Regime   " << to_binary<nbits>(bits) << endl;
		}
		return nr_of_regime_bits;
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
	void extractIEEE754(uint64_t f, int exponentSize, int mantissaSize) {
		int exponentBias = POW2(exponentSize - 1) - 1;
		int16_t exponent = (f >> mantissaSize) & ((1 << exponentSize) - 1);
		uint64_t mantissa = (f & ((1ULL << mantissaSize) - 1));

		// clip exponent
		int rmin = POW2(es) * (2 - nbits);
		int rmax = POW2(es) * (nbits - 2);
		int rf = MIN(MAX(exponent - exponentBias, rmin), rmax);
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
