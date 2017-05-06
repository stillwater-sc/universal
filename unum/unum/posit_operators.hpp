#pragma once 


template<size_t nbits, size_t es>
inline Posit<nbits,es>& operator+(Posit<nbits, es> lhs, const Posit<nbits, es>& rhs) {
	lhs += rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline Posit<nbits, es>& operator-(Posit<nbits, es> lhs, const Posit<nbits, es>& rhs) {
	lhs -= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline Posit<nbits, es>& operator*(Posit<nbits, es> lhs, const Posit<nbits, es>& rhs) {
	lhs *= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline Posit<nbits, es>& operator/(Posit<nbits, es> lhs, const Posit<nbits, es>& rhs) {
	lhs /= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline std::ostream& operator<< (std::ostream& ostr, const Posit<nbits, es>& p) {
	ostr << p.bits;
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator >> (std::istream& istr, Posit<nbits, es>& p) {
	istr >> p.bits;
	return istr;
}

template<size_t nbits, size_t es>
inline bool operator==(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs) { return lhs.bits == rhs.bits; }
template<size_t nbits, size_t es>
inline bool operator!=(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator< (const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs) { return lhs.bits < rhs.bits; }
template<size_t nbits, size_t es>
inline bool operator> (const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es>
inline bool operator<=(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator>=(const Posit<nbits, es>& lhs, const Posit<nbits, es>& rhs) { return !operator< (lhs, rhs); }
