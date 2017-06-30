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
		bits.reset();
		if (rhs == 0) {
			return *this;
		}
		// the posit exponent is useed^k*2^e
		// we need to calculate the scale of the input number and map it to 
		// the minimum useed^k*2^e range to get the bits for the regime and the exponent.

		// useed = 2^2^es
		// 2^scale = (2^2^es)^k * 2^e ->
		// 2^scale = 2^(e + k*2^es) ->
		// scale = e + k*2^es
		// scale - e = k*2^es
		// (scale - e)/2^es = k
		// e = [0, 2^es)
		// if scale < 2^es then 
		//    e = scale
		// else 
		//   (scale - e)>>es >= 0
		//   scale>>2^es - 1 >= 0
		//
		int scale = findBaseExponent(rhs);
		std::cout << "Number scale base is " << scale << std::endl;
		if (rhs >= 0) {
			bits[nbits - 1] = 0;  // sign bit
			// calculate regime and exponent bits
			if (scale == 0) {
				// es bits are all 0
				// regime bits are a run length of k = 0 -> 10
				bits[nbits - 2] = 1;  // first regime bit
				bits[nbits - 3] = 0;  // second regime bit
				bits[nbits - 4] = 0;  // first exponent bit
			}
		}
		else {
			std::cerr << "Negative regime not implemented yet" << std::endl;
		}
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

	// MODIFIERS
	void reset() {
		k = 0;
		exp.reset();
		frac.reset();
		bits.reset();
	}
	// test function:
	void set(std::bitset<nbits> raw) {
		reset();
		bits = raw;
		decode();
	}
	// Get the raw bits of the posit
	std::bitset<nbits> get_raw_bits() const {
		return bits;
	}
	// Set the raw bits of the posit given a binary pattern
	posit<nbits,es>& set_raw_bits(unsigned long value) {
		reset();
		unsigned long mask = 1;
		for ( int i = 0; i < nbits; i++ ) {
			bits.set(i,(value & mask));
			mask = mask << 1;
		}
		// do the decode to cache the posit number interpretation
		decode();
		return *this;
	}

	bool isInfinite() const {
		// +-infinite is a bit string of a sign bit of 1 followed by all 0s
		std::bitset<nbits> tmp(bits << 1);
		return bits[nbits-1] && tmp.none();
	}
	bool isZero() const {
		// zero is a bit string of all 0s
		return !bits.any();
	}
	bool isNegative() const {
		return bits[nbits - 1];
	}
	bool isPositive() const {
		return !bits[nbits - 1];
	}
	void Range() const {
		int minpos_exponent = static_cast<int>(2 - nbits);        //  explicit cast to avoid underflow warning, TODO: is this correct?
		int maxpos_exponent = nbits - 2;
		double useed = (1 << (1 << es));
		std::cout << "useed : " << useed << " Minpos : " << pow(useed, minpos_exponent) << " Maxpos : " << pow(useed, maxpos_exponent) << std::endl;
	}

	int sign() const {
		return (bits[nbits - 1] ? -1 : 1);
	}

	// return regime value
	double regime() const {
		double regime;   // only works for posits smaller than 32bits
						 // useed is 2^(2^es) -> a left-shift of one by 2^es
						 // useed^k -> left-shift of k*2^es
		if (k == 0) return 0.0;
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
		return uint32_t(exp.to_ulong());
	}	

	uint32_t fraction() const {
		return uint32_t(frac.to_ulong());
	}

	// return run-length encoding
	int run_length() const {
		return k;
	}

	// return exponent bits
	std::bitset<es> exponent_bits() const {
		return exp;
	}

	// return fraction bits: nbits - (sign + regime + es)
	// sign is 1 bit, regime at least 2
	std::bitset<nbits - 3 - es> fraction_bits() const {
		return frac;
	}

	// identify the segments
	// precondition: member vars reset with bits containing the value to decode
	int16_t decode() {
		if (bits.none()) {  // special case = 0
			k = -(nbits-1);
			return k;
		}
		if (bits[nbits - 1] == true) {
			if (1 == bits.count()) {	// special case = +-inf
				k = (nbits - 1);
				return k;
			}
		}
		// sign(p)*useed^k*2^exp*fraction
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
		///////////////////////////                             cout << msb << " ";
		int32_t size = 0;
		if (msb >= 0) {	
			size = (msb >= es - 1 ? es : msb + 1);
			///////////////////// cout << " size " << size << " msb " << msb << " ";
			for (int i = 0; i < size; i++) {
				exp[i] = bits[msb - (size - 1) + i];
			}
		}

		/////////////////////////// cout << "fraction bits " << msb - size + 1 << endl;
		// finally, set the fraction bits
		msb = msb - size;
		if (msb >= 0) {
			for (int i = 0; i <= msb; i++) {
				frac[i] = bits[i];
			}
		}
		return k;
	}

	unsigned long to_ulong() {
		unsigned long value = bits.to_ulong();
		return value;
	}
	double to_double() {
		double value = 0.0;

		return value;
	}


private:
	std::bitset<nbits> bits;
	std::bitset<es> exp;
	std::bitset<nbits - 3 - es> frac;
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
