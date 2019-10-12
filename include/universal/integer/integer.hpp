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
#include <vector>

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
inline void convert(int64_t v, integer<nbits>& result) {
	constexpr uint64_t mask = 0x1;
	bool negative = (v < 0 ? true : false);
	result.clear();
	unsigned upper = (nbits <= 64 ? nbits : 64);
	for (unsigned i = 0; i < upper && v != 0; ++i) {
		if (v & mask) result.set(i);
		v >>= 1;
	}
	if (nbits > 64 && negative) {
		// sign extend
		for (unsigned i = upper; i < nbits; ++i) {
			result.set(i);
		}
	}
}
template<size_t nbits>
inline void convert_unsigned(uint64_t v, integer<nbits>& result) {
	constexpr uint64_t mask = 0x1;
	result.clear();
	unsigned upper = (nbits <= 64 ? nbits : 64);
	for (unsigned i = 0; i < upper; ++i) {
		if (v & mask) result.set(i);
		v >>= 1;
	}
}

/*
The rules for detecting overflow in a two's complement sum are simple:
 - If the sum of two positive numbers yields a negative result, the sum has overflowed.
 - If the sum of two negative numbers yields a positive result, the sum has overflowed.
 - Otherwise, the sum has not overflowed.
It is important to note the overflow and carry out can each occur without the other. 
In unsigned numbers, carry out is equivalent to overflow. In two's complement, carry out tells 
you nothing about overflow.

The reason for the rules is that overflow in two's complement occurs, not when a bit is carried out 
out of the left column, but when one is carried into it. That is, when there is a carry into the sign. 
The rules detect this error by examining the sign of the result. A negative and positive added together 
cannot overflow, because the sum is between the addends. Since both of the addends fit within the
allowable range of numbers, and their sum is between them, it must fit as well.

When implementing addition/subtraction on chuncks the overflow condition must be deduced from the 
chunk values. The chunks need to be interpreted as unsigned binary segments.
*/
// integer is an arbitrary size 2's complement integer
template<size_t nbits>
class integer {
public:
	static constexpr unsigned nrBytes = (1 + ((nbits - 1) / 8));
	static constexpr unsigned MS_BYTE = nrBytes - 1;
	static constexpr uint8_t MS_BYTE_MASK = (0xFF >> (nrBytes * 8 - nbits));

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
	// this needs a proxy to be able to create l-values
	// bool operator[](const unsigned int i) const //

	// simpler interface for now, using at(i) and set(i)/reset(i)

