#pragma once
// fixpnt.hpp: definition of an arbitrary configuration binary fixed-point number parameterized in total bits and radix bits
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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

 // supportint types and functions
#include <universal/native/ieee754.hpp>   // IEEE-754 decoders
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/native/integers.hpp>   // manipulators for native integer types

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
#include <universal/number/fixpnt/exceptions.hpp>  // you need the exception types defined, but you may not throw them

// composition types used by fixpnt
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw::universal {

constexpr bool Modulo    = true;
constexpr bool Saturating = !Modulo;

// forward references
template<size_t nbits, size_t rbits, bool arithmetic, typename bt> class fixpnt;
template<size_t nbits, size_t rbits, bool arithmetic, typename bt> fixpnt<nbits, rbits, arithmetic, bt> abs(const fixpnt<nbits, rbits, arithmetic, bt>&);
template<size_t nbits, size_t rbits, bool arithmetic, typename bt> struct fixpntdiv_t;
template<size_t nbits, size_t rbits, bool arithmetic, typename bt> fixpntdiv_t<nbits, rbits, arithmetic, bt> fixpntdiv(const fixpnt<nbits, rbits, arithmetic, bt>&, const fixpnt<nbits, rbits, arithmetic, bt>&);

// fixpntdiv_t for fixpnt<nbits,rbits> to capture quotient and remainder during long division
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
struct fixpntdiv_t {
	fixpnt<nbits, rbits, arithmetic, bt> quot; // quotient
	fixpnt<nbits, rbits, arithmetic, bt> rem;  // remainder
};

template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
bool parse(const std::string& number, fixpnt<nbits, rbits, arithmetic, bt>& v);

// free function generator to create a 1's complement copy of a fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> onesComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	fixpnt<nbits, rbits, arithmetic, bt> ones(value);
	return ones.flip();
}
// free function generator to create the 2's complement of a fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> twosComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	fixpnt<nbits, rbits, arithmetic, bt> twos(value);
	return twos.twosComplement();;
}

// The free function scale calculates the power of 2 exponent that would capture an approximation of a normalized real value
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline int scale(const fixpnt<nbits, rbits, arithmetic, bt>& i) {
	fixpnt<nbits,rbits,arithmetic,bt> v(i);
	if (i.sign()) { // special case handling
		v = twosComplement(v);
		if (v == i) {  // special case of 10000..... largest negative number in 2's complement encoding
			return long(nbits - rbits);
		}
	}
	// calculate scale
	long scale = 0;
	if constexpr (nbits > rbits + 1) {  // subtle bug: in fixpnt numbers with only 1 bit before the radix point, '1' is maxneg, and thus while (v > 1) never completes
		v >>= rbits;
		while (v > 1) {
			++scale;
			v >>= 1;
		}
	}
	return scale;
}

