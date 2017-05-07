#pragma once

#define SHIFT(n) (1 << (n))
#ifndef MIN
#define MIN(a,b) (a) < (b) ? (a) : (b)
#endif
#ifndef MAX
#define MAX(a,b) (a) > (b) ? (a) : (b)
#endif

template<size_t nbits, size_t es> class posit {
public:
	posit<nbits, es>() {
		useed = 1 << (1 << es);
	}
	posit<nbits, es>& operator=(const int& rhs) {
		this->bits = rhs;
		return *this;
	}
	posit<nbits, es>& operator=(const float& rhs) {
		return *this;
	}
	posit<nbits, es>& operator=(const double& rhs) {
		return *this;
	}
	posit<nbits, es>& operator+=(const posit& rhs) {
		// add rhs             this->bits += rhs.bits;
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

	bool isInfinite() {
		// +-infinite is a bit string of a sign bit of 1 followed by all 0s
		std::bitset<nbits> tmp(bits << 1);
		std::cout << bits << " " << tmp << std::endl;
		return bits[nbits-1] && tmp.any();
	}
	bool isZero() {
		// 0 is a bit string of all 0s
		return !bits.any();
	}
	bool isNegative() {
		return bits[nbits-1];
	}
	bool isPositive() {
		return !isNegative();
	}
	void Info()  {
		std::cout << "useed : " << useed << " Minpos : " << pow(useed, 2 - nbits) << " Maxpos : " << pow(useed, nbits - 2) << std::endl;
	}
private:
	std::uint8_t fs;
	std::bitset<nbits> bits;
	std::uint64_t useed;

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
