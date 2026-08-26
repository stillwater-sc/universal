#pragma once
// blocktriple.hpp: definition of a (sign, scale, significand) representation of a generic floating-point value that goes into an arithmetic operation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <cstdio>
#include <cmath>
#include <string>
#include <ios>      // std::streamsize in the to_string declaration
#include <limits>
#include <algorithm>
#include <vector>

// dependent types for stand-alone use of this class
#include <universal/internal/blocktriple/blocktriple_fwd.hpp>
#include <universal/native/ieee754_core.hpp>   // bit manipulation only; the text layer is not needed here (#1334)
#include <universal/native/subnormal.hpp>
#include <universal/utility/find_msb.hpp>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
// blocktriple operation trace options
#include <universal/internal/blocktriple/trace_constants.hpp>

namespace sw { namespace universal {

/*
	The blocktriple is used as a marshalling class to transform
floating-point type number systems into a uniform floating-point
arithmetic class that we can validate and reuse.

The blocktriple design favors performance over encapsulation.
During arithmetic operations, the fraction bits of the arguments
need to be manipulated and extended, and we wanted to avoid
copying these fraction bits into new storage classes.

However, the size of the fraction bit buffers depends on the
arithmetic operator. This implies that at the time of creation
we need to know the intended use, and configure the blocktriple
accordingly.

The different arithmetic operations require different bit widths 
to guarantee correct results. In the blocktriple design we use
the BlockTripleOperator tag to configure the bit width of the
blocksignificand to create the proper size and layout.
  The blocksignificand structures are organized as follows:
    ADD        iii.ffffrrrrrrrrr          3 integer bits, f fraction bits, and 2*fhbits rounding bits
    MUL         ii.ffff'ffff              2 integer bits, 2*f fraction bits
    DIV         ii.ffff'ffff'ffff'rrrr    2 integer bits, 3*f fraction bits, and r rounding bits
TBD SQRT      iiii.ffff'ffff'ffff         4 integer bits, 2*f fraction bits

*/

 // the operator tag and the class declaration (with its default template arguments)
 // live in blocktriple_fwd.hpp, so a header that only names blocktriple can include
 // that instead of these ~70,000 lines (#1334).
// to_binary and to_triple are defined in internal/blocktriple/manipulators.hpp.
// Their DEFAULT ARGUMENTS are established here, at namespace scope, and not on
// the definitions: the in-class friend declarations below cannot carry defaults
// (a friend declaration may only do so when it is also a definition), and once
// the friend declaration is the first one clang sees, adding defaults later is
// an error -- "default arguments cannot be added". gcc accepts it; clang does
// not, which is why this only appeared in CI. See #1334.
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string to_binary(const blocktriple<fbits, op, bt>& a, bool nibbleMarker = false);
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string to_triple(const blocktriple<fbits, op, bt>& a, bool nibbleMarker = true);
template<unsigned fbits, BlockTripleOperator op, typename bt> blocktriple<fbits, op, bt> abs(const blocktriple<fbits, op, bt>& v);

template<unsigned fbits, BlockTripleOperator op, typename bt>
blocktriple<fbits, op, bt>& convert(unsigned long long uint, blocktriple<fbits, op, bt>& tgt) {
	return tgt;
}


/// <summary>
/// Generalized blocktriple representing a (sign, scale, significand) with unrounded arithmetic
/// 
/// For addition and subtraction, blocktriple uses a 2's complement representation of the form iii.fffff.
/// The 3 integer bits are required to capture the negative overflow condition.
/// 
/// For multiplication, blocktriple uses a 1's complement representation of the form ii.fffff.
/// The 2 integer bits are required to capture the overflow condition.
/// 
/// Blocktriple does not normalize the output of ADD/SUB/MUL so that all bits are
/// available for the rounding decision. Number systems that use blocktriple as
/// their general floating-point engine can use the roundUp(targetFbits) method to
/// obtain the rounding direction, and the alignmentShift(targetFbits) method to 
/// obtain the shift required to normalize the fraction bits.
/// </summary>
/// <typeparam name="fbits">number of fraction bits in the significand</typeparam>
/// <typeparam name="bt">block type: one of [uint8_t, uint16_t, uint32_t, uint64_t]</typeparam>
template<unsigned _fbits, BlockTripleOperator _op, typename bt>
class blocktriple {
public:
	static constexpr unsigned fbits = _fbits;  // a convenience and consistency alias
	static constexpr unsigned nbits = fbits;
	typedef bt BlockType;
	static constexpr BlockTripleOperator op = _op;

