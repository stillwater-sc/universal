#pragma once
// exponent.hpp: definition of a posit exponent
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


static constexpr int GEOMETRIC_ROUND_DOWN = -1;
static constexpr int ARITHMETIC_ROUNDING  =  0;
static constexpr int GEOMETRIC_ROUND_UP   =  1;

// exponent
template<size_t nbits, size_t es>
class exponent {
public:
	exponent() {
		reset();
	}
	
	exponent(const exponent& r) = default;
	exponent(exponent&& r) = default;

	exponent& operator=(const exponent& r) = default;
	exponent& operator=(exponent&& r) = default;
	
	
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
	// calculate the exponent given a number's scale and the number of regime bits, 
	// returning a flag that indicates if we need to geometrically round up
	int assign_exponent_bits(int scale, unsigned int nr_of_regime_bits) {
		int rounding_mode = ARITHMETIC_ROUNDING;
		int useed_scale = 1 << es;
		scale = scale < 0 ? -scale + useed_scale : scale;
		_Bits.reset();
		_NrOfBits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
		if (_NrOfBits > 0) {
			unsigned int my_exponent = (es > 0 ? scale % (1 << es) : 0);
			uint64_t mask = (uint64_t(1) << es) >> 1;  // work-around: (es - 1) can be negative, causing a compilation warning
			for (unsigned int i = 0; i < _NrOfBits; i++) {
				_Bits[es - 1 - i] = my_exponent & mask;
				mask >>= 1;
			}
			if (_NrOfBits < es) {
				rounding_mode = my_exponent & mask ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN; // check the next bit to see if we need to geometric round
			}
		}
		else {
			if (es > 0) {
				unsigned int my_exponent = scale % (1 << es);
				uint64_t mask = (uint64_t(1) << es) >> 1;  // work-around: (es - 1) can be negative, causing a compilation warning
				bool rounding_bit = my_exponent & mask;
				rounding_mode = rounding_bit ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN;
			}
		}
		return rounding_mode;
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
