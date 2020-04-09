#pragma once
// mpfloat.hpp: definition of a fixed-size arbitrary mpfloat precision number
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>

//#include "./mpfloat_exceptions.hpp"

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw {
namespace unum {

// forward references
class mpfloat;

inline void convert(int64_t v, mpfloat& result) {
}

inline void convert_unsigned(uint64_t v, mpfloat& result) {
}


bool parse(const std::string& number, mpfloat& v);

// mpfloat is an arbitrary precision and scale linear floating point type
class mpfloat {
public:
	mpfloat() { setzero(); }

	mpfloat(const mpfloat&) = default;
	mpfloat(mpfloat&&) = default;

	mpfloat& operator=(const mpfloat&) = default;
	mpfloat& operator=(mpfloat&&) = default;

	// initializers for native types
	mpfloat(const signed char initial_value)        { *this = initial_value; }
	mpfloat(const short initial_value)              { *this = initial_value; }
	mpfloat(const int initial_value)                { *this = initial_value; }
	mpfloat(const long initial_value)               { *this = initial_value; }
	mpfloat(const long long initial_value)          { *this = initial_value; }
	mpfloat(const char initial_value)               { *this = initial_value; }
	mpfloat(const unsigned short initial_value)     { *this = initial_value; }
	mpfloat(const unsigned int initial_value)       { *this = initial_value; }
	mpfloat(const unsigned long initial_value)      { *this = initial_value; }
	mpfloat(const unsigned long long initial_value) { *this = initial_value; }
	mpfloat(const float initial_value)              { *this = initial_value; }
	mpfloat(const double initial_value)             { *this = initial_value; }
	mpfloat(const long double initial_value)        { *this = initial_value; }

	// access operator for bits
	// this needs a proxy to be able to create l-values
	// bool operator[](const unsigned int i) const //

	// simpler interface for now, using at(i) and set(i)/reset(i)

	// assignment operators for native types
	mpfloat& operator=(const signed char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	mpfloat& operator=(const float rhs) {
		float_assign(rhs);
		return *this;
	}
	mpfloat& operator=(const double rhs) {
		float_assign(rhs);
		return *this;
	}
	mpfloat& operator=(const long double rhs) {
		float_assign(rhs);
		return *this;
	}

#ifdef ADAPTER_POSIT_AND_MPFLOAT
	// POSIT_CONCEPT_GENERALIZATION
	// TODO: SFINAE to assure we only match a posit<nbits,es> concept
	template<typename PositType>
	mpfloat& operator=(const PositType& rhs) {
		convert_p2i(rhs, *this);
		return *this;
	}
#endif // ADAPTER_POSIT_AND_MPFLOAT

	// prefix operators
	mpfloat operator-() const {
		mpfloat negated(*this);
		return negated;
	}
	// one's complement
	mpfloat operator~() const { 
		mpfloat complement(*this);
		return complement;
	}
	// increment
	mpfloat operator++(int) {
		mpfloat tmp(*this);
		operator++();
		return tmp;
	}
	mpfloat& operator++() {
		*this += mpfloat(1);
		return *this;
	}
	// decrement
	mpfloat operator--(int) {
		mpfloat tmp(*this);
		operator--();
		return tmp;
	}
	mpfloat& operator--() {
		*this -= mpfloat(1);
		return *this;
	}
	// conversion operators
	explicit operator float() const { return to_float(); }
	explicit operator double() const { return to_double(); }
	explicit operator long double() const { return to_long_double(); }

	// arithmetic operators
	mpfloat& operator+=(const mpfloat& rhs) {
		return *this;
	}
	mpfloat& operator-=(const mpfloat& rhs) {
		return *this;
	}
	mpfloat& operator*=(const mpfloat& rhs) {
		return *this;
	}
	mpfloat& operator/=(const mpfloat& rhs) {
		return *this;
	}

	// modifiers
	inline void clear() {  }
	inline void setzero() { clear(); }
	// use un-interpreted raw bits to set the bits of the mpfloat
	inline void set_raw_bits(unsigned long long value) {
		clear();
	}
	inline mpfloat& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	inline bool iszero() const {
		return true;
	}
	inline bool isone() const {
		return true;
	}
	inline bool isodd() const {
		return false;
	}
	inline bool iseven() const {
		return !isodd();
	}
	inline bool sign() const { return false; }

protected:
	// HELPER methods

	float to_float() const { 
		float f = 0;
		return f; 
	}
	double to_double() const {
		double d = 0;
		return d;
	}
	long double to_long_double() const {
		long double ld = 0;
		return ld;
	}

	template<typename Ty>
	void float_assign(Ty& rhs) {
		clear();
		long long base = (long long)rhs;
		*this = base;
	}

private:
	uint8_t b[1];

	// convert
	friend std::string str(const mpfloat& value);

	// mpfloat - mpfloat logic comparisons
	friend bool operator==(const mpfloat& lhs, const mpfloat& rhs);

	// mpfloat - literal logic comparisons
	friend bool operator==(const mpfloat& lhs, const long long rhs);

	// literal - mpfloat logic comparisons
	friend bool operator==(const long long lhs, const mpfloat& rhs);

	// find the most significant bit set
	friend signed findMsb(const mpfloat& v);
};

////////////////////////    MPFLOAT functions   /////////////////////////////////


inline mpfloat abs(const mpfloat& a) {
	return a; // (a < 0 ? -a : a);
}


// convert mpfloat to decimal string

std::string str(const mpfloat& value) {
	std::stringstream ss;
	return ss.str();
}

// findMsb takes an mpfloat reference and returns the position of the most significant bit, -1 if v == 0

inline signed findMsb(const mpfloat& v) {
	return -1; // no significant bit found, all bits are zero
}

////////////////////////    INTEGER operators   /////////////////////////////////

// divide mpfloat a and b and return result argument

void divide(const mpfloat& a, const mpfloat& b, mpfloat& quotient) {
}

/// stream operators

// read a mpfloat ASCII format and make a binary mpfloat out of it

bool parse(const std::string& number, mpfloat& value) {
	bool bSuccess = false;

	return bSuccess;
}

// generate an mpfloat format ASCII format

inline std::ostream& operator<<(std::ostream& ostr, const mpfloat& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the mpfloat into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << str(i);

	return ostr << ss.str();
}

// read an ASCII mpfloat format

inline std::istream& operator>>(std::istream& istr, mpfloat& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// mpfloat - mpfloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths

inline bool operator==(const mpfloat& lhs, const mpfloat& rhs) {
	return true;
}

inline bool operator!=(const mpfloat& lhs, const mpfloat& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const mpfloat& lhs, const mpfloat& rhs) {
	return false; // lhs and rhs are the same
}

inline bool operator> (const mpfloat& lhs, const mpfloat& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const mpfloat& lhs, const mpfloat& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const mpfloat& lhs, const mpfloat& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// mpfloat - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const mpfloat& lhs, const long long rhs) {
	return operator==(lhs, mpfloat(rhs));
}

inline bool operator!=(const mpfloat& lhs, const long long rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const mpfloat& lhs, const long long rhs) {
	return operator<(lhs, mpfloat(rhs));
}

inline bool operator> (const mpfloat& lhs, const long long rhs) {
	return operator< (mpfloat(rhs), lhs);
}

inline bool operator<=(const mpfloat& lhs, const long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const mpfloat& lhs, const long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - mpfloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(const long long lhs, const mpfloat& rhs) {
	return operator==(mpfloat(lhs), rhs);
}

inline bool operator!=(const long long lhs, const mpfloat& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const long long lhs, const mpfloat& rhs) {
	return operator<(mpfloat(lhs), rhs);
}

inline bool operator> (const long long lhs, const mpfloat& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const long long lhs, const mpfloat& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const long long lhs, const mpfloat& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// mpfloat - mpfloat binary arithmetic operators
// BINARY ADDITION

inline mpfloat operator+(const mpfloat& lhs, const mpfloat& rhs) {
	mpfloat sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION

inline mpfloat operator-(const mpfloat& lhs, const mpfloat& rhs) {
	mpfloat diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION

inline mpfloat operator*(const mpfloat& lhs, const mpfloat& rhs) {
	mpfloat mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION

inline mpfloat operator/(const mpfloat& lhs, const mpfloat& rhs) {
	mpfloat ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// mpfloat - literal binary arithmetic operators
// BINARY ADDITION

inline mpfloat operator+(const mpfloat& lhs, const long long rhs) {
	return operator+(lhs, mpfloat(rhs));
}
// BINARY SUBTRACTION

inline mpfloat operator-(const mpfloat& lhs, const long long rhs) {
	return operator-(lhs, mpfloat(rhs));
}
// BINARY MULTIPLICATION

inline mpfloat operator*(const mpfloat& lhs, const long long rhs) {
	return operator*(lhs, mpfloat(rhs));
}
// BINARY DIVISION

inline mpfloat operator/(const mpfloat& lhs, const long long rhs) {
	return operator/(lhs, mpfloat(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - mpfloat binary arithmetic operators
// BINARY ADDITION

inline mpfloat operator+(const long long lhs, const mpfloat& rhs) {
	return operator+(mpfloat(lhs), rhs);
}
// BINARY SUBTRACTION

inline mpfloat operator-(const long long lhs, const mpfloat& rhs) {
	return operator-(mpfloat(lhs), rhs);
}
// BINARY MULTIPLICATION

inline mpfloat operator*(const long long lhs, const mpfloat& rhs) {
	return operator*(mpfloat(lhs), rhs);
}
// BINARY DIVISION

inline mpfloat operator/(const long long lhs, const mpfloat& rhs) {
	return operator/(mpfloat(lhs), rhs);
}

} // namespace unum
} // namespace sw
