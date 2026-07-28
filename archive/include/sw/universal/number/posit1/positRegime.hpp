#pragma once
// positRegime.hpp: definition of a posit positRegime
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>

namespace sw { namespace universal {

using namespace sw::universal::internal;

// template class representing the positRegime using <nbits,es> of the containing posit
template<unsigned nbits, unsigned es>
class positRegime {
public:
	positRegime() : _Bits(), _k(0), _run(0), _RegimeBits(0) {}
	
	positRegime(const positRegime& r) = default;
	positRegime(positRegime&& r) = default;

	positRegime& operator=(const positRegime& r) = default;
	positRegime& operator=(positRegime&& r) = default;
	
	inline void reset() {
		_k = 0;
		_RegimeBits = 0;
		_Bits.reset();
	}
	inline unsigned nrBits() const { return _RegimeBits;	}
	int scale() const {
		return _k > 0 ? int(_k) * (1 << es) : -(int(-_k) * (1 << es));
	}

	// return the k-value of the positRegime: useed ^ k
	inline int regime_k() const {
		return _k;
	}
	// the length of the run of the positRegime
	inline int regime_runlength() const {
		return _run;
	}
	long double value() const {
		long double scale;
		int e2 = (1 << es) * _k;
		if (e2 < -63 || e2 > 63) {
			scale = std::pow(2.0l, (long double)e2);  // TODO: needs a native posit implementation
		}
		else {
			if (e2 >= 0) {
				scale = (long double)((uint64_t(1) << e2));
			}
			else {
				scale = std::ldexp(1.0l, e2);
			}
		}
		return scale;
	}
	inline bool iszero() const { return _Bits.none(); }
	inline bitblock<nbits - 1> get() const { return _Bits;	}
	void set(const bitblock<nbits - 1>& raw, unsigned nrOfRegimeBits) {
		_Bits = raw;
		_RegimeBits = nrOfRegimeBits;
	}
	void setzero() {
		_Bits.reset();
		_RegimeBits = nbits - 1;
		_k = 1 - static_cast<int>(nbits);   // by design: this simplifies increment/decrement
	}
	void setinf() {
		_Bits.reset();
		_RegimeBits = nbits - 1;
		_k = static_cast<int>(nbits) - 1;   // by design: this simplifies increment/decrement
	}
	// return the size of a regime encoding for a particular k value
	int regime_size(int k) const {
		if (k < 0) k = -k - 1;
		return (k < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
	}
	unsigned assign(int scale) {
		bool r = scale > 0;
		_k = calculate_k<nbits,es>(scale);
		_run = static_cast<unsigned>(r ? 1 + (scale >> es) : -scale >> es);
		r ? _Bits.set() : _Bits.reset();
		_Bits.set(nbits - 1 - _run - 1, 1 ^ r); // termination bit		
		_RegimeBits = _run + 1;
		return _RegimeBits;
	}
	// construct the regime bit pattern given a number's useed scale, that is, k represents the useed factors of the number
	// k is the unifying abstraction between decoding a posit and converting a float value.
	// Return the number of regime bits. 
	// Usage example: say value is 1024 -> sign = false (not negative), scale is 10: assign_regime_pattern(scale >> es)
	// because useed = 2^es and thus a value of scale 'scale' will contain (scale >> es) number of useed factors
	unsigned assign_regime_pattern(int k) {
		if (k < 0) { // south-east quadrant: patterns 00001---
			_k = int(-k < (static_cast<int>(nbits) - 2) ? k : -(static_cast<int>(nbits) - 2)); // constrain positRegime to minpos
			k = -_k - 1;
			_Bits.reset();
			if (k < static_cast<int>(nbits) - 2) {	// _RegimeBits = (k < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
				_RegimeBits = unsigned(k) + 2;
				_Bits.set(static_cast<int>(nbits) - 1 - _RegimeBits, true);   // set the run-length termination bit
			}
			else {
				_RegimeBits = static_cast<int>(nbits) - 1;
			}

		}
		else {       // north-east quadrant: patterns 11110---		
			_k = int(k < static_cast<int>(nbits) - 2 ? k : static_cast<int>(nbits) - 2); // constrain positRegime to maxpos
			_Bits.set();
			if (k < static_cast<int>(nbits) - 2) {	// _RegimeBits = (std::unsigned(k) < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
				_RegimeBits = unsigned(k) + 2;   
				_Bits.set(nbits - 1 - _RegimeBits, false);   // set the run-length termination bit
			}
			else {
				_RegimeBits = nbits - 1;
			}
		}
		return _RegimeBits;
	}
	bool increment() {
		if (_Bits.all()) return false; // rounding up/down as we are already at minpos/maxpos
		bool carry = increment_unsigned(_Bits,_RegimeBits);
		if (carry) {
			std::cout << "Regime needs to expand" << std::endl;
		}
		else {
			_k++;
		}
		return carry;
	}
private:
	bitblock<nbits - 1>  	_Bits;
	int						_k;
	unsigned				_run;
	unsigned				_RegimeBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned ees>
	friend std::ostream& operator<< (std::ostream& ostr, const positRegime<nnbits, ees>& r);
	template<unsigned nnbits, unsigned ees>
	friend std::istream& operator>> (std::istream& istr, positRegime<nnbits, ees>& r);

	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const positRegime<nnbits, ees>& lhs, const positRegime<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const positRegime<nnbits, ees>& lhs, const positRegime<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const positRegime<nnbits, ees>& lhs, const positRegime<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const positRegime<nnbits, ees>& lhs, const positRegime<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const positRegime<nnbits, ees>& lhs, const positRegime<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const positRegime<nnbits, ees>& lhs, const positRegime<nnbits, ees>& rhs);
};

template<unsigned nbits, unsigned es>
inline int scale(const positRegime<nbits, es>& r) { return r.scale();  }

/////////////////  REGIME operators
template<unsigned nbits, unsigned es>
inline std::ostream& operator<<(std::ostream& ostr, const positRegime<nbits, es>& r) {
	unsigned nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (r._RegimeBits > nrOfRegimeBitsProcessed++) {
			ostr << (r._Bits[unsigned(i)] ? "1" : "0");
		}
		else {
			ostr << "-";
		}
	}
	return ostr;
}

template<unsigned nbits, unsigned es>
inline std::istream& operator>> (std::istream& istr, const positRegime<nbits, es>& r) {
	istr >> r._Bits;
	return istr;
}

template<unsigned nbits, unsigned es>
inline std::string to_string(const positRegime<nbits, es>& r, bool dashExtent = true, bool nibbleMarker = false) {
	std::stringstream sstr;
	bitblock<nbits - 1> bb = r.get();
	unsigned rbits = r.nrBits();
	unsigned nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (r.nrBits() > nrOfRegimeBitsProcessed++) {
			sstr << (bb[unsigned(i)] ? '1' : '0');
			--rbits;
			if (nibbleMarker && rbits != 0 && (rbits % 4) == 0) sstr << '\'';
		}
		else {
			sstr << (dashExtent ? "-" : "");
		}	
	}
	return sstr.str();
}

template<unsigned nbits, unsigned es>
inline bool operator==(const positRegime<nbits, es>& lhs, const positRegime<nbits, es>& rhs) { return lhs._Bits == rhs._Bits && lhs._RegimeBits == rhs._RegimeBits; }
template<unsigned nbits, unsigned es>
inline bool operator!=(const positRegime<nbits, es>& lhs, const positRegime<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nbits, unsigned es>
inline bool operator< (const positRegime<nbits, es>& lhs, const positRegime<nbits, es>& rhs) { return lhs._RegimeBits == rhs._RegimeBits && lhs._Bits < rhs._Bits; }
template<unsigned nbits, unsigned es>
inline bool operator> (const positRegime<nbits, es>& lhs, const positRegime<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nbits, unsigned es>
inline bool operator<=(const positRegime<nbits, es>& lhs, const positRegime<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nbits, unsigned es>
inline bool operator>=(const positRegime<nbits, es>& lhs, const positRegime<nbits, es>& rhs) { return !operator< (lhs, rhs); }

}} // namespace sw::universal

