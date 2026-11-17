#pragma once
#include <universal/utility/exceptions.hpp>  // UNIVERSAL_HAS_EXCEPTIONS (#1925)
// FIXPNT_THROW_DIVIDE_BY_ZERO: a division by zero throws fixpnt_divide_by_zero. fixpnt has no
// NaN or infinity to carry the failure in the value, so it throws unless the application opts
// out, by setting this to 0 or, when this is not set, FIXPNT_THROW_ARITHMETIC_EXCEPTION to 0.
// With it off, x / 0 is maxpos or maxneg (by the dividend's sign) for Saturate and 0 for
// Modulo, and nothing is printed (#1894). Decided here, before the default below, so that a
// FIXPNT_THROW_ARITHMETIC_EXCEPTION the application set is told apart from that default.
#if !defined(FIXPNT_THROW_DIVIDE_BY_ZERO)
#if defined(FIXPNT_THROW_ARITHMETIC_EXCEPTION)
#define FIXPNT_THROW_DIVIDE_BY_ZERO FIXPNT_THROW_ARITHMETIC_EXCEPTION
#else
// throw by default, unless the build has exceptions disabled (#1925)
#define FIXPNT_THROW_DIVIDE_BY_ZERO UNIVERSAL_HAS_EXCEPTIONS
#endif
#endif
// FIXPNT_THROW_ARITHMETIC_EXCEPTION: throw specific exceptions on the other arithmetic errors,
// left to the application to enable. The default lives here rather than in the fixpnt.hpp
// umbrella, so that including core.hpp alone defines it as well (#1436).
#if !defined(FIXPNT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#endif
// the fused dot product accumulator (quire, via fdp.hpp) must honor the same
// exception policy as the fixpnt it accumulates for (#1226)
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
#define QUIRE_THROW_ARITHMETIC_EXCEPTION FIXPNT_THROW_ARITHMETIC_EXCEPTION
#endif
#include <universal/utility/exceptions_check.hpp>  // reject a throw switch without exceptions (#1925)

// fixpnt_impl.hpp: implementation of an arbitrary configuration binary fixed-point number parameterized in total bits and radix bits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>       // the fixed-width integer types
#include <type_traits>   // the type traits used below
#include <algorithm>     // std::min / std::max
#include <string_view>   // std::string_view
#include <string>
#include <map>          // std::map, the character lookup in assign()
#include <iosfwd>       // std::ostream/std::istream in the friend declarations (#1334)
#include <cstdio>       // fprintf(stderr,...) for the diagnostics
#include <vector>
#include <limits>
#include <cmath>         // std::ldexp in to_native
#include <cassert>

// supporting types and functions
#include <universal/native/ieee754_core.hpp>   // IEEE-754 decoders; the text layer is not needed here (#1334)
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/utility/string_parse.hpp>   // shared constexpr scan_prefix / scan_sign / char classifiers

/*
The fixed-point arithmetic can be configured to:
- throw an exception on overflow
- saturation arithmetic: saturate on overflow
- modular arithmetic: quietly overflow into modular values

The quietly overflow configuration is reasonable when you are using
a fixed-point size that captures the dynamics of your computation.
Due to the fact that no special cases are required, the arithmetic
operators will be much faster than saturation.

Compile-time configuration flags are used to select the exception mode.
Run-time configuration is used to select modular vs saturation arithmetic.

You need the exception types defined, but you have the option to throw them
*/
#include <universal/number/fixpnt/exceptions.hpp>

// composition types used by fixpnt
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/shared/integer_cast.hpp>   // the integer-cast rule (#2104, #2111)
#include <universal/internal/blocktriple/blocktriple.hpp>
#ifdef FIXPNT_SCALE_TRACKING
#include <universal/utility/scale_tracker.hpp>
#endif

namespace sw { namespace universal {

#ifndef SW_UNIVERSAL_MODULO_SATURATE_DEFINED
#define SW_UNIVERSAL_MODULO_SATURATE_DEFINED
constexpr bool Modulo    = true;
constexpr bool Saturate = !Modulo;
#endif

// fixpntdiv_t for fixpnt<nbits,rbits> to capture quotient and remainder during long division
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
struct fixpntdiv_t {
	fixpnt<nbits, rbits, arithmetic, bt> quot; // quotient
	fixpnt<nbits, rbits, arithmetic, bt> rem;  // remainder
};

// free function generator to create a 1's complement copy of a fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> onesComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	fixpnt<nbits, rbits, arithmetic, bt> ones(value);
	return ones.flip();
}
// free function generator to create the 2's complement of a fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> twosComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	fixpnt<nbits, rbits, arithmetic, bt> twos(value);
	return twos.twosComplement();
}

// The free function scale calculates the power of 2 exponent that would capture an approximation of a normalized real value
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline int scale(const fixpnt<nbits, rbits, arithmetic, bt>& i) {
	fixpnt<nbits,rbits,arithmetic,bt> v(i);
	if (i.sign()) { // special case handling
		v = twosComplement(v);
		if (v == i) {  // special case of 10000..... largest negative number in 2's complement encoding
			if constexpr (nbits == rbits) {
				return -1;   // maxneg is -2^-1 (#2111)
			}
			else {
				return static_cast<int>(nbits) - static_cast<int>(rbits) - 1;
			}
		}
	}
	// calculate scale
	int scale = 0;
	if (!v.iszero()) {
		for (int bitIndex = static_cast<int>(nbits) - 2; bitIndex >= 0; --bitIndex) {
			if (v.test(static_cast<unsigned>(bitIndex))) {
				scale = bitIndex - static_cast<int>(rbits);
				break;
			}
		}
	}
	return scale;
}

