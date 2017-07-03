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
		//validate();
	}
	posit<nbits, es>& operator=(const char& rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(const int& rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(const long& rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(const long long& rhs) {
		if (rhs == 0) {
			bits.reset();
		}
		decode();
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
	void Range(double* minpos, double* maxpos) const {
		int minpos_exponent = static_cast<int>(2 - nbits);        //  explicit cast to avoid underflow warning, TODO: is this correct?
		int maxpos_exponent = nbits - 2;
		double useed = (1 << (1 << es));
		*minpos = pow(useed, minpos_exponent);
		*maxpos = pow(useed, maxpos_exponent);
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
	// return exponent value
	uint32_t exponent() const {
		return uint32_t(exp.to_ulong());
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
	void validate() throw(char*) {
		if (nbits <= es + 3) {
			throw "Requested es is too large for nbits";
		}
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
		int m = 0;
		std::bitset<nbits> tmp(bits);
		if (tmp[nbits - 1]) {
			tmp = twos_complement(bits);
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

		///////////////////////                            cout << "k = " << int(k) << " m = " << m ;
		// get the exponent bits
		// start of exponent is nbits - (sign_bit + regime_bits)
		int32_t msb = nbits - (3 + m);

		///////////////////////                             cout << msb << " ";
		int32_t size = 0;
		if (msb >= 0 && es > 0) {	
			size = (msb >= es - 1 ? es : msb + 1);
			/////////////////// cout << " size " << size << " msb " << msb << " ";
			for (int i = 0; i < size; i++) {
				exp[i] = tmp[msb - (size - 1) + i];
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
				frac[nbits - 1 - f++] = tmp[i];
			}
		}
		return k;
	}

	unsigned long to_ulong() const {
		unsigned long value = bits.to_ulong();
		return value;
	}

	double to_double() const {
		if (isZero()) {
			return 0.0;
		}
		if (isInfinite()) {
			return INFINITY;
		}

		double value = 0.0;
		double base = 0.0;
		int e = exponent();

		// scale = useed ^ k * 2^e -> 2^(k*2^es) * 2^e = 2^(k*2^es + e)
		int e2 = (k * (1 << es)) + e;
		if (e2 < -63 || e2 > 63) {
			base = pow(2.0, e2);
		}
		else {
			if (e2 >= 0) {			
				base = (uint64_t(1) << e2);
			}
			else {
				base = 1.0 / (uint64_t(1) << -e2);
			}
		}
		value = base + base * fraction();
		if (isNegative()) {	
			value = -value;
		} 
		return value;
	}

	// transform an integer to a posit
	// integers cover only 2 quarters of the number line [0,1..inf], and [0,-1..-inf]
	std::bitset<nbits> from_longlong(int64_t number) {
		bits.reset();
		if (number == 0) {
			decode();
			return bits;
		}
		if (number == 1) {
			bits.set(nbits-2);
			decode();
			return bits;
		}
		if (number == -1) {
			bits.set(nbits - 1);
			bits.set(nbits - 2);
			decode();
			return bits;
		}
		if (number > 1) {
			// positive range =  2^(2r+e)     + 2^(2r+e)       * 0.<f>
			// find the first msb set
			int fbs;
			uint64_t mask = (uint64_t(1) << 63);
			for (int i = 63; i >= 0; --i) {
				if (number & mask) {
					fbs = i;
					break;
				}
				mask >>= 1; 
			}
			// generate the regime pattern for this
			// 2r+e == fbs -> 2r = fbs - e
			// r = (fbs - e)/2
			// base regime = fbs/2
			// exponent = -e/2
			int base = (fbs >> 1);
			cout << "base " << base << endl;
			if (base > nbits - 3) {
				cout << "Overflow: number " << number << " is too big for posit<" << nbits << "," << es << ">" << endl;
				bits.set(nbits - 1);
				return bits;	// return infinite
			}
			k = base;
			// this is a pattern of 1####
			for (int i = 1; i < base; i++) {
				bits.set(nbits - 1 - i);
			}
		}
		return bits;
	}

private:
	std::bitset<nbits> bits;
	std::bitset<es> exp;
	std::bitset<nbits> frac; // fraction is max <nbits - 3 - es>  but for small posits, this yields a negative size, so we simply set it to <nbits> and right-extend
	int8_t k;


	// HELPER methods
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
