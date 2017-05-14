#pragma once

#define POW2(n) (1 << (n))
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
		this->bits = rhs;
		return *this;
	}
	posit<nbits, es>& operator=(const int& rhs) {
		bits.reset();
		int base = findBaseExponent(rhs);
		if (rhs >= 0) {
			// calculate regime and exponent bits
			if (base == 0) {
				// es bits are all 0
				// regime bits are a run length of k = 0 -> 10
				bits[nbits - 1] = 0;  // sign bit
				bits[nbits - 2] = 1;  // first regime bit
				bits[nbits - 3] = 0;  // second regime bit
				bits[nbits - 4] = 0;  // first exponent bit
			}
		}
		else {
			cerr << "Negative regime not implemented yet" << endl;
		}

		this->bits = rhs;
		return *this;
	}
	posit<nbits, es>& operator=(const long& rhs) {
		this->bits = rhs;
		return *this;
	}
	posit<nbits, es>& operator=(const long long& rhs) {
		this->bits = rhs;
		return *this;
	}
	posit<nbits, es>& operator=(const float& rhs) {
		switch (fpclassify(*rhs)) {
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
			extractIEEE754((uint64_t)*rhs, 8, 23);
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

	bool isInfinite() const {
		// +-infinite is a bit string of a sign bit of 1 followed by all 0s
		std::bitset<nbits> tmp(bits << 1);
		//std::cout << bits << " " << tmp << std::endl;
		return bits[nbits-1] && tmp.any();
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
		int minpos_exponent = 2 - nbits;
		int maxpos_exponent = nbits - 2;
		std::cout << "useed : " << useed << " Minpos : " << pow(useed, minpos_exponent) << " Maxpos : " << pow(useed, maxpos_exponent) << std::endl;
	}
private:
	std::uint8_t fs;
	std::bitset<nbits> bits;
	std::uint64_t useed;

	int findBaseExponent(uint64_t number) {
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

	template<size_t nbits, size_t es>
	friend std::ostream& operator<< (std::ostream& ostr, const posit<nbits, es>& p);
	template<size_t nbits, size_t es>
	friend std::istream& operator>> (std::istream& istr, posit<nbits, es>& p);

	template<size_t nbits, size_t es>
	friend bool operator==(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator!=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator< (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator> (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator<=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator>=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs);
};
