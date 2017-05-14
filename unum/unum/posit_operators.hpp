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
	// determine the value of the posit
	int k = 0;   // will contain the k value
	if (p.isZero()) {
		ostr << "zero";
		return ostr;
	}
	else if (p.isInfinite()) {
		ostr << "inf";
		return ostr;
	}
	else {
		k = p.identifyRegime();
	}
	uint16_t regime = 0;
	for (int i = 0; i < k; i++) {
		regime += (p.bits[nbits - 1 - i]) << i;
	}

	// extract the exponent information
	bitset<es> e = 0;
	uint64_t scale;
    if (k >= 0) {
		scale = SCALE_FACTORS[es][k];

		for (int i = 0; i < es; i++) {
			e[es-i-1] = (p.bits[nbits - 1 - k - i]);
		}
	}
	else {
		scale = SCALE_FACTORS[es][k];
	}

	uint64_t fraction = p.bits.to_ulong();
	int64_t value;
	if (p.bits[nbits - 1]) {
		value = -(int64_t)(p.useed * fraction);
	}
	else {
		value = p.useed * fraction;
	}
	ostr << "Sign : " << p.bits[nbits-1]  << " Regime : " << setw(2) << k << " Regime Bits: 0X" << hex << regime << dec << " Exponent : " << e << " Fraction : b" << p.bits;
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