// fixpnt is a binary fixed point number of nbits with rbits after the radix point
// The value of a binary fixed point number is an binary integer that is scaled by a fixed factor, 2^rbits.
// For example, the encoding 0100.0100 is the value 01000100 with an implicit scaling of 2^4 = 16
// => 01000100 = 64 + 4 = 68 -> scaled by 16 = 4.25 -> 4 + 0.25 = 0100 + 0100
template<unsigned _nbits, unsigned _rbits, bool _arithmetic = Modulo, typename bt = uint8_t>
class fixpnt {
public:
	static_assert(_nbits >= _rbits, "fixpnt configuration error: nbits must be greater or equal to rbits");
	static constexpr unsigned  nbits = _nbits;
	static constexpr unsigned  rbits = _rbits;
	static constexpr unsigned  fbits = _rbits;  // creating symmetry with other types in Universal
	static constexpr bool      arithmetic = _arithmetic;
	typedef bt BlockType;
	static constexpr unsigned  bitsInChar = 8;
	static constexpr unsigned  bitsInBlock = sizeof(bt) * bitsInChar;
	static constexpr unsigned  nrBlocks = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr unsigned  MSU = nrBlocks - 1;
	static constexpr bt        MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));

	// constructors
	fixpnt() noexcept = default;
	fixpnt(const fixpnt&) noexcept = default;
	fixpnt(fixpnt&&) noexcept = default;

	constexpr fixpnt& operator=(const fixpnt&) noexcept = default;
	constexpr fixpnt& operator=(fixpnt&&) noexcept = default;

	// decorated/converting constructors

	/// <summary>
	/// construct a new fixpnt from another, sign extend or round when necessary: 
	/// src and tgt fixpnt need to have the same arithmetic and blocktype
	/// </summary>
	/// <param name="a">source fixpnt</param>
	template<unsigned src_nbits, unsigned src_rbits>
	constexpr fixpnt(const fixpnt<src_nbits, src_rbits, arithmetic, bt>& a) noexcept { *this = a; }

	// specific value constructor
	constexpr fixpnt(const SpecificValue code) : _block{ 0 } {
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

	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////   operators


	// assignment operator for blockbinary type
	template<unsigned nnbits, typename Bbt>
	constexpr fixpnt& operator=(const blockbinary<nnbits, Bbt>& rhs) { _block = rhs; return *this; }

	// fixpnt size adapter: convert fixpnt<src_nbits, src_rbits> -> fixpnt<nbits, rbits>.
	// The value is aligned in a blockbinary wide enough to hold it exactly, rounded to nearest
	// even at the target's lsb, and then Saturate clamps to [maxneg, maxpos] while Modulo keeps the
	// low nbits (#2111: Saturate wrapped, and a right shift of nbits or more lost the tie bit).
	template<unsigned src_nbits, unsigned src_rbits>
	constexpr fixpnt& operator=(const fixpnt<src_nbits, src_rbits, arithmetic, bt>& a) noexcept {
		if constexpr (src_rbits == rbits && src_nbits <= nbits) {
			_block = a.bits();   // radix points aligned and no overflow: sign extension only
		}
		else {
			constexpr unsigned lshift = (rbits > src_rbits) ? rbits - src_rbits : 0u;
			constexpr unsigned rshift = (src_rbits > rbits) ? src_rbits - rbits : 0u;
			constexpr unsigned W = ((src_nbits + lshift > nbits) ? src_nbits + lshift : nbits) + 2u;
			using Wide = blockbinary<W, bt, BinaryNumberType::Signed>;
			Wide w(a.bits());   // sign-extends
			if constexpr (lshift > 0) w <<= static_cast<int>(lshift);
			if constexpr (rshift > 0) {
				const bool roundUp = w.roundingMode(rshift);   // the source's lsb is still in w: W > rshift
				w >>= static_cast<int>(rshift);                 // arithmetic: floor of the two's complement value
				if (roundUp) ++w;
			}
			if constexpr (arithmetic == Saturate) {
				const Wide maxposW{ blockbinary<nbits, bt, BinaryNumberType::Signed>(SpecificValue::maxpos) };
				const Wide maxnegW{ blockbinary<nbits, bt, BinaryNumberType::Signed>(SpecificValue::maxneg) };
				if (w > maxposW) { maxpos(); return *this; }
				if (w < maxnegW) { maxneg(); return *this; }
			}
			_block = w;   // the low nbits: the value itself, or its wrap in Modulo
		}
		return *this;
	}

	// initializers for native types
	constexpr fixpnt(signed char initial_value)         noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(short initial_value)               noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(int initial_value)                 noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(long initial_value)                noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(long long initial_value)           noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(char initial_value)                noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(unsigned short initial_value)      noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(unsigned int initial_value)        noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(unsigned long initial_value)       noexcept : fixpnt{convert(initial_value)} {}
	constexpr fixpnt(unsigned long long initial_value)  noexcept : fixpnt{convert(initial_value)} {}
	BIT_CAST_CONSTEXPR fixpnt(float initial_value)      noexcept : fixpnt{convert(initial_value)} {}
	BIT_CAST_CONSTEXPR fixpnt(double initial_value)     noexcept : fixpnt{convert(initial_value)} {}

	// access operator for bits
	// this needs a proxy to be able to create l-values
	// bool operator[](const unsigned int i) const //

	// simpler interface for now, using at(i) and set(i)/reset(i)

	// assignment operators for native types
	constexpr fixpnt& operator=(signed char rhs)        noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(short rhs)              noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(int rhs)                noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(long rhs)               noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(long long rhs)          noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(char rhs)               noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(unsigned short rhs)     noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(unsigned int rhs)       noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(unsigned long rhs)      noexcept { return *this = convert(rhs); }
	constexpr fixpnt& operator=(unsigned long long rhs) noexcept { return *this = convert(rhs); }
	BIT_CAST_CONSTEXPR fixpnt& operator=(float rhs)     noexcept { return *this = convert(rhs); }
	BIT_CAST_CONSTEXPR fixpnt& operator=(double rhs)    noexcept { return *this = convert(rhs); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	fixpnt(long double initial_value)                   noexcept : fixpnt{ convert(initial_value) } {}
	fixpnt& operator=(long double rhs)                  noexcept { return *this = convert(rhs);  }
	explicit operator long double()               const noexcept { return to_native<long double>(); }
#endif

	// assign the value of the textual representation to the fixpnt: can be binary/octal/decimal/hexadecimal
	// We want to make this a constexpr so that we could create arbitrary constants, but Clang
	// doesn't have full STL constexpr support yet, so if failing this.
	fixpnt& assign(const std::string& number) {
		clear();

		// a binary representation is "0b" + intbits + '.' + fracbits, with the radix point at rbits
		bool binaryFormat = false;
		if (number.size() >= 3 && number[0] == '0' && number[1] == 'b') binaryFormat = true;
		if (binaryFormat) {
			unsigned position = 0;
			for (std::string::const_reverse_iterator r = number.rbegin(); r != number.rend(); ++r) {
				if (*r == 'b') {
					break;
				}
				else if (*r == '\'') {
					// ignore delimiters
				}
				else if (*r == '.') {
					if (position != rbits) {
						clear();
						std::fprintf(stderr, "radix in string value is not aligned with fixpnt format\n");
						break;
					}
				}
				else if (*r == '0') {
					setbit(position, false);
					++position;
				}
				else {
					setbit(position, true);
					++position;
				}
			}
		}
		else {
			// a decimal, signed or not, and the 0x / 0o bit patterns: parse() rounds the exact value
			// to nearest even and follows the arithmetic mode; malformed text leaves 0 (#2111: this
			// branch was a stub that set 6.90234375, and "-1.5" threw from the character lookup)
			if (!parse(number, *this)) clear();
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
	constexpr fixpnt operator-() const {
		fixpnt a = sw::universal::twosComplement(*this);
		constexpr fixpnt maxnegative(SpecificValue::maxneg);
		if (a == maxnegative) {
			a.flip(); // approximate but closed to negated value
		}
		return a; 
	}
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
	constexpr fixpnt& operator+=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
			_block += rhs._block;
		}
		else {
			using biggerbb = blockbinary<nbits + 1, bt>;
			biggerbb c = uradd(_block, rhs._block);  // c = a + b
			constexpr fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);   // compile-time constants (#2111)
			biggerbb saturation = maxpos.bits();		
			if (c >= saturation) {
				_block = saturation;
				return *this;
			}
			saturation = maxneg.bits();
			if (c <= saturation) {
				_block = saturation;
				return *this;
			}
			_block = c;
		}
		return *this;
	}
	constexpr fixpnt& operator-=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
			operator+=(sw::universal::twosComplement(rhs));
		}
		else {
			using biggerbb = blockbinary<nbits + 1, bt>;
			biggerbb c = ursub(_block, rhs.bits());  // c = a - b
			constexpr fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);   // compile-time constants (#2111)
			biggerbb saturation = maxpos.bits();
			if (c >= saturation) {
				_block = saturation;
				return *this;
			}
			saturation = maxneg.bits();
			if (c <= saturation) {
				_block = saturation;
				return *this;
			}
			_block = c;
		}
		return *this;
	}
	constexpr fixpnt& operator*=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
