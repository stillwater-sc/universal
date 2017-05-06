#pragma once

template<size_t nbits, size_t es> class Posit {
public:
	Posit<nbits, es>& operator=(const int& rhs) {
		this->bits = rhs;
		return *this;
	}
	Posit<nbits, es>& operator=(const float& rhs) {
		return *this;
	}
	Posit<nbits, es>& operator=(const double& rhs) {
		return *this;
	}
	Posit<nbits, es>& operator+=(const Posit& rhs) {
		this->bits += rhs.bits;
		return *this;
	}
	Posit<nbits, es>& operator-=(const Posit& rhs) {
		this->bits += rhs.bits;
		return *this;
	}
	Posit<nbits, es>& operator*=(const Posit& rhs) {
		this->bits *= rhs.bits;
		return *this;
	}
	Posit<nbits, es>& operator/=(const Posit& rhs) {
		this->bits /= rhs.bits;
		return *this;
	}
	Posit<nbits, es>& operator++() {
		bits++;
		return *this;
	}
	Posit<nbits, es> operator++(int) {
		Posit tmp(*this);
		operator++();
		return tmp;
	}
	Posit<nbits, es>& operator--() {
		bits--;
		return *this;
	}
	Posit<nbits, es> operator--(int) {
		Posit tmp(*this);
		operator--();
		return tmp;
	}
private:
	std::uint8_t sign, nbits;
	std::uint32_t es, bits;

	template<size_t nbits, size_t es>
	friend std::ostream& operator<< (std::ostream& ostr, const Posit<nbits, es>& p);
	template<size_t nbits, size_t es>
	friend std::istream& operator>> (std::istream& istr, Posit<nbits, es>& p);

	template<size_t nbits, size_t es>
	friend bool operator==(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator!=(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator< (const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator> (const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator<=(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs);
	template<size_t nbits, size_t es>
	friend bool operator>=(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs);
};
