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
#include <cassert>

/*
The fixed-point arithmetic can be configured to:
- throw exception on overflow
- saturation arithmetic: saturate on overflow
- modular arithmetic: quietly overflow into modular values

The quietly overflow configuration is reasonable when you are using
a fixed-point size that captures the dynamic of your computation.
Due to the fact that no special cases are required, the arithmetic
operators will be much faster than saturation.

Compile-time configuration flags are used to select the exception mode.
Run-time configuration is used to select modular vs saturation arithmetic.
*/
#include "./fixpnt_exceptions.hpp"  // you need the exception types defined, but you may not throw them
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION

#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
#include "universal/native/ieee-754.hpp"   // IEEE-754 decoders
#include "universal/native/integers.hpp"   // manipulators for native integer types
#include "universal/native/byteArray.hpp"  // manipulators for byte arrays

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

constexpr bool Modular    = true;
constexpr bool Saturation = !Modular;

// forward references
template<size_t nbits, size_t rbits, bool arithmetic> class fixpnt;
template<size_t nbits, size_t rbits, bool arithmetic> struct fixpntdiv_t;
template<size_t nbits, size_t rbits, bool arithmetic> fixpntdiv_t<nbits, rbits, arithmetic> fixpntdiv(const fixpnt<nbits, rbits, arithmetic>&, const fixpnt<nbits, rbits, arithmetic>&);

// fixpntdiv_t for fixpnt<nbits,rbits> to capture quotient and remainder during long division
template<size_t nbits, size_t rbits, bool arithmetic>
struct fixpntdiv_t {
	fixpnt<nbits, rbits, arithmetic> quot; // quotient
	fixpnt<nbits, rbits, arithmetic> rem;  // remainder
};

template<size_t nbits, size_t rbits, bool arithmetic = Modular>
bool parse(const std::string& number, fixpnt<nbits, rbits, arithmetic>& v);

// The free function scale calculates the power of 2 exponent that would capture an approximation of a normalized real value
template<size_t nbits, size_t rbits, bool arithmetic = Modular>
inline int scale(const fixpnt<nbits, rbits, arithmetic>& i) {
	fixpnt<nbits,rbits,arithmetic> v(i);
	if (i.sign()) { // special case handling
		v = twos_complement(v);
		if (v == i) {  // special case of 10000..... largest negative number in 2's complement encoding
			return long(nbits - rbits);
		}
	}
	// calculate scale
	long scale = 0;
	if (nbits > rbits + 1) {  // subtle bug: in fixpnt numbers with only 1 bit before the radix point, '1' is maxneg, and thus while (v > 1) never completes
		v >>= rbits;
		while (v > 1) {
			++scale;
			v >>= 1;
		}
	}
	return scale;
}

// the value of a binary fixed point number is an binary integer that is scaled by a fixed factor, 2^rbits
// so the number 0100.0100 is the value 01000100 with an implicit scaling of 2^4 = 16
// 01000100 = 64 + 4 = 68 -> scaled by 16 = 4.25 -> 4 + 0.25 = 0100 + 0100

	// 01111....11111 is max pos
	// 00000....00001 is min pos
	// 00000....00000 is zero
	// 11111....11111 is min neg
	// 10000....00000 is max min

// maximum value of the fixed point configuration
// what is maxpos when all bits are fraction bits?
//   still #.01111...11111 as the rbits simply define the range this value is scaled by
// when rbits > nbits: is that a valid format? yes, the rbits simply define the range
template<size_t nbits, size_t rbits, bool arithmetic = Modular>
fixpnt<nbits, rbits, arithmetic> maxpos_fixpnt() {
	// maxpos = 01111....1111
	fixpnt<nbits, rbits, arithmetic> maxpos;
	maxpos.flip();
	maxpos.set(nbits - 1, false);
	return maxpos;
}

// maximum negative value of the fixed point configuration
template<size_t nbits, size_t rbits, bool arithmetic = Modular>
fixpnt<nbits, rbits, arithmetic> maxneg_fixpnt() {
	// maxneg = 10000....000
	fixpnt<nbits, rbits, arithmetic> maxneg;
	maxneg.set(nbits - 1, true);
	return maxneg;
}

// minimum positive value of the fixed point configuration
template<size_t nbits, size_t rbits, bool arithmetic = Modular>
fixpnt<nbits, rbits, arithmetic> minpos_fixpnt() {
	// minpos = 0000....00001
	fixpnt<nbits, rbits, arithmetic> minpos;
	minpos.set(0, true);
	return minpos;
}

// minimum positive value of the fixed point configuration
template<size_t nbits, size_t rbits, bool arithmetic = Modular>
fixpnt<nbits, rbits, arithmetic> minneg_fixpnt() {
	// minpos = 11111....11111
	fixpnt<nbits, rbits, arithmetic> minneg;
	minneg.flip();
	return minneg;
}

