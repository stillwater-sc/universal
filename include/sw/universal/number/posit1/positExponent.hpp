#pragma once
// positExponent.hpp: definition of a posit positExponent
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>

namespace sw { namespace universal {

static constexpr int GEOMETRIC_ROUND_DOWN   = -2;
static constexpr int ARITHMETIC_ROUND_DOWN  = -1;
static constexpr int NO_ADDITIONAL_ROUNDING =  0;
static constexpr int ARITHMETIC_ROUND_UP    =  1;
static constexpr int GEOMETRIC_ROUND_UP     =  2;
static constexpr int ARITHMETIC_ROUNDING    =  5;

// positExponent
template<unsigned nbits, unsigned es>
class positExponent {
public:
	positExponent() : _NrOfBits(0) {}
	
	positExponent(const positExponent& r) = default;
	positExponent(positExponent&& r) = default;

	positExponent& operator=(const positExponent& r) = default;
	positExponent& operator=(positExponent&& r) = default;
	
	void reset() {
		_NrOfBits = 0;
		_Bits.reset();
	}
	void setzero() { reset(); }
	unsigned nrBits() const {
		return _NrOfBits;
	}
	int scale() const {
		if constexpr (es == 0) {
			return 0;
		}
		else {
			return int(_Bits.to_ulong());
		}
	}
	long double value() const {
		int s = scale();
		if (s < 0 || s >= 64) {
			return std::ldexp(1.0l, s);
		}
		return static_cast<long double>(uint64_t(1) << s);
	}
	bitblock<es> get() const {
		return _Bits;
	}
	void set(const bitblock<es>& raw, unsigned nrExponentBits) {
		_Bits = raw;
		_NrOfBits = nrExponentBits;
	}
	
	// extract the exponent bits given a pattern and the location of the starting point
	void extract_exponent_bits(const bitblock<nbits>& _raw_bits, unsigned nrRegimeBits) {
		_Bits.reset();
		// start of positExponent is nbits - (sign_bit + regime_bits)
		int msb = int(static_cast<int>(nbits) - 1ull - (1ull + nrRegimeBits));
		if constexpr (es > 0) {
			unsigned nrExponentBits = 0;
			bitblock<es> _exp;
			if (msb >= 0 && es > 0) {
				nrExponentBits = (static_cast<unsigned>(msb) >= es - 1ull ? es : static_cast<unsigned>(msb) + 1ull);
				for (unsigned i = 0; i < nrExponentBits; i++) {
					_exp[static_cast<uint64_t>(es) - 1ull - i] = _raw_bits[static_cast<uint64_t>(msb) - i];
				}
			}
			set(_exp, nrExponentBits);
		}
	}