	static constexpr unsigned bitsInByte = 8ull;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks = 1ull + ((fbits - 1ull) / bitsInBlock);
	static constexpr uint64_t storageMask = (0xFFFF'FFFF'FFFF'FFFFull >> (64ull - bitsInBlock));

	static constexpr unsigned MSU = nrBlocks - 1ull; // MSU == Most significand Unit, as MSB is already taken

	static constexpr unsigned fhbits   = fbits + 1;          // size of all bits
	static constexpr unsigned rbits    = 3;                  // rounding bits assumes you have sticky bit consolidation in normalize, otherwise you need 2 * (fbits + 1) to capture the tie breaking ULPs
	static constexpr unsigned abits    = fbits + rbits;      // size of the addend = fbits plus an additional rbits to capture required rounding bits
	static constexpr unsigned mbits    = 2 * fbits;          // size of the fraction bits of the multiplier
	static constexpr unsigned divbits  = 3 * fbits + 4;      // size of the fraction bits of the divider
	static constexpr unsigned divshift = divbits - fbits;    // alignment shift for divider operands
	static constexpr unsigned sqrtbits = 2 * fhbits;      // size of the square root output
	// we transform input operands into the operation's target output size
	// so that everything is aligned correctly before the operation starts.
	static constexpr unsigned bfbits =
		(op == BlockTripleOperator::ADD ? (3 + abits) :           // we need 3 integer bits (bits left of the radix point) to capture 2's complement and overflow
			(op == BlockTripleOperator::MUL ? (2 + mbits) :       // we need 2 integer bits to capture overflow: multiply happens in 1's complement
				(op == BlockTripleOperator::DIV ? (2 + divbits) : // we need 2 integer bits to capture overflow: divide happens in 1's complement
					(op == BlockTripleOperator::SQRT ? sqrtbits : fhbits+1))));  // REPRESENTATION is the fall through condition and adds a bit to accomodate 2's complement encodings
	// radix point of the OUTPUT of an operator
	static constexpr int radix =
		(op == BlockTripleOperator::ADD ? static_cast<int>(abits) :
			(op == BlockTripleOperator::MUL ? static_cast<int>(mbits) :
				(op == BlockTripleOperator::DIV ? static_cast<int>(divbits) :
					(op == BlockTripleOperator::SQRT ? static_cast<int>(sqrtbits) : static_cast<int>(fbits)))));  // REPRESENTATION is the fall through condition
//	static constexpr BitEncoding encoding =
//		(op == BlockTripleOperator::ADD ? BitEncoding::Twos :
//			(op == BlockTripleOperator::MUL ? BitEncoding::Ones :
//				(op == BlockTripleOperator::DIV ? BitEncoding::Ones :
//					(op == BlockTripleOperator::SQRT ? BitEncoding::Ones : BitEncoding::Ones))));
	static constexpr unsigned normalBits = (bfbits < 64 ? bfbits : 64);
	static constexpr uint64_t normalFormMask = (normalBits == 64) ? 0xFFFF'FFFF'FFFF'FFFFull : (~(0xFFFF'FFFF'FFFF'FFFFull << (normalBits - 1)));
	// to maximize performance, can we make the default blocktype a uint64_t?
	// storage unit for block arithmetic needs to be uin32_t until we can figure out 
	// how to manage carry propagation on uint64_t using intrinsics/assembly code
	using significand_t = sw::universal::blocksignificand<bfbits, bt>;

	static constexpr bt ALL_ONES = bt(~0);
	// generate the special case overflow pattern mask when representation is fbits + 1 < 64
	static constexpr unsigned maxbits = (fbits + 1) < 63 ? (fbits + 1) : 63;
	static constexpr uint64_t overflowPattern = (maxbits < 63) ? (1ull << maxbits) : 0ull; // overflow of 1.11111 to 10.0000

	constexpr blocktriple(const blocktriple&) noexcept = default;
	constexpr blocktriple(blocktriple&&) noexcept = default;

	constexpr blocktriple& operator=(const blocktriple&) noexcept = default;
	constexpr blocktriple& operator=(blocktriple&&) noexcept = default;

	constexpr blocktriple() noexcept : 
		_nan{ false }, 	_inf{ false }, _zero{ true }, 
		_sign{ false }, _scale{ 0 } {} // _significand uses default constructor and static constexpr radix computation

	// decorated constructors
	constexpr blocktriple(signed char iv)        noexcept { *this = iv; }
	constexpr blocktriple(short iv)              noexcept { *this = iv; }
	constexpr blocktriple(int iv)                noexcept { *this = iv; }
	constexpr blocktriple(long iv)               noexcept { *this = iv; }
	constexpr blocktriple(long long iv)          noexcept { *this = iv; }
	constexpr blocktriple(char iv)               noexcept { *this = iv; }
	constexpr blocktriple(unsigned short iv)     noexcept { *this = iv; }
	constexpr blocktriple(unsigned int iv)       noexcept { *this = iv; }
	constexpr blocktriple(unsigned long iv)      noexcept { *this = iv; }
	constexpr blocktriple(unsigned long long iv) noexcept { *this = iv; }
	constexpr blocktriple(float iv)              noexcept { *this = iv; }
	constexpr blocktriple(double iv)             noexcept { *this = iv; }


