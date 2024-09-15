#pragma once
// efloat_impl.hpp: implementation of an adaptive precision binary floating-point number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>

#include <universal/number/efloat/exceptions.hpp>

namespace sw { namespace universal {

// forward references
class efloat;
inline efloat& convert(int64_t v, efloat& result);
inline efloat& convert_unsigned(uint64_t v, efloat& result);
bool parse(const std::string& number, efloat& v);

// efloat is an adaptive precision linear floating-point type
class efloat {
	using BlockType = uint32_t;
public:
	efloat() : sign(false), exp(0) { }

	efloat(const efloat&) = default;
	efloat(efloat&&) = default;

	efloat& operator=(const efloat&) = default;
	efloat& operator=(efloat&&) = default;

	// initializers for native types
	constexpr efloat(signed char iv)        noexcept { *this = iv; }
	constexpr efloat(short iv)              noexcept { *this = iv; }
	constexpr efloat(int iv)                noexcept { *this = iv; }
	constexpr efloat(long iv)               noexcept { *this = iv; }
	constexpr efloat(long long iv)          noexcept { *this = iv; }
	constexpr efloat(char iv)               noexcept { *this = iv; }
	constexpr efloat(unsigned short iv)     noexcept { *this = iv; }
	constexpr efloat(unsigned int iv)       noexcept { *this = iv; }
	constexpr efloat(unsigned long iv)      noexcept { *this = iv; }
	constexpr efloat(unsigned long long iv) noexcept { *this = iv; }
	constexpr efloat(float iv)              noexcept { *this = iv; }
	constexpr efloat(double iv)             noexcept { *this = iv; }


	// assignment operators for native types
	constexpr efloat& operator=(signed char rhs)        noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(short rhs)              noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(int rhs)                noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(long rhs)               noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(long long rhs)          noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(char rhs)               noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(unsigned short rhs)     noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(unsigned int rhs)       noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(unsigned long rhs)      noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(unsigned long long rhs) noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(float rhs)              noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(double rhs)             noexcept { return *this = convert(rhs); }
	constexpr efloat& operator=(long double rhs)        noexcept { return *this = convert(rhs); }

	// conversion operators
	explicit operator float() const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double() const noexcept { return convert_to_ieee754<double>(); }
	explicit operator long double() const noexcept { return convert_to_ieee754<long double>(); }

#if LONG_DOUBLE_SUPPORT
	constexpr efloat(long double iv)                    noexcept : _bits{} { *this = iv; }
	constexpr efloat& operator=(long double rhs)        noexcept { return convert(rhs); }
	explicit operator long double()                 const noexcept { return convert_to_ieee754<long double>(); }
#endif 

	// prefix operators
	efloat operator-() const {
		efloat negated(*this);
		return negated;
}

	// arithmetic operators
	efloat& operator+=(const efloat& rhs) {
		return *this;
	}
	efloat& operator-=(const efloat& rhs) {
		return *this;
	}
	efloat& operator*=(const efloat& rhs) {
		return *this;
	}
	efloat& operator/=(const efloat& rhs) {
		return *this;
	}

	// modifiers
	inline void clear() { sign = false; exp = 0; coef.clear(); }
	inline void setzero() { clear(); }

	inline efloat& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	inline bool iszero() const { return !sign && coef.size() == 0; }
	inline bool isone() const  { return true; }
	inline bool isodd() const  { return false; }
	inline bool iseven() const { return !isodd(); }
	inline bool ispos() const  { return !sign; }
	inline bool ineg() const   { return sign; }
	inline int64_t scale() const { return exp + int64_t(coef.size()); }


	void test(bool _sign, int _exp, std::vector<BlockType>& _coef) {
		sign = _sign;
		coef = _coef;
		exp = _exp;
	}

protected:
	bool                   sign;  // sign of the number: -1 if true, +1 if false, zero is positive
	int64_t                exp;   // exponent of the number
	std::vector<BlockType> coef;  // coefficients of the polynomial

	// HELPER methods

