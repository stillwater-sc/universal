#pragma once
// bfloat16_impl.hpp: definition of the Google Brain Float number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>

#include <universal/utility/bit_cast.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/bfloat16/bfloat16_fwd.hpp>
#include <universal/number/bfloat16/exceptions.hpp>

namespace sw { namespace universal {

	// forward reference
	inline bfloat16 abs(bfloat16);
	inline bfloat16 sqrt(bfloat16);
	inline bfloat16 floor(bfloat16);

// bfloat16 is Google's Brain Float type
class bfloat16 {
		// HELPER methods
	// bfloat16 is the upper 16 bits of an IEEE-754 float; conversion is pure
	// bit-shuffle via sw::bit_cast (constexpr where __builtin_bit_cast is
	// constexpr - i.e. on gcc, clang, MSVC modern). This replaces the previous
	// std::memcpy-based punning, which was decorated constexpr but is not
	// actually constexpr until C++26.
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	BIT_CAST_CONSTEXPR bfloat16& convert_signed(SignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			return convert_ieee754(static_cast<float>(v));
		}
		return *this;
	}
	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	BIT_CAST_CONSTEXPR bfloat16& convert_unsigned(UnsignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			return convert_ieee754(static_cast<float>(v));
		}
		return *this;
	}
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	BIT_CAST_CONSTEXPR bfloat16& convert_ieee754(Real rhs) noexcept {
		// Down-cast wider floats to single precision first; bfloat16 is
		// defined relative to the 32-bit IEEE-754 layout.
		float f = static_cast<float>(rhs);
		uint32_t bits = sw::bit_cast<uint32_t>(f);
		_bits = static_cast<uint16_t>(bits >> 16);
		return *this;
	}
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	BIT_CAST_CONSTEXPR Real convert_to_ieee754() const noexcept {
		uint32_t bits = static_cast<uint32_t>(_bits) << 16;
		float f = sw::bit_cast<float>(bits);
		return static_cast<Real>(f);
	}
public:
	static constexpr unsigned nbits = 16;
	static constexpr unsigned es = 8;

	bfloat16() = default;

	constexpr bfloat16(const bfloat16&) = default;
	constexpr bfloat16(bfloat16&&) = default;

	constexpr bfloat16& operator=(const bfloat16&) = default;
	constexpr bfloat16& operator=(bfloat16&&) = default;