// conversion helpers
template<size_t nbits, size_t rbits, bool arithmetic = Modular>
inline void convert(int64_t v, fixpnt<nbits, rbits, arithmetic>& result) {
	if (0 == v) {
		result.setzero();
		return;
	}
	constexpr uint64_t mask = 0x1;
	bool negative = (v < 0 ? true : false);
	result.clear();
	if (arithmetic == Saturation) {
		// we are implementing saturation for values that are outside of the fixed-point's range
		// check if we are in the representable range
		if (v >= (long double)maxpos_fixpnt<nbits, rbits, arithmetic>()) {
			// set to max value
			result.flip();
			result.set(nbits - 1, false);
			return;
		}
		if (v <= (long double)maxneg_fixpnt<nbits, rbits, arithmetic>()) {
			// set to max neg value
			result.set(nbits - 1, true);
			return;
		}
	}

	// we only have an integer part, and no fraction to convert
	unsigned upper = (nbits < 64 ? nbits : 64);
	for (unsigned i = 0; i < upper - rbits && v != 0; ++i) {
		if (v & mask) result.set(i + rbits);
		v >>= 1;
	}
	if (nbits > 64 && negative) {
		// sign extend
		for (unsigned i = upper; i < nbits; ++i) {
			result.set(i);
		}
	}
}
template<size_t nbits, size_t rbits, bool arithmetic = Modular>
inline void convert_unsigned(uint64_t v, fixpnt<nbits, rbits, arithmetic>& result) {
	if (0 == v) {
		result.setzero();
		return;
	}
	constexpr uint64_t mask = 0x1;
	result.clear();
	if (arithmetic == Saturation) {
		// we are implementing saturation for values that are outside of the fixed-point's range
		// check if we are in the representable range
		if (v >= (long double)maxpos_fixpnt<nbits, rbits, arithmetic>()) {
			// set to max value
			result.flip();
			result.set(nbits - 1, false);
			return;
		}
		if (v <= (long double)maxneg_fixpnt<nbits, rbits, arithmetic>()) {
			// set to max neg value
			result.set(nbits - 1, true);
			return;
		}
	}
	unsigned upper = (nbits <= 64 ? nbits : 64);
	for (unsigned i = 0; i < upper; ++i) {
		if (v & mask) result.set(i);
		v >>= 1;
	}
}

// fixpnt is a binary fixed point number of nbits with rbits after the radix point
template<size_t _nbits, size_t _rbits, bool arithmetic = Modular>
class fixpnt {
public:
	static_assert(_nbits >= _rbits, "fixpnt configuration error: nbits must be greater or equal to rbits");
	static constexpr size_t nbits = _nbits;
	static constexpr size_t rbits = _rbits;
	static constexpr unsigned nrBytes = (1 + ((nbits - 1) / 8));
	static constexpr unsigned mulBytes = (1 + ((2 * nbits - 1) / 8)); // for the odd case of a half filled byte
	static constexpr unsigned MS_BYTE = nrBytes - 1;
	static constexpr uint8_t MS_BYTE_MASK = (0xFF >> (nrBytes * 8 - nbits));

	fixpnt() { setzero(); }

	fixpnt(const fixpnt&) = default;
	fixpnt(fixpnt&&) = default;

	fixpnt& operator=(const fixpnt&) = default;
	fixpnt& operator=(fixpnt&&) = default;

	/// Construct a new fixpnt from another, sign extend when necessary
	template<size_t src_nbits, size_t src_rbits, bool src_arithmetic>
	fixpnt(const fixpnt<src_nbits, src_rbits, src_arithmetic>& a) {
		*this = a;
	}
	template<size_t src_nbits, size_t src_rbits, bool src_arithmetic>
	fixpnt& operator=(const fixpnt<src_nbits, src_rbits, src_arithmetic>& a) {
		std::cout << typeid(a).name() << " goes into " << typeid(*this).name() << std::endl;
		static_assert(src_nbits > nbits, "Source fixpnt is bigger than target: potential loss of precision"); // TODO: do we want this?
		bitcopy(a);
		if (a.sign()) { // sign extend
			for (int i = int(src_nbits); i < int(nbits); ++i) {
				set(i);
			}
		}
	}

	// initializers for native types
	fixpnt(const signed char initial_value)        { *this = initial_value; }
	fixpnt(const short initial_value)              { *this = initial_value; }
	fixpnt(const int initial_value)                { *this = initial_value; }
	fixpnt(const long initial_value)               { *this = initial_value; }
	fixpnt(const long long initial_value)          { *this = initial_value; }
	fixpnt(const char initial_value)               { *this = initial_value; }
	fixpnt(const unsigned short initial_value)     { *this = initial_value; }
	fixpnt(const unsigned int initial_value)       { *this = initial_value; }
	fixpnt(const unsigned long initial_value)      { *this = initial_value; }
	fixpnt(const unsigned long long initial_value) { *this = initial_value; }
	fixpnt(const float initial_value)              { *this = initial_value; }
	fixpnt(const double initial_value)             { *this = initial_value; }
	fixpnt(const long double initial_value)        { *this = initial_value; }

	// access operator for bits
	// this needs a proxy to be able to create l-values
	// bool operator[](const unsigned int i) const //

	// simpler interface for now, using at(i) and set(i)/reset(i)

