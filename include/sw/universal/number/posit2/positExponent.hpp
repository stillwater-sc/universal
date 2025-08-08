#pragma once
// positExponent.hpp: definition of a posit positExponent
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

static constexpr int GEOMETRIC_ROUND_DOWN   = -2;
static constexpr int ARITHMETIC_ROUND_DOWN  = -1;
static constexpr int NO_ADDITIONAL_ROUNDING =  0;
static constexpr int ARITHMETIC_ROUND_UP    =  1;
static constexpr int GEOMETRIC_ROUND_UP     =  2;
static constexpr int ARITHMETIC_ROUNDING    =  5;

// positExponent
template<unsigned nbits, unsigned es, typename bt>
class positExponent {
	constexpr static std::uint32_t MASK = (0xFFFF'FFFFul >> (31ul - es)) >> 1ul;
public:
	positExponent() : _expBits{ 0 }, _nrExpBits{ es } {}
	
	positExponent(const positExponent& r) = default;
	positExponent(positExponent&& r) = default;

	positExponent& operator=(const positExponent& r) = default;
	positExponent& operator=(positExponent&& r) = default;
	
	void reset() {
		_nrExpBits = 0;
		_expBits = 0;
	}
	void setzero() { reset(); }
	unsigned nrBits() const noexcept {
		return _nrExpBits;
	}
	int scale() const noexcept {
		return int(_expBits);
	}
	long double value() const noexcept {
		return (long double)(std::uint64_t(1) << _expBits);
	}
	std::uint32_t bits() const noexcept {
		return _expBits;
	}
	void set(const std::uint32_t& raw, unsigned nrExponentBits) {
		_expBits = raw & MASK;
		_nrExpBits = nrExponentBits;
	}
	void setNrBits(unsigned nrExpBits) noexcept {
		_nrExpBits = nrExpBits;
	}
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		if (i < es) {
			std::uint32_t block = _expBits;
			std::uint32_t null = ~(1ull << i);
			std::uint32_t bit = std::uint32_t(v ? 1u : 0u);
			std::uint32_t mask = std::uint32_t(bit << i);
			_expBits = std::uint32_t((block & null) | mask);
		}
		// nop if out of range
	}
	constexpr bool test(unsigned i) const noexcept {
		if (i < es) {
			std::uint32_t mask = std::uint32_t(1ul << i);
			return bool(_expBits & mask);
		}
		return false; // nop if out of range
	}
	// extract the exponent bits given a pattern and the location of the starting point
	void extract_exponent_bits(const blockbinary<nbits, bt, BinaryNumberType::Signed>& rawPositBits, unsigned nrRegimeBits) {
		reset();
		// start of positExponent is nbits - (sign_bit + regime_bits)
		int msb = static_cast<int>(nbits - 1ull - (1ull + nrRegimeBits));
		if (es > 0) {
			unsigned nrExponentBits = 0;
			if (msb >= 0 && es > 0) {
				nrExponentBits = static_cast<unsigned>(static_cast<unsigned>(msb) >= es - 1ull ? es : (msb + 1));
				for (unsigned i = 0; i < nrExponentBits; i++) {
					setbit(es - 1 - i, rawPositBits.at(msb - i));
				}
			}
			setNrBits(nrExponentBits);
		}
	}

	bool increment() noexcept {
		bool carry = false;
		if constexpr (es > 0) {
			++_expBits;
			if (_expBits & (1ul << es)) carry = true;
		}
		return carry;
	}

private:
	unsigned _expBits;
	unsigned _nrExpBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const positExponent<nnbits, ees, bbt>& e);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::istream& operator>> (std::istream& istr, positExponent<nnbits, ees, bbt>& e);

	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const positExponent<nnbits, ees, bbt>& lhs, const positExponent<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const positExponent<nnbits, ees, bbt>& lhs, const positExponent<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const positExponent<nnbits, ees, bbt>& lhs, const positExponent<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const positExponent<nnbits, ees, bbt>& lhs, const positExponent<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const positExponent<nnbits, ees, bbt>& lhs, const positExponent<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const positExponent<nnbits, ees, bbt>& lhs, const positExponent<nnbits, ees, bbt>& rhs);
};

template<unsigned nbits, unsigned es, typename bt>
inline int scale(const positExponent<nbits, es, bt>& e) { return e.scale(); }

/////////////////// EXPONENT operators
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const positExponent<nbits, es, bt>& e) {
	if constexpr (es > 0) {
		unsigned nrOfExponentBitsProcessed = 0;
		for (unsigned i = 0; i < es; ++i) {
			unsigned bitIndex = es - 1ull - i;
			if (e._nrExpBits > nrOfExponentBitsProcessed++) {
				ostr << (e.test(bitIndex) ? "1" : "0");
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

template<unsigned nbits, unsigned es, typename bt>
inline std::istream& operator>> (std::istream& istr, const positExponent<nbits, es, bt>& e) {
	istr >> e._Bits;
	return istr;
}

template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const positExponent<nbits, es, bt>& e, bool dashExtent = true, bool nibbleMarker = false) {
	using UnsignedExponent = blockbinary<es, bt, BinaryNumberType::Unsigned>;
	std::stringstream s;
	unsigned nrOfExponentBitsProcessed = 0;
	if constexpr (es > 0) {
		for (unsigned i = 0; i < es; ++i) {
			unsigned bitIndex = es - 1ull - i;
			if (e.nrBits() > nrOfExponentBitsProcessed++) {
				UnsignedExponent positExponentBits = e.bits();
				s << (positExponentBits.test(bitIndex) ? '1' : '0');
			}
			else {
				s << (dashExtent ? "-" : "");
			}
			if (nibbleMarker && ((bitIndex % 4) == 0) && bitIndex != 0) s << '\'';
		}
	}
	else {
		s << '~'; // for proper alignment in tables
	}
	return s.str();
}

template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const positExponent<nbits, es, bt>& lhs, const positExponent<nbits, es, bt>& rhs) { return lhs._Bits == rhs._Bits && lhs._NrOfBits == rhs._NrOfBits; }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const positExponent<nbits, es, bt>& lhs, const positExponent<nbits, es, bt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const positExponent<nbits, es, bt>& lhs, const positExponent<nbits, es, bt>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const positExponent<nbits, es, bt>& lhs, const positExponent<nbits, es, bt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const positExponent<nbits, es, bt>& lhs, const positExponent<nbits, es, bt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const positExponent<nbits, es, bt>& lhs, const positExponent<nbits, es, bt>& rhs) { return !operator< (lhs, rhs); }

}}  // namespace sw::universal

