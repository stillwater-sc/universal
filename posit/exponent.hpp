#pragma once
// exponent.hpp: definition of a posit exponent
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


static constexpr int GEOMETRIC_ROUND_DOWN   = -2;
static constexpr int ARITHMETIC_ROUND_DOWN  = -1;
static constexpr int NO_ADDITIONAL_ROUNDING =  0;
static constexpr int ARITHMETIC_ROUND_UP    =  1;
static constexpr int GEOMETRIC_ROUND_UP     =  2;
static constexpr int ARITHMETIC_ROUNDING    =  5;

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
	// returning an indicator which type of rounding is required to complete the posit
	int assign_exponent_bits(int scale, int k, unsigned int nr_of_regime_bits) {
		int rounding_mode = NO_ADDITIONAL_ROUNDING;
		_Bits.reset();
		// we need to get to an adjusted scale that encodes regime and exponent
		// value scale = useed ^ k * 2 ^ exponent = 2^(k*2^es) * 2^e -> k*2^es + e
		// e = scale - k*2^es
		int raw = scale - k*(1 << es);
		unsigned int my_exponent = raw < 0 ? -raw : raw;
		// convert value into bitset
		uint64_t mask = uint64_t(1);
		for (unsigned i = 0; i < es; i++) {
			_Bits[i] = my_exponent & mask;
			mask <<= 1;
		}
		_NrOfBits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
		if (_NrOfBits > 0) {
			if (_NrOfBits < es) {
				rounding_mode = _Bits[es - 1 - _NrOfBits] ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN; // check the next bit to see if we need to geometric round
				std::cout << "truncated exp" << (rounding_mode == GEOMETRIC_ROUND_UP ? " geo-up " : " geo-dw ");
			}
			else {
				if (nbits - 1 - nr_of_regime_bits - es > 0) {
					// use the fraction to determine rounding as this posit has fraction bits
					rounding_mode = ARITHMETIC_ROUNDING;
					std::cout << "arithmetic  rounding ";

				}
				else {
					// this posit is in the geometric regime and has consumed all the bits
					rounding_mode = ARITHMETIC_ROUNDING; //  NO_ADDITIONAL_ROUNDING;
					std::cout << "no rounding alltaken ";
				}
			}
		}
		else {
			// we ran out of bits
			if (es > 0) {
				rounding_mode = _Bits[es-1] ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN;
				std::cout << "no exp left: " << (rounding_mode == GEOMETRIC_ROUND_UP ? " geo-up " : " geo-dw ");
			}
			else {
				// this posit doesn't have an exponent field, 
				// so we need to look at the fraction to see if we need to round up or down
				rounding_mode = ARITHMETIC_ROUNDING;
				std::cout << "ar rounding no e field ";
			}
		}
		return rounding_mode;
	}
	bool increment() {
		bool carry = false;
		if (es > 0) {
			carry = increment_unsigned(_Bits, es);
		}
		return carry;
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
