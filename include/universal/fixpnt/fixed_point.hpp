#pragma once
// fixed_point.hpp: definition of an arbitrary binary fixed-point number
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

#include "./fixpnt_exceptions.hpp"

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
template<size_t nbits, size_t rbits> class fixpnt;
template<size_t nbits, size_t rbits> struct fixpntdiv_t;
template<size_t nbits, size_t rbits> fixpntdiv_t<nbits, rbits> fixpntdiv(const fixpnt<nbits, rbits>&, const fixpnt<nbits, rbits>&);

// fixpntdiv_t for fixpnt<nbits,rbits> to capture quotient and remainder during long division
template<size_t nbits, size_t rbits>
struct fixpntdiv_t {
	fixpnt<nbits,rbits> quot; // quotient
	fixpnt<nbits,rbits> rem;  // remainder
};

template<size_t nbits, size_t rbits>
bool parse(const std::string& number, fixpnt<nbits, rbits>& v);

// scale calculate the power of 2 exponent that would capture an approximation of a normalized real value
template<size_t nbits, size_t rbits>
inline long scale(const fixpnt<nbits, rbits>& i) {
	fixpnt<nbits,rbits> v(i);
	if (i.sign()) { // special case handling
		v = twos_complement(v);
		if (v == i) {  // special case of 10000..... largest negative number in 2's complement encoding
			return long(nbits - 1);
		}
	}
	// calculate scale
	long scale = 0;
	while (v > 1) {
		++scale;
		v >>= 1;
	}
	return scale;
}

