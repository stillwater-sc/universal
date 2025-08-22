#pragma once
// bfloat16_impl.hpp: definition of the Google Brain Float number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>

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
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	constexpr bfloat16& convert_signed(SignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			float f = float(v);
			uint16_t pun[2];
			std::memcpy(pun, &f, 4);
			_bits = pun[1];
		}
		return *this;
	}
	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	constexpr bfloat16& convert_unsigned(UnsignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			float f = float(v);
			uint16_t pun[2];
			std::memcpy(pun, &f, 4);
			_bits = pun[1];
		}
		return *this;
	}
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	constexpr bfloat16& convert_ieee754(Real rhs) noexcept {

		float f = float(rhs);
		uint16_t pun[2];
		std::memcpy(pun, &f, 4);
		_bits = pun[1];
		return *this;
	}
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	constexpr Real convert_to_ieee754() const noexcept {
		float f;
		uint16_t pun[2];
		pun[1] = _bits;
		pun[0] = 0;
		std::memcpy(&f, pun, 4);
		return Real(f);
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

	// initializers for native types
	constexpr bfloat16(signed char iv)                    noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(short iv)                          noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(int iv)                            noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(long iv)                           noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(long long iv)                      noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(char iv)                           noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(unsigned short iv)                 noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(unsigned int iv)                   noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(unsigned long iv)                  noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(unsigned long long iv)             noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(float iv)                          noexcept : _bits{} { *this = iv; }
	constexpr bfloat16(double iv)                         noexcept : _bits{} { *this = iv; }

	// assignment operators for native types
	constexpr bfloat16& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr bfloat16& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr bfloat16& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr bfloat16& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr bfloat16& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr bfloat16& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	constexpr bfloat16& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr bfloat16& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr bfloat16& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr bfloat16& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr bfloat16& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr bfloat16& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator signed char()                 const noexcept { return (signed char)(float(*this)); }
	explicit operator short()                       const noexcept { return short(float(*this)); }
	explicit operator int()                         const noexcept { return int(float(*this)); }
	explicit operator long()                        const noexcept { return long(float(*this)); }
	explicit operator long long()                   const noexcept { return (long long)(float(*this)); }
	explicit operator char()                        const noexcept { return (char)(float(*this)); }
	explicit operator unsigned short()              const noexcept { return (unsigned short)(float(*this)); }
	explicit operator unsigned int()                const noexcept { return (unsigned int)(float(*this)); }
	explicit operator unsigned long()               const noexcept { return (unsigned long)(float(*this)); }
	explicit operator unsigned long long()          const noexcept { return (unsigned long long)(float(*this)); }
	explicit operator float()                       const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double()                      const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	constexpr bfloat16(long double iv)                    noexcept : _bits{} { *this = iv; }
	constexpr bfloat16& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }
	explicit operator long double()                 const noexcept { return convert_to_ieee754<long double>(); }
