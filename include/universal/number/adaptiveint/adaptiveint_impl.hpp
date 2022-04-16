#pragma once
// adaptiveint_impl.hpp: implementation of an adaptive precision binary integer
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

#include <universal/number/adaptiveint/exceptions.hpp>

namespace sw { namespace universal {

// forward references
class adaptiveint;
inline adaptiveint& convert(int64_t v, adaptiveint& result);
inline adaptiveint& convert_unsigned(uint64_t v, adaptiveint& result);
bool parse(const std::string& number, adaptiveint& v);

// adaptiveint is an adaptive precision integer type
class adaptiveint {
	using BlockType = uint32_t;
	static constexpr unsigned BITS_IN_BLOCK = 32;
public:
	adaptiveint() : _sign(false), _blocks(0) { }

	adaptiveint(const adaptiveint&) = default;
	adaptiveint(adaptiveint&&) = default;

	adaptiveint& operator=(const adaptiveint&) = default;
	adaptiveint& operator=(adaptiveint&&) = default;

	// initializers for native types
	explicit adaptiveint(const signed char initial_value)        { *this = initial_value; }
	explicit adaptiveint(const short initial_value)              { *this = initial_value; }
	explicit adaptiveint(const int initial_value)                { *this = initial_value; }
	explicit adaptiveint(const long initial_value)               { *this = initial_value; }
	explicit adaptiveint(const long long initial_value)          { *this = initial_value; }
	explicit adaptiveint(const char initial_value)               { *this = initial_value; }
	explicit adaptiveint(const unsigned short initial_value)     { *this = initial_value; }
	explicit adaptiveint(const unsigned int initial_value)       { *this = initial_value; }
	explicit adaptiveint(const unsigned long initial_value)      { *this = initial_value; }
	explicit adaptiveint(const unsigned long long initial_value) { *this = initial_value; }
	explicit adaptiveint(const float initial_value)              { *this = initial_value; }
	explicit adaptiveint(const double initial_value)             { *this = initial_value; }
	explicit adaptiveint(const long double initial_value)        { *this = initial_value; }

