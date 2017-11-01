#pragma once 
// posit_operators.hpp: definition of posit operators and helpers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <limits>

// ARITHMETIC OPERATORS
template<size_t nbits, size_t es>
inline posit<nbits,es> operator+(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> sum = lhs;
	sum += rhs;
	return sum;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator-(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> diff = lhs;
	diff -= rhs;
	return diff;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator*(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> mul = lhs;
	mul *= rhs;
	return mul;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator/(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

// COMPONENT operators

/////////////////  REGIME
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const regime<nbits, es>& r) {
	unsigned int nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (r._RegimeBits > nrOfRegimeBitsProcessed++) {
			ostr << (r._Bits[i] ? "1" : "0");
		}
		else {
			ostr << "-";
		}
	}
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, const regime<nbits, es>& r) {
	istr >> r._Bits;
	return istr;
}

template<size_t nbits, size_t es>
inline bool operator==(const regime<nbits, es>& lhs, const regime<nbits, es>& rhs) { return lhs._Bits == rhs._Bits && lhs._RegimeBits == rhs._RegimeBits; }
template<size_t nbits, size_t es>
inline bool operator!=(const regime<nbits, es>& lhs, const regime<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator< (const regime<nbits, es>& lhs, const regime<nbits, es>& rhs) { return lhs._RegimeBits == rhs._RegimeBits && lhs._Bits < rhs._Bits; }
template<size_t nbits, size_t es>
inline bool operator> (const regime<nbits, es>& lhs, const regime<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es>
inline bool operator<=(const regime<nbits, es>& lhs, const regime<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator>=(const regime<nbits, es>& lhs, const regime<nbits, es>& rhs) { return !operator< (lhs, rhs); }

/////////////////// EXPONENT
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const exponent<nbits, es>& e) {
	unsigned int nrOfExponentBitsProcessed = 0;
	for (int i = int(es) - 1; i >= 0; --i) {
		if (e._NrOfBits > nrOfExponentBitsProcessed++) {
			ostr << (e._Bits[i] ? "1" : "0");
		}
		else {
			ostr << "-";
		}	
	}
	if (nrOfExponentBitsProcessed == 0) ostr << "~"; // for proper alignment in tables
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, const exponent<nbits, es>& e) {
	istr >> e._Bits;
	return istr;
}

template<size_t nbits, size_t es>
inline bool operator==(const exponent<nbits, es>& lhs, const exponent<nbits, es>& rhs) { return lhs._Bits == rhs._Bits && lhs._NrOfBits == rhs._NrOfBits; }
template<size_t nbits, size_t es>
inline bool operator!=(const exponent<nbits, es>& lhs, const exponent<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator< (const exponent<nbits, es>& lhs, const exponent<nbits, es>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<size_t nbits, size_t es>
inline bool operator> (const exponent<nbits, es>& lhs, const exponent<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es>
inline bool operator<=(const exponent<nbits, es>& lhs, const exponent<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator>=(const exponent<nbits, es>& lhs, const exponent<nbits, es>& rhs) { return !operator< (lhs, rhs); }

////////////////////// FRACTION
template<size_t nfbits>
inline std::ostream& operator<<(std::ostream& ostr, const fraction<nfbits>& f) {
	unsigned int nrOfFractionBitsProcessed = 0;
	if (nfbits > 0) {
		for (int i = nfbits - 1; i >= 0; --i) {
			if (f._NrOfBits > nrOfFractionBitsProcessed++) {
				ostr << (f._Bits[i] ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	return ostr;
}

template<size_t nfbits>
inline std::istream& operator>> (std::istream& istr, const fraction<nfbits>& f) {
	istr >> f._Bits;
	return istr;
}

template<size_t nfbits>
inline bool operator==(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits == rhs._Bits; }
template<size_t nfbits>
inline bool operator!=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return !operator==(lhs, rhs); }
template<size_t nfbits>
inline bool operator< (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return lhs._NrOfBits <= rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<size_t nfbits>
inline bool operator> (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return  operator< (rhs, lhs); }
template<size_t nfbits>
inline bool operator<=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return !operator> (lhs, rhs); }
template<size_t nfbits>
inline bool operator>=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return !operator< (lhs, rhs); }

////////////////// POSIT
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const posit<nbits, es>& p) {
	if (p.isZero()) {
		ostr << double(0.0);
		return ostr;
	}
	else if (p.isInfinite()) {
		ostr << std::numeric_limits<double>::infinity();
		return ostr;
	}	
	ostr << p.to_double();
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, const posit<nbits, es>& p) {
	istr >> p._Bits;
	return istr;
}

template<size_t nbits, size_t es>
inline bool operator==(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return lhs._raw_bits == rhs._raw_bits; }
template<size_t nbits, size_t es>
inline bool operator!=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator< (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return lhs._raw_bits < rhs._raw_bits; }
template<size_t nbits, size_t es>
inline bool operator> (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es>
inline bool operator<=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator>=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator< (lhs, rhs); }


// value operators

////////////////////// VALUE
template<size_t nfbits>
inline std::ostream& operator<<(std::ostream& ostr, const value<nfbits>& v) {
	if (v._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << v.to_double();
	}
	return ostr;
}

template<size_t nfbits>
inline std::istream& operator>> (std::istream& istr, const value<nfbits>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nfbits>
inline bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits; }
template<size_t nfbits>
inline bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator==(lhs, rhs); }
template<size_t nfbits>
inline bool operator< (const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs.to_double() < rhs.to_double(); }
template<size_t nfbits>
inline bool operator> (const value<nfbits>& lhs, const value<nfbits>& rhs) { return  operator< (rhs, lhs); }
template<size_t nfbits>
inline bool operator<=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator> (lhs, rhs); }
template<size_t nfbits>
inline bool operator>=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator< (lhs, rhs); }