//			blockbinary<2 * nbits, bt> c = urmul(_block, rhs._block);
			blockbinary<2 * nbits, bt> c = urmul2(_block, rhs._block);
			bool roundUp = c.roundingMode(rbits);
			c >>= rbits;
			if (roundUp) ++c;
			this->_block = c; // select the lower nbits of the result
		}
		else {
			blockbinary<2 * nbits, bt> c = urmul2(_block, rhs._block);
			constexpr fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);   // compile-time constants (#2111)
			blockbinary<2 * nbits, bt> saturation = maxpos.bits();
			bool roundUp = c.roundingMode(rbits);
			c >>= rbits;
			if (c >= saturation) {
				_block = saturation;
				return *this;
			}
			saturation = maxneg.bits();
			if (c < saturation) {
				_block = saturation;
				return *this;
			}
			if (roundUp) ++c;
			_block = c; // select the lower nbits of the result
		}
		return *this;
	}
	/// Divide this fixpnt by rhs in-place.
	/// In Modulo mode, the result wraps on overflow.
	/// In Saturate mode, the result clamps to maxpos or maxneg on overflow.
	constexpr fixpnt& operator/=(const fixpnt& rhs) {
#if FIXPNT_THROW_DIVIDE_BY_ZERO
		if (rhs.iszero()) throw fixpnt_divide_by_zero();
#else
		if (rhs.iszero()) {
			if constexpr (arithmetic == Saturate) {
				// saturate to maxpos or maxneg based on sign of dividend
				if (isneg())
					*this = fixpnt(SpecificValue::maxneg);
				else
					*this = fixpnt(SpecificValue::maxpos);
			}
			else { // arithmetic == Modulo
				setzero(); // result is undefined
			}
			return *this;
		}
#endif
		if constexpr (arithmetic == Modulo) {
			bool positive = (ispos() && rhs.ispos()) || (isneg() && rhs.isneg());  // XNOR

			// a fixpnt<nbits,rbits> division scale to a fixpnt<2 * nbits + 1, nbits - 1>
			// via an upshift by 2 * rbits of the dividend and un upshift by rbits of the divisor
			constexpr unsigned roundingBits = nbits;
			constexpr unsigned accumulatorSize = 2 * nbits + 2 * rbits + 2 * roundingBits;
			blockbinary<accumulatorSize, bt> dividend(_block);
			if (dividend.isneg()) dividend.twosComplement();
			dividend <<= (2 * (rbits + roundingBits)); // scale up to include rounding bits
			blockbinary<accumulatorSize, bt> divisor(rhs.bits());
			if (divisor.isneg()) divisor.twosComplement();
			divisor <<= rbits + roundingBits;
			// use longdivision to capture remainder for sticky-bit rounding
			quorem<accumulatorSize, bt, BinaryNumberType::Signed> result = longdivision(dividend, divisor);
			blockbinary<accumulatorSize, bt> quotient = result.quo;

			bool roundUp = quotient.roundingMode(roundingBits);
			// OR non-zero remainder into sticky bit: if rounding bits
			// look like an exact tie but remainder is non-zero, round up
			if (!roundUp && !result.rem.iszero()) {
				// check if rounding bits indicate a tie (guard=1, rest=0)
				// with non-zero remainder the true value is > tie, so round up
				constexpr unsigned guardBitPos = roundingBits - 1;
				if (quotient.test(guardBitPos)) roundUp = true;
			}
			quotient >>= roundingBits;
			if (roundUp) ++quotient;
			_block = (positive ? quotient : quotient.twosComplement());
		}
		else {
			bool positive = (ispos() && rhs.ispos()) || (isneg() && rhs.isneg());  // XNOR

			constexpr unsigned roundingBits = nbits;
			constexpr unsigned accumulatorSize = 2 * nbits + 2 * rbits + 2 * roundingBits;
			blockbinary<accumulatorSize, bt> dividend(_block);
			if (dividend.isneg()) dividend.twosComplement();
			dividend <<= (2 * (rbits + roundingBits));
			blockbinary<accumulatorSize, bt> divisor(rhs.bits());
			if (divisor.isneg()) divisor.twosComplement();
			divisor <<= rbits + roundingBits;
			// use longdivision to capture remainder for sticky-bit rounding
			quorem<accumulatorSize, bt, BinaryNumberType::Signed> result = longdivision(dividend, divisor);
			blockbinary<accumulatorSize, bt> quotient = result.quo;

			bool roundUp = quotient.roundingMode(roundingBits);
			// OR non-zero remainder into sticky bit for correct tie-breaking
			if (!roundUp && !result.rem.iszero()) {
				constexpr unsigned guardBitPos = roundingBits - 1;
				if (quotient.test(guardBitPos)) roundUp = true;
			}
			quotient >>= roundingBits;
			if (roundUp) ++quotient;

			// saturation clamping on unsigned magnitude before sign restoration
			constexpr fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);   // compile-time constants (#2111)
			blockbinary<accumulatorSize, bt> maxposBits(maxpos.bits());
			if (positive) {
				if (quotient > maxposBits) {
					_block = maxpos.bits();
					return *this;
				}
				_block = quotient;
			}
			else {
				// |maxneg| = maxpos + 1 in two's complement
				blockbinary<accumulatorSize, bt> maxnegMagnitude(maxposBits);
				++maxnegMagnitude;
				if (quotient > maxnegMagnitude) {
					_block = maxneg.bits();
					return *this;
				}
				quotient.twosComplement();
				_block = quotient;
			}
		}
		return *this;
	}
	constexpr fixpnt& operator%=(const fixpnt& rhs) {
		_block %= rhs._block;
		return *this;
	}
	constexpr fixpnt& operator<<=(const signed shift) {
		_block <<= shift;
		return *this;
	}
	constexpr fixpnt& operator>>=(const signed shift) {
		_block >>= shift;
		return *this;
	}
	
	// modifiers
	constexpr void clear() noexcept { _block.clear(); }
	constexpr void setzero() noexcept { _block.clear(); }
	constexpr void setbit(unsigned bitIndex, bool v = true) noexcept {
		if (bitIndex < nbits) _block.setbit(bitIndex, v);
		// when bitIndex is out-of-bounds, fail silently as no-op
	}
	constexpr void setbits(uint64_t value) noexcept { _block.setbits(value); }

	// specific number system values we would like to have as constexpr
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
	// maximum positive value of the fixed point configuration
	constexpr fixpnt& maxpos() noexcept {
		// what is maxpos when all bits are fraction bits?
		//   still #.01111...11111 as the rbits simply define the range this value is scaled by
		// when rbits > nbits: is that a valid format? By definition, it is not:
		// A compile-time assert has been added to enforce.
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
	// minimum negative value of the fixed point configuration
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
	
		
	// in-place 1's complement
	constexpr fixpnt& flip() noexcept { _block.flip(); return *this; }
	// in-place 2's complement
	constexpr fixpnt& twosComplement() noexcept { _block.twosComplement(); return *this; }

	// selectors
	constexpr bool    sign()                  const noexcept { return _block.sign(); }
	// integer() is floor(x) and fraction() is x - floor(x), in [0, 1), from the encoding: clearing
	// the fraction bits of a two's complement value floors it (#2111: through double, wrong past 53 bits)
	constexpr fixpnt  integer()               const noexcept { fixpnt f(*this); for (unsigned i = 0; i < rbits && i < nbits; ++i) f._block.setbit(i, false); return f; }
	constexpr fixpnt  fraction()              const noexcept { fixpnt f(*this); for (unsigned i = rbits; i < nbits; ++i) f._block.setbit(i, false); return f; }
	constexpr bool    iszero()                const noexcept { return _block.iszero(); }
	constexpr bool    ispos()                 const noexcept { return _block.ispos(); }
	constexpr bool    isneg()                 const noexcept { return _block.isneg(); }
	constexpr bool    isnan()                 const noexcept { return false; }
	constexpr bool    isinf()                 const noexcept { return false; }
	constexpr bool    at(unsigned bitIndex)   const noexcept { return _block.at(bitIndex); }
	constexpr bool    test(unsigned bitIndex) const noexcept { return _block.test(bitIndex); }
	constexpr uint8_t nibble(unsigned n)      const noexcept { return _block.nibble(n); }

	// collect a copy of the underlying bit representation
	constexpr blockbinary<nbits, bt, BinaryNumberType::Signed> bits() const noexcept { return _block; }

	// normalize: decompose fixpnt value into a blocktriple<rbits, REP> for quire accumulation
	template<typename TargetBlockType = bt>
	void normalize(blocktriple<rbits, BlockTripleOperator::REP, TargetBlockType>& tgt) const {
		if (iszero()) { tgt.setzero(); return; }
		tgt.setnormal();
		tgt.setsign(isneg());
		// get magnitude
		blockbinary<nbits, bt, BinaryNumberType::Signed> magnitude = _block;
		if (isneg()) magnitude.twosComplement();
		// find MSB
		int msb = -1;
		for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
			if (magnitude.test(static_cast<unsigned>(i))) { msb = i; break; }
		}
		if (msb < 0) { tgt.setzero(); return; }
		int scale = msb - static_cast<int>(rbits);
		tgt.setscale(scale);
		// copy bits: hidden bit at rbits, fraction bits below
		tgt.setbit(rbits);  // hidden bit
		for (unsigned k = 0; k < rbits && static_cast<int>(k) <= msb; ++k) {
			int src = msb - 1 - static_cast<int>(k);
			if (src >= 0) tgt.setbit(rbits - 1 - k, magnitude.test(static_cast<unsigned>(src)));
		}
	}

