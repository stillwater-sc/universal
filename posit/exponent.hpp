#pragma once
// exponent.hpp: definition of a posit exponent
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
namespace unum {

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
	size_t nrBits() const {
		return _NrOfBits;
	}
	int scale() const {
		return _Bits.to_ulong();
	}
	double value() const {
		return double(uint64_t(1) << scale());
	}
	bitblock<es> get() const {
		return _Bits;
	}
	void set(const bitblock<es>& raw, size_t nrOfExponentBits) {
		_Bits = raw;
		_NrOfBits = nrOfExponentBits;
	}
	// calculate the exponent given a number's scale: esval = Mod[scale, 2^es];
	void assign(int scale) {
		_Bits.reset();
		unsigned int my_exponent = scale < 0 ? -scale >> es : scale >> es;
		// convert value into bitset
		uint32_t mask = uint32_t(1);  // es will be small, so pick a single word sized mask for efficiency
		for (unsigned i = 0; i < es; i++) {
			_Bits[i] = my_exponent & mask;
			mask <<= 1;
		}
	}
	// calculate the exponent given a number's scale and the number of regime bits, 
	// returning an indicator which type of rounding is required to complete the posit
	int assign_exponent_bits(int scale, int k, size_t nr_of_regime_bits) {
		int rounding_mode = NO_ADDITIONAL_ROUNDING;
		_Bits.reset();
		// we need to get to an adjusted scale that encodes regime and exponent
		// value scale = useed ^ k * 2 ^ exponent = 2^(k*2^es) * 2^e -> k*2^es + e
		// e = scale - k*2^es
		int raw = scale - k*(1 << es);
		size_t my_exponent = raw < 0 ? -raw : raw;
		// convert value into bitset
		size_t mask = 0x1;
		for (unsigned i = 0; i < es; i++) {
			_Bits[i] = my_exponent & mask;
			mask <<= 1;
		}
		_NrOfBits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
		if (_NrOfBits > 0) {
			if (_NrOfBits < es) {
				rounding_mode = _Bits[es - 1 - _NrOfBits] ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN; // check the next bit to see if we need to geometric round
				if (_trace_rounding) std::cout << "truncated exp" << (rounding_mode == GEOMETRIC_ROUND_UP ? " geo-up " : " geo-dw ");
			}
			else {
				if (nbits - 1 - nr_of_regime_bits - es > 0) {
					// use the fraction to determine rounding as this posit has fraction bits
					rounding_mode = ARITHMETIC_ROUNDING;
					if (_trace_rounding) std::cout << "arithmetic  rounding ";

				}
				else {
					// this posit is in the geometric regime and has consumed all the bits
					rounding_mode = ARITHMETIC_ROUNDING; //  NO_ADDITIONAL_ROUNDING;
					if (_trace_rounding) std::cout << "no rounding alltaken ";
				}
			}
		}
		else {
			// we ran out of bits
			if (es > 0) {
				rounding_mode = _Bits[es-1] ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN;
				if (_trace_rounding) std::cout << "no exp left: " << (rounding_mode == GEOMETRIC_ROUND_UP ? " geo-up " : " geo-dw ");
			}
			else {
				// this posit doesn't have an exponent field, 
				// so we need to look at the fraction to see if we need to round up or down
				rounding_mode = ARITHMETIC_ROUNDING;
				if (_trace_rounding) std::cout << "ar rounding no e field ";
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
	bitblock<es>    _Bits;
	size_t			_NrOfBits;

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

/////////////////// EXPONENT operators
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

}  // namespace unum

}  // namespace sw

