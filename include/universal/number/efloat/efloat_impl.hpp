#pragma once
// efloat_impl.hpp: implementation of an adaptive precision binary floating-point number system
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
#include <vector>
#include <map>

// supporting types and functions
#include <universal/native/ieee754.hpp>   // IEEE-754 decoders
#include <universal/number/shared/specific_value_encoding.hpp>

/*
The efloat arithmetic can be configured to:
- throw exceptions on invalid arguments and operations
- return a signalling NaN

Compile-time configuration flags are used to select the exception mode.
Run-time configuration is used to select modular vs saturation arithmetic.

You need the exception types defined, but you have the option to throw them
*/
#include <universal/number/efloat/exceptions.hpp>

namespace sw { namespace universal {

// forward references
class efloat;
inline efloat& convert(int64_t v, efloat& result);
inline efloat& convert_unsigned(uint64_t v, efloat& result);
bool parse(const std::string& number, efloat& v);

// efloat is an adaptive precision linear floating-point type
class efloat {

public:
	efloat() : sign(false), exp(0) { }

	efloat(const efloat&) = default;
	efloat(efloat&&) = default;

	efloat& operator=(const efloat&) = default;
	efloat& operator=(efloat&&) = default;

	// initializers for native types
	efloat(signed char iv)                      noexcept { *this = iv; }
	efloat(short iv)                            noexcept { *this = iv; }
	efloat(int iv)                              noexcept { *this = iv; }
	efloat(long iv)                             noexcept { *this = iv; }
	efloat(long long iv)                        noexcept { *this = iv; }
	efloat(char iv)                             noexcept { *this = iv; }
	efloat(unsigned short iv)                   noexcept { *this = iv; }
	efloat(unsigned int iv)                     noexcept { *this = iv; }
	efloat(unsigned long iv)                    noexcept { *this = iv; }
	efloat(unsigned long long iv)               noexcept { *this = iv; }
	efloat(float iv)                            noexcept { *this = iv; }
	efloat(double iv)                           noexcept { *this = iv; }

	// assignment operators for native types
	efloat& operator=(signed char rhs)          noexcept { return convert_signed(rhs); }
	efloat& operator=(short rhs)                noexcept { return convert_signed(rhs); }
	efloat& operator=(int rhs)                  noexcept { return convert_signed(rhs); }
	efloat& operator=(long rhs)                 noexcept { return convert_signed(rhs); }
	efloat& operator=(long long rhs)            noexcept { return convert_signed(rhs); }
	efloat& operator=(char rhs)                 noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned short rhs)       noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned int rhs)         noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned long rhs)        noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned long long rhs)   noexcept { return convert_unsigned(rhs); }
	efloat& operator=(float rhs)                noexcept { return convert_ieee754(rhs); }
	efloat& operator=(double rhs)               noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float()             const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double()            const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	efloat(long double iv)                      noexcept { *this = iv; }
	efloat& operator=(long double rhs) noexcept { return convert_ieee754(rhs); }
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
	efloat& operator+=(double rhs) {
		return *this;
	}
	efloat& operator-=(const efloat& rhs) {
		return *this;
	}
	efloat& operator-=(double rhs) {
		return *this;
	}
	efloat& operator*=(const efloat& rhs) {
		return *this;
	}
	efloat& operator*=(double rhs) {
		return *this;
	}
	efloat& operator/=(const efloat& rhs) {
		return *this;
	}
	efloat& operator/=(double rhs) {
		return *this;
	}

	// modifiers
	void clear() { sign = false; exp = 0; limb.clear(); }
	void setzero() { clear(); }

	efloat& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	bool iszero() const { return !sign && limb.size() == 0; }
	bool isone() const  { return true; }
	bool isodd() const  { return false; }
	bool iseven() const { return !isodd(); }
	bool ispos() const  { return !sign; }
	bool ineg() const   { return sign; }
	int64_t scale() const { return exp; }

protected:
	bool                sign;  // sign of the number: -1 if true, +1 if false, zero is positive
	int64_t             exp;   // exponent of the number
	std::vector<double> limb;  // limbs of the representation

	// HELPER methods

	// convert arithmetic types into an elastic floating-point
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	efloat& convert_signed(SignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	efloat& convert_unsigned(UnsignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	efloat& convert_ieee754(Real rhs) noexcept {

		return *this;
	}


	// convert elastic floating-point to native ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real convert_to_ieee754() const noexcept {
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////    efloat functions   /////////////////////////////////


inline efloat abs(const efloat& a) {
	return a; // (a < 0 ? -a : a);
}

////////////////////////////////////////////////////////////////////////////////
/// stream operators

// read a efloat ASCII format and make a binary efloat out of it

bool parse(const std::string& number, efloat& value) {
	bool bSuccess = false;
	value.clear();
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
	ss << std::setw(width) << std::setprecision(prec) << "TBD";

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

inline bool operator==(const efloat& lhs, double rhs) {
	return operator==(lhs, efloat(rhs));
}

inline bool operator!=(const efloat& lhs, double rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const efloat& lhs, double rhs) {
	return operator<(lhs, efloat(rhs));
}

inline bool operator> (const efloat& lhs, double rhs) {
	return operator< (efloat(rhs), lhs);
}

inline bool operator<=(const efloat& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const efloat& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(double lhs, const efloat& rhs) {
	return operator==(efloat(lhs), rhs);
}

inline bool operator!=(double lhs, const efloat& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (double lhs, const efloat& rhs) {
	return operator<(efloat(lhs), rhs);
}

inline bool operator> (double lhs, const efloat& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(double lhs, const efloat& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(double lhs, const efloat& rhs) {
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

inline efloat operator+(const efloat& lhs, double rhs) {
	return operator+(lhs, efloat(rhs));
}
// BINARY SUBTRACTION

inline efloat operator-(const efloat& lhs, double rhs) {
	return operator-(lhs, efloat(rhs));
}
// BINARY MULTIPLICATION

inline efloat operator*(const efloat& lhs, double rhs) {
	return operator*(lhs, efloat(rhs));
}
// BINARY DIVISION

inline efloat operator/(const efloat& lhs, double rhs) {
	return operator/(lhs, efloat(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary arithmetic operators
// BINARY ADDITION

inline efloat operator+(double lhs, const efloat& rhs) {
	return operator+(efloat(lhs), rhs);
}
// BINARY SUBTRACTION

inline efloat operator-(double lhs, const efloat& rhs) {
	return operator-(efloat(lhs), rhs);
}
// BINARY MULTIPLICATION

inline efloat operator*(double lhs, const efloat& rhs) {
	return operator*(efloat(lhs), rhs);
}
// BINARY DIVISION

inline efloat operator/(double lhs, const efloat& rhs) {
	return operator/(efloat(lhs), rhs);
}

}} // namespace sw::universal