protected:
	// HELPER methods
	// 
	// conversion helpers

#if LONG_DOUBLE_SUPPORT
	// A finite, non-zero long double into a fixpnt that can hold more than 62 significant bits, from
	// every bit of its significand: the 64 extractFields() gives end in a round-to-odd sticky on
	// IEEE binary128 and double-double, and fixpnt<128,64> of 2^40 + 2^-60 lost the 2^-60 (#1517).
	// Bit j of the significand, the leading one j = 0, has weight 2^(scale - j) and lands at
	// position scale + rbits - j. Positions from nbits up wrap away, as they do for a double; below
	// position 0 the first is the guard bit and the rest sticky. Rounded to nearest even on the
	// magnitude, then negated, as the double path does.
	static fixpnt convert_long_double(long double v) {
		fixpnt f;
		f.clear();
		long_double_significand sig(v);
		int  position = sig.scale() + static_cast<int>(rbits);
		bool lsb = false, guard = false, sticky = false;
		while (!sig.empty() && position >= -1) {
			const uint64_t word = sig.next();
			for (int k = 63; k >= 0; --k, --position) {
				if (((word >> k) & 1ull) == 0) continue;
				if (position >= static_cast<int>(nbits)) continue;  // wraps away
				if (position >= 0) {
					f.setbit(static_cast<unsigned>(position));
					if (position == 0) lsb = true;
				}
				else if (position == -1) {
					guard = true;
				}
				else {
					sticky = true;
				}
			}
		}
		if (!sig.empty()) sticky = true;
		// round on the raw bits: a Saturate ++f stops at maxpos, which loses the carry that a
		// magnitude just below 2^(ib-1) needs to become maxneg once negated
		if (guard && (sticky || lsb)) ++f._block;
		if (sig.negative()) f.twosComplement();
		return f;
	}
