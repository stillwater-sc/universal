#pragma once
// exponent.hpp: definition of a posit exponent
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

static constexpr int GEOMETRIC_ROUND_DOWN   = -2;
static constexpr int ARITHMETIC_ROUND_DOWN  = -1;
static constexpr int NO_ADDITIONAL_ROUNDING =  0;
static constexpr int ARITHMETIC_ROUND_UP    =  1;
static constexpr int GEOMETRIC_ROUND_UP     =  2;
static constexpr int ARITHMETIC_ROUNDING    =  5;

// exponent
template<size_t nbits, size_t es, typename bt>
class exponent {
	using UnsignedExponent = blockbinary<es, bt, BinaryNumberType::Unsigned>;
public:
	exponent() : _Bits{ 0 }, _NrOfBits{ es } {}
	
	exponent(const exponent& r) = default;
	exponent(exponent&& r) = default;

	exponent& operator=(const exponent& r) = default;
	exponent& operator=(exponent&& r) = default;
	
	void reset() {
		_NrOfBits = 0;
		_Bits.clear();
	}
	void setzero() { reset(); }
	size_t nrBits() const noexcept {
		return _NrOfBits;
	}
	int scale() const noexcept {
		return int(_Bits);
	}
	long double value() const noexcept {
		return (long double)(uint64_t(1) << scale());
	}
	blockbinary<es, bt, BinaryNumberType::Unsigned> bits() const noexcept {
		return _Bits;
	}
	void set(const UnsignedExponent& raw, size_t nrExponentBits) {
		_Bits = raw;
		_NrOfBits = nrExponentBits;
	}
	
	// extract the exponent bits given a pattern and the location of the starting point
	void extract_exponent_bits(const blockbinary<nbits, bt, BinaryNumberType::Signed>& rawPositBits, size_t nrRegimeBits) {
		_Bits.clear();
		// start of exponent is nbits - (sign_bit + regime_bits)
		int msb = int(static_cast<int>(nbits) - 1ull - (1ull + nrRegimeBits));
		if (es > 0) {
			size_t nrExponentBits = 0;
			blockbinary<es, bt> _exp;
			if (msb >= 0 && es > 0) {
				nrExponentBits = (static_cast<size_t>(msb) >= es - 1ull ? es : static_cast<size_t>(msb) + 1ull);
				for (size_t i = 0; i < nrExponentBits; i++) {
					_exp[es - 1 - i] = rawPositBits[msb - i];
				}
			}
			set(_exp, nrExponentBits);
		}
	}

