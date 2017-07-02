#pragma once 

#include "posit_scale_factors.hpp"

template<size_t nbits>
std::string to_binary(std::bitset<nbits> bits) {
	std::stringstream ss;
	for (int i = nbits - 1; i >= 0; --i) {
		if (bits[i]) {
			ss << "1";
		}
		else {
			ss << "0";
		}
	}
	return ss.str();
}

template<size_t nbits, size_t es>
inline posit<nbits,es> operator+(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs += rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator-(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs -= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator*(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs *= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator/(posit<nbits, es> lhs, const posit<nbits, es>& rhs) {
	lhs /= rhs;
	return lhs;
}

template<size_t nbits, size_t es>
inline std::ostream& operator<< (std::ostream& ostr, const posit<nbits, es>& p) {
        using namespace std;
	// determine the value of the posit
	int k = 0;   // will contain the k value
	if (p.isZero()) {
		ostr << " zero    " << setw(103) << "b" << p.bits;
		return ostr;
	}
	else if (p.isInfinite()) {
		ostr << " infinite" << setw(103) << "b" << p.bits;
		return ostr;
	}
	
	ostr << setw(14) << to_binary(p.get_raw_bits()) 
		<< " Sign : " << setw(2) << p.sign()  
		<< " Regime : " << setw(3) << p.run_length() 
		<< " Exponent : " << setw(5) << p.exponent() 
		<< " Fraction : " << setw(8) << setprecision(7) << p.fraction() 
		<< " Value : " << setw(16) << p.to_double()
		<< setprecision(0);
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator >> (std::istream& istr, const posit<nbits, es>& p) {
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