#endif 

	// prefix operators
	bfloat16 operator-() const noexcept {
		bfloat16 tmp;
		tmp.setbits(_bits ^ 0x8000u);
		return tmp;
	}

	bfloat16& operator++() noexcept {
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
	bfloat16 operator++(int) noexcept {
		bfloat16 tmp(*this);
		operator++();
		return tmp;
	}
	bfloat16& operator--() noexcept {
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
	bfloat16 operator--(int) noexcept {
		bfloat16 tmp(*this);
		operator--();
		return tmp;
	}

	// arithmetic operators
	bfloat16& operator+=(const bfloat16& rhs) {
		*this = float(*this) + float(rhs);
		return *this;
	}
	bfloat16& operator-=(const bfloat16& rhs) {
		*this = float(*this) - float(rhs);
		return *this;
	}
	bfloat16& operator*=(const bfloat16& rhs) {
		*this = float(*this) * float(rhs);
		return *this;
	}
	bfloat16& operator/=(const bfloat16& rhs) {
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
	constexpr bool iszero()    const noexcept { return _bits == 0; }
	constexpr bool isone()     const noexcept { return (_bits & 0x7F00u); }
	constexpr bool isodd()     const noexcept { return (_bits & 0x0001u); }
	constexpr bool iseven()    const noexcept { return !isodd(); }
	          bool isinteger() const noexcept { return (floor(*this) == *this); }
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
		bool isQuietNaN = isNaN && (_bits & 0x0040u) && ((_bits & 0x003Fu) == 0);
		bool isSignalNaN = isNaN && (_bits & 0x003Fu);	
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
	friend bool operator==(bfloat16 lhs, bfloat16 rhs);

	// bfloat16 - literal logic comparisons
	friend bool operator==(bfloat16 lhs, float rhs);

	// literal - bfloat16 logic comparisons
	friend bool operator==(float lhs, bfloat16 rhs);
};

////////////////////////    functions   /////////////////////////////////


inline bfloat16 abs(bfloat16 a) {
	return (a.isneg() ? -a : a);
}


/// stream operators

// parse a bfloat16 ASCII format and make a binary bfloat16 out of it
inline bool parse(const std::string& number, bfloat16& value) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - bfloat16 binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
inline bool operator==(bfloat16 lhs, bfloat16 rhs) {
	// NaN != NaN
	if (lhs.isnan() || rhs.isnan()) return false;
	return lhs._bits == rhs._bits;
}

inline bool operator!=(bfloat16 lhs, bfloat16 rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (bfloat16 lhs, bfloat16 rhs) {
	return (float(lhs) - float(rhs)) < 0;
}

inline bool operator> (bfloat16 lhs, bfloat16 rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(bfloat16 lhs, bfloat16 rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(bfloat16 lhs, bfloat16 rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const bfloat16& lhs, float rhs) {
	return operator==(lhs, bfloat16(rhs));
}

inline bool operator!=(const bfloat16& lhs, float rhs) {
	return !operator==(lhs, bfloat16(rhs));
}

inline bool operator< (const bfloat16& lhs, float rhs) {
	return operator<(lhs, bfloat16(rhs));
}

inline bool operator> (const bfloat16& lhs, float rhs) {
	return operator< (bfloat16(rhs), lhs);
}

inline bool operator<=(const bfloat16& lhs, float rhs) {
	return operator< (lhs, bfloat16(rhs)) || operator==(lhs, bfloat16(rhs));
}

inline bool operator>=(const bfloat16& lhs, float rhs) {
	return !operator< (lhs, bfloat16(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - bfloat16 binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(float lhs, const bfloat16& rhs) {
	return operator==(bfloat16(lhs), rhs);
}

inline bool operator!=(float lhs, const bfloat16& rhs) {
	return !operator==(bfloat16(lhs), rhs);
}

inline bool operator< (float lhs, const bfloat16& rhs) {
	return operator<(bfloat16(lhs), rhs);
}

inline bool operator> (float lhs, const bfloat16& rhs) {
	return operator< (rhs, bfloat16(lhs));
}

inline bool operator<=(float lhs, const bfloat16& rhs) {
	return operator< (bfloat16(lhs), rhs) || operator==(bfloat16(lhs), rhs);
}

inline bool operator>=(float lhs, const bfloat16& rhs) {
	return !operator< (bfloat16(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - bfloat16 binary arithmetic operators
// BINARY ADDITION

inline bfloat16 operator+(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION

inline bfloat16 operator-(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION

inline bfloat16 operator*(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION

inline bfloat16 operator/(bfloat16 lhs, bfloat16 rhs) {
	bfloat16 ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat16 - literal binary arithmetic operators
// BINARY ADDITION

inline bfloat16 operator+(bfloat16 lhs, float rhs) {
	return operator+(lhs, bfloat16(rhs));
}
// BINARY SUBTRACTION

inline bfloat16 operator-(bfloat16 lhs, float rhs) {
	return operator-(lhs, bfloat16(rhs));
}
// BINARY MULTIPLICATION

inline bfloat16 operator*(bfloat16 lhs, float rhs) {
	return operator*(lhs, bfloat16(rhs));
}
// BINARY DIVISION

inline bfloat16 operator/(bfloat16 lhs, float rhs) {
	return operator/(lhs, bfloat16(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - bfloat16 binary arithmetic operators
// BINARY ADDITION

inline bfloat16 operator+(float lhs, bfloat16 rhs) {
	return operator+(bfloat16(lhs), rhs);
}
// BINARY SUBTRACTION

inline bfloat16 operator-(float lhs, bfloat16 rhs) {
	return operator-(bfloat16(lhs), rhs);
}
// BINARY MULTIPLICATION

inline bfloat16 operator*(float lhs, bfloat16 rhs) {
	return operator*(bfloat16(lhs), rhs);
}
// BINARY DIVISION

inline bfloat16 operator/(float lhs, bfloat16 rhs) {
	return operator/(bfloat16(lhs), rhs);
}

}} // namespace sw::universal
