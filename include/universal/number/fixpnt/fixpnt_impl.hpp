#pragma once
// fixpnt_impl.hpp: implementation of an arbitrary configuration binary fixed-point number parameterized in total bits and radix bits
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
#include <cassert>

// supporting types and functions
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

You need the exception types defined, but you have the option to throw them
*/
#include <universal/number/fixpnt/exceptions.hpp> 

// composition types used by fixpnt
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/support/decimal.hpp>
#ifdef FIXPNT_SCALE_TRACKING
#include <universal/utility/scale_tracker.hpp>
#endif

namespace sw { namespace universal {

constexpr bool Modulo    = true;
constexpr bool Saturate = !Modulo;

// fixpntdiv_t for fixpnt<nbits,rbits> to capture quotient and remainder during long division
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
struct fixpntdiv_t {
	fixpnt<nbits, rbits, arithmetic, bt> quot; // quotient
	fixpnt<nbits, rbits, arithmetic, bt> rem;  // remainder
};

// free function generator to create a 1's complement copy of a fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> onesComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	fixpnt<nbits, rbits, arithmetic, bt> ones(value);
	return ones.flip();
}
// free function generator to create the 2's complement of a fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> twosComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	fixpnt<nbits, rbits, arithmetic, bt> twos(value);
	return twos.twosComplement();;
}

