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
	constexpr static std::uint32_t MASK = (0xFFFF'FFFFul >> (31ul - es)) >> 1ul;
public:
	exponent() : _expBits{ 0 }, _nrExpBits{ es } {}
	
	exponent(const exponent& r) = default;
	exponent(exponent&& r) = default;

	exponent& operator=(const exponent& r) = default;
	exponent& operator=(exponent&& r) = default;
	
	void reset() {
		_nrExpBits = 0;
		_expBits = 0;
	}
	void setzero() { reset(); }
	size_t nrBits() const noexcept {
		return _nrExpBits;
	}
	int scale() const noexcept {
		return int(_expBits);
	}
	long double value() const noexcept {
		return (long double)(uint64_t(1) << _expBits);
	}
	std::uint32_t bits() const noexcept {
		return _expBits;
	}
	void set(const std::uint32_t& raw, size_t nrExponentBits) {
		_expBits = raw & MASK;
		_nrExpBits = nrExponentBits;
	}
	void setNrBits(std::uint32_t nrExpBits) noexcept {
		_nrExpBits = nrExpBits;
	}
	constexpr void setbit(size_t i, bool v = true) noexcept {
		if (i < es) {
			std::uint32_t block = _expBits;
			std::uint32_t null = ~(1ull << i);
			std::uint32_t bit = std::uint32_t(v ? 1u : 0u);
			std::uint32_t mask = std::uint32_t(bit << i);
			_expBits = std::uint32_t((block & null) | mask);
		}
		// nop if out of range
	}
	constexpr bool test(size_t i) const noexcept {
		if (i < es) {
			std::uint32_t mask = std::uint32_t(1ul << i);
			return bool(_expBits & mask);
		}
		return false; // nop if out of range
	}
	// extract the exponent bits given a pattern and the location of the starting point
	void extract_exponent_bits(const blockbinary<nbits, bt, BinaryNumberType::Signed>& rawPositBits, size_t nrRegimeBits) {
		reset();
		// start of exponent is nbits - (sign_bit + regime_bits)
		long long msb = static_cast<int>(nbits - 1ull - (1ull + nrRegimeBits));
		if (es > 0) {
			size_t nrExponentBits = 0;
			if (msb >= 0 && es > 0) {
				nrExponentBits = (static_cast<size_t>(msb) >= es - 1ull ? es : static_cast<size_t>(msb + 1));
				for (size_t i = 0; i < nrExponentBits; i++) {
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
	std::uint32_t _expBits;
	std::uint32_t _nrExpBits;

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
		for (size_t i = 0; i < es; ++i) {
			size_t bitIndex = es - 1ull - i;
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
		for (size_t i = 0; i < es; ++i) {
			size_t bitIndex = es - 1ull - i;
			if (e.nrBits() > nrOfExponentBitsProcessed++) {
				UnsignedExponent exponentBits = e.bits();
				s << (exponentBits.test(bitIndex) ? '1' : '0');
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

