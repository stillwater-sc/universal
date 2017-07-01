#pragma once

#include <cmath>
#include <iostream>

#define POW2(n) (uint64_t(1) << (n))
#ifndef MIN
#define MIN(a,b) (a) < (b) ? (a) : (b)
#endif
#ifndef MAX
#define MAX(a,b) (a) > (b) ? (a) : (b)
#endif

// easy to use segment masks
#define FLOAT_SIGN_MASK      0x80000000
#define FLOAT_EXPONENT_MASK  0x7F800000
#define FLOAT_MANTISSA_MASK  0x007FFFFF
#define DOUBLE_SIGN_MASK     0x8000000000000000
#define DOUBLE_EXPONENT_MASK 0x7FF0000000000000
#define DOUBLE_MANTISSA_MASK 0x000FFFFFFFFFFFFF

template<size_t nbits>
std::bitset<nbits> twos_complement(std::bitset<nbits> number) {
	std::bitset<nbits> complement;
	uint64_t value = number.flip().to_ulong();
	value++;
	unsigned long mask = 1;
	for (int i = 0; i < nbits; i++) {
		complement.set(i, (value & mask));
		mask = mask << 1;
	}
	return complement;
}

template<size_t nbits, size_t es> class posit {
public:
	posit<nbits, es>() {
		reset();
	}
	posit<nbits, es>& operator=(const char& rhs) {

		return *this;
	}
	posit<nbits, es>& operator=(const int& rhs) {

		return *this;
	}
	posit<nbits, es>& operator=(const long& rhs) {

		return *this;
	}
	posit<nbits, es>& operator=(const long long& rhs) {

		return *this;
	}
	posit<nbits, es>& operator=(const float& rhs) {
            using namespace std;
		switch (fpclassify(rhs)) {
		case FP_INFINITE:
			bits.reset();
			bits[nbits - 1] = true;
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
	posit<nbits, es>& operator=(const double& rhs) {
		return *this;
	}
	posit<nbits, es>& operator+=(const posit& rhs) {
		// add rhs             this->bits += rhs.bits;
		if (isZero()) {
			bits = rhs.bits;
			return *this;
		}
		else {
			if (rhs.isZero()) {
				return *this;
			}
			else if (isInfinite() && rhs.isInfinite()) {
				return *this;
			}
			else if (isInfinite() || rhs.isInfinite()) {
				return *this;
			}
		}
		return *this;
	}
	posit<nbits, es>& operator-=(const posit& rhs) {
		// subtract rhs        this->bits -= rhs.bits;
		return *this;
	}
	posit<nbits, es>& operator*=(const posit& rhs) {
		// multiply by rhs     this->bits *= rhs.bits;
		return *this;
	}
	posit<nbits, es>& operator/=(const posit& rhs) {
		// multiply by /rhs    this->bits *= /rhs.bits;
		return *this;
	}
	posit<nbits, es>& operator++() {
		// add +1 to fraction bits;
		return *this;
	}
	posit<nbits, es> operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit<nbits, es>& operator--() {
		// add -1 to fraction bits;
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
		std::bitset<nbits> tmp(bits);
		tmp.reset(nbits - 1);
		return bits[nbits - 1] && tmp.none();
	}
	bool isZero() const {
		return bits.none();
	}
	bool isNegative() const {
		return bits[nbits - 1];
	}
	bool isPositive() const {
		return !bits[nbits - 1];
	}
	void Range(double& minpos, double& maxpos) const {
		int minpos_exponent = static_cast<int>(2 - nbits);        //  explicit cast to avoid underflow warning, TODO: is this correct?
		int maxpos_exponent = nbits - 2;
		double useed = (1 << (1 << es));
		minpos = pow(useed, minpos_exponent);
		maxpos = pow(useed, maxpos_exponent);
	}
	// Get the raw bits of the posit
	std::bitset<nbits> get_raw_bits() const {
		return bits;
	}
	int sign() const {
		return (bits[nbits - 1] ? -1 : 1);
	}
	// return regime value
	double regime() const {
		double regime;   // only works for posits smaller than 32bits
						 // useed is 2^(2^es) -> a left-shift of one by 2^es
						 // useed^k -> left-shift of k*2^es
		if (k == 0) return 1.0;
		if (k > 0) {
			regime = (1 << (1 << es)*k);
		}
		else {
			regime = 1.0 / (1 << (1 << es)*-k);
		}

		return regime;
	}
	// return exponent value
	uint32_t exponent() const {
		return uint32_t(1 << exp.to_ulong());
	}
	double fraction() const {
		return double(frac.to_ulong())/(1 << nbits);
	}
	// return run-length of the regime encoding
	int run_length() const {
		return k;
	}
	// return exponent bits
	std::bitset<es> exponent_bits() const {
		return exp;
	}
	// return fraction bits: nbits - 1, right-extended
	std::bitset<nbits> fraction_bits() const {
		return frac;
	}

	// MODIFIERS
	void reset() {
		k = 0;
		exp.reset();
		frac.reset();
		bits.reset();
	}
	void set(std::bitset<nbits> raw) {
		reset();
		bits = raw;
		decode();
	}
	// Set the raw bits of the posit given a binary pattern
	posit<nbits,es>& set_raw_bits(unsigned long value) {
		reset();
		unsigned long mask = 1;
		for ( int i = 0; i < nbits; i++ ) {
			bits.set(i,(value & mask));
			mask = mask << 1;
		}
		// decode to cache the posit number interpretation
		decode();
		return *this;
	}

	// decode the segments: precondition: member vars reset with bits containing the value to decode
	int16_t decode() {
		if (isZero()) {  // special case = 0
			k = -(nbits-1);
			return k;
		}
		if (isInfinite()) {	// special case = +-inf
			k = (nbits - 1);
			return k;
		}

		// if sign(p) is -1, take 2's complement
		std::bitset<nbits> tmp(bits);
		if (bits[nbits - 1]) {
			tmp = twos_complement(bits);
		}
		// let k be the number of identical bits in the regime
		if (tmp[nbits-2] == 1) {   // run length of 1's
			k = 0;   // if a run of 1's k = m - 1
			for (int i = nbits - 3; i >= 0; --i) {
				if (tmp[i] == 1) {
					k++;
				}
				else {
					break;
				}
			}
		}
		else {
			k = -1;  // if a run of 0's k = -m
			for (int i = nbits - 3; i >= 0; --i) {
				if (tmp[i] == 0) {
					k--;
				}
				else {
					break;
				}
			}
		}
		/////////////////////////// cout << "k " << int(k) << " ";
		// get the exponent bits
		// start of exponent is nbits - (sign_bit + regime_bits)
		int32_t msb;
		if (k >= 0) {	// k = 0, 1, 2, ... nbits-2
			msb = nbits - k - 4;
		}
		else {			// k = -1, -2, ... , -(nbits-1)
			msb = nbits + k - 3;
		}
		///////////////////////                             cout << msb << " ";
		int32_t size = 0;
		if (msb >= 0 && es > 0) {	
			size = (msb >= es - 1 ? es : msb + 1);
			/////////////////// cout << " size " << size << " msb " << msb << " ";
			for (int i = 0; i < size; i++) {
				exp[i] = bits[msb - (size - 1) + i];
			}
		}

		//////////////////  cout << "fraction bits " << msb - size + 1 << endl;
		// finally, set the fraction bits
		// we do this so that the fraction is right extended with 0;
		// The max fraction is <nbits - 3 - es>, but we are setting it to <nbits> and right-extent
		// The msb bit of the fraction representes 2^-1, the next 2^-2, etc.
		// If the fraction is empty, we have a fraction of nbits-1 0 bits
		// If the fraction is one bit, we have still have fraction of nbits-1, with the msb representing 2^-1, and the rest are right extended 0's
		msb = msb - size;
		size = (msb < 0 ? 0 : msb + 1);
		if (msb >= 0) {
			int f = 0;
			for (int i = msb; i >= 0; --i) {
				frac[nbits - 1 - f++] = bits[i];
			}
		}
		return k;
	}

	unsigned long to_ulong() {
		unsigned long value = bits.to_ulong();
		return value;
	}

	double to_double() {
		if (isZero()) {
			return 0.0;
		}
		if (isInfinite()) {
			return INFINITY;
		}
		// value = sign * useed ^ k * 2 ^ exp *   1.fraction
		double value = sign() * regime() * exponent();
		if (frac.any()) {
			value = value * (1.0 + fraction());
		}

		return value;
	}


private:
	std::bitset<nbits> bits;
	std::bitset<es> exp;
	std::bitset<nbits> frac; // fraction is max <nbits - 3 - es>  but for small posits, this yields a negative size, so we simply set it to <nbits> and right-extend
	int8_t k;

	int findBaseExponent(uint64_t number) const {
		// find the most significant bit
		int i = 0;
		int size = sizeof(number);
		while (i < size) {
			number >>= 1;
			if (0 == number) {
				return i;
			}
			i++;
		}
		return i;
	}



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