// The free function scale calculates the power of 2 exponent that would capture an approximation of a normalized real value
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline int scale(const fixpnt<nbits, rbits, arithmetic, bt>& i) {
	fixpnt<nbits,rbits,arithmetic,bt> v(i);
	if (i.sign()) { // special case handling
		v = twosComplement(v);
		if (v == i) {  // special case of 10000..... largest negative number in 2's complement encoding
			if constexpr (nbits == rbits) {
				return 0;
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
	fixpnt& operator=(fixpnt&&) noexcept = default;

	// decorated/converting constructors

	/// <summary>
	/// construct a new fixpnt from another, sign extend or round when necessary: 
	/// src and tgt fixpnt need to have the same arithmetic and blocktype
	/// </summary>
	/// <param name="a">source fixpnt</param>
	template<unsigned src_nbits, unsigned src_rbits>
	fixpnt(const fixpnt<src_nbits, src_rbits, arithmetic, bt>& a) noexcept { *this = a; }

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

	// fixpnt size adapter
	template<unsigned src_nbits, unsigned src_rbits>
	fixpnt& operator=(const fixpnt<src_nbits, src_rbits, arithmetic, bt>& a) noexcept {
		// std::cout << typeid(a).name() << " goes into " << typeid(*this).name() << std::endl;
		//		static_assert(src_nbits > nbits, "Source fixpnt is bigger than target: potential loss of precision"); 
		// TODO: do we want prohibit this condition? To be consistent with native types we need to round automatically.
		if constexpr (src_nbits <= nbits) {
			_block = a.bits();
			if constexpr (src_nbits < nbits) {
				if (a.sign()) { // sign extend if necessary
					for (unsigned i = src_nbits; i < nbits; ++i) setbit(i);
				}
			}
#ifdef TODO
			// round: <src_nbits, src_rbits> -> <nbits, rbits>
			// we round on the difference between (src_rbits - rbits) fraction bits
			// and modulo arithmetic, lop of the high order integer bits
			if constexpr (src_rbits > rbits) {
				auto rawbb = a.bits();
				bool roundUp = rawbb.roundingMode(src_rbits - rbits);
				rawbb >>= src_rbits - rbits;
				if (roundUp) ++rawbb;
				_block = rawbb;
			}
#endif
		}
		else {
			// round: <src_nbits, src_rbits> -> <nbits, rbits>
			// we round on the difference between (src_rbits - rbits) fraction bits
			// and modulo arithmetic, lop of the high order integer bits
			if constexpr (src_rbits > rbits) {
				auto rawbb = a.bits();
				bool roundUp = rawbb.roundingMode(src_rbits - rbits);
				rawbb >>= src_rbits - rbits;
				if (roundUp) ++rawbb;
				_block = rawbb;
			}
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
	// we want to make this a constexpr so that we could create arbitrary constants, but Clang
	// doesn't have full STL constexpr support yet, so if failing this.
	fixpnt& assign(const std::string& number) {
		clear();

		// minimum size for a decimal representation is integer + '.' fraction, so at least 3 characters
		// minimum size for a binary representation is "0b" + intbits + '.' + fracbits, so at least 5 characters
		if (number.size() < 3) return *this;
		bool binaryFormat = false;
		if (number[0] == '0' && number[1] == 'b') binaryFormat = true;
//		std::regex binary_regex("0b([01]+)?(.)?([01]+)?$"); // bin does not have a negative size, just raw bits
//		std::regex decimal_regex("/^-?(0|[1-9][0-9]*)?"); // ("/^-?(0|[1-9][0-9]*)?(\.[0-9]+)?(?<=[0-9])(e-?(0|[1-9][0-9]*))?$");
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
//		if (std::regex_match(number, binary_regex)) {
		if (binaryFormat) {
//			std::cout << "found an binary representation\n";
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
						std::cerr << "radix in string value is not aligned with fixpnt format\n";
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
			std::cout << "found a decimal representation: TBD\n";
			int64_t scale = 1;
			int64_t fraction = 0;
			for (std::string::const_reverse_iterator r = number.rbegin();
				r != number.rend();
				++r) {
				if (*r != '.') {
					int64_t digit = charLookup.at(*r);
					fraction += scale * digit;
					scale *= 10;
				}
				else {
					// construct fraction
					*this = 0;
				}
			}
			// TODO: implement decimal string parse for fixpnt
			if (fraction < 0) std::cout << "found a negative decimal representation\n"; // TODO: remove when implemented properly
			*this = 6.90234375;
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
	fixpnt& operator+=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
			_block += rhs._block;
		}
		else {
			using biggerbb = blockbinary<nbits + 1, bt>;
			biggerbb c = uradd(_block, rhs._block);  // c = a + b
			fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
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
	fixpnt& operator-=(const fixpnt& rhs) {
		if constexpr (arithmetic == Modulo) {
			operator+=(sw::universal::twosComplement(rhs));
		}
		else {
			using biggerbb = blockbinary<nbits + 1, bt>;
			biggerbb c = ursub(_block, rhs.bits());  // c = a - b
			fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
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
	fixpnt& operator*=(const fixpnt& rhs) {
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
			fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg); // TODO: can these be static?
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
	fixpnt& operator/=(const fixpnt& rhs) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) throw fixpnt_divide_by_zero();
#else
		if (rhs.iszero()) std::cerr << "fixpnt_divide_by_zero" << std::endl;
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
			blockbinary<accumulatorSize, bt> quotient = dividend / divisor;

//			std::cout << "dividend : " << to_binary(dividend, true) << " : " << dividend << '\n';
//			std::cout << "divisor  : " << to_binary(divisor, true) << " : " << divisor << '\n';
//			std::cout << "quotient : " << to_binary(quotient, true) << " : " << quotient << '\n';

			bool roundUp = quotient.roundingMode(roundingBits);
			quotient >>= roundingBits;
			if (roundUp) ++quotient;
//			std::cout << "quotient : " << to_binary(quotient, true) << " : " << quotient << (roundUp ? " rounded up": " truncated") << '\n';
			_block = (positive ? quotient : quotient.twosComplement());
		}
		else {
			std::cerr << "TBD: Saturate divide not implemented yet\n";
		}
		return *this;
	}
	fixpnt& operator%=(const fixpnt& rhs) {
		_block %= rhs._block;
		return *this;
	}
	fixpnt& operator<<=(const signed shift) {
		_block <<= shift;
		return *this;
	}
	fixpnt& operator>>=(const signed shift) {
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
		// a compile time assert has been added to enforce.
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
	constexpr fixpnt  integer()               const noexcept { return floor(*this); }
	constexpr fixpnt  fraction()              const noexcept { return (*this - integer()); }
	constexpr bool    iszero()                const noexcept { return _block.iszero(); }
	constexpr bool    ispos()                 const noexcept { return _block.ispos(); }
	constexpr bool    isneg()                 const noexcept { return _block.isneg(); }
	constexpr bool    isnan()                 const noexcept { return false; }
	constexpr bool    isinf()                 const noexcept { return false; }
	constexpr bool    at(unsigned bitIndex)   const noexcept { return _block.at(bitIndex); }
	constexpr bool    test(unsigned bitIndex) const noexcept { return _block.test(bitIndex); }
	constexpr uint8_t nibble(unsigned n)      const noexcept { return _block.nibble(n); }

	// collect a copy of the underlying bit representation
	blockbinary<nbits, bt, BinaryNumberType::Signed> bits() const noexcept { return _block; }

protected:
	// HELPER methods
	// 
	// conversion helpers

	// convert arithmetic types into a fixpnt
	template<typename Arith>
	static constexpr fixpnt convert(Arith v) {
		static_assert(std::is_arithmetic_v<Arith>);
		fixpnt f;
		f.clear();
		if constexpr (std::is_integral_v<Arith> && std::is_signed_v<Arith>) {
			if (0 == v) return f;
			if constexpr (arithmetic == Saturate) {
				constexpr fixpnt maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
				// check if we are in the representable range
				if (v >= static_cast<Arith>(maxpos)) { return maxpos; }
				if (v <= static_cast<Arith>(maxneg)) { return maxneg; }
			}
			constexpr unsigned sizeofInteger = 8 * sizeof(v);
			if (v == -v) {
				// v is at maxneg 0x10...000
				if constexpr (sizeofInteger <= (nbits - rbits)) {
					f.setbit(sizeofInteger + rbits - 1);
				}
			}
			else {
				bool negative = (v < 0 ? true : false);
				v = (v < 0 ? -v : v);
				unsigned upper = (sizeofInteger < (nbits - rbits)) ? sizeofInteger : (nbits - rbits);
				for (unsigned i = 0; i < upper; ++i) {
					if (v & 0x1) f.setbit(i + rbits);
					v >>= 1;
				}
				if (negative) f.twosComplement();
			}
		}
		else if constexpr (std::is_unsigned_v<Arith>) {
			if (0 == v) return f;
			if constexpr (arithmetic == Saturate) {
				constexpr fixpnt<nbits, rbits, arithmetic, bt> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
				// check if we are in the representable range
				if (v >= static_cast<Arith>(maxpos)) { return maxpos; }
				if (v <= static_cast<Arith>(maxneg)) { return maxneg; }
			}
			constexpr uint64_t mask = 0x1;
			unsigned upper = (nbits - rbits) <= 64 ? nbits : 64;
			for (unsigned i = 0; i < upper - rbits && v > 0; ++i) {
				if (v & mask) f.setbit(i + rbits); // we have no fractional part in v
				v >>= 1;
			}
		}
		else if constexpr (std::is_floating_point_v<Arith>) {
			if (v == 0.0) return f;
			if constexpr (arithmetic == Saturate) {	// check if the value is in the representable range
				fixpnt a;
				a.maxpos();
				if (v >= float(a)) { return a; } // set to max pos value
				a.maxneg();
				if (v <= float(a)) { return a; } // set to max neg value
			}

			bool s{ false };
			uint64_t unbiasedExponent{ 0 };
			uint64_t fraction{ 0 };
			uint64_t bits{ 0 };
			extractFields(v, s, unbiasedExponent, fraction, bits); // use native conversion
			if (unbiasedExponent > 0) fraction |= (1ull << ieee754_parameter<Arith>::fbits);
			int biasedExponent = static_cast<int>(unbiasedExponent) - ieee754_parameter<Arith>::bias;
			int radixPoint = ieee754_parameter<Arith>::fbits - biasedExponent;

			// our fixed-point has its radixPoint at rbits
			int shiftRight = std::min(radixPoint - int(rbits), 64);
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

				fraction >>= shiftRight;  // shift out the bits we are rounding away
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
				fraction = (s ? (~fraction + 1) : fraction); // if negative, map to two's complement
				f.setbits(fraction);
			}
			else {
				int shiftLeft = -shiftRight;
				if (shiftLeft < (64 - ieee754_parameter<Arith>::fbits)) {  // what is the distance between the MSB and 64?
					// no need to round, just shift the bits in place
					fraction <<= shiftLeft;
					fraction = (s ? (~fraction + 1) : fraction); // if negative, map to two's complement
					f.setbits(fraction);
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
		}
		return f;
	}

	// conversion functions

	// from fixed-point to native signed integer
	template<typename NativeInt>
	typename std::enable_if< std::is_integral_v<NativeInt> && std::is_signed_v<NativeInt>,
		NativeInt>::type to_signed() const {
		if constexpr (nbits <= rbits) return 0;
		constexpr unsigned sizeOfInteger = 8 * sizeof(NativeInt);
		NativeInt ll = 0;
		NativeInt mask = 1;
		unsigned upper = (nbits - rbits) > 64 ? (rbits + 64) : nbits;
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
	typename std::enable_if< std::is_unsigned_v<NativeInt>,
		NativeInt>::type to_unsigned() const {
		return NativeInt(_block.to_long_long());
	}

	template<typename TargetFloat>
	TargetFloat to_native() const {
		// pick up the absolute value of the minimum normal and subnormal exponents 
		constexpr unsigned minNormalExponent = static_cast<unsigned>(-ieee754_parameter<TargetFloat > ::minNormalExp);
		constexpr unsigned minSubnormalExponent = static_cast<unsigned>(-ieee754_parameter<TargetFloat>::minSubnormalExp);
		static_assert(rbits <= minSubnormalExponent, "to_native: fixpnt fraction is too small to represent with requested floating-point type");
		TargetFloat multiplier = 0;
		if constexpr (rbits > minNormalExponent) { // value is a subnormal number
			multiplier = ieee754_parameter<TargetFloat>::minSubnormal;
			for (unsigned i = 0; i < minSubnormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		else {
			// the value is a normal number
			multiplier = ieee754_parameter<TargetFloat>::minNormal;
			for (unsigned i = 0; i < minNormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		// you pop out here with multiplier set to the weight of the starting bit
		fixpnt<nbits, rbits, arithmetic, bt> raw = (sign() ? sw::universal::twosComplement(*this) : *this);
		// construct the value
		TargetFloat value{ 0.0 };
		for (unsigned i = 0; i < nbits; ++i) {
			if (raw.at(i)) value += multiplier;
			multiplier *= 2.0; // these are error free multiplies
		}
		return (sign() ? -value : value);
	}

private:
	blockbinary<nbits, bt, BinaryNumberType::Signed> _block;

	// convert
	template<unsigned nnbits, unsigned rrbits, bool aarithmetic, typename Bbt>
	friend std::string convert_to_decimal_string(const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& value);

	// fixpnt - fixpnt logic comparisons
	template<unsigned nnbits, unsigned rrbits, bool aarithmetic, typename Bbt>
	friend bool operator==(const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& lhs, const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& rhs);
	template<unsigned nnbits, unsigned rrbits, bool aarithmetic, typename Bbt>
	friend bool operator< (const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& lhs, const fixpnt<nnbits, rrbits, aarithmetic, Bbt>& rhs);
};


/// Magnitude of a fixpnt. Expensive as fixpnts are encoded as 2's complement, so we need to test, copy, complement, and return.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> abs(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	fixpnt<nbits, rbits, arithmetic, bt> tmp(v);
	if (v.isneg()) tmp.twosComplement();
	return tmp;
}

////////////////////////    FIXED-POINT functions   /////////////////////////////////

////////////////////////    FIXED-POINT operators   /////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary logic operators

// equal: precondition is that the block-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (lhs._block == rhs._block);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return (lhs._block < rhs._block);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary logic operators
// remember that literals are const types, so you need to refer to a literal like 0 as a const int

// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal float/double binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
 
#if LONG_DOUBLE_SUPPORT
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return !operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator<(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs)) || operator==(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return !operator< (lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal float/double - fixpnt binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

#if LONG_DOUBLE_SUPPORT
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator==(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator!=(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator< (long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator<(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator> (long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (rhs, fixpnt<nbits, rbits, arithmetic, bt>(lhs));
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator<=(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs) || operator==(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline bool operator>=(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return !operator< (fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - fixpnt binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> ratio = lhs;
	ratio %= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal binary arithmetic operators

///////////////////// int

// BINARY left shift
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator<<(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> tmp = lhs;
	return tmp <<= rhs;
}
// BINARY right shift
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator>>(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	fixpnt<nbits, rbits, arithmetic, bt> tmp = lhs;
	return tmp >>= rhs;
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, int rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////////  long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////  long long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

///////////////////// unsigned int
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned int rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////////  unsigned long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
/////////////  unsigned long long
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, unsigned long long rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - fixpnt binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// // BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(unsigned int lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// // BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(unsigned long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(unsigned long long lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// fixpnt - literal float/double/long double binary arithmetic operators

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, float rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, double rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator+(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator-(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator*(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator/(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(const fixpnt<nbits, rbits, arithmetic, bt>& lhs, long double rhs) {
	return operator%(lhs, fixpnt<nbits, rbits, arithmetic, bt>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal double - fixpnt binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(float lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal double - fixpnt binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal double - fixpnt binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator+(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator+(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator-(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator-(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator*(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator*(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator/(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator/(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}
// BINARY REMAINDER
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> operator%(long double lhs, const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	return operator%(fixpnt<nbits, rbits, arithmetic, bt>(lhs), rhs);
}

///////////////////////////// IOSTREAM operators ///////////////////////////////////////////////

// convert fixpnt to decimal string, i.e. "-1234.5678"
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
std::string convert_to_decimal_string(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	std::stringstream str;
	if (value.iszero()) {
		str << '0';
		if constexpr (rbits > 0) {
			str << '.';
			for (unsigned i = 0; i < rbits; ++i) {
				str << '0';
			}
		}
		return str.str();
	}
	if (value.sign()) str << '-';
	support::decimal partial, multiplier;
	fixpnt<nbits, rbits, arithmetic, bt> number;
	number = value.sign() ? sw::universal::twosComplement(value) : value;
	if constexpr (nbits > rbits) {
		// convert the fixed point: start by handling the integer part
		multiplier.setdigit(1);
		// convert fixpnt to decimal by adding and doubling multipliers
		for (unsigned i = rbits; i < nbits; ++i) {
			if (number.at(i)) {
				support::add(partial, multiplier);
			}
			support::add(multiplier, multiplier);
		}
		for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
			str << (int)*rit;
		}
	}
	else {
		str << '0';
	}

	if constexpr (rbits > 0) {
		str << ".";
		// and secondly, the fraction part
		support::decimal range, discretizationLevels, step;
		// create the decimal range we are discretizing
		range.setdigit(1);
		range.shiftLeft(rbits);
		// calculate the discretization levels of this range
		discretizationLevels.setdigit(1);
		for (unsigned i = 0; i < rbits; ++i) {
			support::add(discretizationLevels, discretizationLevels);
		}
		step = div(range, discretizationLevels);
		// now construct the value of this range by adding the fraction samples
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
		unsigned nrLeadingZeros = static_cast<unsigned>(range.size() - partial.size() - 1);
		for (unsigned i = 0; i < nrLeadingZeros; ++i) str << '0';
		unsigned digitsWritten = nrLeadingZeros;
		for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
			str << (int)*rit;
			++digitsWritten;
		}
		if (digitsWritten < rbits) { // deal with trailing 0s
			for (unsigned i = digitsWritten; i < rbits; ++i) {
				str << '0';
			}
		}
	}
	return str.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
/// stream operators

// ostream output generates an ASCII format for the fixed-point argument
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
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
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
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
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string to_binary(const fixpnt<nbits, rbits, arithmetic, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	if constexpr (nbits > rbits) {
		for (int i = static_cast<int>(nbits) - 1; i >= static_cast<int>(rbits); --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && (i - rbits) > 0 && (i - rbits) % 4 == 0) s << '\'';
		}
	}
	else {
		s << '0';
	}
	s << '.';
	if constexpr (rbits > 0) {
		for (int i = int(rbits) - 1; i >= 0; --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && (rbits - i) % 4 == 0 && i != 0) s << '\'';
		}
	}
	return s.str();
}

// to_triple generates a triple (sign,scale,fraction) representation of the fixed-point number
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string to_triple(const fixpnt<nbits, rbits, arithmetic, bt>& number) {
	std::stringstream ss;
	ss << (number.sign() ? "(-," : "(+,");
	ss << scale(number) << ',';
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		ss << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
	}
	ss << (rbits == 0 ? "~)" : ")");
	return ss.str();
}

}} // namespace sw::universal
