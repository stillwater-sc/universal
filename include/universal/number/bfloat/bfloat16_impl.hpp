#pragma once
// bfloat16_impl.hpp: definition of the Google Brain Float number system
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
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
#include <universal/number/bfloat/bfloat16_fwd.hpp>
#include <universal/number/bfloat/exceptions.hpp>

namespace sw { namespace universal {

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
	constexpr bfloat16(signed char initial_value)        noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(short initial_value)              noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(int initial_value)                noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(long initial_value)               noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(long long initial_value)          noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(char initial_value)               noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(unsigned short initial_value)     noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(unsigned int initial_value)       noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(unsigned long initial_value)      noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(unsigned long long initial_value) noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(float initial_value)              noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(double initial_value)             noexcept : _bits{} { *this = initial_value; }
	constexpr bfloat16(long double initial_value)        noexcept : _bits{} { *this = initial_value; }

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
	constexpr bfloat16& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float() const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double() const noexcept { return convert_to_ieee754<double>(); }
	explicit operator long double() const noexcept { return convert_to_ieee754<long double>(); }

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
	constexpr void setbits(unsigned short value) noexcept { _bits = value; }

	constexpr bfloat16& minpos() noexcept { _bits = 0x0080u; return *this; }
	constexpr bfloat16& maxpos() noexcept { _bits = 0x7F7Fu; return *this; }
	constexpr bfloat16& zero()   noexcept { _bits = 0x0000u; return *this; }
	constexpr bfloat16& minneg() noexcept { _bits = 0x8080u; return *this; }
	constexpr bfloat16& maxneg() noexcept { _bits = 0xFF7Fu; return *this; }

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
bool parse(const std::string& number, bfloat16& value) {
	bool bSuccess = false;

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

std::string to_binary(bfloat16 bf, bool bNibbleMarker = false) {
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
	return (float(lhs) - float(rhs)) > 0;
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
