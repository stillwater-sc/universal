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


template<size_t nbits, size_t es> class posit {
public:
	posit<nbits, es>() {
		useed = 1 << (1 << es);
	}
	posit<nbits, es>& operator=(const char& rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(const int& rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(const long& rhs) {
		*this = (long long)(rhs);
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
	// test function:
	void set(std::bitset<nbits> raw) {
		bits = raw;
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
		std::cout << "useed : " << useed << " Minpos : " << pow(useed, minpos_exponent) << " Maxpos : " << pow(useed, maxpos_exponent) << std::endl;
	}

	// Set the raw bits of the posit
	void SetBits(unsigned long value) {
		unsigned long mask = 1;
		for ( int i = 0; i < nbits; i++ ) {
			bits.set(i,(value & mask));
			mask = mask << 1;
		}
		useed = (1 << (1 << es));
	}

	unsigned long to_ulong() {
		unsigned long value = bits.to_ulong();
		return value;
	}

private:
	std::bitset<nbits> bits;
	std::uint64_t useed;

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

	// identify the regime bits
	int identifyRegime() const {
		int k = 0;
		// sign(p)*useed^k*2^exp*fraction
		// let k be the number of identical bits in the regime
		if (bits[nbits - 2] == 1) {
			k = 0;   // if a run of 1's k = m - 1
			for (int i = nbits - 3; i >= 0; --i) {
				if (bits[i] == 1) {
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
				if (bits[i] == 0) {
					k--;
				}
				else {
					break;
				}
			}
		}
		return k;
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
