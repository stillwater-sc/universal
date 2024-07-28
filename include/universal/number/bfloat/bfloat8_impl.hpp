#pragma once
// bfloat8_impl.hpp: definition of the Google Brain Float number system
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
#include <universal/number/bfloat/bfloat8_fwd.hpp>
#include <universal/number/bfloat/exceptions.hpp>

namespace sw { namespace universal {


// bfloat8 is Google's Brain Float type
class bfloat8 {
		// HELPER methods
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	constexpr bfloat8& convert_signed(SignedInt v) noexcept {
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
	constexpr bfloat8& convert_unsigned(UnsignedInt v) noexcept {
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
	constexpr bfloat8& convert_ieee754(Real rhs) noexcept {
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

	bfloat8() = default;

	constexpr bfloat8(const bfloat8&) = default;
	constexpr bfloat8(bfloat8&&) = default;

	constexpr bfloat8& operator=(const bfloat8&) = default;
	constexpr bfloat8& operator=(bfloat8&&) = default;

	// specific value constructor
	constexpr bfloat8(const SpecificValue code) noexcept : _bits{} {
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
	constexpr bfloat8(signed char initial_value)        noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(short initial_value)              noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(int initial_value)                noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(long initial_value)               noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(long long initial_value)          noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(char initial_value)               noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(unsigned short initial_value)     noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(unsigned int initial_value)       noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(unsigned long initial_value)      noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(unsigned long long initial_value) noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(float initial_value)              noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(double initial_value)             noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat8(long double initial_value)        noexcept : _bits{} { *this = initial_value; }

	// assignment operators for native types
	constexpr bfloat8& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr bfloat8& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr bfloat8& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr bfloat8& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr bfloat8& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr bfloat8& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	constexpr bfloat8& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr bfloat8& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr bfloat8& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr bfloat8& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr bfloat8& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr bfloat8& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }
	constexpr bfloat8& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float() const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double() const noexcept { return convert_to_ieee754<double>(); }
	explicit operator long double() const noexcept { return convert_to_ieee754<long double>(); }

	// prefix operators
	bfloat8 operator-() const noexcept {
		bfloat8 tmp;
		tmp.setbits(_bits ^ 0x8000u);
		return tmp;
	}

	bfloat8& operator++() noexcept {
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
	bfloat8 operator++(int) noexcept {
		bfloat8 tmp(*this);
		operator++();
		return tmp;
	}
	bfloat8& operator--() noexcept {
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
	bfloat8 operator--(int) noexcept {
		bfloat8 tmp(*this);
		operator--();
		return tmp;
	}

	// arithmetic operators
	bfloat8& operator+=(const bfloat8& rhs) {
		*this = float(*this) + float(rhs);
		return *this;
	}
	bfloat8& operator-=(const bfloat8& rhs) {
		*this = float(*this) - float(rhs);
		return *this;
	}
	bfloat8& operator*=(const bfloat8& rhs) {
		*this = float(*this) * float(rhs);
		return *this;
	}
	bfloat8& operator/=(const bfloat8& rhs) {
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
	constexpr void setbits(unsigned short value) noexcept { _bits = value; }

	constexpr bfloat8& minpos() noexcept { _bits = 0x0001u; return *this; }
	constexpr bfloat8& maxpos() noexcept { _bits = 0x7F7Fu; return *this; }
	constexpr bfloat8& zero()   noexcept { _bits = 0x0000u; return *this; }
	constexpr bfloat8& minneg() noexcept { _bits = 0x8001u; return *this; }
	constexpr bfloat8& maxneg() noexcept { _bits = 0xFF7Fu; return *this; }

	/// <summary>
	/// assign the value of the string representation to the bfloat8
	/// </summary>
	/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
	/// <returns>reference to this cfloat</returns>
	/// Clang doesn't support constexpr yet on string manipulations, so we need to make it conditional
	CONSTEXPRESSION bfloat8& assign(const std::string& str) noexcept {
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
	constexpr bool isinteger() const noexcept { return false; } // return (floor(*this) == *this) ? true : false; }
	constexpr bool ispos()     const noexcept { return !(_bits & 0x8000u); }
	constexpr bool isneg()     const noexcept { return (_bits & 0x8000u); }
	constexpr bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept { 
		bool negative = isneg();
		bool isNaN    = (_bits & 0x7F80u) && (_bits & 0x007F);
		bool isNegNaN = isNaN && negative;
		bool isPosNaN = isNaN && !negative;	
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
			(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
				(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}
	constexpr bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept { 
		bool negative = isneg();
		bool isInf    = (_bits & 0x7F80u);
		bool isNegInf = isInf && negative;
		bool isPosInf = isInf && !negative;
		return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
			(InfType == INF_TYPE_NEGATIVE ? isNegInf :
				(InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}
	constexpr bool sign()   const noexcept { return isneg(); }
	constexpr int  scale()  const noexcept { int biased = static_cast<int>((_bits & 0x7F80u) >> 7); return biased - 127; }
	constexpr unsigned short bits() const noexcept { return _bits; }

protected:
	unsigned short _bits;

private:

	// bfloat8 - bfloat8 logic comparisons
	friend bool operator==(bfloat8 lhs, bfloat8 rhs);

	// bfloat8 - literal logic comparisons
	friend bool operator==(bfloat8 lhs, float rhs);

	// literal - bfloat8 logic comparisons
	friend bool operator==(float lhs, bfloat8 rhs);
};

////////////////////////    functions   /////////////////////////////////


inline bfloat8 abs(bfloat8 a) {
	return (a.isneg() ? -a : a);
}


/// stream operators

// parse a bfloat8 ASCII format and make a binary bfloat8 out of it
bool parse(const std::string& number, bfloat8& value) {
	bool bSuccess = false;
	value.zero();
	return bSuccess;
}

// generate an bfloat8 format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, bfloat8 bf) {
	return ostr << float(bf);
}

// read an ASCII bfloat8 format
inline std::istream& operator>>(std::istream& istr, bfloat8& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a bfloat8 value\n";
	}
	return istr;
}

////////////////// string operators

std::string to_binary(bfloat8 bf, bool bNibbleMarker = false) {
	std::stringstream s;
	unsigned short bits = bf.bits();
	unsigned short mask = 0x8000u;
	s << (bits & mask ? "0b1." : "0x0.");
	mask >>= 1;
	for (unsigned i = 1; i < 16; ++i) {
		 if (9 == i) {
			s << '.';
		}
		else if (bNibbleMarker && (4 == i || 8 == i || 12 == i)) {
			s << '\'';
		}
		
		s << (bits & mask ? '1' : '0');
		mask >>= 1;
	}
	return s.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat8 - bfloat8 binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
inline bool operator==(bfloat8 lhs, bfloat8 rhs) {
	// NaN != NaN
	if (lhs.isnan() || rhs.isnan()) return false;
	return lhs._bits == rhs._bits;
}

inline bool operator!=(bfloat8 lhs, bfloat8 rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (bfloat8 lhs, bfloat8 rhs) {
	return (float(lhs) - float(rhs)) > 0;
}

inline bool operator> (bfloat8 lhs, bfloat8 rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(bfloat8 lhs, bfloat8 rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(bfloat8 lhs, bfloat8 rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat8 - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const bfloat8& lhs, float rhs) {
	return operator==(lhs, bfloat8(rhs));
}

inline bool operator!=(const bfloat8& lhs, float rhs) {
	return !operator==(lhs, bfloat8(rhs));
}

inline bool operator< (const bfloat8& lhs, float rhs) {
	return operator<(lhs, bfloat8(rhs));
}

inline bool operator> (const bfloat8& lhs, float rhs) {
	return operator< (bfloat8(rhs), lhs);
}

inline bool operator<=(const bfloat8& lhs, float rhs) {
	return operator< (lhs, bfloat8(rhs)) || operator==(lhs, bfloat8(rhs));
}

inline bool operator>=(const bfloat8& lhs, float rhs) {
	return !operator< (lhs, bfloat8(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - bfloat8 binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(float lhs, const bfloat8& rhs) {
	return operator==(bfloat8(lhs), rhs);
}

inline bool operator!=(float lhs, const bfloat8& rhs) {
	return !operator==(bfloat8(lhs), rhs);
}

inline bool operator< (float lhs, const bfloat8& rhs) {
	return operator<(bfloat8(lhs), rhs);
}

inline bool operator> (float lhs, const bfloat8& rhs) {
	return operator< (rhs, bfloat8(lhs));
}

inline bool operator<=(float lhs, const bfloat8& rhs) {
	return operator< (bfloat8(lhs), rhs) || operator==(bfloat8(lhs), rhs);
}

inline bool operator>=(float lhs, const bfloat8& rhs) {
	return !operator< (bfloat8(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat8 - bfloat8 binary arithmetic operators
// BINARY ADDITION

inline bfloat8 operator+(bfloat8 lhs, bfloat8 rhs) {
	bfloat8 sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION

inline bfloat8 operator-(bfloat8 lhs, bfloat8 rhs) {
	bfloat8 diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION

inline bfloat8 operator*(bfloat8 lhs, bfloat8 rhs) {
	bfloat8 mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION

inline bfloat8 operator/(bfloat8 lhs, bfloat8 rhs) {
	bfloat8 ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// bfloat8 - literal binary arithmetic operators
// BINARY ADDITION

inline bfloat8 operator+(bfloat8 lhs, float rhs) {
	return operator+(lhs, bfloat8(rhs));
}
// BINARY SUBTRACTION

inline bfloat8 operator-(bfloat8 lhs, float rhs) {
	return operator-(lhs, bfloat8(rhs));
}
// BINARY MULTIPLICATION

inline bfloat8 operator*(bfloat8 lhs, float rhs) {
	return operator*(lhs, bfloat8(rhs));
}
// BINARY DIVISION

inline bfloat8 operator/(bfloat8 lhs, float rhs) {
	return operator/(lhs, bfloat8(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - bfloat8 binary arithmetic operators
// BINARY ADDITION

inline bfloat8 operator+(float lhs, bfloat8 rhs) {
	return operator+(bfloat8(lhs), rhs);
}
// BINARY SUBTRACTION

inline bfloat8 operator-(float lhs, bfloat8 rhs) {
	return operator-(bfloat8(lhs), rhs);
}
// BINARY MULTIPLICATION

inline bfloat8 operator*(float lhs, bfloat8 rhs) {
	return operator*(bfloat8(lhs), rhs);
}
// BINARY DIVISION

inline bfloat8 operator/(float lhs, bfloat8 rhs) {
	return operator/(bfloat8(lhs), rhs);
}

}} // namespace sw::universal