	// specific value constructor
	constexpr bfloat16(const SpecificValue code) noexcept : _bits{} {
		switch (code) {
		case SpecificValue::infpos:
			setinf(false);
			break;
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::infneg:
			setinf(true);
			break;
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::qnan:
		case SpecificValue::nar:
			setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	// initializers for native types. Integer ctors are BIT_CAST_CONSTEXPR
	// because they route through convert_ieee754 (via convert_signed /
	// convert_unsigned), which is BIT_CAST_CONSTEXPR.
	BIT_CAST_CONSTEXPR bfloat16(signed char iv)                    noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(short iv)                          noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(int iv)                            noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(long iv)                           noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(long long iv)                      noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(char iv)                           noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(unsigned short iv)                 noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(unsigned int iv)                   noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(unsigned long iv)                  noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16(unsigned long long iv)             noexcept : _bits{} { *this = iv; }
	explicit BIT_CAST_CONSTEXPR bfloat16(float iv)                 noexcept : _bits{} { *this = iv; }
	explicit BIT_CAST_CONSTEXPR bfloat16(double iv)                noexcept : _bits{} { *this = iv; }

	// assignment operators for native types
	BIT_CAST_CONSTEXPR bfloat16& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(char rhs)               noexcept {
		// Plain char is implementation-defined as either signed or unsigned;
		// dispatch on its signedness so negative values on signed-char
		// platforms (the common case) sign-extend correctly.
		if constexpr (std::is_signed_v<char>) {
			return convert_signed(rhs);
		}
		else {
			return convert_unsigned(static_cast<unsigned char>(rhs));
		}
	}
	BIT_CAST_CONSTEXPR bfloat16& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	BIT_CAST_CONSTEXPR bfloat16& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators (BIT_CAST_CONSTEXPR via convert_to_ieee754)
	explicit BIT_CAST_CONSTEXPR operator signed char()        const noexcept { return (signed char)(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator short()              const noexcept { return short(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator int()                const noexcept { return int(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator long()               const noexcept { return long(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator long long()          const noexcept { return (long long)(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator char()               const noexcept { return (char)(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator unsigned short()     const noexcept { return (unsigned short)(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator unsigned int()       const noexcept { return (unsigned int)(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator unsigned long()      const noexcept { return (unsigned long)(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator unsigned long long() const noexcept { return (unsigned long long)(float(*this)); }
	explicit BIT_CAST_CONSTEXPR operator float()              const noexcept { return convert_to_ieee754<float>(); }
	explicit BIT_CAST_CONSTEXPR operator double()             const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	explicit BIT_CAST_CONSTEXPR bfloat16(long double iv)           noexcept : _bits{} { *this = iv; }
	BIT_CAST_CONSTEXPR bfloat16& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }
	explicit BIT_CAST_CONSTEXPR operator long double()        const noexcept { return convert_to_ieee754<long double>(); }
#endif

	// prefix operators
	constexpr bfloat16 operator-() const noexcept {
		bfloat16 tmp;
		tmp.setbits(_bits ^ 0x8000u);
		return tmp;
	}

	constexpr bfloat16& operator++() noexcept {
		if (isneg()) {
			if (_bits == 0x8001u) { // pattern 1.00.001 == minneg
				_bits = 0;
			}
			else {
				--_bits;
			}
		}
		else {
			if (_bits == 0x7FFFu) { // pattern = qnan
				_bits = 0xFFFFu;  // pattern = snan 
			}
			else {
				++_bits;
			}
		}
		return *this;
	}
	constexpr bfloat16 operator++(int) noexcept {
		bfloat16 tmp(*this);
		operator++();
		return tmp;
	}
	constexpr bfloat16& operator--() noexcept {
		if (sign()) {
			++_bits;
		}
		else {
			if (_bits == 0) { // pattern 0.00.000 = 0
				_bits = 0x8001u; // pattern 1.00.001 = minneg
			}
			else {
				--_bits;
			}
		}
		return *this;
	}
	constexpr bfloat16 operator--(int) noexcept {
		bfloat16 tmp(*this);
		operator--();
		return tmp;
	}

	// arithmetic operators - cast to float, compute, cast back. Both legs use
	// BIT_CAST_CONSTEXPR conversions, and IEEE-754 float arithmetic itself is
	// constexpr in C++20, so the compound is BIT_CAST_CONSTEXPR overall.
	BIT_CAST_CONSTEXPR bfloat16& operator+=(const bfloat16& rhs) {
		*this = float(*this) + float(rhs);
		return *this;
	}
	BIT_CAST_CONSTEXPR bfloat16& operator-=(const bfloat16& rhs) {
		*this = float(*this) - float(rhs);
		return *this;
	}
	BIT_CAST_CONSTEXPR bfloat16& operator*=(const bfloat16& rhs) {
		*this = float(*this) * float(rhs);
		return *this;
	}
	BIT_CAST_CONSTEXPR bfloat16& operator/=(const bfloat16& rhs) {
		*this = float(*this) / float(rhs);
		return *this;
	}

	// modifiers
	constexpr void clear() noexcept { _bits = 0; }
	constexpr void setzero() noexcept { clear(); }
	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING) noexcept {
		_bits = (NaNType == NAN_TYPE_SIGNALLING ? 0xFF81u : 0x7F81u);
	}
	constexpr void setinf(bool sign = false) noexcept { 
		_bits = (sign ? 0xFF80u : 0x7F80u); 
	}
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned short bit = (1u << i);
		if (v) {
			_bits |= bit;
		}
		else {
			_bits &= ~bit;
		}
	}
	constexpr void setbits(unsigned value) noexcept { _bits = (value & 0xFFFFu); }

	constexpr bfloat16& minpos() noexcept { _bits = 0x0001u; return *this; }
	constexpr bfloat16& maxpos() noexcept { _bits = 0x7F7Fu; return *this; }
	constexpr bfloat16& zero()   noexcept { _bits = 0x0000u; return *this; }
	constexpr bfloat16& minneg() noexcept { _bits = 0x8001u; return *this; }
	constexpr bfloat16& maxneg() noexcept { _bits = 0xFF7Fu; return *this; }

	/// <summary>
	/// assign the value of the string representation to the bfloat16
	/// </summary>
	/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
	/// <returns>reference to this cfloat</returns>
	/// Clang doesn't support constexpr yet on string manipulations, so we need to make it conditional
	bfloat16& assign(const std::string& str) noexcept {
		clear();
		unsigned nrChars = static_cast<unsigned>(str.size());
		unsigned nrBits = 0;
		unsigned nrDots = 0;
		std::string bits;
		if (nrChars > 2) {
			if (str[0] == '0' && str[1] == 'b') {
				for (unsigned i = 2; i < nrChars; ++i) {
					char c = str[i];
					switch (c) {
					case '0':
					case '1':
						++nrBits;
						bits += c;
						break;
					case '.':
						++nrDots;
						bits += c;
						break;
					case '\'':
						// consume this delimiting character
						break;
					default:
						std::cerr << "string contained a non-standard character: " << c << '\n';
						return *this;
					}
				}
			}
			else {
				std::cerr << "string must start with 0b: instead input pattern was " << str << '\n';
				return *this;
			}
		}
		else {
			std::cerr << "string is too short\n";
			return *this;
		}

		if (nrBits != nbits) {
			std::cerr << "number of bits in the string is " << nrBits << " and needs to be " << nbits << '\n';
			return *this;
		}
		if (nrDots != 2) {
			std::cerr << "number of segment delimiters in string is " << nrDots << " and needs to be 2 for a cfloat<>\n";
			return *this;
		}

		// assign the bits
		int field{ 0 };  // three fields: sign, exponent, mantissa: fields are separated by a '.'
		int nrExponentBits{ -1 };
		unsigned bit = nrBits;
		for (unsigned i = 0; i < bits.size(); ++i) {
			char c = bits[i];
			if (c == '.') {
				++field;
				if (field == 2) { // just finished parsing exponent field: we can now check the number of exponent bits
					if (nrExponentBits != es) {
						std::cerr << "provided binary string representation does not contain " << es << " exponent bits. Found " << nrExponentBits << ". Reset to 0\n";
						clear();
						return *this;
					}
				}
			}
			else {
				setbit(--bit, c == '1');
			}
			if (field == 1) { // exponent field
				++nrExponentBits;
			}
		}
		if (field != 2) {
			std::cerr << "provided binary string did not contain three fields separated by '.': Reset to 0\n";
			clear();
			return *this;
		}
		return *this;
	}

	// selectors
	constexpr bool iszero()    const noexcept { return (_bits == 0x0000u) || (_bits == 0x8000u); }
	constexpr bool isone()     const noexcept { return (_bits == 0x3F80u); }
	constexpr bool isodd()     const noexcept { return (_bits & 0x0001u); }
	constexpr bool iseven()    const noexcept { return !isodd(); }
	// not constexpr because of floor()
	bool isinteger() const noexcept {
		if (isnan() || isinf()) return false;
		return (floor(*this) == *this);
	}
	constexpr bool ispos()     const noexcept { return !isneg(); }
	constexpr bool isneg()     const noexcept { return (_bits & 0x8000u); }
	/*
	* IEEE 754-2008 specifies the encoding of NaN and Infinity in the following way:
	 Sign | Exponent | Mantissa
	----- | -------- | -------- -
		x | 11111111 | 1000000   Quiet NaN(qNaN)
		x | 11111111 | 0xxxxxx   Signaling NaN(sNaN)
	*/
	constexpr bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept { 
		// bool negative = isneg(); is not used to determine NaN
		bool isNaN    = ((_bits & 0x7F80u) == 0x7f80u) && ((_bits & 0x007Fu) != 0);
		bool isQuietNaN = isNaN && ((_bits & 0x0040u) != 0); // MSB of mantissa is 1
		bool isSignalNaN = isNaN && ((_bits & 0x0040u) == 0); // MSB of mantissa is 0	
		return (NaNType == NAN_TYPE_EITHER ? isNaN :
			(NaNType == NAN_TYPE_SIGNALLING ? isSignalNaN :
				(NaNType == NAN_TYPE_QUIET ? isQuietNaN : false)));
	}
	constexpr bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept { 
		bool negative = isneg();
		bool isInf = ((_bits & 0x7F80u) == 0x7f80u) && ((_bits & 0x007fu) == 0u);  // all exponent bits set, no mantissa bits set
		bool isNegInf = isInf && negative;
		bool isPosInf = isInf && !negative;
		return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
			(InfType == INF_TYPE_NEGATIVE ? isNegInf :
				(InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}
	constexpr bool sign()   const noexcept { return isneg(); }
	constexpr int  scale()  const noexcept { int biased = static_cast<int>((_bits & 0x7F80u) >> 7); return biased - 127; }
	constexpr unsigned short bits() const noexcept { return _bits; }

	constexpr bool test(unsigned bitIndex) const noexcept { return at(bitIndex); }
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex < nbits) {
			uint16_t word = _bits;
			uint16_t mask = uint16_t(1ull << bitIndex);
			return (word & mask);
		}
		return false;
	}
	constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < 4) {
			uint16_t word = _bits;
			int nibbleIndexInWord = int(n);
			uint16_t mask = uint16_t(0xF << (nibbleIndexInWord * 4));
			uint16_t nibblebits = uint16_t(mask & word);
			return uint8_t(nibblebits >> (nibbleIndexInWord * 4));
		}
		return 0;
	}
	constexpr uint8_t exponent() const noexcept {
		uint8_t e = static_cast<uint8_t>((_bits & 0x7f80) >> 7);
		return e;
	}
	constexpr uint8_t fraction() const noexcept {
		uint8_t f = static_cast<uint8_t>(_bits & 0x7f);
		return f;
	}

protected:
	unsigned short _bits;

private:

	// bfloat16 - bfloat16 logic comparisons
	friend constexpr bool operator==(bfloat16 lhs, bfloat16 rhs);

	// bfloat16 - literal logic comparisons (BIT_CAST_CONSTEXPR via bfloat16(float))
	friend BIT_CAST_CONSTEXPR bool operator==(bfloat16 lhs, float rhs);

	// literal - bfloat16 logic comparisons (BIT_CAST_CONSTEXPR via bfloat16(float))
	friend BIT_CAST_CONSTEXPR bool operator==(float lhs, bfloat16 rhs);
};

////////////////////////    functions   /////////////////////////////////


inline bfloat16 abs(bfloat16 a) {
	return (a.isneg() ? -a : a);
}


/// stream operators

// parse a bfloat16 ASCII format and make a binary bfloat16 out of it
inline bool parse(const std::string& /* number */, bfloat16& value) {
	bool bSuccess = false;
	value.zero();
	return bSuccess;
}

// generate an bfloat16 format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, bfloat16 bf) {
	return ostr << float(bf);
}

// read an ASCII bfloat16 format
inline std::istream& operator>>(std::istream& istr, bfloat16& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a bfloat16 value\n";
	}
	return istr;
}

////////////////// string operators

inline std::string to_binary(bfloat16 bf, bool bNibbleMarker = false) {
	std::stringstream s;
	unsigned short bits = bf.bits();
	unsigned short mask = 0x8000u;
	s << (bits & mask ? "0b1." : "0x0.");
	mask >>= 1;
	// exponent bits
	for (unsigned i = 0; i < 8; ++i) {
		if (bNibbleMarker && (4 == i)) {
			s << '\'';
		}
		s << (bits & mask ? '1' : '0');
		mask >>= 1;
	}
	s << '.';
	for (unsigned i = 0; i < 7; ++i) {
		if (bNibbleMarker && (3 == i)) {
			s << '\'';
		}	
		s << (bits & mask ? '1' : '0');
		mask >>= 1;
	}
	return s.str();
}

// native semantic representation: radix-2, delegates to to_binary
inline std::string to_native(bfloat16 v, bool nibbleMarker = false) {
	return to_binary(v, nibbleMarker);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - bfloat16 binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
constexpr inline bool operator==(bfloat16 lhs, bfloat16 rhs) {
	// NaN != NaN
	if (lhs.isnan() || rhs.isnan()) return false;
	// IEEE-754 signed zero: +0 and -0 compare equal even though their bit
	// patterns differ (0x0000 vs 0x8000). operator-(bfloat16(0)) produces
	// the 0x8000 pattern directly, so this case is observable in practice.
	if (lhs.iszero() && rhs.iszero()) return true;
	return lhs._bits == rhs._bits;
}

constexpr inline bool operator!=(bfloat16 lhs, bfloat16 rhs) {
	return !operator==(lhs, rhs);
}

// operator< is BIT_CAST_CONSTEXPR (depends on the float() conversion)
BIT_CAST_CONSTEXPR inline bool operator< (bfloat16 lhs, bfloat16 rhs) {
	return (float(lhs) - float(rhs)) < 0;
}

BIT_CAST_CONSTEXPR inline bool operator> (bfloat16 lhs, bfloat16 rhs) {
	return operator< (rhs, lhs);
}

BIT_CAST_CONSTEXPR inline bool operator<=(bfloat16 lhs, bfloat16 rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

BIT_CAST_CONSTEXPR inline bool operator>=(bfloat16 lhs, bfloat16 rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - literal binary logic operators
// All take bfloat16 by value to match the friend declarations and to enable
// BIT_CAST_CONSTEXPR (the bfloat16(float) ctor is BIT_CAST_CONSTEXPR).

BIT_CAST_CONSTEXPR inline bool operator==(bfloat16 lhs, float rhs) {
	return operator==(lhs, bfloat16(rhs));
}

BIT_CAST_CONSTEXPR inline bool operator!=(bfloat16 lhs, float rhs) {
	return !operator==(lhs, bfloat16(rhs));
}

BIT_CAST_CONSTEXPR inline bool operator< (bfloat16 lhs, float rhs) {
	return operator<(lhs, bfloat16(rhs));
}

BIT_CAST_CONSTEXPR inline bool operator> (bfloat16 lhs, float rhs) {
	return operator< (bfloat16(rhs), lhs);
}

BIT_CAST_CONSTEXPR inline bool operator<=(bfloat16 lhs, float rhs) {
	return operator< (lhs, bfloat16(rhs)) || operator==(lhs, bfloat16(rhs));
}

BIT_CAST_CONSTEXPR inline bool operator>=(bfloat16 lhs, float rhs) {
	return !operator< (lhs, bfloat16(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - bfloat16 binary logic operators

BIT_CAST_CONSTEXPR inline bool operator==(float lhs, bfloat16 rhs) {
	return operator==(bfloat16(lhs), rhs);
}

BIT_CAST_CONSTEXPR inline bool operator!=(float lhs, bfloat16 rhs) {
	return !operator==(bfloat16(lhs), rhs);
}

BIT_CAST_CONSTEXPR inline bool operator< (float lhs, bfloat16 rhs) {
	return operator<(bfloat16(lhs), rhs);
}

BIT_CAST_CONSTEXPR inline bool operator> (float lhs, bfloat16 rhs) {
	return operator< (rhs, bfloat16(lhs));
}

BIT_CAST_CONSTEXPR inline bool operator<=(float lhs, bfloat16 rhs) {
	return operator< (bfloat16(lhs), rhs) || operator==(bfloat16(lhs), rhs);
}

BIT_CAST_CONSTEXPR inline bool operator>=(float lhs, bfloat16 rhs) {
	return !operator< (bfloat16(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - bfloat16 binary arithmetic operators
// BIT_CAST_CONSTEXPR via the compound-assignment operators (which are themselves
// BIT_CAST_CONSTEXPR via float()). This is the #725 public-API acceptance form:
// `constexpr auto c = a + b;` works once these are decorated.

BIT_CAST_CONSTEXPR inline bfloat16 operator+(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 sum = lhs;
	sum += rhs;
	return sum;
}

BIT_CAST_CONSTEXPR inline bfloat16 operator-(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 diff = lhs;
	diff -= rhs;
	return diff;
}

BIT_CAST_CONSTEXPR inline bfloat16 operator*(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 mul = lhs;
	mul *= rhs;
	return mul;
}

BIT_CAST_CONSTEXPR inline bfloat16 operator/(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - literal binary arithmetic operators

BIT_CAST_CONSTEXPR inline bfloat16 operator+(bfloat16 lhs, float rhs) {
	return operator+(lhs, bfloat16(rhs));
}

BIT_CAST_CONSTEXPR inline bfloat16 operator-(bfloat16 lhs, float rhs) {
	return operator-(lhs, bfloat16(rhs));
}

BIT_CAST_CONSTEXPR inline bfloat16 operator*(bfloat16 lhs, float rhs) {
	return operator*(lhs, bfloat16(rhs));
}

BIT_CAST_CONSTEXPR inline bfloat16 operator/(bfloat16 lhs, float rhs) {
	return operator/(lhs, bfloat16(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - bfloat16 binary arithmetic operators

BIT_CAST_CONSTEXPR inline bfloat16 operator+(float lhs, bfloat16 rhs) {
	return operator+(bfloat16(lhs), rhs);
}

BIT_CAST_CONSTEXPR inline bfloat16 operator-(float lhs, bfloat16 rhs) {
	return operator-(bfloat16(lhs), rhs);
}

BIT_CAST_CONSTEXPR inline bfloat16 operator*(float lhs, bfloat16 rhs) {
	return operator*(bfloat16(lhs), rhs);
}

BIT_CAST_CONSTEXPR inline bfloat16 operator/(float lhs, bfloat16 rhs) {
	return operator/(bfloat16(lhs), rhs);
}

}} // namespace sw::universal