	// convert arithmetic types into an elastic floating-point
	template<typename Arith>
	static constexpr efloat convert(Arith v) noexcept {
		static_assert(std::is_arithmetic_v<Arith>);
		efloat f;
		f.clear();
		if constexpr (std::is_integral_v<Arith> && std::is_signed_v<Arith>) {
		}
		else if constexpr (std::is_unsigned_v<Arith>) {
		}
		else if constexpr (std::is_floating_point_v<Arith>) {
		}
		return f;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	constexpr Real convert_to_ieee754() const noexcept {
		float f{ 0 };

		return Real(f);
	}

private:

	// efloat - efloat logic comparisons
	friend bool operator==(const efloat& lhs, const efloat& rhs);

	// efloat - literal logic comparisons
	friend bool operator==(const efloat& lhs, const long long rhs);

	// literal - efloat logic comparisons
	friend bool operator==(const long long lhs, const efloat& rhs);

	// find the most significant bit set
	friend signed findMsb(const efloat& v);
};

inline efloat& convert(int64_t v, efloat& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

inline efloat& convert_unsigned(uint64_t v, efloat& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

////////////////////////    MPFLOAT functions   /////////////////////////////////


inline efloat abs(const efloat& a) {
	return a; // (a < 0 ? -a : a);
}


// findMsb takes an efloat reference and returns the position of the most significant bit, -1 if v == 0

inline signed findMsb(const efloat& v) {
	return -1; // no significant bit found, all bits are zero
}

////////////////////////    INTEGER operators   /////////////////////////////////

// divide efloat a and b and return result argument

void divide(const efloat& a, const efloat& b, efloat& quotient) {
}

/// stream operators

// read a efloat ASCII format and make a binary efloat out of it

bool parse(const std::string& number, efloat& value) {
	bool bSuccess = false;

	return bSuccess;
}

// generate an efloat format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const efloat& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the efloat into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
//	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));

	return ostr << ss.str();
}

// read an ASCII efloat format

inline std::istream& operator>>(std::istream& istr, efloat& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - efloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths

inline bool operator==(const efloat& lhs, const efloat& rhs) {
	return true;
}

inline bool operator!=(const efloat& lhs, const efloat& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const efloat& lhs, const efloat& rhs) {
	return false; // lhs and rhs are the same
}

inline bool operator> (const efloat& lhs, const efloat& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const efloat& lhs, const efloat& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const efloat& lhs, const efloat& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const efloat& lhs, const long long rhs) {
	return operator==(lhs, efloat(rhs));
}

inline bool operator!=(const efloat& lhs, const long long rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const efloat& lhs, const long long rhs) {
	return operator<(lhs, efloat(rhs));
}

inline bool operator> (const efloat& lhs, const long long rhs) {
	return operator< (efloat(rhs), lhs);
}

inline bool operator<=(const efloat& lhs, const long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const efloat& lhs, const long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(const long long lhs, const efloat& rhs) {
	return operator==(efloat(lhs), rhs);
}

inline bool operator!=(const long long lhs, const efloat& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const long long lhs, const efloat& rhs) {
	return operator<(efloat(lhs), rhs);
}

inline bool operator> (const long long lhs, const efloat& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const long long lhs, const efloat& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const long long lhs, const efloat& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - efloat binary arithmetic operators
// BINARY ADDITION

inline efloat operator+(const efloat& lhs, const efloat& rhs) {
	efloat sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION

inline efloat operator-(const efloat& lhs, const efloat& rhs) {
	efloat diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION

inline efloat operator*(const efloat& lhs, const efloat& rhs) {
	efloat mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION

inline efloat operator/(const efloat& lhs, const efloat& rhs) {
	efloat ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - literal binary arithmetic operators
// BINARY ADDITION

inline efloat operator+(const efloat& lhs, const long long rhs) {
	return operator+(lhs, efloat(rhs));
}
// BINARY SUBTRACTION

inline efloat operator-(const efloat& lhs, const long long rhs) {
	return operator-(lhs, efloat(rhs));
}
// BINARY MULTIPLICATION

inline efloat operator*(const efloat& lhs, const long long rhs) {
	return operator*(lhs, efloat(rhs));
}
// BINARY DIVISION

inline efloat operator/(const efloat& lhs, const long long rhs) {
	return operator/(lhs, efloat(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary arithmetic operators
// BINARY ADDITION

inline efloat operator+(const long long lhs, const efloat& rhs) {
	return operator+(efloat(lhs), rhs);
}
// BINARY SUBTRACTION

inline efloat operator-(const long long lhs, const efloat& rhs) {
	return operator-(efloat(lhs), rhs);
}
// BINARY MULTIPLICATION

inline efloat operator*(const long long lhs, const efloat& rhs) {
	return operator*(efloat(lhs), rhs);
}
// BINARY DIVISION

inline efloat operator/(const long long lhs, const efloat& rhs) {
	return operator/(efloat(lhs), rhs);
}

}} // namespace sw::universal
