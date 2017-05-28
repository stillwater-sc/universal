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
		ostr << " zero    " << setw(103) << "b" << p.bits;
		return ostr;
	}
	else if (p.isInfinite()) {
		ostr << " infinite" << setw(103) << "b" << p.bits;
		return ostr;
	}
	else {
		k = p.identifyRegime();
	}
	// transform into run length of the regime bits
	int m;
	if (k < 0) {
		m = 1 - k;
	}
	else {
		m = k + 2;
		if (m == nbits) {
			m--;  // special case of a full run of 1's or 0's
		}
	}
	uint16_t regime = 0;
	for (int i = 0; i < m; i++) {
		regime += (p.bits[nbits - 1 - i]) << i;
	}

	// extract the exponent and fraction information
	bitset<es> e; e.reset();
	bitset<nbits - es - 1> fraction; fraction.reset();	// needs to be specified in template const ints
	
	int msbExp = nbits - 2 - m;
	if (msbExp >= 0) {
		// extract exponent bits
		int lsbExp = msbExp - es;
		if (lsbExp <= 0) {
			for (int i = 0; i < msbExp; i++) {
				e[i] = p.bits[i];
			}
		}
		else {
			for (int i = 0; i < es; i++) {
				e[i] = p.bits[lsbExp + i];
			}
		}
		int msbFraction = nbits - 2 - m - es;
		if (msbFraction >= 0) {
			// extract the fraction bits
			for (int i = 0; i < msbFraction; i++) {
				fraction[i] = p.bits[i];
			}
		}
		else {
			// no fraction bits left
		}
	}
	else {
		// no exponent bits left, which implies no fraction bits either
	}

	uint64_t scale;
    if (k >= 0) {
		scale = SCALE_FACTORS[es][k];
	}
	else {
		scale = SCALE_FACTORS[es][k];
	}

	int64_t value;
	if (p.bits[nbits - 1]) {
		value = -(int64_t)(scale * (p.useed + fraction.to_ullong()) >> POW2(es));
	}
	else {
		value = scale * (p.useed + fraction.to_ullong())>>POW2(es);
	}
	ostr << setw(14) << value << " Sign : " << p.bits[nbits-1]  << " Regime : " << setw(3) << k << " Regime Bits: 0X" << hex << setw(4) << regime << dec << " Exponent : " << setw(5) << e << " Fraction : b" << fraction << " Raw Bits : b" << p.bits;
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