#endif

	// 2^e in the floating-point type Real, by exact repeated scaling so it can be evaluated at
	// compile time (std::ldexp is not constexpr): +inf when 2^e overflows Real, and 0 when it is
	// below Real's smallest subnormal
	template<typename Real>
	static constexpr Real exact_pow2(int e) {
		if (e >= std::numeric_limits<Real>::max_exponent) return std::numeric_limits<Real>::infinity();
		if (e < std::numeric_limits<Real>::min_exponent - std::numeric_limits<Real>::digits) return Real(0);
		Real r = Real(1);
		const Real step = (e >= 0) ? Real(2) : Real(0.5);
		for (int n = (e >= 0) ? e : -e; n > 0; --n) r *= step;
		return r;
	}

	// convert arithmetic types into a fixpnt
	template<typename Arith>
	static constexpr fixpnt convert(Arith v) {
		static_assert(std::is_arithmetic_v<Arith>);
		fixpnt f;
		f.clear();
		// the integer part of a fixpnt has ib bits, sign included, so it holds the integers in
		// [-2^(ib-1), 2^(ib-1)). The range check is done on that bound in the source type:
		// converting maxpos/maxneg into Arith instead wraps as soon as ib exceeds Arith's width.
		constexpr unsigned ib = nbits - rbits;
		if constexpr (std::is_integral_v<Arith> && std::is_signed_v<Arith>) {
			if (0 == v) return f;
			constexpr unsigned sizeofInteger = 8 * sizeof(v);
			if constexpr (arithmetic == Saturate) {
				if constexpr (ib == 0) {
					// no integer bits: the range is [-0.5, 0.5), so every nonzero integer is out of range
					return (v > 0 ? fixpnt(SpecificValue::maxpos) : fixpnt(SpecificValue::maxneg));
				}
				else if constexpr (ib < sizeofInteger) {
					// 2^(ib-1) fits Arith; when ib >= sizeofInteger every Arith value fits the fixpnt
					constexpr Arith bound = static_cast<Arith>(Arith(1) << (ib - 1));
					if (v >= bound) return fixpnt(SpecificValue::maxpos);
					if (v < -bound) return fixpnt(SpecificValue::maxneg);
				}
			}
			if (v == std::numeric_limits<Arith>::min()) {
				// v is -2^(sizeofInteger-1): set that bit and sign-extend it through the integer part,
				// or leave zero when Modulo wraps it away (-2^(sizeofInteger-1) mod 2^ib == 0)
				if constexpr (sizeofInteger <= ib) {
					for (unsigned i = sizeofInteger + rbits - 1; i < nbits; ++i) f.setbit(i);
				}
			}
			else {
				bool negative = (v < 0 ? true : false);
				v = (v < 0 ? -v : v);
				constexpr unsigned upper = (sizeofInteger < ib) ? sizeofInteger : ib;
				for (unsigned i = 0; i < upper; ++i) {
					if (v & 0x1) f.setbit(i + rbits);
					v >>= 1;
				}
				if (negative) f.twosComplement();
			}
		}
		else if constexpr (std::is_unsigned_v<Arith>) {
			if (0 == v) return f;
			constexpr unsigned sizeofInteger = 8 * sizeof(v);
			if constexpr (arithmetic == Saturate) {
				// an unsigned value can only overflow upward, past maxpos
				if constexpr (ib == 0) {
					return fixpnt(SpecificValue::maxpos);
				}
				else if constexpr (ib - 1 < sizeofInteger) {
					// 2^(ib-1) fits Arith; otherwise every Arith value fits the fixpnt
					constexpr Arith bound = static_cast<Arith>(Arith(1) << (ib - 1));
					if (v >= bound) return fixpnt(SpecificValue::maxpos);
				}
			}
			// keep the low ib bits: Modulo wraps there, and Saturate has already clamped what does not fit
			constexpr unsigned upper = (sizeofInteger < ib) ? sizeofInteger : ib;
			for (unsigned i = 0; i < upper && v > 0; ++i) {
				if (v & 0x1) f.setbit(i + rbits); // we have no fractional part in v
				v >>= 1;
			}
		}
		else if constexpr (std::is_floating_point_v<Arith>) {
			if (v == 0.0) return f;
			if constexpr (arithmetic == Saturate) {	// check if the value is in the representable range
				// Clamp at +/-2^(ib-1), a power of two and so exact in every floating-point format.
				// A |v| below it rounds to at most 2^(ib-1): for a negative v that is maxneg, and for
				// a positive v it carries into the sign bit, which is caught after the conversion.
				// Comparing against float(maxpos) instead let maxpos + 3/4 ulp through, and it
				// wrapped to maxneg (#1915).
				constexpr Arith bound = exact_pow2<Arith>(static_cast<int>(ib) - 1);
				fixpnt a;
				if (v >= bound) { return a.maxpos(); }
				if (v <= -bound) { return a.maxneg(); }
			}

#if LONG_DOUBLE_SUPPORT
			if constexpr (std::is_same_v<Arith, long double> && nbits > 62) {
				fixpnt r = convert_long_double(v);
				if constexpr (arithmetic == Saturate) {
					if (v > 0 && r.sign()) r.maxpos();  // rounded up into the sign bit
				}
				return r;
			}
#endif
			bool s{ false };
			uint64_t unbiasedExponent{ 0 };
			uint64_t fraction{ 0 };
			uint64_t bits{ 0 };
			extractFields(v, s, unbiasedExponent, fraction, bits); // use native conversion
			if (unbiasedExponent > 0) fraction |= (1ull << ieee754_parameter<Arith>::fbits);
			int biasedExponent = static_cast<int>(unbiasedExponent) - ieee754_parameter<Arith>::bias;
			int radixPoint = ieee754_parameter<Arith>::fbits - biasedExponent;

			// our fixed-point has its radixPoint at rbits
			// A shift past fbits + 1 puts the leading bit below the guard position: the value rounds
			// to zero. Do not cap the shift at 64 first: for an x87 long double (fbits 63) the
			// leading bit then became the guard bit, and 0.3L rounded to 1 at fixpnt<16,0>.
			int shiftRight = radixPoint - int(rbits);
			if (shiftRight > (ieee754_parameter<Arith>::fbits + 1)) return f; // return zero
			if (shiftRight > 0) {
				// we need to round the raw bits
				// collect guard, round, and sticky bits
				// this same logic will work for the case where
				// we only have a guard bit and no round and/or sticky bits
				// because the mask logic will make round and sticky both 0
				// so no need to special case it
				uint64_t mask = (1ull << (shiftRight - 1));
				bool guard = (mask & fraction);
				mask >>= 1;
				bool round = (mask & fraction);
				if (shiftRight > 1) {
					mask = (0xFFFF'FFFF'FFFF'FFFFull << (shiftRight - 2));
					mask = ~mask;
				}
				else {
					mask = 0;
				}
				bool sticky = (mask & fraction);

				// shift out the bits we are rounding away; a shift of 64 (x87 fbits + 1) leaves nothing
				fraction = (shiftRight < 64) ? (fraction >> shiftRight) : 0;
				bool lsb = (fraction & 0x1ul);
				//  ... lsb | guard  round sticky   round
				//       x     0       x     x       down
				//       0     1       0     0       down  round to even
				//       1     1       0     0        up   round to even
				//       x     1       0     1        up
				//       x     1       1     0        up
				//       x     1       1     1        up
				if (guard) {
					if (lsb && (!round && !sticky)) ++fraction; // round to even
					if (round || sticky) ++fraction;
				}
				// negate in the full nbits: a 64-bit two's complement would leave the bits above 64 zero
				f.setbits(fraction);
				if (s) f.twosComplement();
			}
			else {
				int shiftLeft = -shiftRight;
				if (shiftLeft < (64 - ieee754_parameter<Arith>::fbits)) {  // what is the distance between the MSB and 64?
					// no need to round, just shift the bits in place
					fraction <<= shiftLeft;
					f.setbits(fraction);
					if (s) f.twosComplement(); // negate in the full nbits, as above
				}
				else {
					// we need to project the bits we have on the fixpnt
					for (unsigned i = 0; i < ieee754_parameter<Arith>::fbits + 1; ++i) {
						if (fraction & 0x01) {
							f.setbit(i + shiftLeft);
						}
						fraction >>= 1;
					}
					if (s) f.twosComplement();
				}
			}
			if constexpr (arithmetic == Saturate) {
				if (!s && f.sign()) f.maxpos();  // a positive value rounded up into the sign bit
			}
		}
		return f;
	}

	// conversion functions

	// Integer casts follow the library's integer-cast rule (shared/integer_cast.hpp, #2104) in both
	// arithmetic modes (#2111): the value truncates toward zero, and past the range it saturates to
	// +/-max(), an unsigned type to [0, max() - 1]. fixpnt has no NaN. Before #2111 a signed cast
	// floored ((int)-1.5 was -2) and an unsigned cast returned the raw encoding, unscaled.
	template<typename NativeInt>
	constexpr NativeInt to_integer() const noexcept {
		blockbinary<nbits + 1, bt, BinaryNumberType::Signed> m(_block);   // room for |maxneg|
		const bool negative = m.sign();
		if (negative) m.twosComplement();
		std::uint64_t magnitude = 0;
		bool overflow = false;
		for (unsigned i = rbits; i <= nbits; ++i) {
			if (!m.test(i)) continue;
			const unsigned k = i - rbits;
			if (k >= 64) overflow = true; else magnitude |= (std::uint64_t(1) << k);
		}
		return integer_from_magnitude<NativeInt>(negative, overflow, magnitude);
	}

	// from fixed-point to native signed integer
	template<typename NativeInt>
	constexpr typename std::enable_if< std::is_integral_v<NativeInt> && std::is_signed_v<NativeInt>,
		NativeInt>::type to_signed() const {
		return to_integer<NativeInt>();
	}

	// from fixed-point to native unsigned integer
	template<typename NativeInt>
	constexpr typename std::enable_if< std::is_unsigned_v<NativeInt>,
		NativeInt>::type to_unsigned() const {
		return to_integer<NativeInt>();
	}

	// to a native floating-point type, rounded once to nearest even (#2111: it added the bits one
	// at a time in TargetFloat, which rounded at every step once nbits passed its precision). The
	// magnitude's bits down to the target's lsb at this scale -- digits bits, fewer in its subnormal
	// range -- are rounded from the guard and sticky bits below, then scaled by a power of 2.
	template<typename TargetFloat>
	constexpr TargetFloat to_native() const {
		using L = std::numeric_limits<TargetFloat>;
		blockbinary<nbits + 1, bt, BinaryNumberType::Signed> m(_block);   // room for |maxneg|
		const bool negative = m.sign();
		if (negative) m.twosComplement();
		int msb = -1;
		for (int i = static_cast<int>(nbits); i >= 0; --i) {
			if (m.test(static_cast<unsigned>(i))) { msb = i; break; }
		}
		if (msb < 0) return TargetFloat(0);
		const int e = msb - static_cast<int>(rbits);                    // the value is in [2^e, 2^(e+1))
		int r = e - (L::digits - 1);                                     // the target's lsb at this scale
		if (r < L::min_exponent - L::digits) r = L::min_exponent - L::digits;   // its subnormal range
		int k = r + static_cast<int>(rbits);                             // that lsb as a fixpnt bit index
		if (k < 0) { k = 0; r = -static_cast<int>(rbits); }              // every bit fits: exact
		TargetFloat significand(0);                                      // at most digits bits: exact
		for (int i = msb; i >= k; --i) {
			significand = significand * TargetFloat(2) + (m.test(static_cast<unsigned>(i)) ? TargetFloat(1) : TargetFloat(0));
		}
		if (k > 0) {
			const bool guard  = m.test(static_cast<unsigned>(k - 1));
			const bool sticky = (k >= 2) && m.any(static_cast<unsigned>(k - 2));
			const bool odd    = m.test(static_cast<unsigned>(k));
			if (guard && (sticky || odd)) significand += TargetFloat(1);   // 2^digits at most: exact
		}
		TargetFloat value(0);
		if (std::is_constant_evaluated()) value = significand * exact_pow2<TargetFloat>(r);
		else value = std::ldexp(significand, r);
		return negative ? -value : value;
	}

private:
	blockbinary<nbits, bt, BinaryNumberType::Signed> _block;

	// convert
	template<unsigned nnbits, unsigned rrbits, bool aarithmetic, typename Bbt>
	friend std::string convert_to_decimal_string(const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& value);

	// fixpnt - fixpnt logic comparisons
	template<unsigned nnbits, unsigned rrbits, bool aarithmetic, typename Bbt>
	friend constexpr bool operator==(const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& lhs, const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& rhs);
	template<unsigned nnbits, unsigned rrbits, bool aarithmetic, typename Bbt>
	friend constexpr bool operator< (const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& lhs, const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& rhs);
};


