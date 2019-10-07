#pragma once
// integer.hpp: definition of arbitrary integer configurations
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>

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
template<size_t nbits> class integer;
template<size_t nbits> integer<nbits> max_int();
template<size_t nbits> integer<nbits> min_int();

template<size_t nbits>
inline integer<nbits> max_int() {
	// two's complement max is 01111111
	integer<nbits> mx;
	mx.set_raw_bits(0x7F); // TODO
	return mx;
}
template<size_t nbits>
inline integer<nbits> min_int() {
	// two's complement min is 10000000
	integer<nbits> mn;
	mn.set_raw_bits(0x80);  // TODO
	return mn;
}

template<size_t nbits>
class integer {
public:
	static constexpr size_t nrBytes = (1 + ((nbits - 1) / 8));

	integer() { setzero(); }

	integer(const integer&) = default;
	integer(integer&&) = default;

	integer& operator=(const integer&) = default;
	integer& operator=(integer&&) = default;

	/// Construct a new integer from another
	template<size_t srcbits>
	integer(const integer<srcbits>& a) {
		static_assert(srcbits > nbits, "Source integer is bigger than target: potential loss of precision");

	}

	// initializers for native types
	integer(const signed char initial_value) { *this = initial_value; }
	integer(const short initial_value) { *this = initial_value; }
	integer(const int initial_value) { *this = initial_value; }
	integer(const long initial_value) { *this = initial_value; }
	integer(const long long initial_value) { *this = initial_value; }
	integer(const char initial_value) { *this = initial_value; }
	integer(const unsigned short initial_value) { *this = initial_value; }
	integer(const unsigned int initial_value) { *this = initial_value; }
	integer(const unsigned long initial_value) { *this = initial_value; }
	integer(const unsigned long long initial_value) { *this = initial_value; }
	integer(const float initial_value) { *this = initial_value; }
	integer(const double initial_value) { *this = initial_value; }
	integer(const long double initial_value) { *this = initial_value; }

	// access operator for bits
	bool operator[](const unsigned int i) const {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t mask = 1 << (i % 8);
			return (byte & mask);
		}
		throw "bit index out of bounds";
	}
	// assignment operators for native types
	integer& operator=(const signed char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const float rhs) {
		return float_assign(rhs);
	}
	integer& operator=(const double rhs) {
		return float_assign(rhs);
	}
	integer& operator=(const long double rhs) {
		return float_assign(rhs);
	}

	// conversion operators