	// conversion operators
	constexpr blocktriple& operator=(signed char rhs)        noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(short rhs)              noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(int rhs)                noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(long rhs)               noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(long long rhs)          noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(char rhs)               noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned short rhs)     noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned int rhs)       noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned long rhs)      noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned long long rhs) noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr blocktriple& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// type conversion
	CONSTEXPRESSION blocktriple& assign(const std::string& bitPattern) noexcept {
		clear();
		unsigned nrChars = bitPattern.size();
		std::string bits;
		if (nrChars > 2 && bitPattern[0] == '0' && bitPattern[1] == 'b') {
			for (unsigned i = 2; i < nrChars; ++i) {
				char c = bitPattern[i];
				switch (c) {
				case '0':
				case '1':
					bits += c;
					break;
				case '\'':
					// simply ignore this delimiting character
					break;
				default:
					std::fprintf(stderr, "bitPattern contained a non-standard character: %c\n", c);
					return *this;
				}
			}
		}
		else {
			std::fprintf(stderr, "bitPattern must start with 0b: instead input pattern was %s\n", bitPattern.c_str());
			return *this;
		}

		unsigned nrBits = bits.size();
		if (nrBits != bfbits) {
			std::fprintf(stderr, "nr of bits in bitPattern is %u and needs to be %u\n", nrBits, unsigned(bfbits));
			return *this;
		}
		// assign the bits
		unsigned bit = nrBits - 1;
		for (unsigned i = 0; i < bits.size(); ++i) {
			char c = bits[i];
			setbit(bit - i, c == '1');
		}
		return *this;
	}

	// explicit conversion operators
	explicit operator float()                            const noexcept { return to_native<float>(); }
	explicit operator double()                           const noexcept { return to_native<double>(); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	explicit operator long double()                      const noexcept { return to_native<long double>(); }
	BIT_CAST_CONSTEXPR blocktriple(long double iv)		       noexcept { *this = iv; }
	BIT_CAST_CONSTEXPR blocktriple& operator=(long double rhs) noexcept { return convert_ieee754(rhs); }
#endif

	// arithmetic operators
	constexpr blocktriple& operator-() noexcept {
		_sign = !_sign;
		return *this;
	}

	// logical bit shift operators
	constexpr blocktriple& operator<<=(int leftShift) noexcept {
		if (leftShift == 0) return *this;
		if (leftShift < 0) return operator>>=(-leftShift);
		_scale -= leftShift;
		_significand <<= leftShift;
		return *this;
	}
	constexpr blocktriple& operator>>=(int rightShift) noexcept {
		if (rightShift == 0) return *this;
		if (rightShift < 0) return operator<<=(-rightShift);
		_scale += rightShift;
		_significand >>= rightShift;
		return *this;
	}
	
	constexpr blocktriple& bitShift(int leftShift) noexcept {
		_significand <<= leftShift;  // only manipulate the bits, not the scale
		return *this;
	}

	/// <summary>
	/// roundingDecision returns a pair<bool, unsigned> to direct the rounding and right shift
	/// </summary>
	/// <param name="adjustment">adjustment for subnormals </param>
	/// <returns>std::pair<bool, unsigned> of rounding direction (up is true, down is false), and the right shift</returns>
	constexpr std::pair<bool, unsigned> roundingDecision(int adjustment = 0) const noexcept {
		// preconditions: blocktriple is in 1's complement form, and not a denorm
		// this implies that the scale of the significand is 0 or 1
		unsigned significandScale = static_cast<unsigned>(significandscale());
		// find the shift that gets us to the lsb
		unsigned shift = significandScale + static_cast<unsigned>(radix) - fbits;
		bool roundup = _significand.roundingDirection(shift + static_cast<unsigned>(adjustment));
		return std::pair<bool, unsigned>(roundup, shift + static_cast<unsigned>(adjustment));
	}

	// apply a 2's complement recoding of the fraction bits
	inline constexpr blocktriple& twosComplement() noexcept {
		_significand.twosComplement();
		return *this;
	}

	// modifiers
	constexpr void clear()                             noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		_sign = false;
		_scale = 0;
		_significand.clear();
	}
	constexpr void setzero(bool sign = false)          noexcept {
		clear();
		_sign = sign;
	}
	constexpr void setnan(bool sign = true)            noexcept {
		clear();
		_nan = true;
		_inf = false;
		_zero = false;
		_sign = sign;   // if true, signalling NaN, otherwise quiet
	}
	constexpr void setinf(bool sign = true)            noexcept {
		clear();
		_inf = true;
		_zero = false;
		_sign = sign;
	}
	constexpr void setpos()                            noexcept { _sign = false; }
	constexpr void setnormal()                         noexcept {
		_nan = false;
		_inf = false;
		_zero = false;
	}
	constexpr void setsign(bool s)                     noexcept { _sign = s; }
	constexpr void setscale(int scale)                 noexcept { _scale = scale; }
	constexpr void setradix()                          noexcept { _significand.setradix(radix); }
	constexpr void setradix(int _radix)                noexcept { _significand.setradix(_radix); }
	constexpr void setbit(unsigned index, bool v = true) noexcept { _significand.setbit(index, v); }
	constexpr void setbits(uint64_t raw)               noexcept {
		// the setbits() api cannot be modified as it is shared by all number systems
		// as a standard mechanism for the test suites to set bits.
		// However, blocktriple uses extra state to encode the special values,
		// and the test can't use this interface to set that. 
		// Thus the caller (typically the test suite) must manage this special state.
		// _scale must be set by caller, so that the same raw bit pattern can 
		// span different scales
		// 		
		// blocktriple non-special values are always in normalized form
		_nan = false; _inf = false;
		_significand.setradix(radix);
		// Here we just check for 0 special case
		if (raw == 0) {
			_zero = true;
			_significand.clear();
		}
		else {
			_zero = false;
			_significand.setbits(raw);
		}
	}
	constexpr void setblock(unsigned i, const bt& block) noexcept { _significand.setblock(i, block); }
	constexpr void set(bool sign, int scale, uint64_t raw, bool inf = false, bool nan = false) noexcept {
		_nan = nan;
		_inf = inf;
		_sign = sign;
		_scale = scale;
		if (raw == 0) {
			_zero = true;
			_significand.clear();
		}
		else {
			_zero = false;
			_significand.setradix(radix);
			_significand.setbits(raw);
		}
	}

	// selectors
	constexpr bool isnan()                const noexcept { return _nan; }
	constexpr bool isinf()                const noexcept { return _inf; }
	constexpr bool iszero()               const noexcept { return _zero; }
	constexpr bool ispos()                const noexcept { return !_sign; }
	constexpr bool isneg()                const noexcept { return _sign; }
	constexpr bool sign()                 const noexcept { return _sign; }
	constexpr int  scale()                const noexcept { return _scale; }
	constexpr int  significandscale()     const noexcept {
		int sigScale = 0;
		for (int i = bfbits - 1; i >= radix; --i) {
			if (_significand.at(static_cast<unsigned>(i))) {
				sigScale = i - radix;
				break;
			}
		}
		return sigScale;
	}
	constexpr significand_t significand() const noexcept { return _significand; }
	constexpr significand_t fraction()    const noexcept { return _significand.fraction(); }
	constexpr uint64_t significand_ull()  const noexcept { return _significand.significand_ull(); } // fast path when bfbits <= 64 to get the significand bits out of the representation
	constexpr uint64_t fraction_ull()     const noexcept { return _significand.fraction_ull(); }
	constexpr bool at(unsigned index)     const noexcept { return _significand.at(index); }
	constexpr bool test(unsigned index)   const noexcept { return _significand.at(index); }
	constexpr bool any(unsigned index)    const noexcept { return _significand.any(index); }
	constexpr bt block(unsigned b)        const noexcept { return _significand.block(b); }

	// trace helpers; defined out-of-line in blocktriple_debug.hpp so this header
	// needs no <iostream>. Only reached when the matching _trace_btriple_* flag
	// is on, which requires that header to be included.
	void traceSignificandOp(const char* header, const char* label, const blocktriple& lhs, const blocktriple& rhs) const;
	void traceNormalizedOp(const char* header, const char* label, const blocktriple& lhs, const blocktriple& rhs) const;

	// helper debug function; defined out-of-line in blocktriple_debug.hpp so this
	// header needs no <iostream>. Include that header to call it.