// fixpnt is a binary fixed point number of nbits with rbits after the radix point
template<size_t _nbits, size_t _rbits, bool _arithmetic = Modulo, typename bt = uint8_t>
class fixpnt {
public:
	static_assert(_nbits >= _rbits, "fixpnt configuration error: nbits must be greater or equal to rbits");
	static constexpr size_t nbits = _nbits;
	static constexpr size_t rbits = _rbits;
	static constexpr bool   arithmetic = _arithmetic;
	typedef bt BlockType;
	static constexpr size_t bitsInChar = 8;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInChar;
	static constexpr size_t nrBlocks = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr size_t MSU = nrBlocks - 1;
	static constexpr bt     MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));

	constexpr fixpnt() noexcept : bb(0) {}

	constexpr fixpnt(const fixpnt&) noexcept = default;
	fixpnt(fixpnt&&) noexcept = default;

	constexpr fixpnt& operator=(const fixpnt&) noexcept = default;
	fixpnt& operator=(fixpnt&&) noexcept = default;

	// decorated/converting constructors

	/// <summary>
	/// construct a new fixpnt from another, sign extend when necessary: 
	/// src and tgt fixpnt need to have the same arithmetic and blocktype
	/// </summary>
	/// <param name="a">value to convert</param>
	template<size_t src_nbits, size_t src_rbits>
	fixpnt(const fixpnt<src_nbits, src_rbits, arithmetic, bt>& a) noexcept {
		*this = a;
	}

	// specific value constructor
	constexpr fixpnt(const SpecificValue code) : bb{ 0 } {
		switch (code) {
		case SpecificValue::infpos:
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::infneg:
		case SpecificValue::maxneg:
			maxneg();
			break;
		}
	}
	/////////   operators

	template<size_t src_nbits, size_t src_rbits>
	fixpnt& operator=(const fixpnt<src_nbits, src_rbits, arithmetic, bt>& a) noexcept {
		std::cout << typeid(a).name() << " goes into " << typeid(*this).name() << std::endl;
//		static_assert(src_nbits > nbits, "Source fixpnt is bigger than target: potential loss of precision"); 
// TODO: do we want prohibit this condition? To be consistent with native types we need to round down automatically.
		if (src_nbits <= nbits) {
			bb = a.bb;
			if (a.sign()) { // sign extend
				for (size_t i = src_nbits; i < nbits; ++i) setbit(i);
			}
		}
		else {
			// round down
			std::cerr << "rounding to smaller fixpnt not implemented yet\n";
		}
		return *this;
	}

	// initializers for native types
	constexpr fixpnt(signed char initial_value)        noexcept { *this = initial_value; }
	constexpr fixpnt(short initial_value)              noexcept { *this = initial_value; }
	constexpr fixpnt(int initial_value)                noexcept { *this = initial_value; }
	constexpr fixpnt(long initial_value)               noexcept { *this = initial_value; }
	constexpr fixpnt(long long initial_value)          noexcept { *this = initial_value; }
	constexpr fixpnt(char initial_value)               noexcept { *this = initial_value; }
	constexpr fixpnt(unsigned short initial_value)     noexcept { *this = initial_value; }
	constexpr fixpnt(unsigned int initial_value)       noexcept { *this = initial_value; }
	constexpr fixpnt(unsigned long initial_value)      noexcept { *this = initial_value; }
	constexpr fixpnt(unsigned long long initial_value) noexcept { *this = initial_value; }
	CONSTEXPRESSION fixpnt(float initial_value)        noexcept { *this = initial_value; }
	CONSTEXPRESSION fixpnt(double initial_value)       noexcept { *this = initial_value; }

	// access operator for bits
	// this needs a proxy to be able to create l-values
	// bool operator[](const unsigned int i) const //

	// simpler interface for now, using at(i) and set(i)/reset(i)

	// assignment operators for native types
	constexpr fixpnt& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr fixpnt& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr fixpnt& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr fixpnt& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr fixpnt& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr fixpnt& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	constexpr fixpnt& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr fixpnt& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr fixpnt& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr fixpnt& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION fixpnt& operator=(float rhs)        noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION fixpnt& operator=(double rhs)       noexcept { return convert_ieee754(rhs); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	CONSTEXPRESSION fixpnt(long double initial_value)   noexcept { *this = initial_value; }
	CONSTEXPRESSION fixpnt& operator=(long double rhs)  noexcept { return convert_ieee754(rhs);  }
	CONSTEXPRESSION explicit operator long double() const noexcept { return to_native<long double>(); }
#endif

	// assignment operator for blockbinary type
	template<size_t nnbits, typename Bbt>
	constexpr fixpnt& operator=(const blockbinary<nnbits, Bbt>& rhs) { bb = rhs; return *this; }

	// conversion operator between different fixed point formats with the same rbits
	template<size_t src_bits>
	fixpnt& operator=(const fixpnt<src_bits, rbits, arithmetic, bt>& src) {
		if (src_bits <= nbits) {
			// simple copy of the bytes
			for (unsigned i = 0; i < unsigned(src.nrBlocks); ++i) {
				bb[i] = src.block(i);
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
	constexpr fixpnt operator-() const { return sw::universal::twosComplement(*this); }
	// one's complement
	constexpr fixpnt operator~() const { 
		fixpnt complement(*this);
		complement.flip(); 
		return complement;
	}
	// increment by 1 ULP
	constexpr fixpnt operator++(int) {
		fixpnt tmp(*this);
		operator++();
		return tmp;
	}
	// increment by 1 ULP
	constexpr fixpnt& operator++() {
		fixpnt increment;
		increment.setbits(0x1);
		*this += increment;
		return *this;
	}
	// decrement by 1 ULP
	constexpr fixpnt operator--(int) {
		fixpnt tmp(*this);
		operator--();
		return tmp;
	}
	// decrement by 1 ULP
	constexpr fixpnt& operator--() {
		fixpnt decrement;
		decrement.setbits(0x1);
		return *this -= decrement;
	}
	// conversion operators
// Maybe remove explicit, MTL compiles, but we have lots of double computation then
	explicit constexpr operator unsigned short()     const noexcept { return to_unsigned<unsigned short>(); }
	explicit constexpr operator unsigned int()       const noexcept { return to_unsigned<unsigned int>(); }
	explicit constexpr operator unsigned long()      const noexcept { return to_unsigned<unsigned long>(); }
	explicit constexpr operator unsigned long long() const noexcept { return to_unsigned<unsigned long long>(); }
	explicit constexpr operator short()              const noexcept { return to_signed<short>(); }
	explicit constexpr operator int()                const noexcept { return to_signed<int>(); }
	explicit constexpr operator long()               const noexcept { return to_signed<long>(); }
	explicit constexpr operator long long()          const noexcept { return to_signed<long long>(); }
	explicit constexpr operator float()              const noexcept { return to_native<float>(); }
	explicit constexpr operator double()             const noexcept { return to_native<double>(); }

	// arithmetic operators
	fixpnt& operator+=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
			bb += rhs.bb;
		}
		else {
			using biggerbb = blockbinary<nbits + 1, bt>;
			biggerbb c = uradd(bb, rhs.bb);  // c = a + b
			fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
			biggerbb saturation = maxpos.getbb();		
			if (c >= saturation) {
				bb = saturation;
				return *this;
			}
			saturation = maxneg.getbb();
			if (c <= saturation) {
				bb = saturation;
				return *this;
			}
			bb = c;
		}
		return *this;
	}
	fixpnt& operator-=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
			operator+=(sw::universal::twosComplement(rhs));
		}
		else {
			using biggerbb = blockbinary<nbits + 1, bt>;
			biggerbb c = ursub(bb, rhs.getbb());  // c = a - b
			fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
			biggerbb saturation = maxpos.getbb();
			if (c >= saturation) {
				bb = saturation;
				return *this;
			}
			saturation = maxneg.getbb();
			if (c <= saturation) {
				bb = saturation;
				return *this;
			}
			bb = c;
		}
		return *this;
	}
	fixpnt& operator*=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
