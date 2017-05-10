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
	int k;   // will contain the k value
	if (p.isZero()) {
		ostr << "zero";
	}
	else if (p.isInfinite()) {
		ostr << "inf";
	}
	else {
		// sign(p)*useed^k*2^exp*fraction
		// let k be the number of identical bits in the regime
		if (p.bits[nbits-1] == 1) {
			k = 0;   // if a run of 1's k = m - 1
			for (int i = nbits - 2; i > 0; --i) {
				if (p.bits[i] == 1) {
					k++;
				}
				else {
					break;
				}
			}
		}
		else {
			k = -1;  // if a run of 0's k = -m
			for (int i = nbits - 2; i > 0; --i) {
				if (p.bits[i] == 0) {
					k--;
				}
				else {
					break;
				}
			}
		}
	}
	int64_t value;
	uint64_t scale;
    if (k > 0) {
		scale = SCALE_FACTORS[es][k];
	}
	else {
			ostr << "Between 0 and 1 region not implemented yet";
	}
	uint64_t fraction = p.bits.to_ulong();
	if (p.bits[nbits - 1]) {
		value = -(int64_t)(scale * fraction);
	}
	else {
		value = scale * fraction;
	}
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