public:
	// to_string: defined out-of-line in internal/blocktriple/manipulators.hpp so
	// this header needs no <sstream>. Include that header to call it.
	std::string to_string(std::streamsize precision = 7, std::streamsize width = 0,
		bool fixed = false, bool scientific = true, bool internal = false,
		bool left = false, bool showpos = false, bool uppercase = false,
		char fill = ' ') const;

	void constexprClassParameters() const;

	/////////////////////////////////////////////////////////////
	// ALU operators

	/// <summary>
	/// add two fixed-point numbers with fbits fraction bits and a leading 1
	/// yielding an unrounded sum of 3+fbits. (currently we generate a 3+(2*fbits) result as we haven't implemented the sticky bit optimization)
	/// This sum can overflow, be normal, or denormal. 
	/// Since we are not rounding
	/// we cannot act on overflow as we would potentially shift
	/// rounding state out, and thus the output must be processed
	/// by the calling environment. We can act on denormalized
	/// encodings, so these are processed in this function.
	/// To avoid fraction bit copies, the input arguments
	/// must be prepared by the calling environment, and 
	/// this function only manipulates the bits.
	/// </summary>
	/// <param name="lhs">ephemeral blocktriple that may get modified</param>
	/// <param name="rhs">ephemeral blocktriple that may get modified</param>
	/// <param name="result">unrounded sum</param>
	constexpr void add(blocktriple& lhs, blocktriple& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = std::max(lhs_scale, rhs_scale);

		// avoid copy by directly manipulating the fraction bits of the arguments
		int scaleDiff = lhs_scale - rhs_scale;
		// sticky bit calculation: abits = 1 hidden, f fraction, and r rounding bits: 1+f+r
		// lhs ->  h.ffffrrr     h = hidden, f = fraction, and r is rounding bits
		// rhs ->  h.ffffrrr
		// some shift of say rhs
		// rhs ->       hffffrrr
		//                 | this is our sticky bit in the normalized argument
		// sticky = righShift
		if (scaleDiff < 0) {
			bool sticky = lhs.any(static_cast<unsigned>(-scaleDiff));
			lhs >>= -scaleDiff;
			lhs.setbit(0, sticky);
		}
		else { //if (scaleDiff > 0) {
			bool sticky = rhs.any(static_cast<unsigned>(scaleDiff));
			rhs >>= scaleDiff;
			rhs.setbit(0, sticky);
		}
		if (lhs.isneg()) lhs._significand.twosComplement();
		if (rhs.isneg()) rhs._significand.twosComplement();

		_significand.add(lhs._significand, rhs._significand);  // do the bit arithmetic manipulation
		_significand.setradix(radix);                          // set the radix interpretation of the output

		if constexpr (_trace_btriple_add) {
			traceSignificandOp("blocksignificand unrounded add: just the significand values",
			                   "sum", lhs, rhs);
		}

		if (_significand.iszero()) {
			clear();
		}
		else {
			_zero = false;
			if (_significand.test(bfbits-1)) {  // is the result negative?
				_significand.twosComplement();
				_sign = true;
			}
			_scale = scale_of_result;
			// leave 01#.ffff to output processing: this is an overflow condition
			// 001.ffff is a perfect normalized format
			// fix 000.#### denormalized state to normalized
			if (!_significand.test(bfbits-2) && !_significand.test(bfbits-3)) {
				// found a denormalized form to normalize: find MSB
				int msb = _significand.msb(); // zero case has been taken care off above
				int leftShift = static_cast<int>(bfbits) - 3 - msb;
				_significand <<= leftShift;
				_scale -= leftShift;
			}
		}

		if constexpr (_trace_btriple_add) { traceNormalizedOp("blocktriple normalized add", "sum", lhs, rhs); }
	}

	constexpr void sub(blocktriple& lhs, blocktriple& rhs) {
		add(lhs, rhs.twosComplement());
	}

	/// <summary>
	/// multiply two real numbers with fbits fraction bits 
	/// yielding an 2*(1+fbits) unrounded product.
	/// 
	/// This product can overflow, be normal, or denormal. 
	/// Since we are not rounding
	/// we cannot act on overflow as we would potentially shift
	/// rounding state out, and thus the output must be processed
	/// by the calling environment. We can act on denormalized
	/// encodings, so these are processed in this function.
	/// To avoid fraction bit copies, the input arguments
	/// must be prepared by the calling environment, and 
	/// this function only manipulates the bits.	
	/// /// </summary>
	/// <param name="lhs">ephemeral blocktriple that may get modified</param>
	/// <param name="rhs">ephemeral blocktriple that may get modified</param>
	/// <param name="result">unrounded sum</param>
	constexpr void mul(blocktriple& lhs, blocktriple& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = lhs_scale + rhs_scale;

		// avoid copy by directly manipulating the fraction bits of the arguments
		_significand.mul(lhs._significand, rhs._significand);  // do the bit arithmetic manipulation
		_significand.setradix(2*fbits);                        // set the radix interpretation of the output

		if constexpr (_trace_btriple_mul) { traceSignificandOp("blocksignificand unrounded mul", "mul", lhs, rhs); }
		if (_significand.iszero()) {
			clear();
		}
		else {
			_zero = false;
			_scale = scale_of_result;
			_sign = (lhs.sign() == rhs.sign()) ? false : true;
			// the result may overflow, but we can't normalize the overflow as
			// this would remove an lsb that might impact the rounding.
			// The design we use here is that the raw ALUs do not normalize overflow
			// that is left to the conversion stage were we need to apply rounding rules

			// we also may have gotten a denormalized number, which we do need
			// to normalize. This constitutes a left shift and thus we would
			// not lose any rounding information by doing so.
			if (!_significand.test(bfbits - 1) && !_significand.test(bfbits - 2) ) {
				// found a denormalized form, thus need to normalize: find MSB
				int msb = _significand.msb(); // zero case has been taken care off above
				int leftShift = static_cast<int>(bfbits) - 3 - msb;
				_significand <<= leftShift;
				_scale -= leftShift;
			}
		}
		if constexpr (_trace_btriple_mul) { traceNormalizedOp("blocktriple normalized mul", "mul", lhs, rhs); }
	}

	constexpr void div(blocktriple& lhs, blocktriple& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = lhs_scale - rhs_scale;

		// avoid copy by directly manipulating the fraction bits of the arguments
		_significand.div(lhs._significand, rhs._significand);
		_significand.setradix(radix);

		if constexpr (_trace_btriple_div) { traceSignificandOp("blocksignificand unrounded div", "div", lhs, rhs); }
		if (_significand.iszero()) {
			clear();
		}
		else {
			_zero = false;
			_scale = scale_of_result;
			_sign = lhs.sign() != rhs.sign();
			// the result may overflow, but we can't normalize the overflow as
			// this would remove an lsb that might impact the rounding.
			// The design we use here is that the raw ALUs do not normalize overflow
			// that is left to the conversion stage were we need to apply rounding rules

			// we also may have gotten a denormalized number, which we do need
			// to normalize. This constitutes a left shift and thus we would
			// not lose any rounding information by doing so.
			if (!_significand.test(bfbits - 1) && !_significand.test(bfbits - 2)) {
				// found a denormalized form, thus need to normalize: find MSB
				int msb = _significand.msb(); // zero case has been taken care off above
				int leftShift = static_cast<int>(bfbits) - 2 - msb;
				_significand <<= leftShift;
				_scale -= leftShift;
			}
		}
		if constexpr (_trace_btriple_div) { traceNormalizedOp("blocktriple normalized div", "div", lhs, rhs); }
	}