//			blockbinary<2 * nbits, bt> c = urmul(this->bb, rhs.bb);
			blockbinary<2 * nbits, bt> c = urmul2(this->bb, rhs.bb);
			bool roundUp = c.roundingMode(rbits);
			c >>= rbits;
			if (roundUp) ++c;
			this->bb = c; // select the lower nbits of the result
		}
		else {
			blockbinary<2 * nbits, bt> c = urmul2(this->bb, rhs.bb);
			fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
			blockbinary<2 * nbits, bt> saturation = maxpos.getbb();
			bool roundUp = c.roundingMode(rbits);
			c >>= rbits;
			if (c >= saturation) {
				bb = saturation;
				return *this;
			}
			saturation = maxneg.getbb();
			if (c < saturation) {
				bb = saturation;
				return *this;
			}
			if (roundUp) ++c;
			this->bb = c; // select the lower nbits of the result
		}
		return *this;
	}
	fixpnt& operator/=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
			constexpr size_t roundingDecisionBits = 4; // guard, round, and 2 sticky bits
			blockbinary<roundingDecisionBits, bt> roundingBits;
			blockbinary<2 * nbits + roundingDecisionBits, bt> c = urdiv(this->bb, rhs.bb, roundingBits);
			std::cout << to_binary(*this) << " / " << to_binary(rhs) << std::endl;