// conversion helpers
template<size_t nbits, size_t rbits>
inline void convert(int64_t v, fixpnt<nbits, rbits>& result) {
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
template<size_t nbits, size_t rbits>
inline void convert_unsigned(uint64_t v, fixpnt<nbits, rbits>& result) {
	constexpr uint64_t mask = 0x1;
	result.clear();
	unsigned upper = (nbits <= 64 ? nbits : 64);
	for (unsigned i = 0; i < upper; ++i) {
		if (v & mask) result.set(i);
		v >>= 1;
	}
}

// fixpnt is a binary fixed point number of nbits with rbits after the radix point
template<size_t _nbits, size_t _rbits>
class fixpnt {
public:
	static constexpr size_t nbits = _nbits;
	static constexpr size_t rbits = _rbits;
	static constexpr unsigned nrBytes = (1 + ((nbits - 1) / 8));
	static constexpr unsigned MS_BYTE = nrBytes - 1;
	static constexpr uint8_t MS_BYTE_MASK = (0xFF >> (nrBytes * 8 - nbits));

	fixpnt() { setzero(); }

	fixpnt(const fixpnt&) = default;
	fixpnt(fixpnt&&) = default;

	fixpnt& operator=(const fixpnt&) = default;
	fixpnt& operator=(fixpnt&&) = default;

	/// Construct a new fixpnt from another, sign extend when necessary
	template<size_t src_nbits, size_t src_rbits>
	fixpnt(const fixpnt<src_nbits,src_rbits>& a) {
		static_assert(src_nbits > nbits, "Source fixpnt is bigger than target: potential loss of precision"); // TODO: do we want this?
		bitcopy(a);
		if (a.sign()) { // sign extend
			for (int i = int(src_nbits); i < int(nbits); ++i) {
				set(i);
			}
		}
	}

	// initializers for native types
	fixpnt(const signed char initial_value) { *this = initial_value; }
	fixpnt(const short initial_value) { *this = initial_value; }
	fixpnt(const int initial_value) { *this = initial_value; }
	fixpnt(const long initial_value) { *this = initial_value; }
	fixpnt(const long long initial_value) { *this = initial_value; }
	fixpnt(const char initial_value) { *this = initial_value; }
	fixpnt(const unsigned short initial_value) { *this = initial_value; }
	fixpnt(const unsigned int initial_value) { *this = initial_value; }
	fixpnt(const unsigned long initial_value) { *this = initial_value; }
	fixpnt(const unsigned long long initial_value) { *this = initial_value; }
	fixpnt(const float initial_value) { *this = initial_value; }
	fixpnt(const double initial_value) { *this = initial_value; }
	fixpnt(const long double initial_value) { *this = initial_value; }

	// access operator for bits
	// this needs a proxy to be able to create l-values
	// bool operator[](const unsigned int i) const //

	// simpler interface for now, using at(i) and set(i)/reset(i)

	// assignment operators for native types
	fixpnt& operator=(const signed char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			convert_unsigned(rhs, *this);
		}
		return *this;
	}
	fixpnt& operator=(const float rhs) {
		float_assign(rhs);
		return *this;
	}
	fixpnt& operator=(const double rhs) {
		float_assign(rhs);
		return *this;
	}
	fixpnt& operator=(const long double rhs) {
		float_assign(rhs);
		return *this;
	}

#ifdef POSIT_CONCEPT_GENERALIZATION
	// TODO: SFINAE to assure we only match a posit<nbits,es> concept
	template<typename PositType>
	fixpnt& operator=(const PositType& rhs) {
		// get the scale of the posit value
		int scale = sw::unum::scale(rhs);
		if (scale < 0) {
			*this = 0;
			return *this;
		}
		if (scale == 0) {
			*this = 1;
		}
		else {
			// gather all the fraction bits
			// sw::unum::bitblock<p.fhbits> significant = sw::unum::significant<p.nbits, p.es, p.fbits>(p);
			sw::unum::bitblock<rhs.fhbits> significant = sw::unum::significant<rhs.nbits, rhs.es, rhs.fbits>(rhs);
			// the radix point is at fbits, to make an fixpnt out of this
			// we shift that radix point fbits to the right.
			// that is equivalent to a scale of 2^fbits
			this->clear();
			int msb = (nbits < rhs.fbits + 1) ? nbits : rhs.fbits + 1;
			for (int i = msb - 1; i >= 0; --i) {
				this->set(i, significant[i]);
			}
			int shift = scale - rhs.fbits;  // if scale > fbits we need to shift left
			*this <<= shift;
			if (rhs.isneg()) {
				this->flip();
				*this += 1;
			}
		}
		return *this;
	}
#endif

	// prefix operators
	fixpnt operator-() const {
		fixpnt negated(*this);
		negated.flip();
		negated += 1;
		return negated;
	}
	// one's complement
	fixpnt operator~() const { 
		fixpnt complement(*this);
		complement.flip(); 
		return complement;
	}
	// increment
	fixpnt operator++(int) {
		fixpnt tmp(*this);
		operator++();
		return tmp;
	}
	fixpnt& operator++() {
		*this += fixpnt(1);
		b[MS_BYTE] = b[MS_BYTE] & MS_BYTE_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	// decrement
	fixpnt operator--(int) {
		fixpnt tmp(*this);
		operator--();
		return tmp;
	}
	fixpnt& operator--() {
		*this -= fixpnt(1);
		b[MS_BYTE] = b[MS_BYTE] & MS_BYTE_MASK; // assert precondition of properly nulled leading non-bits
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
	fixpnt& operator+=(const fixpnt& rhs) {
		fixpnt sum;
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
	fixpnt& operator-=(const fixpnt& rhs) {
		operator+=(twos_complement(rhs));
		return *this;
	}
	fixpnt& operator*=(const fixpnt& rhs) {
		fixpnt base(*this);
		fixpnt multiplicant(rhs);
		clear();
		for (unsigned i = 0; i < nbits; ++i) {
			if (base.at(i)) {
				operator+=(multiplicant);
			}
			multiplicant <<= 1;
		}
		// since we used operator++, which enforces the nulling of leading bits
		// we don't need to null here
		return *this;
	}
	fixpnt& operator/=(const fixpnt& rhs) {
		fixpntdiv_t<nbits,rbits> divresult = fixpntdiv<nbits,rbits>(*this, rhs);
		*this = divresult.quot;
		return *this;
	}
	fixpnt& operator%=(const fixpnt& rhs) {
		fixpntdiv_t<nbits, rbits> divresult = fixpntdiv<nbits, rbits>(*this, rhs);
		*this = divresult.rem;
		return *this;
	}
	fixpnt& operator<<=(const signed shift) {
		if (shift == 0) return *this;
		if (shift < 0) {
			operator>>=(-shift);
			return *this;
		}
		if (nbits <= unsigned(shift)) {
			clear();
			return *this;
		}
		fixpnt target;
		for (unsigned i = shift; i < nbits; ++i) {  // TODO: inefficient as it works at the bit level
			target.set(i, at(i - shift));
		}
		*this = target;
		return *this;
	}
	fixpnt& operator>>=(const signed shift) {
		if (shift == 0) return *this;
		if (shift < 0) {
			operator<<=(-shift);
			return *this;
		}
		if (nbits <= unsigned(shift)) {
			clear();
			return *this;
		}
		fixpnt target;
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
		throw "fixpnt bit index out of bounds";
	}
	inline void reset(unsigned int i) {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t mask = ~(1 << (i % 8));
			b[i / 8] = byte & mask;
			return;
		}
		throw "fixpnt bit index out of bounds";
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
		throw "fixpnt bit index out of bounds";
	}
	inline void setbyte(unsigned i, uint8_t value) {
		if (i < nrBytes) { b[i] = value; return; }
		throw fixpnt_byte_index_out_of_bounds{};
	}
	// use un-interpreted raw bits to set the bits of the fixpnt
	inline void set_raw_bits(unsigned long long value) {
		clear();
		for (unsigned i = 0; i < nrBytes; ++i) {
			b[i] = value & 0xFF;
			value >>= 8;
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		b[MS_BYTE] = MS_BYTE_MASK & b[MS_BYTE];
	}
	inline fixpnt& assign(const std::string& txt) {
		if (!parse(txt, *this)) {
			std::cerr << "Unable to parse: " << txt << std::endl;
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		b[MS_BYTE] = MS_BYTE_MASK & b[MS_BYTE];
		return *this;
	}
	// pure bit copy of source fixpnt, no sign extension
	template<size_t src_nbits, size_t src_rbits>
	inline void bitcopy(const fixpnt<src_nbits, src_rbits>& src) {
		int lastByte = (nrBytes < src.nrBytes ? nrBytes : src.nrBytes);
		clear();
		for (int i = 0; i < lastByte; ++i) {
			b[i] = src.byte(i);
		}
		b[MS_BYTE] = b[MS_BYTE] & MS_BYTE_MASK; // assert precondition of properly nulled leading non-bits
	}
	// in-place one's complement
	inline fixpnt& flip() {
		for (unsigned i = 0; i < nrBytes; ++i) {
			b[i] = ~b[i];
		}
		b[MS_BYTE] = b[MS_BYTE] & MS_BYTE_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
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
	inline uint8_t byte(unsigned int i) const {
		if (i < nrBytes) return b[i];
		throw fixpnt_byte_index_out_of_bounds{};
	}

protected:
	// HELPER methods

	// conversion functions
	short to_short() const {
		constexpr unsigned sizeofshort = 8 * sizeof(short);
		short s = 0;
		short mask = 1;
		unsigned upper = (nbits < sizeofshort ? nbits : sizeofshort);
		for (unsigned i = 0; i < upper; ++i) {
			s |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < sizeofshort) { // sign extend
			for (unsigned i = upper; i < sizeofshort; ++i) {
				s |= mask;
				mask <<= 1;
			}
		}
		return s;
	}
	int to_int() const {
		constexpr unsigned sizeofint = 8 * sizeof(int);
		int value = 0;
		int mask = 1;
		unsigned upper = (nbits < sizeofint ? nbits : sizeofint);
		for (unsigned i = 0; i < upper; ++i) {
			value |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < sizeofint) { // sign extend
			for (unsigned i = upper; i < sizeofint; ++i) {
				value |= mask;
				mask <<= 1;
			}
		}
		return value;
	}
	long to_long() const {
		constexpr unsigned sizeoflong = 8 * sizeof(long);
		long l = 0;
		long mask = 1;
		unsigned upper = (nbits < sizeoflong ? nbits : sizeoflong);
		for (unsigned i = 0; i < upper; ++i) {
			l |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < sizeoflong) { // sign extend
			for (unsigned i = upper; i < sizeoflong; ++i) {
				l |= mask;
				mask <<= 1;
			}
		}
		return l;
	}
	long long to_long_long() const {
		constexpr unsigned sizeoflonglong = 8 * sizeof(long long);
		long long ll = 0;
		long long mask = 1;
		unsigned upper = (nbits < sizeoflonglong ? nbits : sizeoflonglong);
		for (unsigned i = 0; i < upper; ++i) {
			ll |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < sizeoflonglong) { // sign extend
			for (unsigned i = upper; i < sizeoflonglong; ++i) {
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
		char* p = (char*)&ul;
		for (int i = 0; i < nrBytes; ++i) {
			*(p + i) = b[i];
		}
		return ul;
	}
	unsigned long long to_ulong_long() const {
		unsigned long long ull = 0;
		char* p = (char*)&ull;
		for (int i = 0; i < nrBytes; ++i) {
			*(p + i) = b[i];
		}
		return ull;
	}
	float to_float() const { 
		float f = float((long long)(*this)) / float(0x1ll << rbits);   // TODO: only works for nbits < 64
		return f; 
	}
	double to_double() const {
		double d = double((long long)(*this)) / double(0x1ll << rbits);   // TODO: only works for nbits < 64
		return d;
	}
	long double to_long_double() const {
		long double ld = (long double)((long long)(*this)) / (long double)(0x1ll << rbits);   // TODO: only works for nbits < 64
		return ld;
	}

	template<typename Ty>
	void float_assign(Ty& rhs) {
		clear();
		// the value of a binary fixed point number is an binary integer that is scaled by a fixed factor, 2^rbits
		// so the number 0100.0100 is the value 01000100 with an implicit scaling of 2^4 = 16
		// 01000100 = 64 + 4 = 68 -> scaled by 16 = 4 + 0.25 = 0100 + 0100

		// generate the representation of one and cast to Ty
		Ty one = Ty(0x1ll << rbits);
		Ty tmp = rhs * one;
		*this = uint64_t(tmp);
	}

private:
	//array<uint8_t, (1 + ((nbits - 1) / 8))> bytes;
	uint8_t b[nrBytes];

	// convert
	template<size_t nnbits, size_t rrbits>
	friend std::string convert_to_decimal_string(const fixpnt<nnbits, rrbits>& value);

	// fixpnt - fixpnt logic comparisons
	template<size_t nnbits, size_t rrbits>
	friend bool operator==(const fixpnt<nnbits, rrbits>& lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator!=(const fixpnt<nnbits, rrbits>& lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator< (const fixpnt<nnbits, rrbits>& lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator> (const fixpnt<nnbits, rrbits>& lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator<=(const fixpnt<nnbits, rrbits>& lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator>=(const fixpnt<nnbits, rrbits>& lhs, const fixpnt<nnbits, rrbits>& rhs);

	// fixpnt - literal logic comparisons
	template<size_t nnbits, size_t rrbits>
	friend bool operator==(const fixpnt<nnbits, rrbits>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator!=(const fixpnt<nnbits, rrbits>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator< (const fixpnt<nnbits, rrbits>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator> (const fixpnt<nnbits, rrbits>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator<=(const fixpnt<nnbits, rrbits>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator>=(const fixpnt<nnbits, rrbits>& lhs, const long long rhs);

	// literal - fixpnt logic comparisons
	template<size_t nnbits, size_t rrbits>
	friend bool operator==(const long long lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator!=(const long long lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator< (const long long lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator> (const long long lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator<=(const long long& lhs, const fixpnt<nnbits, rrbits>& rhs);
	template<size_t nnbits, size_t rrbits>
	friend bool operator>=(const long long lhs, const fixpnt<nnbits, rrbits>& rhs);

	// find the most significant bit set
	template<size_t nnbits, size_t rrbits>
	friend signed findMsb(const fixpnt<nnbits, rrbits>& v);
};

// paired down implementation of a decimal type to generate decimal representations for fixpnt<nbits,rbits> types
namespace impl {
	// Decimal representation as a set of decimal digits with sign used for creating decimal representations of the fixpnts
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
	// we need to transform the fixpnt into a string
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

////////////////////////    INTEGER functions   /////////////////////////////////

template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> twos_complement(const fixpnt<nbits, rbits>& value) {
	fixpnt<nbits, rbits> complement = ~value;
	++complement;
	return complement;
}

// convert fixpnt to decimal string
template<size_t nbits, size_t rbits>
std::string convert_to_decimal_string(const fixpnt<nbits, rbits>& value) {
	if (value.iszero()) {
		return std::string("0");
	}
	fixpnt<nbits, rbits> number = value.sign() ? twos_complement(value) : value;
	impl::decimal partial, multiplier;
	partial.push_back(0); partial.sign = false;
	multiplier.push_back(1); multiplier.sign = false;
	// convert fixpnt to decimal by adding and doubling multipliers
	for (unsigned i = rbits; i < nbits; ++i) {
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

// findMsb takes an fixpnt<nbits,rbits> reference and returns the position of the most significant bit, -1 if v == 0
template<size_t nbits, size_t rbits>
inline signed findMsb(const fixpnt<nbits, rbits>& v) {
	for (signed i = v.nrBytes - 1; i >= 0; --i) {
		if (v.b[i] != 0) {
			uint8_t mask = 0x80;
			for (signed j = 7; j >= 0; --j) {
				if (v.b[i] & mask) {
					return i * 8 + j;
				}
				mask >>= 1;
			}
		}
	}
	return -1; // no significant bit found, all bits are zero
}

////////////////////////    INTEGER operators   /////////////////////////////////

// divide fixpnt<nbits,rbits> a and b and return result argument
template<size_t nbits, size_t rbits>
void divide(const fixpnt<nbits, rbits>& a, const fixpnt<nbits, rbits>& b, fixpnt<nbits, rbits>& quotient) {
	if (b == fixpnt<nbits, rbits>(0)) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		throw fixpnt_divide_by_zero{};
#else
		std::cerr << "fixpnt_divide_by_zero\n";
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
	}
	fixpntdiv_t<nbits, rbits> divresult = fixpntdiv<nbits>(a, b);
	quotient = divresult.quot;
}

// calculate remainder of fixpnt<nbits,rbits> a and b and return result argument
template<size_t nbits, size_t rbits>
void remainder(const fixpnt<nbits, rbits>& a, const fixpnt<nbits, rbits>& b, fixpnt<nbits, rbits>& remainder) {
	if (b == fixpnt<nbits, rbits>(0)) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		throw fixpnt_divide_by_zero{};
#else
		std::cerr << "fixpnt_divide_by_zero\n";
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
	}
	fixpntdiv_t<nbits, rbits> divresult = fixpntdiv<nbits>(a, b);
	remainder = divresult.rem;
}

// divide fixpnt<nbits, rbits> a and b and return result argument
template<size_t nbits, size_t rbits>
fixpntdiv_t<nbits, rbits> fixpntdiv(const fixpnt<nbits, rbits>& _a, const fixpnt<nbits, rbits>& _b) {
	if (_b == fixpnt<nbits,rbits>(0)) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		throw fixpnt_divide_by_zero{};
#else
		std::cerr << "fixpnt_divide_by_zero\n";
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_negative = _a.sign();
	bool b_negative = _b.sign();
	bool result_negative = (a_negative ^ b_negative);
	fixpnt<nbits + 1, rbits> a; a.bitcopy(a_negative ? -_a : _a);
	fixpnt<nbits + 1, rbits> b; b.bitcopy(b_negative ? -_b : _b);
	fixpntdiv_t<nbits, rbits> divresult;
	if (a < b) {
		divresult.rem = _a; // a % b = a when a / b = 0
		return divresult; // a / b = 0 when b > a
	}
	// initialize the long division
	fixpnt<nbits + 1, rbits> accumulator = a;
	// prepare the subtractand
	fixpnt<nbits + 1, rbits> subtractand = b;
	int msb_b = findMsb(b);
	int msb_a = findMsb(a);
	int shift = msb_a - msb_b;
	subtractand <<= shift;
	// long division
	for (int i = shift; i >= 0; --i) {
		if (subtractand <= accumulator) {
			accumulator -= subtractand;
			divresult.quot.set(i);
		}
		else {
			divresult.quot.reset(i);
		}
		subtractand >>= 1;
	}
	if (result_negative) {  // take 2's complement
		divresult.quot.flip();
		divresult.quot += 1;
	} 
	if (_a < fixpnt<nbits, rbits>(0)) {
		divresult.rem = -accumulator;
	}
	else {
		divresult.rem = accumulator;
	}

	return divresult;
}

/// stream operators

// read a fixed-point ASCII format and make a binary fixpnt out of it
template<size_t nbits, size_t rbits>
bool parse(const std::string& number, fixpnt<nbits, rbits>& value) {
	bool bSuccess = false;
	value.clear();
	// check if the txt is an fixpnt form: [0123456789]+
	std::regex decimal_regex("[0-9]+");
	std::regex octal_regex("^0[1-7][0-7]*$");
	std::regex hex_regex("0[xX][0-9a-fA-F']+");
	// setup associative array to map chars to nibbles
	std::map<char, int> charLookup{
		{ '0', 0 },
		{ '1', 1 },
		{ '2', 2 },
		{ '3', 3 },
		{ '4', 4 },
		{ '5', 5 },
		{ '6', 6 },
		{ '7', 7 },
		{ '8', 8 },
		{ '9', 9 },
		{ 'a', 10 },
		{ 'b', 11 },
		{ 'c', 12 },
		{ 'd', 13 },
		{ 'e', 14 },
		{ 'f', 15 },
		{ 'A', 10 },
		{ 'B', 11 },
		{ 'C', 12 },
		{ 'D', 13 },
		{ 'E', 14 },
		{ 'F', 15 },
	};
	if (std::regex_match(number, octal_regex)) {
		std::cout << "found an octal representation\n";
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			std::cout << "char = " << *r << std::endl;
		}
		bSuccess = false; // TODO
	}
	else if (std::regex_match(number, hex_regex)) {
		//std::cout << "found a hexadecimal representation\n";
		// each char is a nibble
		int byte;
		int byteIndex = 0;
		bool odd = false;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			if (*r == '\'') {
				// ignore
			}
			else if (*r == 'x' || *r == 'X') {
				// we have reached the end of our parse
				if (odd) {
					// complete the most significant byte
					value.setbyte(byteIndex, byte);
				}
				bSuccess = true;
			}
			else {
				if (odd) {
					byte += charLookup.at(*r) << 4;
					value.setbyte(byteIndex, byte);
					++byteIndex;
				}
				else {
					byte = charLookup.at(*r);
				}
				odd = !odd;
			}
		}
	}
	else if (std::regex_match(number, decimal_regex)) {
		//std::cout << "found a decimal fixpnt representation\n";
		fixpnt<nbits, rbits> scale = 1;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			fixpnt<nbits, rbits> digit = charLookup.at(*r);
			value += scale * digit;
			scale *= 10;
		}
		bSuccess = true;
	}

	return bSuccess;
}

// generate an fixpnt format ASCII format
template<size_t nbits, size_t rbits>
inline std::ostream& operator<<(std::ostream& ostr, const fixpnt<nbits, rbits>& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the fixpnt into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << convert_to_decimal_string(i);

	return ostr << ss.str();
}

// read an ASCII fixpnt format
template<size_t nbits, size_t rbits>
inline std::istream& operator>>(std::istream& istr, fixpnt<nbits, rbits>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators
template<size_t nbits, size_t rbits>
inline std::string to_binary(const fixpnt<nbits, rbits>& number) {
	std::stringstream ss;
	for (int i = nbits - 1; i >= 0; --i) {
		ss << (number.at(i) ? "1" : "0");
	}
	return ss.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary logic operators

// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits>
inline bool operator==(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	for (unsigned i = 0; i < lhs.nrBytes; ++i) {
		if (lhs.b[i] != rhs.b[i]) return false;
	}
	return true;
}
template<size_t nbits, size_t rbits>
inline bool operator!=(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator< (const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	bool lhs_is_negative = lhs.sign();
	bool rhs_is_negative = rhs.sign();
	if (lhs_is_negative && !rhs_is_negative) return true;
	if (rhs_is_negative && !lhs_is_negative) return false;
	// arguments have the same sign
	for (int i = nbits - 1; i >= 0; --i) {
		bool a = lhs.at(i);
		bool b = rhs.at(i);
		if (a ^ b) {
			if (a == false) {
				return true; 
			}
			else {
				return false;
			}
		}
	}
	return false; // lhs and rhs are the same
}
template<size_t nbits, size_t rbits>
inline bool operator> (const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator< (rhs, lhs);
}
template<size_t nbits, size_t rbits>
inline bool operator<=(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator>=(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits>
inline bool operator==(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits>(rhs));
}
template<size_t nbits, size_t rbits>
inline bool operator!=(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator< (const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits>(rhs));
}
template<size_t nbits, size_t rbits>
inline bool operator> (const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator< (fixpnt<nbits, rbits>(rhs), lhs);
}
template<size_t nbits, size_t rbits>
inline bool operator<=(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator>=(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<size_t nbits, size_t rbits>
inline bool operator==(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator==(fixpnt<nbits, rbits>(lhs), rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator!=(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator< (const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator<(fixpnt<nbits, rbits>(lhs), rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator> (const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator< (rhs, lhs);
}
template<size_t nbits, size_t rbits>
inline bool operator<=(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits>
inline bool operator>=(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator+(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	fixpnt<nbits, rbits> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator-(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	fixpnt<nbits, rbits> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator*(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	fixpnt<nbits, rbits> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator/(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	fixpnt<nbits, rbits> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator%(const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs) {
	fixpnt<nbits, rbits> ratio = lhs;
	ratio %= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator+(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator-(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator*(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator/(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator%(const fixpnt<nbits, rbits>& lhs, const long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator+(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator+(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator-(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator-(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator*(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator*(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY DIVISION
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator/(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator/(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits>
inline fixpnt<nbits, rbits> operator%(const long long lhs, const fixpnt<nbits, rbits>& rhs) {
	return operator%(fixpnt<nbits, rbits>(lhs), rhs);
}


} // namespace unum
} // namespace sw
