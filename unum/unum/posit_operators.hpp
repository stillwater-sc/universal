#pragma once 


template<size_t region, size_t exponent, size_t fraction>
inline Posit<region,exponent,fraction>& operator+(Posit<region, exponent, fraction> lhs, const Posit<region, exponent, fraction>& rhs) {
	lhs += rhs;
	return lhs;
}

template<size_t region, size_t exponent, size_t fraction>
inline Posit<region, exponent, fraction>& operator-(Posit<region, exponent, fraction> lhs, const Posit<region, exponent, fraction>& rhs) {
	lhs -= rhs;
	return lhs;
}

template<size_t region, size_t exponent, size_t fraction>
inline Posit<region, exponent, fraction>& operator*(Posit<region, exponent, fraction> lhs, const Posit<region, exponent, fraction>& rhs) {
	lhs *= rhs;
	return lhs;
}

template<size_t region, size_t exponent, size_t fraction>
inline Posit<region, exponent, fraction>& operator/(Posit<region, exponent, fraction> lhs, const Posit<region, exponent, fraction>& rhs) {
	lhs /= rhs;
	return lhs;
}

template<size_t region, size_t exponent, size_t fraction>
inline std::ostream& operator<< (std::ostream& ostr, const Posit<region, exponent, fraction>& p) {
	ostr << p.bits;
	return ostr;
}

template<size_t region, size_t exponent, size_t fraction>
inline std::istream& operator >> (std::istream& istr, Posit<region, exponent, fraction>& p) {
	istr >> p.bits;
	return istr;
}

template<size_t region, size_t exponent, size_t fraction>
inline bool operator==(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs) { return lhs.bits == rhs.bits; }
template<size_t region, size_t exponent, size_t fraction>
inline bool operator!=(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs) { return !operator==(lhs, rhs); }
template<size_t region, size_t exponent, size_t fraction>
inline bool operator< (const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs) { return lhs.bits < rhs.bits; }
template<size_t region, size_t exponent, size_t fraction>
inline bool operator> (const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs) { return  operator< (rhs, lhs); }
template<size_t region, size_t exponent, size_t fraction>
inline bool operator<=(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs) { return !operator> (lhs, rhs); }
template<size_t region, size_t exponent, size_t fraction>
inline bool operator>=(const Posit<region, exponent, fraction>& lhs, const Posit<region, exponent, fraction>& rhs) { return !operator< (lhs, rhs); }