/// Magnitude of a fixpnt. Expensive as fixpnts are encoded as 2's complement, so we need to test, copy, complement, and return.
/// |maxneg| is not representable: Saturate gives maxpos, and Modulo wraps it to maxneg (#2111).
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr fixpnt<nbits, rbits, arithmetic, bt> abs(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	fixpnt<nbits, rbits, arithmetic, bt> tmp(v);
	if (v.isneg()) {
		tmp.twosComplement();
		if constexpr (arithmetic == Saturate) {
			if (tmp.isneg()) tmp.maxpos();   // v was maxneg
		}
	}
	return tmp;
}

////////////////////////    FIXED-POINT functions   /////////////////////////////////

////////////////////////    FIXED-POINT operators   /////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary logic operators

// equal: precondition is that the block-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (lhs._block == rhs._block);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (lhs._block < rhs._block);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - integer comparisons compare the values exactly (#2111): both go into a fixpnt wide
// enough to hold either. Converting the integer to the narrower fixpnt first wrapped it in Modulo,
// so fixpnt<8,4,Modulo>(-8) == 8 was true. Returns -1, 0 or 1 as f is below, at or above v.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt, typename Int>
constexpr int fixpnt_compare_integer(const fixpnt<nbits, rbits, arithmetic, bt>& f, Int v) {
	static_assert(std::is_integral_v<Int>, "fixpnt_compare_integer: an integer type");
	constexpr unsigned ibits = 8u * static_cast<unsigned>(sizeof(Int)) + 1u;   // an unsigned value and its sign
	if constexpr (nbits >= rbits + ibits) {
		const fixpnt<nbits, rbits, arithmetic, bt> b(v);   // every Int fits
		return (f < b) ? -1 : ((b < f) ? 1 : 0);
	}
	else {
		using Wide = fixpnt<rbits + ibits, rbits, arithmetic, bt>;
		Wide a;
		a = f;           // widening with the same radix point: exact
		const Wide b(v); // fits
		return (a < b) ? -1 : ((b < a) ? 1 : 0);
	}
}

