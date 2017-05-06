#pragma once

template<size_t nbits, size_t es> class posit {
public:
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
		this->bits += rhs.bits;
		return *this;
	}
	posit<nbits, es>& operator-=(const posit& rhs) {
		this->bits += rhs.bits;
		return *this;
	}
	posit<nbits, es>& operator*=(const posit& rhs) {
		this->bits *= rhs.bits;
		return *this;
	}
	posit<nbits, es>& operator/=(const posit& rhs) {
		this->bits /= rhs.bits;
		return *this;
	}
	posit<nbits, es>& operator++() {
		bits++;
		return *this;
	}
	posit<nbits, es> operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit<nbits, es>& operator--() {
		bits--;
		return *this;
	}
	posit<nbits, es> operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}
private:
	std::uint8_t sign, nbits;
	std::uint32_t es, bits;

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