	// calculate the positExponent given a number's scale: esval = Mod[scale, 2^es];
	// DEPRECATED
	void _assign(int scale) {
		_Bits.reset();
		unsigned int my_positExponent = (scale < 0) ? (-scale >> es) : (scale >> es);
		// convert value into bitset
		uint32_t mask = uint32_t(1);  // es will be small, so pick a single word sized mask for efficiency
		for (unsigned i = 0; i < es; i++) {
			_Bits[i] = my_positExponent & mask;
			mask <<= 1;
		}
	}
	// calculate the positExponent given a number's scale and the number of regime bits, 
	// returning an indicator which type of rounding is required to complete the posit
	// DEPRECATED
	int assign_exponent_bits(int scale, int k, unsigned nrRegimeBits) {
		int rounding_mode = NO_ADDITIONAL_ROUNDING;
		_Bits.reset();
		// we need to get to an adjusted scale that encodes regime and positExponent
		// value scale = useed ^ k * 2 ^ positExponent = 2^(k*2^es) * 2^e -> k*2^es + e
		// e = scale - k*2^es
		int raw = scale - k*(1 << es);
		unsigned my_positExponent = raw < 0 ? -raw : raw;
		// convert value into bitset
		unsigned mask = 0x1;
		for (unsigned i = 0; i < es; i++) {
			_Bits[i] = my_positExponent & mask;
			mask <<= 1;
		}
		_NrOfBits = (nbits - 1 - nrRegimeBits > es ? es : nbits - 1 - nrRegimeBits);
		if (_NrOfBits > 0) {
			if (_NrOfBits < es) {
				rounding_mode = _Bits[static_cast<uint64_t>(es) - 1ull - _NrOfBits] ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN; // check the next bit to see if we need to geometric round
				if (_trace_rounding) std::cout << "truncated exp" << (rounding_mode == GEOMETRIC_ROUND_UP ? " geo-up " : " geo-dw ");
			}
			else {
				if (nbits - 1 - nrRegimeBits - es > 0) {
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
				// this posit doesn't have an positExponent field, 
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
	unsigned			_NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned ees>
	friend std::ostream& operator<< (std::ostream& ostr, const positExponent<nnbits, ees>& e);
	template<unsigned nnbits, unsigned ees>
	friend std::istream& operator>> (std::istream& istr, positExponent<nnbits, ees>& e);

	template<unsigned nnbits, unsigned ees>
	friend bool operator==(const positExponent<nnbits, ees>& lhs, const positExponent<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator!=(const positExponent<nnbits, ees>& lhs, const positExponent<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator< (const positExponent<nnbits, ees>& lhs, const positExponent<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator> (const positExponent<nnbits, ees>& lhs, const positExponent<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator<=(const positExponent<nnbits, ees>& lhs, const positExponent<nnbits, ees>& rhs);
	template<unsigned nnbits, unsigned ees>
	friend bool operator>=(const positExponent<nnbits, ees>& lhs, const positExponent<nnbits, ees>& rhs);
};

template<unsigned nbits, unsigned es>
inline int scale(const positExponent<nbits, es>& e) { return e.scale(); }

/////////////////// EXPONENT operators
template<unsigned nbits, unsigned es>
inline std::ostream& operator<<(std::ostream& ostr, const positExponent<nbits, es>& e) {
	unsigned nrOfExponentBitsProcessed = 0;
	if constexpr (es > 0) {
		for (int i = int(es) - 1; i >= 0; --i) {
			if (e._NrOfBits > nrOfExponentBitsProcessed++) {
				ostr << (e._Bits[unsigned(i)] ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	else {
		ostr << "~"; // for proper alignment in tables
	}
	return ostr;
}

template<unsigned nbits, unsigned es>
inline std::istream& operator>> (std::istream& istr, const positExponent<nbits, es>& e) {
	istr >> e._Bits;
	return istr;
}

template<unsigned nbits, unsigned es>
inline std::string to_string(const positExponent<nbits, es>& e, bool dashExtent = true, bool nibbleMarker = false) {
	std::stringstream sstr;
	unsigned nrOfExponentBitsProcessed = 0;
	unsigned ebits = e.nrBits();
	if constexpr (es > 0) {
		for (int i = int(es) - 1; i >= 0; --i) {
			if (e.nrBits() > nrOfExponentBitsProcessed++) {
				bitblock<es> bb = e.get();
				sstr << (bb[unsigned(i)] ? '1' : '0');
			}
			else {
				sstr << (dashExtent ? "-" : "");
			}
			--ebits;
			if (nibbleMarker && ebits != 0 && (ebits % 4) == 0) sstr << '\'';
		}
	}
	else {
		sstr << '~'; // for proper alignment in tables
	}
	return sstr.str();
}

template<unsigned nbits, unsigned es>
inline bool operator==(const positExponent<nbits, es>& lhs, const positExponent<nbits, es>& rhs) { return lhs._Bits == rhs._Bits && lhs._NrOfBits == rhs._NrOfBits; }
template<unsigned nbits, unsigned es>
inline bool operator!=(const positExponent<nbits, es>& lhs, const positExponent<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nbits, unsigned es>
inline bool operator< (const positExponent<nbits, es>& lhs, const positExponent<nbits, es>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<unsigned nbits, unsigned es>
inline bool operator> (const positExponent<nbits, es>& lhs, const positExponent<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nbits, unsigned es>
inline bool operator<=(const positExponent<nbits, es>& lhs, const positExponent<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nbits, unsigned es>
inline bool operator>=(const positExponent<nbits, es>& lhs, const positExponent<nbits, es>& rhs) { return !operator< (lhs, rhs); }

}}  // namespace sw::universal
