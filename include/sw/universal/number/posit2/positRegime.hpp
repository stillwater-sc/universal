#pragma once
// positRegime.hpp: definition of a posit positRegime
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

using namespace sw::universal::internal;

// Forward definitions
template<unsigned nbits, unsigned es, typename bt> constexpr int calculate_k(int);

// template class representing the positRegime using <nbits,es> of the containing posit
template<unsigned nbits, unsigned es, typename bt>
class positRegime {
	using RegimeBits = blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned>;
public:
	positRegime() : _block(), _k(0), _run(0), _nrRegimeBits(0) {}
	
	positRegime(const positRegime& r) = default;
	positRegime(positRegime&& r) = default;

	positRegime& operator=(const positRegime& r) = default;
	positRegime& operator=(positRegime&& r) = default;
	
	inline void reset() {
		_k = 0;
		_nrRegimeBits = 0;
		_block.clear();
	}
	inline size_t nrBits() const { return _nrRegimeBits;	}
	int scale() const {
		return _k > 0 ? int(_k) * (1 << es) : -(int(-_k) * (1 << es));
	}

	// return the k-value of the positRegime: useed ^ k
	inline int positRegime_k() const {
		return _k;
	}
	// the length of the run of the positRegime
	inline int positRegime_run() const {
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
	inline bool iszero() const { return _block.none(); }
	inline blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> bits() const { return _block; }
	void set(const blockbinary<nbits - 1, bt>& raw, unsigned nrOfRegimeBits) {
		_block = raw;
		_nrRegimeBits = nrOfRegimeBits;
	}
	void setzero() {
		_block.clear();
		_nrRegimeBits = nbits - 1;
		_k = 1 - static_cast<int>(nbits);   // by design: this simplifies increment/decrement
	}
	void setinf() {
		_block.clear();
		_nrRegimeBits = nbits - 1;
		_k = static_cast<int>(nbits) - 1;   // by design: this simplifies increment/decrement
	}
	// return the size of a regime encoding for a particular k value
	int regime_size(int k) const {
		if (k < 0) k = -k - 1;
		return (k < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
	}
	size_t assign(int scale) {
		bool r = scale > 0;
		_k = calculate_k<nbits,es,bt>(scale);
		_run = static_cast<unsigned>(r ? 1 + (scale >> es) : -scale >> es);
		r ? _block.set() : _block.clear();
		_block.set(nbits - 1 - _run - 1, 1 ^ r); // termination bit		
		_nrRegimeBits = _run + 1;
		return _nrRegimeBits;
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
			_block.clear();
			if (k < static_cast<int>(nbits) - 2) {	// _nrRegimeBits = (k < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
				_nrRegimeBits = static_cast<unsigned>(k + 2);
				_block.setbit(static_cast<int>(nbits) - 1 - _nrRegimeBits, true);   // set the run-length termination bit
			}
			else {
				_nrRegimeBits = static_cast<int>(nbits) - 1;
			}

		}
		else {       // north-east quadrant: patterns 11110---		
			_k = int(k < static_cast<int>(nbits) - 2 ? k : static_cast<int>(nbits) - 2); // constrain positRegime to maxpos
			_block.set();
			if (k < static_cast<int>(nbits) - 2) {	// _nrRegimeBits = (std::size_t(k) < static_cast<int>(nbits) - 2 ? k + 2 : nbits - 1);
				_nrRegimeBits = static_cast<unsigned>(k + 2);   
				_block.setbit(nbits - 1 - _nrRegimeBits, false);   // set the run-length termination bit
			}
			else {
				_nrRegimeBits = nbits - 1;
			}
		}
		return _nrRegimeBits;
	}
	bool increment() {
		if (_block.all()) return false; // rounding up/down as we are already at minpos/maxpos
		bool carry = increment_unsigned(_block,_nrRegimeBits);
		if (carry) {
			std::cout << "Regime needs to expand" << std::endl;
		}
		else {
			_k++;
		}
		return carry;
	}
private:
	RegimeBits 	_block;
	int			_k;
	unsigned	_run;
	unsigned	_nrRegimeBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const positRegime<nnbits, ees, bbt>& r);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::istream& operator>> (std::istream& istr, positRegime<nnbits, ees, bbt>& r);

	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const positRegime<nnbits, ees, bbt>& lhs, const positRegime<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const positRegime<nnbits, ees, bbt>& lhs, const positRegime<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const positRegime<nnbits, ees, bbt>& lhs, const positRegime<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const positRegime<nnbits, ees, bbt>& lhs, const positRegime<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const positRegime<nnbits, ees, bbt>& lhs, const positRegime<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const positRegime<nnbits, ees, bbt>& lhs, const positRegime<nnbits, ees, bbt>& rhs);
};

template<unsigned nbits, unsigned es, typename bt>
inline int scale(const positRegime<nbits, es, bt>& r) { return r.scale();  }

/////////////////  REGIME operators
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const positRegime<nbits, es, bt>& r) {
	blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> bb = r.bits();
	unsigned nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (r._nrRegimeBits > nrOfRegimeBitsProcessed++) {
			ostr << (bb.test(unsigned(i)) ? '1' : '0');
		}
		else {
			ostr << '-';
		}
	}
	return ostr;
}

template<unsigned nbits, unsigned es, typename bt>
inline std::istream& operator>> (std::istream& istr, const positRegime<nbits, es, bt>& r) {
	istr >> r._block;
	return istr;
}

template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const positRegime<nbits, es, bt>& r, bool dashExtent = true, bool nibbleMarker = false) {
	std::stringstream s;
	blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> bb = r.bits();
	unsigned nrOfRegimeBitsProcessed = 0;
	for (unsigned i = 0; i < nbits - 1; ++i) {
		unsigned bitIndex = nbits - 2ul - i;
		if (r.nrBits() > nrOfRegimeBitsProcessed++) {
			s << (bb.test(bitIndex) ? '1' : '0');
			if (nibbleMarker && ((bitIndex % 4) == 0) && bitIndex != 0) s << '\'';
		}
		else {
			s << (dashExtent ? "-" : "");
		}	
	}
	return s.str();
}

template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const positRegime<nbits, es, bt>& lhs, const positRegime<nbits, es, bt>& rhs) { return lhs._block == rhs._block && lhs._nrRegimeBits == rhs._nrRegimeBits; }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const positRegime<nbits, es, bt>& lhs, const positRegime<nbits, es, bt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const positRegime<nbits, es, bt>& lhs, const positRegime<nbits, es, bt>& rhs) { return lhs._nrRegimeBits == rhs._nrRegimeBits && lhs._block < rhs._block; }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const positRegime<nbits, es, bt>& lhs, const positRegime<nbits, es, bt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const positRegime<nbits, es, bt>& lhs, const positRegime<nbits, es, bt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const positRegime<nbits, es, bt>& lhs, const positRegime<nbits, es, bt>& rhs) { return !operator< (lhs, rhs); }

}} // namespace sw::universal