// Maybe remove explicit, MTL compiles, but we have lots of double computation then
	explicit operator unsigned short() const { return to_ushort(); }
	explicit operator unsigned int() const { return to_uint(); }
	explicit operator unsigned long() const { return to_ulong(); }
	explicit operator unsigned long long() const { return to_ulong_long(); }
	explicit operator short() const { return to_short(); }
	explicit operator int() const { return to_int(); }
	explicit operator long() const { return to_long(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator float() const { return to_float(); }
	explicit operator double() const { return to_double(); }
	explicit operator long double() const { return to_long_double(); }

	// arithmetic operators
	integer& operator+=(const integer& rhs) {
		return *this;
	}
	integer& operator-=(const integer& rhs) {
		return *this;
	}
	integer& operator*=(const integer& rhs) {
		return *this;
	}
	integer& operator/=(const integer& rhs) {
		return *this;
	}
	integer& operator%=(const integer& rhs) {
		return *this;
	}

	// modifiers
	inline void clear() { std::memset(&b, 0, nrBytes); }
	inline void setzero() { clear(); }
	// use un-interpreted raw bits to set the bits of the integer
	void set_raw_bits(unsigned long long value) {
		for (unsigned i = 0; i < nrBytes; ++i) {
			b[i] = value & 0xFF;
			value >>= 1;
		}
	}

	// selectors
	inline bool iszero() const {
		for (unsigned i = 0; i < nrBytes; ++i) {
			if (b[i] != 0x00) return false;
		}
		return true;
	}
protected:
	// HELPER methods
	uint8_t byte(unsigned int i) const {
		if (i < nrBytes) {
			return b[i];
		}
		throw "byte index out of bound";
	}
	// conversion functions
	short to_short() const {
		short s = 0;
		return s;
	}
	int to_int() const {
		int i = 0;
		return i;
	}
	long to_long() const {
		long l = 0;
		return l;
	}
	long long to_long_long() const {
		long long ll = 0;
		return ll;
	}
	unsigned short to_ushort() const {
		if (iszero()) return 0;
		unsigned short us;
		char* p = (char*)&us;
		*p = b[0];
		*(p + 1) = b[1];
		return us;
	}
	unsigned int to_uint() const {
		unsigned int ui;
		char* p = (char*)&ui;
		*p = b[0];
		*(p + 1) = b[1];
		*(p + 2) = b[2];
		*(p + 3) = b[3];
		return ui;
	}
	unsigned long to_ulong() const {
		unsigned long ul = 0;
		return ul;
	}
	unsigned long long to_ulong_long() const {
		unsigned long long ull = 0;
		return ull;
	}
	float to_float() const { return 0.0f; }
	double to_double() const { return 0.0; }
	long double to_long_double() const { return 0.0l; }

	template<typename Ty>
	integer& float_assign(Ty& rhs) {

	}

private:
	//array<uint8_t, (1 + ((nbits - 1) / 8))> bytes;
	uint8_t b[nrBytes];

	// convert
	template<size_t nnbits>
	friend std::string convert_to_decimal_string(const integer<nnbits>& value);

	// integer - integer logic comparisons
	template<size_t nnbits>
	friend bool operator==(const integer<nnbits>& lhs, const integer<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator!=(const integer<nnbits>& lhs, const integer<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator< (const integer<nnbits>& lhs, const integer<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator> (const integer<nnbits>& lhs, const integer<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator<=(const integer<nnbits>& lhs, const integer<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator>=(const integer<nnbits>& lhs, const integer<nnbits>& rhs);
};

void add(std::vector<char>& lhs, const std::vector<char>& rhs) {
	std::vector<char> _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
//	if (negative != rhs.negative) {  // different signs
	//	_rhs.setsign(!rhs.sign());
//		return operator-=(_rhs);
//	}
//	else {
		// same sign implies this->negative is invariant
//	}
	size_t l = lhs.size();
	size_t r = _rhs.size();
	// zero pad the shorter decimal
	if (l < r) {
		lhs.insert(lhs.end(), r - l, 0);
	}
	else {
		_rhs.insert(_rhs.end(), l - r, 0);
	}
	std::vector<char>::iterator lit = lhs.begin();
	std::vector<char>::iterator rit = _rhs.begin();
	char carry = 0;
	for (; lit != lhs.end() || rit != _rhs.end(); ++lit, ++rit) {
		*lit += *rit + carry;
		if (*lit > 9) {
			carry = 1;
			*lit -= 10;
		}
		else {
			carry = 0;
		}
	}
	if (carry) lhs.push_back(1);
}

// helper to deal with multiplying decimal representations
std::vector<char> mul(const std::vector<char>& lhs, const std::vector<char>& rhs) {
	bool signOfFinalResult = false; // (lhs.negative != rhs.negative) ? true : false;
	std::vector<char> product;
	// find the smallest decimal to minimize the amount of work
	size_t l = lhs.size();
	size_t r = rhs.size();
	std::vector<char>::const_iterator sit, bit; // sit = smallest iterator, bit = biggest iterator
	if (l < r) {
		size_t position = 0;
		for (sit = lhs.begin(); sit != lhs.end(); ++sit) {
			std::vector<char> partial_sum;
			partial_sum.insert(partial_sum.end(), r + position, 0);
			std::vector<char>::iterator pit = partial_sum.begin() + position;
			char carry = 0;
			for (bit = rhs.begin(); bit != rhs.end() || pit != partial_sum.end(); ++bit, ++pit) {
				char digit = *sit * *bit + carry;
				*pit = digit % 10;
				carry = digit / 10;
			}
			if (carry) partial_sum.push_back(carry);
			add(product, partial_sum);
//			std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
			++position;
		}
	}
	else {
		size_t position = 0;
		for (sit = rhs.begin(); sit != rhs.end(); ++sit) {
			std::vector<char> partial_sum;
			partial_sum.insert(partial_sum.end(), l + position, 0);
			std::vector<char>::iterator pit = partial_sum.begin() + position;
			char carry = 0;
			for (bit = lhs.begin(); bit != lhs.end() || pit != partial_sum.end(); ++bit, ++pit) {
				char digit = *sit * *bit + carry;
				*pit = digit % 10;
				carry = digit / 10;
			}
			if (carry) partial_sum.push_back(carry);
			add(product, partial_sum);
//			std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
			++position;
		}
	}
//	product.unpad();
	return product;
//	setsign(signOfFinalResult);
}

////////////////// INTEGER operators

// convert integer to decimal string
template<size_t nbits>
std::string convert_to_decimal_string(const integer<nbits>& value) {
	if (value.iszero()) {
		return std::string("0");
	}
	constexpr size_t nrBytes = value.nrBytes;
	std::vector<char> partial, multiplier;
	partial.push_back('1');
	multiplier.push_back('1');
	// convert integer to decimal by multiplication by powers of 2
	for (unsigned i = 0; i < nbits; ++i) {
		if (value[i]) {

		}
	}
}
/// stream operators

// read a integer ASCII format and make a binary integer out of it
template<size_t nbits>
bool parse(std::string& txt, integer<nbits>& i) {
	bool bSuccess = false;
	// check if the txt is an integer form: [0123456789]+
	std::regex decimal_regex("[0123456789]+");
	std::regex octal_regex("[0][01234567]+");
	std::regex hex_regex("[0x][0123456789aAbBcCdDeEfF]");
	if (std::regex_match(txt, decimal_regex)) {
		std::cout << "found a decimal integer representation\n";

		bSuccess = false; // TODO
	}
	else if (std::regex_match(txt, octal_regex)) {
		std::cout << "found an octal representation\n";

		bSuccess = false; // TODO
	}
	else if (std::regex_match(txt, hex_regex)) {
		std::cout << "found a hexadecimal representation\n";

		bSuccess = false;  // TODO
	}
	return bSuccess;
}

// generate an integer format ASCII format
template<size_t nbits>
inline std::ostream& operator<<(std::ostream& ostr, const integer<nbits>& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << convert_to_decimal_string(i);

	return ostr << ss.str();
}

// read an ASCII integer format
template<size_t nbits>
inline std::istream& operator>> (std::istream& istr, integer<nbits>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators
template<size_t nbits>
inline std::string to_binary(const integer<nbits>& number) {
	std::stringstream ss;
	ss << "TBD";
	return ss.str();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// integer binary logic operators

template<size_t nbits>
inline bool operator==(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	for (unsigned i = 0; i < lhs.nrBytes; ++i) {
		if (lhs.b[i] != rhs.b[i]) return false;
	}
	return true;
}
template<size_t nbits>
inline bool operator!=(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits>
inline bool operator< (const integer<nbits>& lhs, const integer<nbits>& rhs) {
	return true; // TODO
}
template<size_t nbits>
inline bool operator> (const integer<nbits>& lhs, const integer<nbits>& rhs) {
	return operator< (rhs, lhs);
}
template<size_t nbits>
inline bool operator<=(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits>
inline bool operator>=(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	return !operator< (lhs, rhs);
}

/////////////////////////////////////////////////////////////////////////////////////////
// intege binary arithmetic operators
// BINARY ADDITION
template<size_t nbits>
inline integer<nbits> operator+(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	integer<nbits> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits>
inline integer<nbits> operator-(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	integer<nbits> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits>
inline integer<nbits> operator*(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	integer<nbits> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits>
inline integer<nbits> operator/(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	integer<nbits> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// BINARY REMAINDER
template<size_t nbits>
inline integer<nbits> operator%(const integer<nbits>& lhs, const integer<nbits>& rhs) {
	integer<nbits> ratio = lhs;
	ratio /= rhs;
	return ratio;
}


} // namespace unum
} // namespace sw