	// assignment operators for native types
	adaptiveint& operator=(const signed char rhs)        { return convert(rhs, *this); }
	adaptiveint& operator=(const short rhs)              { return convert(rhs, *this); }
	adaptiveint& operator=(const int rhs)                { return convert(rhs, *this); }
	adaptiveint& operator=(const long rhs)               { return convert(rhs, *this); }
	adaptiveint& operator=(const long long rhs)          { return convert(rhs, *this); }
	adaptiveint& operator=(const char rhs)               { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(const unsigned short rhs)     { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(const unsigned int rhs)       { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(const unsigned long rhs)      { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(const unsigned long long rhs) { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(const float rhs)              { return float_assign(rhs); }
	adaptiveint& operator=(const double rhs)             { return float_assign(rhs); }
	adaptiveint& operator=(const long double rhs)        { return float_assign(rhs); }

	// prefix operators
	adaptiveint operator-() const {
		adaptiveint negated(*this);
		negated.setsign(!_sign);
		return negated;
	}

	// conversion operators
	explicit operator float() const { return float(toNativeFloatingPoint()); }
	explicit operator double() const { return float(toNativeFloatingPoint()); }
	explicit operator long double() const { return toNativeFloatingPoint(); }

	// arithmetic operators
	adaptiveint& operator+=(const adaptiveint& rhs) {
		return *this;
	}
	adaptiveint& operator-=(const adaptiveint& rhs) {
		return *this;
	}
	adaptiveint& operator*=(const adaptiveint& rhs) {
		return *this;
	}
	adaptiveint& operator/=(const adaptiveint& rhs) {
		return *this;
	}

	// modifiers
	inline void clear() noexcept { _sign = false; _blocks.clear(); }
	inline void setzero() noexcept { clear(); }
	inline void setsign(bool sign = true) noexcept { _sign = sign; }
	// use un-interpreted raw bits to set the bits of the adaptiveint
	inline void setbits(unsigned long long value) {
		clear();
		std::uint32_t low  = static_cast<std::uint32_t>(value & 0x0000'0000'FFFF'FFFF);
		std::uint32_t high = static_cast<std::uint32_t>((value & 0xFFFF'FFFF'0000'0000) >> BITS_IN_BLOCK);
		if (high > 0) {
			_blocks.push_back(high);
			_blocks.push_back(low);
		}
		else if (low > 0) {
			_blocks.push_back(low);
		}
	}
	inline adaptiveint& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	inline bool iszero() const noexcept { return !_sign && _blocks.size() == 0; }
	inline bool isone()  const noexcept { return true; }
	inline bool isodd()  const noexcept { return (_blocks.size() > 0) ? (_blocks[0] & 0x1) : false; }
	inline bool iseven() const noexcept { return !isodd(); }
	inline bool ispos()  const noexcept { return !_sign; }
	inline bool ineg()   const noexcept { return _sign; }

	inline int scale()   const noexcept { return findMsb(); }

	inline std::uint32_t block(unsigned b) const noexcept {
		if (b < _blocks.size()) {
			return _blocks[b];
		}
		return 0u;
	}
	inline unsigned limbs() const noexcept { return _blocks.size(); }


	// findMsb takes an adaptiveint reference and returns the position of the most significant bit, -1 if v == 0
	inline int findMsb() const noexcept {
		int nrBlocks = _blocks.size();
		if (nrBlocks == 0) return -1; // no significant bit found, all bits are zero
		int msb = nrBlocks * BITS_IN_BLOCK;
		for (int b = nrBlocks - 1; b >= 0; --b) {
			std::uint32_t segment = _blocks[b];
			std::uint32_t mask = 0x8000'0000;
			for (int i = BITS_IN_BLOCK - 1; i >= 0; --i) {
				--msb;
				if (segment & mask) return msb;
				mask >>= 1;
			}
		}
		return -1; // no significant bit found, all bits are zero
	}

	// convert to string containing digits number of digits
	std::string str(size_t nrDigits = 0) const {

		return std::string("tbd");
	}

protected:
	bool                   _sign;    // sign of the number: -1 if true, +1 if false, zero is positive
	std::vector<BlockType> _blocks;  // building blocks representing a 1's complement magnitude

	// HELPER methods



	// convert to native floating-point, use conversion rules to cast down to float and double
	long double toNativeFloatingPoint() const {
		long double ld = 0;
		return ld;
	}

	template<typename Ty>
	adaptiveint& float_assign(Ty& rhs) {
		clear();
		long long base = (long long)rhs;
		*this = base;
		return *this;
	}

private:

	// adaptiveint - adaptiveint logic comparisons
	friend bool operator==(const adaptiveint& lhs, const adaptiveint& rhs);

	// adaptiveint - literal logic comparisons
	friend bool operator==(const adaptiveint& lhs, const long long rhs);

	// literal - adaptiveint logic comparisons
	friend bool operator==(const long long lhs, const adaptiveint& rhs);

	// find the most significant bit set
	friend signed findMsb(const adaptiveint& v);
};

inline adaptiveint& convert(int64_t v, adaptiveint& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

inline adaptiveint& convert_unsigned(uint64_t v, adaptiveint& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		// convert 
	}
	return result;
}

////////////////////////    adaptiveint functions   /////////////////////////////////


inline adaptiveint abs(const adaptiveint& a) {
	return a; // (a < 0 ? -a : a);
}

////////////////////////    INTEGER operators   /////////////////////////////////

// divide adaptiveint a and b and return result argument

void divide(const adaptiveint& a, const adaptiveint& b, adaptiveint& quotient) {
}

/// stream operators

// read a adaptiveint ASCII format and make a binary adaptiveint out of it

bool parse(const std::string& number, adaptiveint& value) {
	bool bSuccess = false;

	return bSuccess;
}

// generate an adaptiveint format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const adaptiveint& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the adaptiveint into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));

	return ostr << ss.str();
}

// read an ASCII adaptiveint format

inline std::istream& operator>>(std::istream& istr, adaptiveint& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators

inline std::string to_binary(const adaptiveint& a, bool nibbleMarker = true) {
	if (a.limbs() == 0) return std::string("0x0");

	std::stringstream s;
	s << "0x";
	for (int b = a.limbs() - 1; b >= 0; --b) {
		std::uint32_t segment = a.block(b);
		std::uint32_t mask = 0x8000'0000;
		for (int i = 31; i >= 0; --i) {
			s << ((segment & mask) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
			mask >>= 1;
		}
	}

	return s.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - adaptiveint binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths

inline bool operator==(const adaptiveint& lhs, const adaptiveint& rhs) {
	return true;
}

inline bool operator!=(const adaptiveint& lhs, const adaptiveint& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const adaptiveint& lhs, const adaptiveint& rhs) {
	return false; // lhs and rhs are the same
}

inline bool operator> (const adaptiveint& lhs, const adaptiveint& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const adaptiveint& lhs, const adaptiveint& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const adaptiveint& lhs, const adaptiveint& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const adaptiveint& lhs, const long long rhs) {
	return operator==(lhs, adaptiveint(rhs));
}

inline bool operator!=(const adaptiveint& lhs, const long long rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const adaptiveint& lhs, const long long rhs) {
	return operator<(lhs, adaptiveint(rhs));
}

inline bool operator> (const adaptiveint& lhs, const long long rhs) {
	return operator< (adaptiveint(rhs), lhs);
}

inline bool operator<=(const adaptiveint& lhs, const long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const adaptiveint& lhs, const long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - adaptiveint binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(const long long lhs, const adaptiveint& rhs) {
	return operator==(adaptiveint(lhs), rhs);
}

inline bool operator!=(const long long lhs, const adaptiveint& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const long long lhs, const adaptiveint& rhs) {
	return operator<(adaptiveint(lhs), rhs);
}

inline bool operator> (const long long lhs, const adaptiveint& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const long long lhs, const adaptiveint& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const long long lhs, const adaptiveint& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - adaptiveint binary arithmetic operators
// BINARY ADDITION

inline adaptiveint operator+(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION

inline adaptiveint operator-(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION

inline adaptiveint operator*(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION

inline adaptiveint operator/(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - literal binary arithmetic operators
// BINARY ADDITION

inline adaptiveint operator+(const adaptiveint& lhs, const long long rhs) {
	return operator+(lhs, adaptiveint(rhs));
}
// BINARY SUBTRACTION

inline adaptiveint operator-(const adaptiveint& lhs, const long long rhs) {
	return operator-(lhs, adaptiveint(rhs));
}
// BINARY MULTIPLICATION

inline adaptiveint operator*(const adaptiveint& lhs, const long long rhs) {
	return operator*(lhs, adaptiveint(rhs));
}
// BINARY DIVISION

inline adaptiveint operator/(const adaptiveint& lhs, const long long rhs) {
	return operator/(lhs, adaptiveint(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - adaptiveint binary arithmetic operators
// BINARY ADDITION

inline adaptiveint operator+(const long long lhs, const adaptiveint& rhs) {
	return operator+(adaptiveint(lhs), rhs);
}
// BINARY SUBTRACTION

inline adaptiveint operator-(const long long lhs, const adaptiveint& rhs) {
	return operator-(adaptiveint(lhs), rhs);
}
// BINARY MULTIPLICATION

inline adaptiveint operator*(const long long lhs, const adaptiveint& rhs) {
	return operator*(adaptiveint(lhs), rhs);
}
// BINARY DIVISION

inline adaptiveint operator/(const long long lhs, const adaptiveint& rhs) {
	return operator/(adaptiveint(lhs), rhs);
}

}} // namespace sw::universal
