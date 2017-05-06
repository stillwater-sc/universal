#pragma once

template<size_t region, size_t exponent, size_t fraction> class Posit {
public:
	Posit<region, exponent, fraction>& operator=(const int& rhs) {
		this->bits = rhs;
		return *this;
	}
	Posit<region, exponent, fraction>& operator=(const float& rhs) {
		return *this;
	}
	Posit<region, exponent, fraction>& operator=(const double& rhs) {
		return *this;
	}
	Posit<region, exponent, fraction>& operator+=(const Posit& rhs) {
		this->bits += rhs.bits;
		return *this;
	}
	Posit<region, exponent, fraction>& operator-=(const Posit& rhs) {
		this->bits += rhs.bits;
		return *this;
	}
	Posit<region, exponent, fraction>& operator*=(const Posit& rhs) {
		this->bits *= rhs.bits;
		return *this;
	}
	Posit<region, exponent, fraction>& operator/=(const Posit& rhs) {
		this->bits /= rhs.bits;
		return *this;
	}
	Posit<region, exponent, fraction>& operator++() {
		bits++;
		return *this;
	}
	Posit<region, exponent, fraction> operator++(int) {
		Posit tmp(*this);
		operator++();
		return tmp;
	}
	Posit<region, exponent, fraction>& operator--() {
		bits--;
		return *this;
	}
	Posit<region, exponent, fraction> operator--(int) {
		Posit tmp(*this);
		operator--();
		return tmp;
	}
private:
	std::uint8_t sign, region;
	std::uint32_t exponent, fraction, bits;

	template<size_t region, size_t exponent, size_t fraction>
	friend std::ostream& operator<< (std::ostream& ostr, const Posit<region, exponent, fraction>& p);
	template<size_t region, size_t exponent, size_t fraction>
	friend std::istream& operator>> (std::istream& istr, Posit<region, exponent, fraction>& p);

	template<size_t region, size_t exponent, size_t fraction>
	friend bool operator==(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs);
	template<size_t region, size_t exponent, size_t fraction>
	friend bool operator!=(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs);
	template<size_t region, size_t exponent, size_t fraction>
	friend bool operator< (const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs);
	template<size_t region, size_t exponent, size_t fraction>
	friend bool operator> (const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs);
	template<size_t region, size_t exponent, size_t fraction>
	friend bool operator<=(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs);
	template<size_t region, size_t exponent, size_t fraction>
	friend bool operator>=(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs);
};