//			std::cout << to_binary(this->bb) << " / " << to_binary(rhs.bb) << " = " << to_binary(c) << " rounding bits " << to_binary(roundingBits);
			bool roundUp = c.roundingMode(rbits + roundingDecisionBits);
			c >>= rbits + nbits + roundingDecisionBits - 1;
			if (roundUp) ++c;
//			std::cout << " rounded " << to_binary(c) << std::endl;
			this->bb = c; // select the lower nbits of the result
		}
		else {
			std::cerr << "saturating divide not implemented yet\n";
		}
		return *this;
	}
	fixpnt& operator%=(const fixpnt& rhs) {
		bb %= rhs.bb;
		return *this;
	}
	fixpnt& operator<<=(const signed shift) {
		bb <<= shift;
		return *this;
	}
	fixpnt& operator>>=(const signed shift) {
		bb >>= shift;
		return *this;
	}
	
	// modifiers
	inline constexpr void clear() noexcept { bb.clear(); }
	inline constexpr void setzero() noexcept { bb.clear(); }

	// specific number system values we would like to have as constexpr

	// the value of a binary fixed point number is an binary integer that is scaled by a fixed factor, 2^rbits
// so the number 0100.0100 is the value 01000100 with an implicit scaling of 2^4 = 16
// 01000100 = 64 + 4 = 68 -> scaled by 16 = 4.25 -> 4 + 0.25 = 0100 + 0100

	// 01111....11111 is max pos
	// 00000....00001 is min pos
	// 00000....00000 is zero
	// 11111....11111 is min neg
	// 10000....00000 is max min


// minimum positive value of the fixed point configuration
	constexpr fixpnt& minpos() noexcept {
		static_assert(rbits <= nbits, "incorrect configuration of fixed-point number: nbits >= rbits");
		// minpos = 0000....00001
		clear();
		setbit(0, true);
		return *this;
	}

	// maximum value of the fixed point configuration
	// what is maxpos when all bits are fraction bits?
	//   still #.01111...11111 as the rbits simply define the range this value is scaled by
	// when rbits > nbits: is that a valid format? By definition, it is not:
	// a compile time assert has been added to enforce.
	constexpr fixpnt& maxpos() noexcept {
		static_assert(rbits <= nbits, "incorrect configuration of fixed-point number: nbits >= rbits");
		// maxpos = 01111....1111
		clear();
		flip();
		setbit(nbits - 1, false);
		return *this;
	}
	// zero
	constexpr fixpnt& zero() noexcept {
		clear();
		return *this;
	}
	// minimum positive value of the fixed point configuration
	constexpr fixpnt& minneg() noexcept {
		static_assert(rbits <= nbits, "incorrect configuration of fixed-point number: nbits >= rbits");
		// minneg = 11111....11111
		clear();
		flip();
		return *this;
	}

	// maximum negative value of the fixed point configuration
	constexpr fixpnt& maxneg() noexcept {
		static_assert(rbits <= nbits, "incorrect configuration of fixed-point number: nbits >= rbits");
		// maxneg = 10000....0000
		clear();
		setbit(nbits - 1);
		return *this;
	}
	inline constexpr void setbit(size_t bitIndex, bool v = true) noexcept {
		if (bitIndex < nbits) bb.setbit(bitIndex, v);
		// when bitIndex is out-of-bounds, fail silently as no-op
	}
	// use un-interpreted raw bits to set the bits of the fixpnt: TODO: expand the API to support fixed-points > 64 bits
	inline constexpr void setbits(uint64_t value) noexcept { bb.setbits(value); }
	inline fixpnt& assign(const std::string& txt) noexcept {
		if (!parse(txt, *this)) {
			std::cerr << "Unable to parse: " << txt << std::endl;
		}
		// must enforce precondition for fast comparison by  
		// properly nulling bits that are outside of nbits
		return *this;
	}
	// in-place 1's complement
	inline constexpr fixpnt& flip() noexcept { bb.flip(); return *this; }
	// in-place 2's complement
	inline constexpr fixpnt& twosComplement() noexcept { bb.twosComplement(); return *this; }

	// selectors
	inline constexpr bool sign()   const noexcept { return bb.sign(); }
	inline constexpr bool iszero() const noexcept { return bb.iszero(); }
	inline constexpr bool ispos()  const noexcept { return bb.ispos(); }
	inline constexpr bool isneg()  const noexcept { return bb.isneg(); }
	inline constexpr bool at(size_t bitIndex) const noexcept { return bb.at(bitIndex); }
	inline constexpr bool test(size_t bitIndex) const noexcept { return bb.test(bitIndex); }
	inline blockbinary<nbits, bt> getbb() const noexcept { return blockbinary<nbits, bt>(bb); }