// fixpnt - literal binary logic operators
// remember that literals are const types, so you need to refer to a literal like 0 as a const int

// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0) || (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0) || (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0) || (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0) || (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0) || (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return (fixpnt_compare_integer(lhs, rhs) < 0) || (fixpnt_compare_integer(lhs, rhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return !(fixpnt_compare_integer(lhs, rhs) < 0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0) || (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0) || (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0) || (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0) || (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0) || (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) < 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (fixpnt_compare_integer(rhs, lhs) > 0) || (fixpnt_compare_integer(rhs, lhs) == 0);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !(fixpnt_compare_integer(rhs, lhs) > 0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal float/double binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
 
#if LONG_DOUBLE_SUPPORT
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal float/double - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

#if LONG_DOUBLE_SUPPORT
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator==(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator!=(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator< (long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator> (long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator<=(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline bool operator>=(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> ratio = lhs;
	ratio %= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary arithmetic operators

///////////////////// int

// BINARY left shift
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator<<(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> tmp = lhs;
	return tmp <<= rhs;
}
// BINARY right shift
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator>>(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> tmp = lhs;
	return tmp >>= rhs;
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////////  long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////  long long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

///////////////////// unsigned int
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////////  unsigned long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////  unsigned long long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// // BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// // BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal float/double/long double binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal double - fixpnt binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal double - fixpnt binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal double - fixpnt binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator+(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator-(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator*(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator/(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
constexpr inline fixpnt<nbits, rbits, arithmetic, bt> operator%(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

///////////////////////////// IOSTREAM operators ///////////////////////////////////////////////


}} // namespace sw::universal