	// assignment operators for native types
	integer& operator=(const signed char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
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

	// one's complement
	integer operator~() const { 
		integer<nbits> complement(*this);
		complement.flip(); 
		return complement;
	}

	integer operator++(int) {
		integer<nbits> tmp(*this);
		operator++();
		return tmp;
	}
	integer& operator++() {
		*this += integer<nbits>(1);
		return *this;
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
		integer<nbits> sum;
		bool carry = false;
		for (unsigned i = 0; i < nrBytes; ++i) {
			// cast up so we can test for overflow
			uint16_t l = uint16_t(b[i]);
			uint16_t r = uint16_t(rhs.b[i]);
			uint16_t s = l + r + (carry ? uint16_t(0x0001) : uint16_t(0x0000));
			carry = (s > 255 ? true : false);
			sum.b[i] = (uint8_t)(s & 0xFF);
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		sum.b[MS_BYTE] = MS_BYTE_MASK & sum.b[MS_BYTE];
		//if (carry) throw "overflow";
		*this = sum;
		return *this;
	}
	integer& operator-=(const integer& rhs) {
		operator+=(twos_complement(rhs));
		return *this;
	}
	integer& operator*=(const integer& rhs) {
		integer<nbits> base(*this);
		integer<nbits> multiplicant(rhs);
		clear();
		for (unsigned i = 0; i < nbits; ++i) {
			if (base.at(i)) {
				operator+=(multiplicant);
			}
			multiplicant <<= 1;
		}
		return *this;
	}
	integer& operator/=(const integer& rhs) {
		return *this;
	}
	integer& operator%=(const integer& rhs) {
		return *this;
	}
	integer& operator<<=(const unsigned shift) {
		integer<nbits> target;
		for (unsigned i = shift; i < nbits; ++i) {  // TODO: inefficient as it works at the bit level
			target.set(i, at(i - shift));
		}
		*this = target;
		return *this;
	}
	integer& operator>>=(const unsigned shift) {
		integer<nbits> target;
		for (int i = nbits - 1; i >= int(shift); --i) {  // TODO: inefficient as it works at the bit level
			target.set(i - shift, at(i));
		}
		*this = target;
		return *this;
	}
	// modifiers
	inline void clear() { std::memset(&b, 0, nrBytes); }
	inline void setzero() { clear(); }
	inline void set(unsigned int i) {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t mask = 1 << (i % 8);
			b[i / 8] = byte | mask;
			return;
		}
		throw "integer<nbits> bit index out of bounds";
	}
	inline void reset(unsigned int i) {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t mask = ~(1 << (i % 8));
			b[i / 8] = byte & mask;
			return;
		}
		throw "integer<nbits> bit index out of bounds";
	}
	inline void set(unsigned i, bool v) {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t null = ~(1 << (i % 8));
			uint8_t bit = (v ? 1 : 0);
			uint8_t mask = (bit << (i % 8));
			b[i / 8] = (byte & null) | mask;
			return;
		}
		throw "integer<nbits> bit index out of bounds";
	}
	// use un-interpreted raw bits to set the bits of the integer
	void set_raw_bits(unsigned long long value) {
		clear();
		for (unsigned i = 0; i < nrBytes; ++i) {
			b[i] = value & 0xFF;
			value >>= 8;
		}
	}
	// in-place one's complement
	inline void flip() {
		for (unsigned i = 0; i < nrBytes; ++i) {
			b[i] = ~b[i];
		}
	}

	// selectors
	inline bool iszero() const {
		for (unsigned i = 0; i < nrBytes; ++i) {
			if (b[i] != 0x00) return false;
		}
		return true;
	}
	inline bool sign() const { return at(nbits - 1); }
	inline bool at(unsigned int i) const {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t mask = 1 << (i % 8);
			return (byte & mask);
		}
		throw "bit index out of bounds";
	}
	inline signed findMsb() const {
		for (signed i = nrBytes - 1; i >= 0; --i) {
			if (b[i] != 0) {
				uint8_t mask = 0x80;
				for (signed j = 7; j >= 0; --j) {
					if (b[i] & mask) {
						return i*8 + j;
					}
					mask >>= 1;
				}
			}
		}
		return -1;
	}
	// divide bitsets a and b and return result in bitset result.
	template<size_t nbits>
	void divide(const integer<nbits>& a, const integer<nbits>& b, integer<2 * nbits>& result) {
		integer<nbits> subtractand, accumulator;
		result.reset();
		accumulator = a;
		int msb = findMsb(b);
		if (msb < 0) {
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
			throw integer_divide_by_zero{};
#else
			std::cerr << "integer_divide_by_zero\n";
#endif // INTEGER_THROW_ARITHMETIC_EXCEPTION
		}
		else {
			int shift = operand_size - msb - 1;
			// prepare the subtractand
			subtractand = b;
			subtractand <<= shift;
			for (int i = operand_size - msb - 1; i >= 0; --i) {
				if (subtractand <= accumulator) {
#ifdef DEBUG
					bool borrow = subtract(accumulator, subtractand);
					assert(borrow == true);
#else
					accumulator -= subtractand;
#endif
					result.set(i);
				}
				else {
					result.reset(i);
				}
				subtractand >>= 1;
			}
		}
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
		short mask = 1;
		unsigned upper = (nbits < 8 * sizeof(short) ? nbits : 8 * sizeof(short));
		for (unsigned i = 0; i < upper; ++i) {
			s |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < nbits) { // sign extend
			for (unsigned i = upper; i < nbits; ++i) {
				s |= mask;
				mask <<= 1;
			}
		}
		return s;
	}
	int to_int() const {
		int i = 0;
		int mask = 1;
		unsigned upper = (nbits < 8 * sizeof(int) ? nbits : 8 * sizeof(int));
		for (unsigned i = 0; i < upper; ++i) {
			i |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < nbits) { // sign extend
			for (unsigned i = upper; i < nbits; ++i) {
				i |= mask;
				mask <<= 1;
			}
		}
		return i;
	}
	long to_long() const {
		long l = 0;
		long mask = 1;
		unsigned upper = (nbits < 8 * sizeof(long) ? nbits : 8 * sizeof(long));
		for (unsigned i = 0; i < upper; ++i) {
			l |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < nbits) { // sign extend
			for (unsigned i = upper; i < nbits; ++i) {
				l |= mask;
				mask <<= 1;
			}
		}
		return l;
	}
	long long to_long_long() const {
		long long ll = 0;
		long long mask = 1;
		unsigned upper = (nbits < 8 * sizeof(long long) ? nbits : 8 * sizeof(long long));
		for (unsigned i = 0; i < upper; ++i) {
			ll |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < nbits) { // sign extend
			for (unsigned i = upper; i < nbits; ++i) {
				ll |= mask;
				mask <<= 1;
			}
		}
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

// paired down implementation of a decimal type to generate decimal representations for integer<nbits> types
namespace impl {
	// Decimal representation as a set of decimal digits with sign used for creating decimal representations of the integers
class decimal : public std::vector<uint8_t> {
public:
	bool sign;
	// remove any leading zeros from a decimal representation
	void unpad() {
		int n = (int)size();
		for (int i = n - 1; i > 0; --i) {
			if (operator[](i) == 0) pop_back();
		}
	}
private:
	friend std::ostream& operator<<(std::ostream& ostr, const decimal& d);
};

// generate an ASCII decimal format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const decimal& d) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	//std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	if (d.sign) ss << '-';
	for (decimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ostr << ss.str();
}
// forward reference
void sub(decimal& lhs, const decimal& rhs);

bool less(const decimal& lhs, const decimal& rhs) {
	// this logic assumes that there is no padding in the operands
	size_t l = lhs.size();
	size_t r = rhs.size();
	if (l < r) return true;
	if (l > r) return false;
	// numbers are the same size, need to compare magnitude
	decimal::const_reverse_iterator ritl = lhs.rbegin();
	decimal::const_reverse_iterator ritr = rhs.rbegin();
	for (; ritl != lhs.rend() || ritr != rhs.rend(); ++ritl, ++ritr) {
		if (*ritl < *ritr) return true;
		if (*ritl > *ritr) return false;
		// if the digits are equal we need to check the next set
	}
	// at this point we know the two operands are the same
	return false;
}
void add(decimal& lhs, const decimal& rhs) {
	decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
	if (lhs.sign != rhs.sign) {  // different signs
		_rhs.sign = !rhs.sign;
		return sub(lhs, _rhs);
	}
	else {
		// same sign implies this->negative is invariant
	}
	size_t l = lhs.size();
	size_t r = _rhs.size();
	// zero pad the shorter decimal
	if (l < r) {
		lhs.insert(lhs.end(), r - l, 0);
	}
	else {
		_rhs.insert(_rhs.end(), l - r, 0);
	}
	decimal::iterator lit = lhs.begin();
	decimal::iterator rit = _rhs.begin();
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
void sub(decimal& lhs, const decimal& rhs) {
	decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
	bool sign = lhs.sign;
	if (lhs.sign != rhs.sign) {
		_rhs.sign = !rhs.sign;
		return add(lhs, _rhs);
	}
	// largest value must be subtracted from
	size_t l = lhs.size();
	size_t r = _rhs.size();
	// zero pad the shorter decimal
	if (l < r) {
		lhs.insert(lhs.end(), r - l, 0);
		std::swap(lhs, _rhs);
		sign = !sign;
	}
	else if (r < l) {
		_rhs.insert(_rhs.end(), l - r, 0);
	}
	else {
		// the operands are the same size, thus we need to check their magnitude
		if (less(lhs, _rhs)) {
			std::swap(lhs, _rhs);
			sign = !sign;
		}
	}
	decimal::iterator lit = lhs.begin();
	decimal::iterator rit = _rhs.begin();
	char borrow = 0;
	for (; lit != lhs.end() || rit != _rhs.end(); ++lit, ++rit) {
		if (*rit > *lit - borrow) {
			*lit = 10 + *lit - borrow - *rit;
			borrow = 1;
		}
		else {
			*lit = *lit - borrow - *rit;
			borrow = 0;
		}
	}
	if (borrow) std::cout << "can this happen?" << std::endl;
	lhs.unpad();
	lhs.sign = sign;
}
void mul(decimal& lhs, const decimal& rhs) {
	bool signOfFinalResult = (lhs.sign != rhs.sign) ? true : false;
	decimal product;
	// find the smallest decimal to minimize the amount of work
	size_t l = lhs.size();
	size_t r = rhs.size();
	decimal::const_iterator sit, bit; // sit = smallest iterator, bit = biggest iterator
	if (l < r) {
		size_t position = 0;
		for (sit = lhs.begin(); sit != lhs.end(); ++sit) {
			decimal partial_sum;
			partial_sum.insert(partial_sum.end(), r + position, 0);
			decimal::iterator pit = partial_sum.begin() + position;
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
			decimal partial_sum;
			partial_sum.insert(partial_sum.end(), l + position, 0);
			decimal::iterator pit = partial_sum.begin() + position;
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
	product.unpad();
	product.sign = signOfFinalResult;
	lhs = product;
}

} // namespace impl




////////////////// INTEGER operators

template<size_t nbits>
inline integer<nbits> twos_complement(const integer<nbits>& value) {
	integer<nbits> complement = ~value;
	++complement;
	return complement;
}

// convert integer to decimal string
template<size_t nbits>
std::string convert_to_decimal_string(const integer<nbits>& value) {
	if (value.iszero()) {
		return std::string("0");
	}
	integer<nbits> number = value.sign() ? twos_complement(value) : value;
	impl::decimal partial, multiplier;
	partial.push_back(0); partial.sign = false;
	multiplier.push_back(1); multiplier.sign = false;
	// convert integer to decimal by multiplication by powers of 2
	for (unsigned i = 0; i < nbits; ++i) {
		if (number.at(i)) {
			impl::add(partial, multiplier);
			// std::cout << partial << std::endl;
		}
		impl::add(multiplier, multiplier);
	}
	std::stringstream ss;
	if (value.sign()) ss << '-';
	for (impl::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
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
	for (int i = nbits - 1; i >= 0; --i) {
		ss << (number.at(i) ? "1" : "0");
	}
	return ss.str();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// integer binary logic operators

// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
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