	// calculate the exponent given a number's scale: esval = Mod[scale, 2^es];
	// DEPRECATED
	void _assign(int scale) {
		_Bits.clear();
		unsigned int my_exponent = (scale < 0) ? (-scale >> es) : (scale >> es);
		// convert value into bitset
		uint32_t mask = uint32_t(1);  // es will be small, so pick a single word sized mask for efficiency
		for (unsigned i = 0; i < es; i++) {
			_Bits[i] = my_exponent & mask;
			mask <<= 1;
		}
	}
	// calculate the exponent given a number's scale and the number of regime bits, 
	// returning an indicator which type of rounding is required to complete the posit
	// DEPRECATED
	int assign_exponent_bits(int scale, int k, size_t nrRegimeBits) {
		int rounding_mode = NO_ADDITIONAL_ROUNDING;
		_Bits.clear();
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
		_NrOfBits = (nbits - 1 - nrRegimeBits > es ? es : nbits - 1 - nrRegimeBits);
		if (_NrOfBits > 0) {
			if (_NrOfBits < es) {
				rounding_mode = _Bits[es - 1 - _NrOfBits] ? GEOMETRIC_ROUND_UP : GEOMETRIC_ROUND_DOWN; // check the next bit to see if we need to geometric round
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
	blockbinary<es, bt, BinaryNumberType::Unsigned>    _Bits;
	size_t			_NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const exponent<nnbits, ees, bbt>& e);
	template<size_t nnbits, size_t ees, typename bbt>
	friend std::istream& operator>> (std::istream& istr, exponent<nnbits, ees, bbt>& e);

	template<size_t nnbits, size_t ees, typename bbt>
	friend bool operator==(const exponent<nnbits, ees, bbt>& lhs, const exponent<nnbits, ees, bbt>& rhs);
	template<size_t nnbits, size_t ees, typename bbt>
	friend bool operator!=(const exponent<nnbits, ees, bbt>& lhs, const exponent<nnbits, ees, bbt>& rhs);
	template<size_t nnbits, size_t ees, typename bbt>
	friend bool operator< (const exponent<nnbits, ees, bbt>& lhs, const exponent<nnbits, ees, bbt>& rhs);
	template<size_t nnbits, size_t ees, typename bbt>
	friend bool operator> (const exponent<nnbits, ees, bbt>& lhs, const exponent<nnbits, ees, bbt>& rhs);
	template<size_t nnbits, size_t ees, typename bbt>
	friend bool operator<=(const exponent<nnbits, ees, bbt>& lhs, const exponent<nnbits, ees, bbt>& rhs);
	template<size_t nnbits, size_t ees, typename bbt>
	friend bool operator>=(const exponent<nnbits, ees, bbt>& lhs, const exponent<nnbits, ees, bbt>& rhs);
};

template<size_t nbits, size_t es, typename bt>
inline int scale(const exponent<nbits, es, bt>& e) { return e.scale(); }

/////////////////// EXPONENT operators
template<size_t nbits, size_t es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const exponent<nbits, es, bt>& e) {
	size_t nrOfExponentBitsProcessed = 0;
	if constexpr (es > 0) {
		for (int i = int(es) - 1; i >= 0; --i) {
			if (e._NrOfBits > nrOfExponentBitsProcessed++) {
				ostr << (e._Bits[size_t(i)] ? "1" : "0");
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

template<size_t nbits, size_t es, typename bt>
inline std::istream& operator>> (std::istream& istr, const exponent<nbits, es, bt>& e) {
	istr >> e._Bits;
	return istr;
}

template<size_t nbits, size_t es, typename bt>
inline std::string to_string(const exponent<nbits, es, bt>& e, bool dashExtent = true, bool nibbleMarker = false) {
	using UnsignedExponent = blockbinary<es, bt, BinaryNumberType::Unsigned>;
	std::stringstream s;
	size_t nrOfExponentBitsProcessed = 0;
	if constexpr (es > 0) {
		for (size_t bitIndex = 0; bitIndex < es; ++bitIndex) {
			size_t i = es - bitIndex;
			if (e.nrBits() > nrOfExponentBitsProcessed++) {
				UnsignedExponent exponentBits = e.bits();
				s << (exponentBits[i] ? '1' : '0');
			}
			else {
				s << (dashExtent ? "-" : "");
			}
			if (nibbleMarker && ((i % 4) == 0) && i != 0) s << '\'';
		}
	}
	else {
		s << '~'; // for proper alignment in tables
	}
	return s.str();
}

template<size_t nbits, size_t es, typename bt>
inline bool operator==(const exponent<nbits, es, bt>& lhs, const exponent<nbits, es, bt>& rhs) { return lhs._Bits == rhs._Bits && lhs._NrOfBits == rhs._NrOfBits; }
template<size_t nbits, size_t es, typename bt>
inline bool operator!=(const exponent<nbits, es, bt>& lhs, const exponent<nbits, es, bt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es, typename bt>
inline bool operator< (const exponent<nbits, es, bt>& lhs, const exponent<nbits, es, bt>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<size_t nbits, size_t es, typename bt>
inline bool operator> (const exponent<nbits, es, bt>& lhs, const exponent<nbits, es, bt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es, typename bt>
inline bool operator<=(const exponent<nbits, es, bt>& lhs, const exponent<nbits, es, bt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es, typename bt>
inline bool operator>=(const exponent<nbits, es, bt>& lhs, const exponent<nbits, es, bt>& rhs) { return !operator< (lhs, rhs); }

}}  // namespace sw::universal

