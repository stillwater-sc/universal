#pragma once
// exponent.hpp: definition of a posit exponent
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// exponent
template<size_t nbits, size_t es>
class exponent {
public:
	exponent() {
		reset();
	}
	exponent(const exponent& e) {
		_Bits = e._Bits;
		_NrOfBits = e._NrOfBits;
	}
	exponent& operator=(const exponent& e) {
		_Bits = e._Bits;
		_NrOfBits = e._NrOfBits;
		return *this;
	}
	void reset() {
		_NrOfBits = 0;
		_Bits.reset();
	}
	unsigned int nrBits() const {
		return _NrOfBits;
	}
	int scale() const {
		return _Bits.to_ulong();
	}
	double value() const {
		return double(uint64_t(1) << scale());
	}
	std::bitset<es> get() const {
		return _Bits;
	}
	void set(const std::bitset<es>& raw, int nrOfExponentBits) {
		_Bits = raw;
		_NrOfBits = nrOfExponentBits;
	}
	// calculate the exponent given a number's scale and the number of regime bits, returning the number of exponent bits assigned
	unsigned int assign_exponent_bits(unsigned int msb, unsigned int nr_of_regime_bits) {
		_Bits.reset();
		_NrOfBits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
		if (_NrOfBits > 0) {
			unsigned int my_exponent = (es > 0 ? msb % (1 << es) : 0);
			uint64_t mask = (uint64_t(1) << es) >> 1;  // (es - 1) can be negative, causing a compilation warning
			for (unsigned int i = 0; i < _NrOfBits; i++) {
				_Bits[es - 1 - i] = my_exponent & mask;
				mask >>= 1;
			}
		}
		return _NrOfBits;
	}
	bool increment() {
		return increment_unsigned(_Bits, _NrOfBits);
	}
private:
	std::bitset<es> _Bits;
	unsigned int	_NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const exponent<nnbits, ees>& e);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, exponent<nnbits, ees>& e);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
};
