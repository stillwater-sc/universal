#pragma once

#include <cmath>
#include <iostream>
#include "posit_regime_lookup.hpp"

inline uint64_t two_to_the_power(int n) {
	return (uint64_t(1) << n);
}

// find the most significant bit set: first bit is at position 1, so that no bits set returns 0
unsigned int findMostSignificantBit(uint64_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	unsigned int base = 0;
	if (x & 0xFFFFFFFF00000000) { base += 32; x >>= 32; }
	if (x & 0x00000000FFFF0000) { base += 16; x >>= 16; }
	if (x & 0x000000000000FF00) { base += 8;  x >>= 8; }
	if (x & 0x00000000000000F0) { base += 4;  x >>= 4; }
	return base + bval[x];
}

unsigned int findMostSignificantBit(int64_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint64_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xFFFFFFFF00000000) { base += 32; tmp >>= 32; }
	if (tmp & 0x00000000FFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x000000000000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00000000000000F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

unsigned int findMostSignificantBit(int32_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint32_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xFFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x0000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x000000F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

unsigned int findMostSignificantBit(int16_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint16_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xFF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

unsigned int findMostSignificantBit(int8_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint8_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xF0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

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
		validate();
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
		if (rhs == 0) {
			bits.reset();
			return *this;
		}
		cout << "Assignment operator with value " << rhs << endl;
		int msb;
		if (isPositive()) {
			msb = findMostSignificantBit(rhs)-1;
			if (msb > maxpos_scale()) {
				cerr << "msb = " << msb << " and maxpos_scale() = " << maxpos_scale() << endl;
				cerr << "Can't represent " << rhs << " with posit<" << nbits << "," << es << ">: maxpos = " << (1 << maxpos_scale()) << endl;
			}
			// we need to find the regime for this rhs
			// regime represents a scale factor of useed ^ k, where k ranges from [-nbits-1, nbits-2]
			// regime @ k = 0 -> 1
			// regime @ k = 1 -> (1 << (1 << es) ^ 1 = 2
			// regime @ k = 2 -> (1 << (1 << es) ^ 2 = 4
			// regime @ k = 3 -> (1 << (1 << es) ^ 3 = 8
			// the left shift of the regime is simply k * 2^es
			// which means that the msb of the regime is simply k*2^es
			// TODO: do you want to calculate how many bits the regime is?
			// yes: because then you can figure out if you have exponent bits and fraction bits left.
		}
		else {
			// take a two's complement
			cout << "Negative numbers not implemented yet" << endl;
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
	posit<nbits, es>& operator+=(const posit rhs) {
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
	unsigned int useed() {
		return (1 << (1 << es));
	}
	// what are you trying to capture with this method? TODO
	// return the position of the msb of the largest binary number representable by this posit?
	// this would be maxpos
	unsigned int maxpos_scale() {
		int maxpos_exponent = nbits - 2;
		return maxpos_exponent * (1 << es);
	}
	// TODO: what would minpos_scale represent?
	unsigned int minpos_scale() {
		int minpos_exponent = static_cast<int>(2 - nbits);
		return minpos_exponent * (1 << es);
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
	// return fraction bits: nbits - 3, right-extended
	std::bitset<nbits-3> fraction_bits() const {
		return frac;
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
			k = -int(nbits-1);
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
				exp[i] = tmp[msb - (size - 1) + i];
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
				frac[nbits - 4 - f++] = tmp[i];
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
			// (2^(2^es))^k * 2^e -> shift is k*2^es + e
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
			// scale of the number is 2^(fbs+1)
			// scale of the regime is 2^(k*2^es + e)
			// k*2^es = fbs+1 -> k = (fbs+1) >> es
			int k = ((fbs << 1) >> (es+es));
			cout << "number to convert " << number << " fbs " << fbs << " k " << k << " ";
			if (k > nbits - 2) {
				cout << "Overflow: number " << number << " is too big for posit<" << nbits << "," << es << ">" << endl;
				bits.set(nbits - 1);
				decode();
				return bits;	// return infinite
			}
			// this is always a pattern of 01####
			bits.reset(nbits - 1);
			bits.set(nbits - 2);
			// k = 0 -> 10
			// k = 1 -> 110
			// k = 2 -> 1110
			// k = nbits-2 => 1111
			int r = nbits - 2;
			for (int i = 0; i < k; i++) {
				r--;
				bits.set(r);
			}
			if (k == nbits - 2) {
				bits.set(0);
			}
			else {
				bits.reset(nbits - 3 - k);
			}
			// set the exponent bits
			// the regime takes the base to useed^k
			// we have exponent and fraction bits if k*2^es < fsb
			int shift = (k << es);
			int msb = fbs - shift;	
			cout << "shift " << shift << " msb " << msb << " ";

			// set the fraction bits
		}
		decode();
		return bits;
	}

	// scale returns the shifts to normalize the number =  regime + exponent shifts
	int scale() const {
		// how many shifts represent the regime?
		// regime = useed ^ k = 2 ^ (k*(2 ^ e))
		// scale = useed ^ k * 2^e 
		return k*(1 << es) + exp.to_ulong();
	}

private:
	std::bitset<nbits> bits;
	std::bitset<es> exp;
	std::bitset<nbits-3> frac; // fraction is max <nbits - 1 sign bit - minimum 2 regime bits>
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
