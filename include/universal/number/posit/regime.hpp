#pragma once
// regime.hpp: definition of a posit regime
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

using namespace sw::universal::internal;

// Forward definitions
template<size_t nbits, size_t es> constexpr int calculate_k(int);

// template class representing the regime using <nbits,es> of the containing posit
template<size_t nbits, size_t es>
class regime {
public:
	regime() : _Bits(), _k(0), _run(0), _RegimeBits(0) {}
	
	regime(const regime& r) = default;
	regime(regime&& r) = default;

	regime& operator=(const regime& r) = default;
	regime& operator=(regime&& r) = default;
	
	inline void reset() {
		_k = 0;
		_RegimeBits = 0;
		_Bits.reset();
	}
	inline size_t nrBits() const { return _RegimeBits;	}
	int scale() const {
		return _k > 0 ? int(_k) * (1 << es) : -(int(-_k) * (1 << es));
	}

	// return the k-value of the regime: useed ^ k
	inline int regime_k() const {
		return _k;
	}
	// the length of the run of the regime
	inline int regime_run() const {
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
				scale = 1.0l / (long double)(uint64_t(1) << -e2);
			}
		}
		return scale;
	}
	inline bool iszero() const { return _Bits.none(); }
	inline bitblock<nbits - 1> get() const { return _Bits;	}
	void set(const bitblock<nbits - 1>& raw, size_t nrOfRegimeBits) {
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
	size_t assign(int scale) {
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
	size_t assign_regime_pattern(int k) {
		if (k < 0) { // south-east quadrant: patterns 00001---
			_k = int(-k < (static_cast<int>(nbits) - 2) ? k : -(static_cast<int>(nbits) - 2)); // constrain regime to minpos
			k = -_k - 1;
			_Bits.reset();
			if (k < static_cast<int>(nbits) - 2) {	// _RegimeBits = (k < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
				_RegimeBits = size_t(k) + 2;
				_Bits.set(static_cast<int>(nbits) - 1 - _RegimeBits, true);   // set the run-length termination bit
			}
			else {
				_RegimeBits = static_cast<int>(nbits) - 1;
			}

		}
		else {       // north-east quadrant: patterns 11110---		
			_k = int(k < static_cast<int>(nbits) - 2 ? k : static_cast<int>(nbits) - 2); // constrain regime to maxpos
			_Bits.set();
			if (k < static_cast<int>(nbits) - 2) {	// _RegimeBits = (std::size_t(k) < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
				_RegimeBits = size_t(k) + 2;   
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
	size_t					_RegimeBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const regime<nnbits, ees>& r);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, regime<nnbits, ees>& r);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
};

template<size_t nbits, size_t es>
inline int scale(const regime<nbits, es>& r) { return r.scale();  }

/////////////////  REGIME operators
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const regime<nbits, es>& r) {
	size_t nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (r._RegimeBits > nrOfRegimeBitsProcessed++) {
			ostr << (r._Bits[size_t(i)] ? "1" : "0");
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
inline std::string to_string(const regime<nbits, es>& r, bool dashExtent = true, bool nibbleMarker = false) {
	std::stringstream sstr;
	bitblock<nbits - 1> bb = r.get();
	size_t nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (r.nrBits() > nrOfRegimeBitsProcessed++) {
			sstr << (bb[size_t(i)] ? '1' : '0');
			if (nibbleMarker && ((i % 4) == 0) && i != 0) sstr << '\'';
		}
		else {
			sstr << (dashExtent ? "-" : "");
		}	
	}
	return sstr.str();
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

}} // namespace sw::universal

