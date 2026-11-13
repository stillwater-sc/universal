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
#include <type_traits>
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
	constexpr blocktriple(unsigned char iv)      noexcept { *this = iv; }
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
	// plain char is signed or unsigned depending on the platform: convert it as the type it is
	constexpr blocktriple& operator=(char rhs)               noexcept {
		if constexpr (std::is_signed_v<char>) return convert_signed_integer(static_cast<signed char>(rhs));
		else return convert_unsigned_integer(static_cast<unsigned char>(rhs));
	}
	constexpr blocktriple& operator=(unsigned char rhs)      noexcept { return convert_unsigned_integer(rhs); }
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
	// no default for the sign: it defaulted to negative, and every caller that omitted it
	// turned +inf into -inf (#1719)
	constexpr void setinf(bool sign)                   noexcept {
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
	/// </summary>
	/// <param name="lhs">ephemeral blocktriple that may get modified</param>
	/// <param name="rhs">ephemeral blocktriple that may get modified</param>
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
	// Order of the magnitudes of two finite values, -1, 0 or 1, independent of their layout: an
	// operator result is left unnormalized (a MUL product of 2.25 is 10.01b at radix 2*fbits with
	// scale 0) while a native value is 1.001b at its operand radix with scale 1. Compare the
	// binade of the leading one, then the bits aligned at that leading one; nothing is discarded.
	static constexpr int compare_magnitude(const blocktriple& a, const blocktriple& b) noexcept {
		const int ma = a._significand.msb(), mb = b._significand.msb();
		if (ma < 0 || mb < 0) return (ma < 0) ? ((mb < 0) ? 0 : -1) : 1;   // a zero significand
		// in 64 bits: setscale() accepts any int, and scale + msb could overflow before the radix cancels it
		const long long ea = static_cast<long long>(a._scale) + ma - a._significand.radix();
		const long long eb = static_cast<long long>(b._scale) + mb - b._significand.radix();
		if (ea != eb) return (ea < eb) ? -1 : 1;
		significand_t x(a._significand), y(b._significand);
		x <<= (static_cast<int>(bfbits) - 1 - ma);
		y <<= (static_cast<int>(bfbits) - 1 - mb);
		if (x == y) return 0;
		return x.unsigned_less(y) ? -1 : 1;
	}

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
	/// round a significand of srcbits bits to the fbits + 1 bits of this blocktriple,
	/// to nearest, ties to even.
	///
	/// raw holds the source significand with its leading (hidden) bit at srcbits - 1.
	/// When fbits + 1 < srcbits the low bits are rounded away, and the result has its
	/// hidden bit at fbits; a carry out of 1.11..1 increments _scale. When fbits + 1 >=
	/// srcbits nothing is rounded and raw is returned UNCHANGED: the caller places it
	/// with placeSignificand(), because the hidden bit may sit beyond the 64 bits of a
	/// uint64_t (fbits >= 64).
	/// </summary>
	template<unsigned srcbits>
	constexpr uint64_t round(uint64_t raw) noexcept {
		static_assert(srcbits >= 1 && srcbits <= 64, "round: a source significand holds 1 to 64 bits");
		if constexpr (fbits + 1 < srcbits) {
			constexpr unsigned drop = srcbits - fbits - 1;   // 1 <= drop <= 63
			constexpr uint64_t half = (1ull << (drop - 1));  // weight of the guard bit
			const uint64_t rest = raw & ((half << 1) - 1ull);
			raw >>= drop;
			if (rest > half || (rest == half && (raw & 1ull))) {
				++raw;
				if (raw == (1ull << (fbits + 1))) {  // 1.11..1 + ulp = 10.00..0
					++_scale;
					raw >>= 1;
				}
			}
		}
		return raw;
	}

	// set the significand from the bits returned by round<srcbits>(), with the hidden bit at fbits
	template<unsigned srcbits>
	constexpr void placeSignificand(uint64_t rounded) noexcept {
		_significand.setbits(rounded);
		if constexpr (fbits + 1 > srcbits) {
			_significand <<= static_cast<int>(fbits + 1 - srcbits);
		}
	}

	template<typename Ty>
	constexpr inline blocktriple& convert_unsigned_integer(const Ty& rhs) noexcept {
		static_assert(std::is_unsigned_v<Ty>, "convert_unsigned_integer requires an unsigned type");
		clear();
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = false;
		return convert_magnitude<8 * sizeof(Ty)>(static_cast<uint64_t>(rhs));
	}
	template<typename Ty>
	constexpr inline blocktriple& convert_signed_integer(const Ty& rhs) noexcept {
		static_assert(std::is_signed_v<Ty>, "convert_signed_integer requires a signed type");
		clear();
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = (rhs < 0);
		// the magnitude in the unsigned domain: -rhs overflows for the most negative value
		using Unsigned = std::make_unsigned_t<Ty>;
		const Unsigned bits = static_cast<Unsigned>(rhs);
		const Unsigned magnitude = _sign ? static_cast<Unsigned>(Unsigned(0) - bits) : bits;
		return convert_magnitude<8 * sizeof(Ty)>(static_cast<uint64_t>(magnitude));
	}
	// a non-zero integer magnitude of at most sizeInBits bits
	template<unsigned sizeInBits>
	constexpr blocktriple& convert_magnitude(uint64_t raw) noexcept {
		_scale = static_cast<int>(find_msb(raw)) - 1; // find_msb is 1-based; raw != 0
		raw <<= (sizeInBits - 1u - static_cast<unsigned>(_scale));  // leading bit at sizeInBits - 1
		placeSignificand<sizeInBits>(round<sizeInBits>(raw));
		alignForOperator();
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
#if LONG_DOUBLE_SUPPORT
		if constexpr (std::is_same_v<Real, long double>) {
			return convert_long_double(rhs);
		}
		else
#endif
		{
			// float and double: the significand fits a uint64_t (the else keeps a long double,
			// whose significand may not, from instantiating this path)
			constexpr unsigned srcfbits = ieee754_parameter<Real>::fbits;
			_nan = false;
			_inf = false;
			_zero = false;
			_sign = s;
			if (rawExponent == 0ull) {
				// subnormal: 0.fff * 2^(1 - bias); normalize so the leading one becomes the hidden bit
				const int msb = static_cast<int>(find_msb(rawFraction)) - 1;  // 0-based, rawFraction != 0
				_scale = 1 - ieee754_parameter<Real>::bias - (static_cast<int>(srcfbits) - msb);
				rawFraction <<= (srcfbits - static_cast<unsigned>(msb));
			}
			else {
				_scale = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;  // unbias the exponent
				rawFraction |= (1ull << srcfbits);  // add the hidden bit
			}
			placeSignificand<srcfbits + 1>(round<srcfbits + 1>(rawFraction));
			alignForOperator();
			return *this;
		}
	}

	// move a significand held as 1.fff (fbits + 1 bits, hidden bit at fbits) to the operator's layout
	constexpr void alignForOperator() noexcept {
		switch(op) {
		case BlockTripleOperator::REP:
			_significand.setradix(fbits);
			break;
		case BlockTripleOperator::ADD:
			_significand.setradix(abits);
			_significand <<= rbits;
			break;
		case BlockTripleOperator::MUL:
			// the operand layout mul() consumes: radix at fbits, the product lands at 2*fbits
			_significand.setradix(fbits);
			break;
		case BlockTripleOperator::DIV:
			// the operand layout div() consumes, as cfloat's normalizeDivision builds it
			_significand.setradix(radix);
			_significand <<= static_cast<int>(divshift);
			break;
		case BlockTripleOperator::SQRT:
			_significand.setradix(2 * fbits);
			_significand <<= fbits;
			break;
		}
	}

#if LONG_DOUBLE_SUPPORT
	// A finite, non-zero long double, rounded to fbits + 1 significand bits, to nearest even, from
	// all of its bits. The uint64_t path above gets 64 bits from extractFields(), whose last is a
	// round-to-odd sticky on IEEE binary128 and double-double: a blocktriple of fbits >= 62 took it
	// for a real bit or rounded twice, and a subnormal long double was not converted at all (#1517).
	blocktriple& convert_long_double(long double rhs) noexcept {
		long_double_significand sig(rhs);
		_nan   = false;
		_inf   = false;
		_zero  = false;
		_sign  = sig.negative();
		_scale = sig.scale();
		int  position = static_cast<int>(fbits);  // of the next bit: the leading one is the hidden bit
		bool guard = false, sticky = false;
		while (!sig.empty() && position >= -1) {  // the kept bits, then the guard bit at -1
			const uint64_t bits = sig.next();
			for (int k = 63; k >= 0; --k, --position) {
				const bool bit = ((bits >> k) & 1ull) != 0;
				if (position >= 0) {
					if (bit) _significand.setbit(static_cast<unsigned>(position));
				}
				else if (position == -1) guard = bit;
				else sticky = sticky || bit;
			}
		}
		sticky = sticky || !sig.empty();
		if (guard && (sticky || _significand.test(0))) {
			_significand.increment();
			if (_significand.test(fhbits)) {  // 1.11...1 + ulp = 10.00...0
				_significand.clear();
				_significand.setbit(fbits);
				++_scale;
			}
		}
		alignForOperator();
		return *this;
	}
#endif

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
		// only the ADD layout is 2's complement; the MUL, DIV and SQRT layouts hold an unsigned
		// magnitude whose top bit is an overflow bit, and REP's top bit is always clear
		Real v = Real(_significand.to_double(op == BlockTripleOperator::ADD));
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



// IEEE-754 comparison semantics: NaN is unordered (every comparison with it is false, and
// != is true), +0 == -0, and -inf < every finite value < +inf.
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator==(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) {
	if (lhs._nan || rhs._nan) return false;
	if (lhs._zero || rhs._zero) return lhs._zero && rhs._zero;
	if (lhs._inf || rhs._inf) return lhs._inf && rhs._inf && lhs._sign == rhs._sign;
	return lhs._sign == rhs._sign && blocktriple<fbits, op, bt>::compare_magnitude(lhs, rhs) == 0;
}
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator!=(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator< (const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) {
	if (lhs._nan || rhs._nan) return false;
	if (lhs._inf) return lhs._sign && !(rhs._inf && rhs._sign);   // -inf is below all but -inf
	if (rhs._inf) return !rhs._sign;                                // a non-inf lhs is below +inf only
	if (lhs._zero) return rhs._zero ? false : !rhs._sign;
	if (rhs._zero) return lhs._sign;
	if (lhs._sign != rhs._sign) return lhs._sign;
	const int order = blocktriple<fbits, op, bt>::compare_magnitude(lhs, rhs);
	return lhs._sign ? (order > 0) : (order < 0);
}
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator> (const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator<=(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return operator< (lhs, rhs) || operator==(lhs, rhs); }
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator>=(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return operator< (rhs, lhs) || operator==(lhs, rhs); }


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
