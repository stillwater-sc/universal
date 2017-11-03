#pragma once
// regime.hpp: definition of a posit regime
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// template class representing the regime
template<size_t nbits, size_t es>
class regime {
public:
	regime() : _Bits(), _k(0), _RegimeBits(0) {}
	
	regime(const regime& r) = default;
	regime(regime&& r) = default;

	regime& operator=(const regime& r) = default;
	regime& operator=(regime&& r) = default;
	
	void reset() {
		_k = 0;
		_RegimeBits = 0;
		_Bits.reset();
	}
	unsigned int nrBits() const {
		return _RegimeBits;
	}
	int scale() const {
		return int(_k) << es;
	}
	// return the k-value of the regime: useed ^ k
	int regime_k() const {
		return _k;
	}
	double value() const {
		double scale;
		int e2 = (1 << es) * _k;
		if (e2 < -63 || e2 > 63) {
			scale = pow(2.0, e2);
		}
		else {
			if (e2 >= 0) {
				scale = double((uint64_t(1) << e2));
			}
			else {
				scale = double(1.0) / double(uint64_t(1) << -e2);
			}
		}
		return scale;
	}
	bool isZero() const {
		return _Bits.none();
	}
	std::bitset<nbits - 1> get() const {
		return _Bits;
	}
	void set(const std::bitset<nbits - 1>& raw, unsigned int nrOfRegimeBits) {
		_Bits = raw;
		_RegimeBits = nrOfRegimeBits;
	}
	void setZero() {
		_Bits.reset();
		_RegimeBits = nbits - 1;
		_k = 1 - static_cast<int>(nbits);   // by design: this simplifies increment/decrement
	}
	void setInfinite() {
		_Bits.reset();
		_RegimeBits = nbits - 1;
		_k = static_cast<int>(nbits) - 1;   // by design: this simplifies increment/decrement
	}
	// construct the regime bit pattern given a number's scale and returning the number of regime bits
	unsigned int assign_regime_pattern(bool sign, int k) {
		_Bits.reset();
		if (k < 0) {
			_k = int8_t(-k < nbits-2 ? k : -(static_cast<int>(nbits) - 2)); // constrain regime to minpos
			k = -_k - 1;
			uint64_t my_regime = REGIME_BITS[k];
			uint64_t mask = REGIME_BITS[0];
			_RegimeBits = (k < nbits - 2 ? k + 2 : nbits - 1);
			for (unsigned int i = 0; i < _RegimeBits; i++) {
				_Bits[nbits - 2 - i] = !(my_regime & mask);
				mask >>= 1;
			}

		}
		else {
			_k = int8_t(k < nbits - 2 ? k : nbits - 2); // constrain regime to maxpos
			uint64_t my_regime = REGIME_BITS[k];
			uint64_t mask = REGIME_BITS[0];
			_RegimeBits = (std::size_t(k) < nbits - 2 ? k + 2 : nbits - 1);
			for (unsigned int i = 0; i < _RegimeBits; i++) {
				_Bits[nbits - 2 - i] = my_regime & mask;
				mask >>= 1;
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
	std::bitset<nbits - 1>	_Bits;
	int8_t					_k;
	unsigned int			_RegimeBits;

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