private:
	// special cases to keep track of
	bool _nan; // most dominant state
	bool _inf; // second most dominant state
	bool _zero;// third most dominant special case

	// the triple (sign, scale, significand)
	bool _sign;
	int  _scale;

protected:
	significand_t _significand;

	// helpers

private:
	/// <summary>
	/// round a set of source bits to the present representation.
	/// srcbits is the number of bits of significand in the source representation
	/// round is intended for rounding raw IEEE-754 bits only
	/// </summary>
	/// <param name="raw">the raw unrounded bits</param>
	/// <returns></returns>
	template<unsigned srcbits, typename Real>
	constexpr uint64_t round(uint64_t raw) noexcept {
		if constexpr (fbits < srcbits) {
			// round to even: lsb guard round sticky
			// collect guard, round, and sticky bits
			// this same logic will work for the case where
			// we only have a guard bit and no round and/or sticky bits
			// because the mask logic will make round and sticky both 0

			// example: rounding the bits of a float to our fbits 
			// float significand: 24bits : 0bhfff'ffff'ffff'ffff'ffff'ffff; h is hidden, f are fraction bits
			// blocktriple target: 10bits: 0bhfff'ffff'fff    hidden bit is implicit, 10 fraction bits
			//                                           lg'rs
			//                             0b0000'0000'0001'0000'0000'0000; guard mask == 1 << srcbits - fbits - 2: 24 - 10 - 2 = 12
			constexpr unsigned upper = ieee754_parameter<Real>::nbits + 2;
			constexpr int offset = fbits + 2;
			constexpr int fullShift = srcbits - offset;  // srcbits includes the hidden bit, fbits does not
			constexpr unsigned shift = (fullShift < 0 ? 0 : fullShift);
			uint64_t mask = (srcbits < offset ? 0 : (1ull << shift));
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			if constexpr (shift > 1 && shift < upper) { // protect against a negative shift
				uint64_t allones(uint64_t(~0));
				mask = uint64_t(allones << (shift - 1));
				mask = ~mask;
			}
			else {
				mask = 0;
			}
			bool sticky = (mask & raw);

			raw >>= (shift + 1);  // shift out the bits we are rounding away
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
				if (raw == overflowPattern) {
					++_scale;
					raw >>= 1;
				}
			}
		}
		else {
			constexpr unsigned shift = fbits - srcbits;
			if constexpr (shift < ieee754_parameter<Real>::nbits) {
				raw <<= shift;
			}
			else {
#if !BIT_CAST_IS_CONSTEXPR
				std::fprintf(stderr, "round: shift %d is too large (>= 64)\n", int(shift));
#endif
			}
		}
		return raw;
	}

	template<typename Ty>
	constexpr inline blocktriple& convert_unsigned_integer(const Ty& rhs) noexcept {
		clear();
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = false;
		uint64_t raw = static_cast<uint64_t>(rhs);
		_scale = static_cast<int>(find_msb(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr unsigned sizeInBits = 8 * sizeof(Ty);
		uint64_t shift = sizeInBits - _scale - 1;
		raw <<= shift;
		uint64_t rounded_bits = round<sizeInBits, uint64_t>(raw);
		switch (op) {
		case BlockTripleOperator::ADD:
			_significand.setradix(fbits);
			break;
		case BlockTripleOperator::MUL:
			_significand.setradix(fbits);
			break;
		case BlockTripleOperator::DIV:
			_significand.setradix(2 * fbits);
			break;
		case BlockTripleOperator::SQRT:
			_significand.setradix(2 * fbits);
			break;
		case BlockTripleOperator::REP:
			_significand.setradix(fbits);
			break;
		}
		_significand.setbits(rounded_bits);
		return *this;
	}
	template<typename Ty>
	constexpr inline blocktriple& convert_signed_integer(const Ty& rhs) noexcept {
		clear();
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = (rhs < 0);
		uint64_t raw = static_cast<uint64_t>(_sign ? -rhs : rhs);
		_scale = static_cast<int>(find_msb(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr unsigned sizeInBits = 8 * sizeof(Ty);
		uint64_t shift = sizeInBits - _scale - 1;
		raw <<= shift;
		uint64_t rounded_bits = round<sizeInBits, uint64_t>(raw);  // TODO: there is something wrong here: that second template param only supports float types
		switch (op) {
		case BlockTripleOperator::ADD:
			_significand.setradix(fbits);
			break;
		case BlockTripleOperator::MUL:
			_significand.setradix(fbits);
			break;
		case BlockTripleOperator::DIV:
			_significand.setradix(2 * fbits);
			break;
		case BlockTripleOperator::SQRT:
			_significand.setradix(2 * fbits);
			break;
		case BlockTripleOperator::REP:
			_significand.setradix(fbits);
			break;
		}
		_significand.setbits(rounded_bits);
		return *this;
	}

	template<typename Real>
	inline CONSTEXPRESSION blocktriple& convert_ieee754(Real rhs) noexcept {   // TODO: deal with subnormals and inf
		clear();

		// extract raw IEEE-754 bits
		bool s{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(rhs, s, rawExponent, rawFraction, bits);

		// special case handling
		if (rawExponent == ieee754_parameter<Real>::eallset) { // nan and inf
			// IEEE-754: exponent all ones with a ZERO fraction is infinity, and with ANY
			// non-zero fraction is a NaN, whatever the payload.  This used to compare the
			// fraction for equality against three specific payloads, so every other one --
			// including the canonical signalling 0x1 -- missed all three and fell through to
			// the numeric path below, turning a NaN into a finite value.  Fourth instance of
			// the same copy-pasted defect; issue #1303.
			if (rawFraction == 0ull) {
				// 1.11111111.0000000.......000000000 -inf
				// 0.11111111.0000000.......000000000 +inf
				_nan = false;
				_inf = true;
				_zero = false;   // clear() set it; a special is not a zero
				_sign = s;  // + or - infinity
				_scale = 10000;
				return *this;
			}
			// the fraction's MSB is the quiet bit; a blocktriple carries the distinction in
			// its sign, matching cfloat's convention of sign 1 for signalling
			constexpr uint64_t quietbit = ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask;
			_nan = true;
			_inf = false;
			_zero = false;   // clear() set it; a special is not a zero
			_sign = ((rawFraction & quietbit) == 0ull);   // clear quiet bit == signalling
			_scale = 0;
			return *this;
		}
		if (rhs == 0.0f) { // IEEE rule: this is valid for + and - 0.0
			_nan = false;
			_inf = false;
			_zero = true;
			_sign = s;
			_scale = 0;
			return *this;
		}
		if (rawExponent == 0ull) {
			// value is a subnormal: TBD
#if ! BIT_CAST_IS_CONSTEXPR
			std::fprintf(stderr, "subnormal value TBD\n");
#endif
		}
		else {
			int exponent = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;  // unbias the exponent

			// normal number, not zero
			_nan = false;
			_inf = false;
			_zero = false;
			_sign = s;
			_scale = exponent;

			// add the hidden bit
			rawFraction |= (1ull << ieee754_parameter<Real>::fbits);
			uint64_t rounded_bits = round<ieee754_parameter<Real>::fbits+1, Real>(rawFraction);
			_significand.setbits(rounded_bits);
			switch(op) {
			case BlockTripleOperator::REP:
				_significand.setradix(fbits);
				break;
			case BlockTripleOperator::ADD:
				_significand.setradix(abits);
				_significand <<= rbits;
				break;
			case BlockTripleOperator::MUL:
				_significand.setradix(2*fbits);
				_significand <<= fbits;
				break;
			case BlockTripleOperator::DIV:
				_significand.setradix(2*fbits);
				_significand <<= fbits;
				break;
			case BlockTripleOperator::SQRT:
				_significand.setradix(2 * fbits);
				_significand <<= fbits;
				break;
			}
		}

		return *this;
	}

	template<typename Real>
	constexpr Real to_native() const noexcept {
		if (_nan) { if (_sign) return std::numeric_limits<Real>::signaling_NaN(); else return std::numeric_limits<Real>::quiet_NaN(); }
		if (_inf) { if (_sign) return -std::numeric_limits<Real>::infinity(); else return std::numeric_limits<Real>::infinity(); }
		if (_zero) {
			Real v(0);
			if (_sign) {
				return -v;
			}
			else {
				return v;
			}
		}
		Real v = Real(_significand);
		v *= std::pow(Real(2.0f), Real(_scale));
		Real s = (_sign ? Real(-1.0) : Real(1.0));
		return s * v;
	}

public:

private:

	// Format exponent as +/-NNN (handles arbitrarily large exponents)
	static void append_exponent(std::string& str, long long e) {
		str += (e < 0 ? '-' : '+');
		e = (e < 0) ? -e : e;
		std::string digits;
		if (e == 0) { digits = "00"; }
		else { while (e > 0) { digits += static_cast<char>('0' + e % 10); e /= 10; } }
		while (digits.length() < 2) digits += '0';
		std::reverse(digits.begin(), digits.end());
		str += digits;
	}

public:


	// no friend declaration for operator>>: it is defined in
	// internal/blocktriple/iostream.hpp and needs no private access (it assigns
	// through the public constructor). See #1334.
	//
	// to_binary and to_triple DO keep theirs: they read _sign, _scale and
	// _significand directly, so friendship is load-bearing there.
	// declare as friends to avoid needing a marshalling function to get significand bits out
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend std::string to_binary(const blocktriple<ffbits, oop, bbt>&, bool);
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend std::string to_triple(const blocktriple<ffbits, oop, bbt>&, bool);

	// logic operators
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend bool operator==(const blocktriple<ffbits, oop, bbt>& lhs, const blocktriple<ffbits, oop, bbt>& rhs);
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend bool operator!=(const blocktriple<ffbits, oop, bbt>& lhs, const blocktriple<ffbits, oop, bbt>& rhs);
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend bool operator< (const blocktriple<ffbits, oop, bbt>& lhs, const blocktriple<ffbits, oop, bbt>& rhs);
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend bool operator> (const blocktriple<ffbits, oop, bbt>& lhs, const blocktriple<ffbits, oop, bbt>& rhs);
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend bool operator<=(const blocktriple<ffbits, oop, bbt>& lhs, const blocktriple<ffbits, oop, bbt>& rhs);
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend bool operator>=(const blocktriple<ffbits, oop, bbt>& lhs, const blocktriple<ffbits, oop, bbt>& rhs);
};

////////////////////// operators



template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator==(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._significand == rhs._significand && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator!=(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator< (const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) {
	if (lhs._inf) {
		if (rhs._inf) return false; else return true; // everything is less than -infinity
	}
	else {
		if (rhs._inf) return false;
	}

	if (lhs._zero) {
		if (rhs._zero) return false; // they are both 0
		if (rhs._sign) return false; else return true;
	}
	if (rhs._zero) {
		if (lhs._sign) return true; else return false;
	}
	if (lhs._sign) {
		if (rhs._sign) {	// both operands are negative
			if (lhs._scale > rhs._scale) {
				return true;	// lhs is more negative
			}
			else {
				if (lhs._scale == rhs._scale) {
					// compare the fraction, which is an unsigned value
					if (lhs._significand == rhs._significand) return false; // they are the same value
					if (lhs._significand > rhs._significand) {
						return true; // lhs is more negative
					}
					else {
						return false; // lhs is less negative
					}
				}
				else {
					return false; // lhs is less negative
				}
			}
		}
		else {
			return true; // lhs is negative, rhs is positive
		}
	}
	else {
		if (rhs._sign) {
			return false; // lhs is positive, rhs is negative
		}
		else {
			if (lhs._scale > rhs._scale) {
				return false; // lhs is more positive
			}
			else {
				if (lhs._scale == rhs._scale) {
					// compare the fractions
					if (lhs._significand == rhs._significand) return false; // they are the same value
					if (lhs._significand > rhs._significand) {
						return false; // lhs is more positive than rhs
					}
					else {
						return true; // lhs is less positive than rhs
					}
				}
				else {
					return true; // lhs is less positive
				}
			}
		}
	}
}
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator> (const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator<=(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator>=(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return !operator< (lhs, rhs); }


////////////////////////////////// string conversion functions //////////////////////////////



template<unsigned fbits, BlockTripleOperator op, typename bt>
blocktriple<fbits> abs(const blocktriple<fbits, op, bt>& a) {
	blocktriple<fbits, op, bt> absolute(a);
	absolute.setpos();
	return absolute;
}


}} // namespace sw::universal

// When any blocktriple trace switch is on, add/mul/div instantiate the helpers
// declared above, so their definitions must be in the translation unit. Pull
// them in automatically rather than leaving every caller to remember: the
// failure mode is a link error, which -fsyntax-only cannot see. The include is
// safe from here -- blocktriple_debug.hpp includes this header, and #pragma once
// makes that a no-op, so the out-of-line definitions land after the class.
//
// constexprClassParameters() has no macro to key on, so calling it still
// requires including <universal/internal/blocktriple/blocktriple_debug.hpp>
// explicitly. It is an opt-in debug facility, not something the arithmetic
// reaches on its own.
#if defined(BLOCKTRIPLE_VERBOSE_OUTPUT) || defined(BLOCKTRIPLE_TRACE_ALL) \
 || defined(BLOCKTRIPLE_TRACE_ADD) || defined(BLOCKTRIPLE_TRACE_SUB) \
 || defined(BLOCKTRIPLE_TRACE_MUL) || defined(BLOCKTRIPLE_TRACE_DIV)
#include <universal/internal/blocktriple/blocktriple_debug.hpp>
#endif