	// assignment operators for native types
	fixpnt& operator=(const signed char rhs) {
		convert(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const short rhs) {
		convert(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const int rhs) {
		convert(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const long rhs) {
		convert(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const long long rhs) {
		convert(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const char rhs) {
		convert_unsigned(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const unsigned short rhs) {
		convert_unsigned(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const unsigned int rhs) {
		convert_unsigned(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const unsigned long rhs) {
		convert_unsigned(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const unsigned long long rhs) {
		convert_unsigned(rhs, *this);
		return *this;
	}
	fixpnt& operator=(const float rhs) {
		if (rhs == 0.0f) {
			setzero();
			return *this;
		}
		float_decoder decoder;
		decoder.f = rhs;
		uint32_t raw = (1 << 23) | decoder.parts.fraction; // TODO: this only works for normalized numbers 1.###, need a test for denorm
		int radixPoint = 23 - (decoder.parts.exponent - 127); // move radix point to the right if scale > 0, left if scale < 0
		// our fixed-point has its radixPoint at rbits
		int shiftRight = radixPoint - int(rbits);
		// do we need to round?
		if (shiftRight > 0) {
			// yes, round the raw bits
			// collect guard, round, and sticky bits
			// this same logic will work for the case where 
			// we only have a guard bit and no round and/or sticky bits
			// because the mask logic will make round and sticky both 0
			// so no need to special case it
			uint32_t mask = (1 << (shiftRight - 1));
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			mask = (0xFFFFFFFF << (shiftRight - 2));
			mask = ~mask;
			bool sticky = (mask & raw);
			
			raw >>= shiftRight;  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1);
			//  ... lsb | guard  round sticky   round
			//       x     0       x     x       down
			//       0     1       0     0       down  round to even
			//       1     1       0     0        up   round to even
			//       x     1       0     1        up
			//       x     1       1     0        up
			//       x     1       1     1        up
			if (guard) {
				if (lsb && (!round && !sticky)) ++raw; // round to even
				if (round || sticky) ++raw;
			}
		}
		raw = (decoder.parts.sign == 0) ? raw : (~raw + 1); // map to two's complement
		set_raw_bits(raw);
		return *this;
	}
	fixpnt& operator=(const double rhs) {
		if (rhs == 0.0) {
			setzero();
			return *this;
		}
		double_decoder decoder;
		decoder.d = rhs;
		uint64_t raw = (uint64_t(1) << 52) | decoder.parts.fraction;
		int radixPoint = 52 - (int(decoder.parts.exponent) - 1023);  // move radix point to the right if scale > 0, left if scale < 0
		// our fixed-point has its radixPoint at rbits
		int shiftRight = radixPoint - int(rbits);
		// do we need to round?
		if (shiftRight > 0) {
			// yes, round the raw bits
			// collect guard, round, and sticky bits
			// this same logic will work for the case where 
			// we only have a guard bit  and no round and sticky bits
			// because the mask logic will make round and sticky both 0
			// so no need to special case it
			uint64_t mask = (uint64_t(1) << (shiftRight - 1));
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			mask = (0xFFFFFFFFFFFFFFFF << (shiftRight - 2));
			mask = ~mask;
			bool sticky = (mask & raw);

			raw >>= shiftRight;  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1);
			//  ... lsb | guard  round sticky   round
			//       x     0       x     x       down
			//       0     1       0     0       down  round to even
			//       1     1       0     0        up   round to even
			//       x     1       0     1        up
			//       x     1       1     0        up
			//       x     1       1     1        up
			if (guard) {
				if (lsb && (!round && !sticky)) ++raw; // round to even
				if (round || sticky) ++raw;
			}
		}
		raw = (decoder.parts.sign == 0) ? raw : (~raw + 1); // map to two's complement
		set_raw_bits(raw);
		return *this;
	}
	fixpnt& operator=(const long double rhs) {
		if (rhs == 0.0l) {
			setzero();
			return *this;
		}
		//long_double_decoder decoder;
		//decoder.ld = rhs;
		std::cerr << "assignment from long double not implemented yet\n";
		float_assign(rhs);
		return *this;
	}

	// conversion operator between different fixed point formats with the same rbits
	template<size_t src_bits>
	fixpnt& operator=(const fixpnt<src_bits, rbits, arithmetic>& src) {
		if (src_bits <= nbits) {
			// simple copy of the bytes
			for (unsigned i = 0; i < unsigned(src.nrBytes); ++i) {
				b[i] = src.byte(i);
			}
			if (src < 0) {
				// we need to sign extent
				for (unsigned i = nbits; i < unsigned(src_bits); ++i) {
					this->set(i, true);
				}
			}
		}
		else {
			throw "to be implemented";
		}
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
	explicit operator unsigned short() const     { return to_ushort(); }
	explicit operator unsigned int() const       { return to_uint(); }
	explicit operator unsigned long() const      { return to_ulong(); }
	explicit operator unsigned long long() const { return to_ulong_long(); }
	explicit operator short() const              { return convert_signed<short>(); }
	explicit operator int() const                { return convert_signed<int>(); }
	explicit operator long() const               { return convert_signed<long>(); }
	explicit operator long long() const          { return convert_signed<long long>(); }
	explicit operator float() const              { return to_float(); }
	explicit operator double() const             { return to_double(); }
	explicit operator long double() const        { return to_long_double(); }

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
		// TODO: how are we going to deal with overflow?
		uint8_t accumulator[mulBytes], multiplicant[mulBytes];
		for (unsigned i = 0; i < nrBytes; ++i) {
			accumulator[i]            = uint8_t(0);
			accumulator[i + nrBytes]  = uint8_t(0);
			multiplicant[i]           = rhs.b[i];
			multiplicant[i + nrBytes] = (rhs.sign() ? uint8_t(0xFF) : uint8_t(0x00)); // sign extend if needed
		}
		for (unsigned i = 0; i < nbits; ++i) {
			uint8_t byte = b[i / 8];
			uint8_t mask = 1 << (i % 8);
			if (byte & mask) { // check the multiplication bit
				addBytes(accumulator, multiplicant, mulBytes);
				// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
				accumulator[mulBytes - 1] = MS_BYTE_MASK & accumulator[mulBytes - 1];
			}
			shiftLeft(multiplicant, mulBytes);
		}
//		displayByteArray("accu", accumulator, mulBytes);

		// if rbit >= 1 we need to round
		// accumulator is a 2*nbits, 2*rbits representation

		int roundingDecision = 0;
		if (rbits > 0) {		// capture rounding bits
			roundingDecision = round(accumulator, mulBytes, int(rbits)-1);
			//std::cout << (roundingDecision == 0 ? "tie" : (roundingDecision > 0 ? "up" : "down")) << std::endl;
		}
		// shift the radix point back
		shiftRight(accumulator, mulBytes, rbits);
//		displayByteArray("accu", accumulator, mulBytes);
		if (roundingDecision > 0) {
			uint8_t plusOne[mulBytes];
			plusOne[0] = uint8_t(0x01);
			for (unsigned i = 1; i < mulBytes; ++i) {
				plusOne[i] = uint8_t(0);
			}
			addBytes(accumulator, plusOne, mulBytes);
		}
//		displayByteArray("accu", accumulator, mulBytes);
		clear();
		// copy the value in
		for (unsigned i = 0; i < nrBytes; ++i) {
			b[i] = accumulator[i];
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		b[MS_BYTE] = MS_BYTE_MASK & b[MS_BYTE];
		return *this;
	}
	fixpnt& operator/=(const fixpnt& rhs) {
		fixpntdiv_t<nbits,rbits,arithmetic> divresult = fixpntdiv<nbits,rbits,arithmetic>(*this, rhs);
		*this = divresult.quot;
		return *this;
	}
	fixpnt& operator%=(const fixpnt& rhs) {
		fixpntdiv_t<nbits,rbits,arithmetic> divresult = fixpntdiv<nbits,rbits,arithmetic>(*this, rhs);
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
	inline void set(size_t i) {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t mask = 1 << (i % 8);
			b[i / 8] = byte | mask;
			return;
		}
		throw "fixpnt bit index out of bounds";
	}
	inline void reset(size_t i) {
		if (i < nbits) {
			uint8_t byte = b[i / 8];
			uint8_t mask = ~(1 << (i % 8));
			b[i / 8] = byte & mask;
			return;
		}
		throw "fixpnt bit index out of bounds";
	}
	inline void set(size_t i, bool v) {
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
	inline void setbyte(size_t i, uint8_t value) {
		if (i < nrBytes) { b[i] = value; return; }
		throw fixpnt_byte_index_out_of_bounds{};
	}
	// use un-interpreted raw bits to set the bits of the fixpnt
	inline void set_raw_bits(size_t value) {
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
	inline void bitcopy(const fixpnt<src_nbits, src_rbits> & src) {
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
	inline bool at(size_t i) const {
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
	// from fixed-point to native
	template<typename Integer>
	typename std::enable_if< std::is_integral<Integer>::value && std::is_signed<Integer>::value,
	                Integer>::type convert_signed() const {
		if (nbits <= rbits) return 0;
		constexpr unsigned sizeOfInteger = 8 * sizeof(Integer);
		Integer ll = 0;
		Integer mask = 1;
		unsigned upper = (nbits < sizeOfInteger ? nbits : sizeOfInteger);
		for (unsigned i = rbits; i < upper; ++i) {
			ll |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < (sizeOfInteger+rbits)) { // sign extend
			for (unsigned i = upper; i < (sizeOfInteger+rbits); ++i) {
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
		// minimum positive normal value of a single precision float == 2^-126
		// float minpos_normal = 1.1754943508222875079687365372222e-38
		// minimum positive(subnormal) value is 2^-149
		// float minpos_subnormal = 1.4012984643248170709237295832899e-45
		static_assert(rbits <= 149, "to_float: fixpnt fraction is too small to represent with native float");
		float multiplier = 0;
		if (rbits > 126) { // value is a subnormal number
			multiplier = 1.4012984643248170709237295832899e-45;
			for (size_t i = 0; i < 149 - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		else {
			// the value is a normal number
			multiplier = 1.1754943508222875079687365372222e-38;
			for (size_t i = 0; i < 126 - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		// you pop out here with the starting bit value
		fixpnt<nbits, rbits> raw = (sign() ? twos_complement(*this) : *this);
		// construct the value
		float value = 0;
		for (size_t i = 0; i < nbits; ++i) {
			if (raw.at(i)) value += multiplier;
			multiplier *= 2.0; // these are error free multiplies
		}
		return (sign() ? -value : value);
	}
	double to_double() const {
		// minimum positive normal value of a double precision float == 2^-1022
		// double dbl_minpos_normal = 2.2250738585072013830902327173324e-308;
		// minimum positive subnormal value of a double precision float == 2 ^ -1074
		// double dbl_minpos_subnormal = 4.940656458412465441765687928622e-324;
		static_assert(rbits <= 1074, "to_double: fixpnt fraction is too small to represent with native double");
		double multiplier = 0;
		if (rbits > 1022) { // value is a subnormal number
			multiplier = 4.940656458412465441765687928622e-324;
			for (size_t i = 0; i < 1074 - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		else {
			// the value is a normal number
			multiplier = 2.2250738585072013830902327173324e-308;
			for (size_t i = 0; i < 1022 - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		// you pop out here with the starting bit value
		fixpnt<nbits, rbits> raw = (sign() ? twos_complement(*this) : *this);
		// construct the value
		double value = 0;
		for (size_t i = 0; i < nbits; ++i) {
			if (raw.at(i)) value += multiplier;
			multiplier *= 2.0; // these are error free multiplies
		}
		return (sign() ? -value : value);

	}
	long double to_long_double() const {  // TODO : this is not a valid implementation
		int64_t value = 0;
		uint64_t mask = 1;
		for (size_t i = 0; i < nbits; ++i) {
			value |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign()) { // sign extend
			for (size_t i = nbits; i < 64; ++i) {
				value |= mask;
				mask <<= 1;
			}
		}
		long double numerator = (long double)value;
		long double denominator = (long double)(uint64_t(0x1) << rbits);
		return numerator / denominator;
	}

	// from native to fixed-point
	template<typename Ty>
	void float_assign(Ty& rhs) {
		clear();
		if (arithmetic == Saturation) {
			// we are implementing saturation for values that are outside of the fixed-point's range
			// check if we are in the representable range
			if (rhs >= (Ty)maxpos_fixpnt<nbits, rbits, arithmetic>()) {
				// set to max value
				flip();
				set(nbits - 1, false);
				return;
			}
			if (rhs <= (Ty)maxneg_fixpnt<nbits, rbits, arithmetic>()) {
				// set to max neg value
				set(nbits - 1, true);
				return;
			}
		}
		// for proper rounding, we need to get the full bit representation

		// generate the representation of one and cast to Ty
		Ty one = Ty(0x1ll << rbits);
		Ty tmp = rhs * one;
		*this = uint64_t(tmp);
	}

private:
	//array<uint8_t, (1 + ((nbits - 1) / 8))> bytes;
	uint8_t b[nrBytes];

	// convert
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend std::string convert_to_decimal_string(const fixpnt<nnbits, rrbits, aarithmetic>& value);

	// fixpnt - fixpnt logic comparisons
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator==(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator!=(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator< (const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator> (const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator<=(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator>=(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);

	// fixpnt - literal logic comparisons
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator==(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator!=(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator< (const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator> (const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator<=(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const long long rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator>=(const fixpnt<nnbits, rrbits, aarithmetic>& lhs, const long long rhs);

	// literal - fixpnt logic comparisons
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator==(const long long lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator!=(const long long lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator< (const long long lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator> (const long long lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator<=(const long long& lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend bool operator>=(const long long lhs, const fixpnt<nnbits, rrbits, aarithmetic>& rhs);

	// find the most significant bit set
	template<size_t nnbits, size_t rrbits, bool aarithmetic>
	friend signed findMsb(const fixpnt<nnbits, rrbits, aarithmetic>& v);
};

// paired down implementation of a decimal type to generate decimal representations for fixpnt<nbits,rbits> types
namespace impl {

// Decimal representation as a set of decimal digits with sign used for creating decimal representations of the fixpnts
class decimal : public std::vector<uint8_t> {
public:
	decimal() { setzero(); }
	decimal(const decimal&) = default;
	decimal(decimal&&) = default;
	decimal& operator=(const decimal&) = default;
	decimal& operator=(decimal&&) = default;

	inline bool sign() const { return _sign; }
	inline bool iszero() const { return (size() == 1 && at(0) == 0) ? true : false; }
	inline bool ispos() const {
		return (!iszero() && _sign == false) ? true : false;
	}
	inline bool isneg() const { 
		return (!iszero() && _sign == true) ? true : false;
	}
	inline void setzero() {
		clear();
		push_back(0);
		_sign = false;
	}
	inline void setpos() { _sign = false; }
	inline void setneg() {	_sign = true; }
	inline void setsign(bool sign) { _sign = sign; }
	inline void setdigit(const uint8_t d, bool negative = false) {
		clear();
		push_back(d);
		_sign = negative;
	}
	
	// remove any leading zeros from a decimal representation
	void unpad() {
		int n = (int)size();
		for (int i = n - 1; i > 0; --i) {
			if (operator[](i) == 0) {
				pop_back();
			}
			else {
				return; // found the most significant digit
			}
		}
	}
	// shift left operator for decimal
	void shiftLeft(size_t orders) {
		for (size_t i = 0; i < orders; ++i) {
			this->insert(this->begin(), 0);
		}
	}
	// shift right operator for decimal
	void shiftRight(size_t orders) {
		if (size() <= orders) {
			this->setzero();
		}
		else {
			for (size_t i = 0; i < orders; ++i) {
				this->erase(this->begin());
			}
		}
	}
private:
	bool _sign;
	friend std::ostream& operator<<(std::ostream& ostr, const decimal& d);
};

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
bool lessOrEqual(const decimal& lhs, const decimal& rhs) {
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
	return true;
}
// in-place addition (equivalent to +=)
void add(decimal& lhs, const decimal& rhs) {
	decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
	if (lhs.sign() != rhs.sign()) {  // different signs
		_rhs.setsign(!rhs.sign());
		sub(lhs, _rhs);
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
void convert_to_decimal(long long v, decimal& d) {
	using namespace std;
	d.setzero();
	bool sign = false;
	if (v == 0) return;
	if (v < 0) {
		sign = true; // negative number
		// transform to sign-magnitude on positive side
		v *= -1;
	}
	uint64_t mask = 0x1;
	// IMPORTANT: can't use initializer or assignment operator as it would yield 
	// an infinite loop calling convert_to_decimal. So we need to construct the
	// decimal from first principals
	decimal base; // can't use base(1) semantics here as it would cause an infinite loop
	base.setdigit(1);
	while (v) { // minimum loop iterations; exits when no bits left
		if (v & mask) {
			add(d, base);
		}
		add(base, base);
		v >>= 1;
	}
	// finally set the sign
	d.setsign(sign);
}
// in-place subtraction (equivalent to -=)
void sub(decimal& lhs, const decimal& rhs) {
	decimal _rhs(rhs);   // is this copy necessary? I introduced it to have a place to pad
	bool sign = lhs.sign();
	if (lhs.sign() != rhs.sign()) {
		_rhs.setsign(!rhs.sign());
		add(lhs, _rhs);
		return;
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
		lhs.setpos();
		_rhs.setpos();
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
	if (lhs.iszero()) { // special case of zero having positive sign
		lhs.setpos();
	}
	else {
		lhs.setsign(sign);
	}
}

// in-place multiplication (equivalent to *=)
void mul(decimal& lhs, const decimal& rhs) {
	// special case
	if (lhs.iszero() || rhs.iszero()) {
		lhs.setzero();
		return;
	}
	bool signOfFinalResult = (lhs.sign() != rhs.sign()) ? true : false;
	decimal product;
	// find the smallest decimal to minimize the amount of work
	size_t l = lhs.size();
	size_t r = rhs.size();
	decimal::const_iterator sit, bit; // sit = smallest iterator, bit = biggest iterator
	if (l < r) {
		size_t position = 0;
		for (sit = lhs.begin(); sit != lhs.end(); ++sit) {
			decimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
			partial_sum.insert(partial_sum.end(), r + position, 0);
			decimal::iterator pit = partial_sum.begin() + position;
			char carry = 0;
			for (bit = rhs.begin(); bit != rhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
				uint8_t digit = *sit * *bit + carry;
				*pit = digit % 10;
				carry = digit / 10;
			}
			if (carry) partial_sum.push_back(carry);
			add(product, partial_sum);
			//				std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
			++position;
		}
	}
	else {
		size_t position = 0;
		for (sit = rhs.begin(); sit != rhs.end(); ++sit) {
			decimal partial_sum; partial_sum.clear(); // TODO: this is silly, create and immediately destruct to make the insert work
			partial_sum.insert(partial_sum.end(), l + position, 0);
			decimal::iterator pit = partial_sum.begin() + position;
			char carry = 0;
			for (bit = lhs.begin(); bit != lhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
				uint8_t digit = *sit * *bit + carry;
				*pit = digit % 10;
				carry = digit / 10;
			}
			if (carry) partial_sum.push_back(carry);
			add(product, partial_sum);
			//				std::cout << "partial sum " << partial_sum << " intermediate product " << product << std::endl;
			++position;
		}
	}
	product.unpad();
	lhs = product;
	lhs.setsign(signOfFinalResult);
}

// find largest multiplier
decimal findLargestMultiple(const decimal& lhs, const decimal& rhs) {
	// check argument assumption	assert(0 <= lhs && lhs >= 9 * rhs);
	decimal one, remainder, multiplier;
	one.setdigit(1);
	remainder = lhs; remainder.setpos();
	multiplier.setdigit(0);
	for (int i = 0; i <= 11; ++i) {  // function works for 9 into 99, just as an aside
		if (remainder.ispos() && !remainder.iszero()) {  // test for proper > 0
			sub(remainder, rhs); //  remainder -= rhs;
			add(multiplier, one);  // ++multiplier
		}
		else {
			if (remainder.isneg()) {  // we went too far
				sub(multiplier, one); // --multiplier
			}
			// else implies remainder is 0										
			break;
		}
	}
	return multiplier;
}

// find the order of the most significant digit, precondition decimal is unpadded
inline int findMsd(const decimal& v) {
	int msd = int(v.size()) - 1;
	if (msd == 0 && v.iszero()) return -1; // no significant digit found, all digits are zero
	//assert(v.at(msd) != 0); // indicates the decimal wasn't unpadded
	return msd;
}

// integer division (equivalent to /=)
decimal div(const decimal& _a, const decimal& _b) {
	if (_b.iszero()) {
		throw "Divide by 0";
	}
	// generate the absolute values to do long division 
	bool a_negative = _a.sign();
	bool b_negative = _b.sign();
	bool result_negative = (a_negative ^ b_negative);
	decimal a(_a); a.setpos();
	decimal b(_b); b.setpos();
	decimal quotient; // zero
	if (less(a, b)) {
		return quotient; // a / b = 0 when b > a
	}
	// initialize the long division
	decimal accumulator = a;
	// prepare the subtractand
	decimal subtractand = b;
	int msd_b = findMsd(b);
	int msd_a = findMsd(a);
	int shift = msd_a - msd_b;
	subtractand.shiftLeft(shift);
	// long division
	for (int i = shift; i >= 0; --i) {
		if (lessOrEqual(subtractand, accumulator)) {
			decimal multiple = findLargestMultiple(accumulator, subtractand);
			// std::cout << accumulator << " / " << subtractand << " = " << multiple << std::endl;
			// accumulator -= multiple * subtractand;
			decimal partial;
			partial = subtractand;
			mul(partial, multiple);
			sub(accumulator, partial);
			uint8_t multiplier = multiple[0];
			quotient.insert(quotient.begin(), multiplier);
		}
		else {
			quotient.insert(quotient.begin(), 0);
		}
		subtractand.shiftRight(1);
		if (subtractand.iszero()) break;
	}
	if (result_negative) {
		quotient.setneg();
	}
	quotient.unpad();
	return quotient;
}

// generate an ASCII decimal format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const decimal& d) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the fixpnt into a string
	std::stringstream ss;

	//std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	if (d.sign()) ss << '-';
	for (decimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ostr << ss.str();
}

} // namespace impl

////////////////////////    INTEGER functions   /////////////////////////////////

template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> twos_complement(const fixpnt<nbits, rbits, arithmetic>& value) {
	fixpnt<nbits, rbits> complement = ~value;
	fixpnt<nbits, rbits> increment;
	increment.set_raw_bits(0x1);
	complement += increment;
	return complement;
}

// convert fixpnt to decimal string, i.e. "-1234.5678"
template<size_t nbits, size_t rbits, bool arithmetic>
std::string convert_to_decimal_string(const fixpnt<nbits, rbits, arithmetic>& value) {
	std::stringstream ss;
	if (value.iszero()) {
		ss << '0';
		if (rbits > 0) {
			ss << '.';
			for (size_t i = 0; i < rbits; ++i) {
				ss << '0';
			}
		}
		return ss.str();
	}
	if (value.sign()) ss << '-';
	impl::decimal partial, multiplier;
	fixpnt<nbits, rbits> number;
	number = value.sign() ? twos_complement(value) : value;
	if (nbits > rbits) {
		// convert the fixed point by first handling the integer part
		multiplier.setdigit(1);
		// convert fixpnt to decimal by adding and doubling multipliers
		for (unsigned i = rbits; i < nbits; ++i) {
			if (number.at(i)) {
				impl::add(partial, multiplier);
				//std::cout << partial << std::endl;
			}
			impl::add(multiplier, multiplier);
		}
		for (impl::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
			ss << (int)*rit;
		}
	}
	else {
		ss << '0';
	}

	if (rbits > 0) {
		ss << ".";
		// and secondly, the fraction part
		impl::decimal range, discretizationLevels, step;
		// create the decimal range we are discretizing
		range.setdigit(1);
		range.shiftLeft(rbits);  
		// calculate the discretization levels of this range
		discretizationLevels.setdigit(1);
		for (size_t i = 0; i < rbits; ++i) {
			impl::add(discretizationLevels, discretizationLevels);
		}
		step = div(range, discretizationLevels);
		// now construct the parts of this range the fraction samples
		partial.setzero();
		multiplier.setdigit(1);
		// convert the fraction part
		for (unsigned i = 0; i < rbits; ++i) {
			if (number.at(i)) {
				impl::add(partial, multiplier);
			}
			impl::add(multiplier, multiplier);
		}
		impl::mul(partial, step);
		// leading 0s will cause the partial to be represented incorrectly
		// if we simply convert it to digits.
		// The partial represents the parts in the range, so we can deduce
		// the number of leading zeros by comparing to the length of range
		size_t nrLeadingZeros = range.size() - partial.size() - 1;
		for (size_t i = 0; i < nrLeadingZeros; ++i) ss << '0';
		size_t digitsWritten = nrLeadingZeros;
		for (impl::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
			ss << (int)*rit;
			++digitsWritten;
		}
		if (digitsWritten < rbits) { // deal with trailing 0s
			for (size_t i = digitsWritten; i < rbits; ++i) {
				ss << '0';
			}
		}
	}
	return ss.str();
}

// findMsb takes an fixpnt<nbits,rbits> reference and returns the position of the most significant bit, -1 if v == 0
template<size_t nbits, size_t rbits, bool arithmetic>
inline signed findMsb(const fixpnt<nbits, rbits, arithmetic>& v) {
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
template<size_t nbits, size_t rbits, bool arithmetic>
void divide(const fixpnt<nbits, rbits, arithmetic>& a, const fixpnt<nbits, rbits, arithmetic>& b, fixpnt<nbits, rbits, arithmetic>& quotient) {
	if (b == fixpnt<nbits, rbits, arithmetic>(0)) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		throw fixpnt_divide_by_zero{};
#else
		std::cerr << "fixpnt_divide_by_zero\n";
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
	}
	fixpntdiv_t<nbits, rbits, arithmetic> divresult = fixpntdiv<nbits,rbits,arithmetic>(a, b);
	quotient = divresult.quot;
}

// calculate remainder of fixpnt<nbits,rbits> a and b and return result argument
template<size_t nbits, size_t rbits, bool arithmetic>
void remainder(const fixpnt<nbits, rbits, arithmetic>& a, const fixpnt<nbits, rbits, arithmetic>& b, fixpnt<nbits, rbits, arithmetic>& remainder) {
	if (b == fixpnt<nbits, rbits, arithmetic>(0)) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		throw fixpnt_divide_by_zero{};
#else
		std::cerr << "fixpnt_divide_by_zero\n";
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
	}
	fixpntdiv_t<nbits, rbits, arithmetic> divresult = fixpntdiv<nbits, rbits, arithmetic>(a, b);
	remainder = divresult.rem;
}

// divide fixpnt<nbits, rbits> a and b and return result argument
template<size_t nbits, size_t rbits, bool arithmetic>
fixpntdiv_t<nbits, rbits, arithmetic> fixpntdiv(const fixpnt<nbits, rbits, arithmetic>& _a, const fixpnt<nbits, rbits, arithmetic>& _b) {
	if (_b == fixpnt<nbits,rbits,arithmetic>(0)) {
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
	fixpnt<nbits + 1, rbits, arithmetic> a; a.bitcopy(a_negative ? -_a : _a);
	fixpnt<nbits + 1, rbits, arithmetic> b; b.bitcopy(b_negative ? -_b : _b);
	fixpntdiv_t<nbits, rbits, arithmetic> divresult;
	if (a < b) {
		divresult.rem = _a; // a % b = a when a / b = 0
		return divresult; // a / b = 0 when b > a
	}
	// initialize the long division
	fixpnt<nbits + 1, rbits, arithmetic> accumulator = a;
	// prepare the subtractand
	fixpnt<nbits + 1, rbits, arithmetic> subtractand = b;
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
	if (_a < fixpnt<nbits, rbits, arithmetic>(0)) {
		divresult.rem = -accumulator;
	}
	else {
		divresult.rem = accumulator;
	}

	return divresult;
}

/// stream operators

// read a fixed-point ASCII format and make a binary fixpnt out of it
template<size_t nbits, size_t rbits, bool arithmetic>
bool parse(const std::string& number, fixpnt<nbits, rbits, arithmetic>& value) {
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
		fixpnt<nbits, rbits, arithmetic> scale = 1;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			fixpnt<nbits, rbits, arithmetic> digit = charLookup.at(*r);
			value += scale * digit;
			scale *= 10;
		}
		bSuccess = true;
	}

	return bSuccess;
}

// generate an fixpnt format ASCII format
template<size_t nbits, size_t rbits, bool arithmetic>
inline std::ostream& operator<<(std::ostream& ostr, const fixpnt<nbits, rbits, arithmetic>& i) {
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
template<size_t nbits, size_t rbits, bool arithmetic>
inline std::istream& operator>>(std::istream& istr, fixpnt<nbits, rbits, arithmetic>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators
template<size_t nbits, size_t rbits, bool arithmetic>
inline std::string to_binary(const fixpnt<nbits, rbits, arithmetic>& number) {
	std::stringstream ss;
	for (int i = int(nbits) - 1; i >= int(rbits); --i) {
		ss << (number.at(i) ? '1' : '0');
	}
	ss << '.';
	for (int i = int(rbits) - 1; i >= 0; --i) {
		ss << (number.at(i) ? '1' : '0');
	}
	return ss.str();
}

template<size_t nbits, size_t rbits, bool arithmetic>
inline std::string to_triple(const fixpnt<nbits, rbits, arithmetic>& number) {
	std::stringstream ss;
	ss << (number.sign() ? "(-," : "(+,");
	ss << scale(number) << ',';
	for (int i = int(rbits) - 1; i >= 0; --i) {
		ss << (number.at(i) ? '1' : '0');
	}
	ss << (rbits == 0 ? "~)" : ")");
	return ss.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary logic operators

// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	for (unsigned i = 0; i < lhs.nrBytes; ++i) {
		if (lhs.b[i] != rhs.b[i]) return false;
	}
	return true;
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
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
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator< (rhs, lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator==(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator!=(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator< (const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator> (const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator< (rhs, lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator<=(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool operator>=(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator+(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	fixpnt<nbits, rbits> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator-(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	fixpnt<nbits, rbits> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator*(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	fixpnt<nbits, rbits> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator/(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	fixpnt<nbits, rbits> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator%(const fixpnt<nbits, rbits, arithmetic>& lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	fixpnt<nbits, rbits> ratio = lhs;
	ratio %= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator+(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator-(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator*(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator/(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator%(const fixpnt<nbits, rbits, arithmetic>& lhs, const long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator+(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator+(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator-(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator-(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator*(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator*(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator/(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator/(fixpnt<nbits, rbits>(lhs), rhs);
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> operator%(const long long lhs, const fixpnt<nbits, rbits, arithmetic>& rhs) {
	return operator%(fixpnt<nbits, rbits>(lhs), rhs);
}


} // namespace unum
} // namespace sw
