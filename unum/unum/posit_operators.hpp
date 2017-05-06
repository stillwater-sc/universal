#pragma once 


template<size_t nbits, size_t es>
inline posit<nbits,es>& operator+(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs += rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline posit<nbits, es>& operator-(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs -= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline posit<nbits, es>& operator*(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs *= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline posit<nbits, es>& operator/(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs /= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline std::ostream& operator<< (std::ostream& ostr, const posit<nbits, es>& p) {
	ostr << p.bits;
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator >> (std::istream& istr, posit<nbits, es>& p) {
	istr >> p.bits;
	return istr;
}

template<size_t nbits, size_t es>
inline bool operator==(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return lhs.bits == rhs.bits; }
template<size_t nbits, size_t es>
inline bool operator!=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator< (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return lhs.bits < rhs.bits; }
template<size_t nbits, size_t es>
inline bool operator> (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es>
inline bool operator<=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator>=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator< (lhs, rhs); }