protected:
	// HELPER methods
	// 
	// conversion helpers

	// convert a signed integer into a fixpnt
	// TODO: this method does not protect against being called with an unsigned integer
	template<typename SignedInt>
	inline constexpr fixpnt& convert_signed(SignedInt v) {
		clear();
		if (0 == v) return *this;
		if constexpr (arithmetic == Saturating) { 
			constexpr fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
			// check if we are in the representable range
			if (v >= static_cast<SignedInt>(maxpos)) { return *this = maxpos; }
			if (v <= static_cast<SignedInt>(maxneg)) { return *this = maxneg; }
		}
		bool negative = (v < 0 ? true : false);
		v = (v < 0 ? -v : v); // TODO: deal with maxneg?
		v <<= rbits; // we are modeling the fixed-point as a binary with a shift
		unsigned upper = (nbits < 64 ? nbits : 64);
		for (unsigned i = 0; i < upper; ++i) {
			if (v & 0x1) setbit(i);
			v >>= 1;
		}
		if (negative) twosComplement();
		return *this;
	}
	// convert an unsigned integer into a fixpnt
	// TODO: this method does not protect against being called with an signed integer
	template<typename UnsignedInt>
	inline constexpr fixpnt& convert_unsigned(UnsignedInt v) {
		clear();
		if (0 == v) return *this;
		if constexpr (arithmetic == Saturating) {	
			constexpr fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
			// check if we are in the representable range
			if (v >= static_cast<UnsignedInt>(maxpos)) { return *this = maxpos; }
			if (v <= static_cast<UnsignedInt>(maxneg)) { return *this = maxneg; }
		}
		constexpr uint64_t mask = 0x1;
		unsigned upper = (nbits <= 64 ? nbits : 64);
		for (unsigned i = 0; i < upper - rbits && v > 0; ++i) {
			if (v & mask) setbit(i + rbits); // we have no fractional part in v
			v >>= 1;
		}
		return *this;
	}

	template<typename Real>
	inline constexpr fixpnt& convert_ieee754(Real rhs) {
		clear();
		if (rhs == 0.0) return *this;
		if constexpr (arithmetic == Saturating) {	// check if the value is in the representable range
			fixpnt<nbits, rbits, arithmetic, bt> a;
			a.maxpos();
			if (rhs >= float(a)) { return *this = a; } // set to max pos value
			a.maxneg();
			if (rhs <= float(a)) { return *this = a; } // set to max neg value
		}

		bool s{ false };
		uint64_t unbiasedExponent{ 0 };
		uint64_t raw{ 0 };
		extractFields(rhs, s, unbiasedExponent, raw); // use native conversion
		if (unbiasedExponent > 0) raw |= (1ull << ieee754_parameter<Real>::fbits);
		int radixPoint = ieee754_parameter<Real>::fbits - (static_cast<int>(unbiasedExponent) - ieee754_parameter<Real>::bias);

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
			uint64_t mask = (1ull << (shiftRight - 1));
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			if (shiftRight > 1) {
				mask = (0xFFFF'FFFF'FFFF'FFFFull << (shiftRight - 2));
				mask = ~mask;
			}
			else {
				mask = 0;
			}
			bool sticky = (mask & raw);

			raw >>= shiftRight;  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1ul);
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
		raw = (s ? (~raw + 1) : raw); // if negative, map to two's complement
		setbits(raw);
		return *this;
	}

	// conversion functions

	// from fixed-point to native signed integer
	template<typename NativeInt>
	typename std::enable_if< std::is_integral<NativeInt>::value && std::is_signed<NativeInt>::value,
		NativeInt>::type to_signed() const {
		if constexpr (nbits <= rbits) return 0;
		constexpr unsigned sizeOfInteger = 8 * sizeof(NativeInt);
		NativeInt ll = 0;
		NativeInt mask = 1;
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
	
	// from fixed-point to native unsigned integer
	template<typename NativeInt>
	typename std::enable_if< std::is_integral<NativeInt>::value&& std::is_unsigned<NativeInt>::value,
		NativeInt>::type to_unsigned() const {
		return NativeInt(bb.to_long_long());
	}

	template<typename TargetFloat>
	TargetFloat to_native() const {
		// pick up the absolute value of the minimum normal and subnormal exponents 
		constexpr size_t minNormalExponent = static_cast<size_t>(-ieee754_parameter<TargetFloat > ::minNormalExp);
		constexpr size_t minSubnormalExponent = static_cast<size_t>(-ieee754_parameter<TargetFloat>::minSubnormalExp);
		static_assert(rbits <= minSubnormalExponent, "to_native: fixpnt fraction is too small to represent with requested floating-point type");
		TargetFloat multiplier = 0;
		if constexpr (rbits > minNormalExponent) { // value is a subnormal number
			multiplier = ieee754_parameter<TargetFloat>::minSubnormal;
			for (size_t i = 0; i < minSubnormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		else {
			// the value is a normal number
			multiplier = ieee754_parameter<TargetFloat>::minNormal;
			for (size_t i = 0; i < minNormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		// you pop out here with the starting bit value
		fixpnt<nbits, rbits, arithmetic, bt> raw = (sign() ? sw::universal::twosComplement(*this) : *this);
		// construct the value
		TargetFloat value{ 0.0 };
		for (size_t i = 0; i < nbits; ++i) {
			if (raw.at(i)) value += multiplier;
			multiplier *= 2.0; // these are error free multiplies
		}
		return (sign() ? -value : value);
	}

#ifdef DEPRECATED

	unsigned short to_ushort() const {
		return static_cast<unsigned short>(to_ulong_long());
	}
	unsigned int to_uint() const {
		return static_cast<unsigned int>(to_ulong_long());
	}
	unsigned long to_ulong() const {
		return static_cast<unsigned long>(to_ulong_long());
	}
	unsigned long long to_ulong_long() const {
		return static_cast<unsigned long long>(bb.to_long_long());
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
		fixpnt<nbits, rbits, arithmetic, bt> raw = (sign() ? sw::universal::twosComplement(*this) : *this);
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
		fixpnt<nbits, rbits, arithmetic, bt> raw = (sign() ? sw::universal::twosComplement(*this) : *this);
		// construct the value
		double value = 0;
		for (size_t i = 0; i < nbits; ++i) {
			if (raw.at(i)) value += multiplier;
			multiplier *= 2.0; // these are error free multiplies
		}
		return (sign() ? -value : value);

	}
	long double to_long_double() const {  // TODO : this is not a valid implementation
		return (long double)to_double();
	}

	// from native to fixed-point
	template<typename Ty>
	void float_assign(Ty& rhs) {
		clear();
		if constexpr (arithmetic == Saturating) {
			// we are implementing saturation for values that are outside of the fixed-point's range
			// check if we are in the representable range
			fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
			if (rhs >= (Ty)maxpos) {
				// set to max value
				flip();
				setbit(nbits - 1, false);
				return;
			}
			if (rhs <= (Ty)maxneg) {
				// set to max neg value
				setbit(nbits - 1, true);
				return;
			}
		}
		// for proper rounding, we need to get the full bit representation

		// generate the representation of one and cast to Ty
		Ty one = Ty(0x1ll << rbits);
		Ty tmp = rhs * one;
		*this = uint64_t(tmp);
	}
#endif

private:
	blockbinary<nbits, bt> bb;

	// convert
	template<size_t nnbits, size_t rrbits, bool aarithmetic, typename Bbt>
	friend std::string convert_to_decimal_string(const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& value);

	// fixpnt - fixpnt logic comparisons
	template<size_t nnbits, size_t rrbits, bool aarithmetic, typename Bbt>
	friend bool operator==(const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& lhs, const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& rhs);
	template<size_t nnbits, size_t rrbits, bool aarithmetic, typename Bbt>
	friend bool operator< (const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& lhs, const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& rhs);
};


/// Magnitude of a fixpnt. Expensive as fixpnts are encoded as 2's complement, so we need to test, copy, complement, and return.
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> abs(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	fixpnt<nbits, rbits, arithmetic, bt> tmp(v);
	if (v.isneg()) tmp.twosComplement();
	return tmp;
}

////////////////////////    FIXED-POINT functions   /////////////////////////////////

#ifdef LATER
// findMsb takes an fixpnt<nbits,rbits> reference and returns the position of the most significant bit, -1 if v == 0
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline signed findMsb(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	for (signed i = v.nrBlocks - 1; i >= 0; --i) {
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
#endif

////////////////////////    FIXED-POINT operators   /////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary logic operators

// equal: precondition is that the block-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (lhs.bb == rhs.bb);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (lhs.bb < rhs.bb);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary logic operators
// remember that literals are const types, so you need to refer to a literal like 0 as a const int

// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const int rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const int rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const int rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const int rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const int rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const int rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const long long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned int rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned int rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned int rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned int rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned int rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned int rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const unsigned long long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal float/double binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const float rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const float rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const float rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const float rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const float rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const float rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const double rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const double rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const double rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const double rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const double rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const double rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
 
//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal float/double - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator==(const double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator!=(const double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator< (const double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator> (const double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator<=(const double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool operator>=(const double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> ratio = lhs;
	ratio %= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary arithmetic operators

///////////////////// int
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////////  long
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////  long long
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

///////////////////// unsigned int
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////////  unsigned long
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////  unsigned long long
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal double binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal double - fixpnt binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}


///////////////////////////// IOSTREAM operators ///////////////////////////////////////////////

/// stream operators

///////////////////////////////////////////////////////////////////////////////////////////////
/// support detail

// paired down implementation of a decimal type to generate decimal representations for fixpnt<nbits,rbits> types
namespace support {

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
		inline void setneg() { _sign = true; }
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
				if (operator[](size_t(i)) == 0) {
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
		uint8_t borrow = 0;
		for (; lit != lhs.end() || rit != _rhs.end(); ++lit, ++rit) {
			if (*rit > *lit - borrow) {
				*lit = uint8_t(uint8_t(10) + *lit - borrow - *rit);
				borrow = 1;
			}
			else {
				*lit = uint8_t(*lit - borrow - *rit);
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
				decimal::iterator pit = partial_sum.begin() + static_cast<const int64_t>(position);
				uint8_t carry = 0;
				for (bit = rhs.begin(); bit != rhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
					uint8_t digit = uint8_t(*sit * *bit + carry);
					*pit = uint8_t(digit % 10);
					carry = uint8_t(digit / 10);
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
				decimal::iterator pit = partial_sum.begin() + static_cast<const int64_t>(position);
				uint8_t carry = 0;
				for (bit = lhs.begin(); bit != lhs.end() && pit != partial_sum.end(); ++bit, ++pit) {
					uint8_t digit = uint8_t(*sit * *bit + carry);
					*pit = uint8_t(digit % 10);
					carry = uint8_t(digit / 10);
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
		int shift = msd_a - msd_b; // precondition is a >= b, shift >= 0
		subtractand.shiftLeft(size_t(shift));
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

///////////////////////////////////////////////////////////////////////////////////////////////

// convert fixpnt to decimal string, i.e. "-1234.5678"
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
std::string convert_to_decimal_string(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	std::stringstream ss;
	if (value.iszero()) {
		ss << '0';
		if constexpr (rbits > 0) {
			ss << '.';
			for (size_t i = 0; i < rbits; ++i) {
				ss << '0';
			}
		}
		return ss.str();
	}
	if (value.sign()) ss << '-';
	support::decimal partial, multiplier;
	fixpnt<nbits, rbits, arithmetic, bt> number;
	number = value.sign() ? sw::universal::twosComplement(value) : value;
	if constexpr (nbits > rbits) {
		// convert the fixed point by first handling the integer part
		multiplier.setdigit(1);
		// convert fixpnt to decimal by adding and doubling multipliers
		for (unsigned i = rbits; i < nbits; ++i) {
			if (number.at(i)) {
				support::add(partial, multiplier);
				//std::cout << partial << std::endl;
			}
			support::add(multiplier, multiplier);
		}
		for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
			ss << (int)*rit;
		}
	}
	else {
		ss << '0';
	}

	if constexpr (rbits > 0) {
		ss << ".";
		// and secondly, the fraction part
		support::decimal range, discretizationLevels, step;
		// create the decimal range we are discretizing
		range.setdigit(1);
		range.shiftLeft(rbits);
		// calculate the discretization levels of this range
		discretizationLevels.setdigit(1);
		for (size_t i = 0; i < rbits; ++i) {
			support::add(discretizationLevels, discretizationLevels);
		}
		step = div(range, discretizationLevels);
		// now construct the parts of this range the fraction samples
		partial.setzero();
		multiplier.setdigit(1);
		// convert the fraction part
		for (unsigned i = 0; i < rbits; ++i) {
			if (number.at(i)) {
				support::add(partial, multiplier);
			}
			support::add(multiplier, multiplier);
		}
		support::mul(partial, step);
		// leading 0s will cause the partial to be represented incorrectly
		// if we simply convert it to digits.
		// The partial represents the parts in the range, so we can deduce
		// the number of leading zeros by comparing to the length of range
		size_t nrLeadingZeros = range.size() - partial.size() - 1;
		for (size_t i = 0; i < nrLeadingZeros; ++i) ss << '0';
		size_t digitsWritten = nrLeadingZeros;
		for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
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

// read a fixed-point ASCII format and make a binary fixpnt out of it
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
bool parse(const std::string& number, fixpnt<nbits, rbits, arithmetic, bt>& value) {
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
		int byte{ 0 };
		int byteIndex{ 0 };
		bool odd{ false };
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
		fixpnt<nbits, rbits, arithmetic, bt> scale = 1;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			fixpnt<nbits, rbits, arithmetic, bt> digit = charLookup.at(*r);
			value += scale * digit;
			scale *= 10;
		}
		bSuccess = true;
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/// stream operators

// ostream output generates an ASCII format for the fixed-point argument
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const fixpnt<nbits, rbits, arithmetic, bt>& i) {
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

// istream input reads an ASCII format and assigns value to the fixed-point argument
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline std::istream& operator>>(std::istream& istr, fixpnt<nbits, rbits, arithmetic, bt>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// string converters

// to_binary generates a binary presentation of the fixed-point number
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline std::string to_binary(const fixpnt<nbits, rbits, arithmetic, bt>& number, bool bNibbleMarker = false) {
	std::stringstream ss;
	ss << 'b';
	for (int i = static_cast<int>(nbits) - 1; i >= static_cast<int>(rbits); --i) {
		ss << (number.at(static_cast<size_t>(i)) ? '1' : '0');
		if (bNibbleMarker && (i - rbits) > 0 && (i - rbits) % 4 == 0) ss << '\'';
	}
	ss << '.';
	if constexpr (rbits > 0) {
		for (int i = int(rbits) - 1; i >= 0; --i) {
			ss << (number.at(static_cast<size_t>(i)) ? '1' : '0');
			if (bNibbleMarker && (rbits - i) % 4 == 0 && i != 0) ss << '\'';
		}
	}
	return ss.str();
}

// to_triple generates a triple (sign,scale,fraction) representation of the fixed-point number
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline std::string to_triple(const fixpnt<nbits, rbits, arithmetic, bt>& number) {
	std::stringstream ss;
	ss << (number.sign() ? "(-," : "(+,");
	ss << scale(number) << ',';
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		ss << (number.at(static_cast<size_t>(i)) ? '1' : '0');
	}
	ss << (rbits == 0 ? "~)" : ")");
	return ss.str();
}

} // namespace sw::universal
