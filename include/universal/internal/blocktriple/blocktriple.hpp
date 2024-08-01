#pragma once
// blocktriple.hpp: definition of a (sign, scale, significant) representation of a generic floating-point value that goes into an arithmetic operation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>

// check on required compilation guards
// should be defined by calling environment, just catching it here just in case it is not
#ifndef LONG_DOUBLE_SUPPORT
#pragma message("LONG_DOUBLE_SUPPORT is not defined")
#define LONG_DOUBLE_SUPPORT 0
#endif
#if ! BIT_CAST_IS_CONSTEXPR
#pragma message("BIT_CAST_IS_CONSTEXPR is not defined")
#define BIT_CAST_CONSTEXPR
#endif
#if !defined(CONSTEXPRESSION)
#define CONSTEXPRESSION
#endif

// dependent types for stand-alone use of this class
#include <universal/native/integers.hpp> // to_binary(uint64_t)
#include <universal/native/ieee754.hpp>
#include <universal/native/subnormal.hpp>
#include <universal/utility/find_msb.hpp>
#include <universal/internal/blocksignificant/blocksignificant.hpp>
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

	for add and subtract
	blocksignificant = 00h.ffffeee <- three bits before radix point, fraction bits plus 3 rounding bits
	unsigned bfbits = fbits + 3;

	for multiply
	unsigned bfbits = 2*fhbits;
	*/

 // operator specialization tag for blocktriple
enum class BlockTripleOperator { REP, ADD, MUL, DIV, SQRT };
inline std::ostream& operator<<(std::ostream& ostr, const BlockTripleOperator& op) {
	switch (op) {
	case BlockTripleOperator::REP:
		ostr << "REP";
		break;
	case BlockTripleOperator::ADD:
		ostr << "ADD";
		break;
	case BlockTripleOperator::MUL:
		ostr << "MUL";
		break;
	case BlockTripleOperator::DIV:
		ostr << "DIV";
		break;
	case BlockTripleOperator::SQRT:
		ostr << "SQRT";
		break;
	default:
		ostr << "NOP";
	}
	return ostr;
}

// Forward definitions
template<unsigned fbits, BlockTripleOperator op, typename bt> class blocktriple;
template<unsigned fbits, BlockTripleOperator op, typename bt> blocktriple<fbits, op, bt> abs(const blocktriple<fbits, op, bt>& v);

template<unsigned fbits, BlockTripleOperator op, typename bt>
blocktriple<fbits, op, bt>& convert(unsigned long long uint, blocktriple<fbits, op, bt>& tgt) {
	return tgt;
}

// Generate a type tag for this type: blocktriple<fbits, operator, unsigned int>
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string type_tag(const blocktriple<fbits, op, bt>& = {}) {
	std::stringstream s;
	s << "blocktriple<"
	  << std::setw(3) << fbits << ", "
      << op << ", "
	  << typeid(bt).name() << '>';
	return s.str();
}

/// <summary>
/// Generalized blocktriple representing a (sign, scale, significant) with unrounded arithmetic
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
/// <typeparam name="fbits">number of fraction bits in the significant</typeparam>
/// <typeparam name="bt">block type: one of [uint8_t, uint16_t, uint32_t, uint64_t]</typeparam>
template<unsigned _fbits, BlockTripleOperator _op = BlockTripleOperator::ADD, typename bt = std::uint32_t>
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

	static constexpr unsigned MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken

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
	using Significant = sw::universal::blocksignificant<bfbits, bt>;

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
		_sign{ false }, _scale{ 0 } {} // _significant uses default constructor and static constexpr radix computation

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
					std::cerr << "bitPattern contained a non-standard character: " << c << '\n';
					return *this;
				}
			}
		}
		else {
			std::cerr << "bitPattern must start with 0b: instead input pattern was " << bitPattern << '\n';
			return *this;
		}

		unsigned nrBits = bits.size();
		if (nrBits != bfbits) {
			std::cerr << "nr of bits in bitPattern is " << nrBits << " and needs to be " << bfbits << '\n';
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
	explicit operator float()                          const noexcept { return to_native<float>(); }
	explicit operator double()                         const noexcept { return to_native<double>(); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	CONSTEXPRESSION blocktriple(long double iv)				noexcept { *this = iv; }
	CONSTEXPRESSION blocktriple& operator=(long double rhs) noexcept { return convert_ieee754(rhs); }
	explicit operator long double()                   const noexcept { return to_native<long double>(); }
#endif

	// logical bit shift operators
	constexpr blocktriple& operator<<=(int leftShift) noexcept {
		if (leftShift == 0) return *this;
		if (leftShift < 0) return operator>>=(-leftShift);
		_scale -= leftShift;
		_significant <<= leftShift;
		return *this;
	}
	constexpr blocktriple& operator>>=(int rightShift) noexcept {
		if (rightShift == 0) return *this;
		if (rightShift < 0) return operator<<=(-rightShift);
		_scale += rightShift;
		_significant >>= rightShift;
		return *this;
	}
	
	constexpr blocktriple& bitShift(int leftShift) noexcept {
		_significant <<= leftShift;  // only manipulate the bits, not the scale
		return *this;
	}

	/// <summary>
	/// roundingDecision returns a pair<bool, unsigned> to direct the rounding and right shift
	/// </summary>
	/// <param name="adjustment">adjustment for subnormals </param>
	/// <returns>std::pair<bool, unsigned> of rounding direction (up is true, down is false), and the right shift</returns>
	constexpr std::pair<bool, unsigned> roundingDecision(int adjustment = 0) const noexcept {
		// preconditions: blocktriple is in 1's complement form, and not a denorm
		// this implies that the scale of the significant is 0 or 1
		unsigned significantScale = static_cast<unsigned>(significantscale());
		// find the shift that gets us to the lsb
		unsigned shift = significantScale + static_cast<unsigned>(radix) - fbits;
		bool roundup = _significant.roundingDirection(shift + adjustment);
		return std::pair<bool, unsigned>(roundup, shift + adjustment);
	}

	// apply a 2's complement recoding of the fraction bits
	inline constexpr blocktriple& twosComplement() noexcept {
		_significant.twosComplement();
		return *this;
	}

	// modifiers
	constexpr void clear()                             noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		_sign = false;
		_scale = 0;
		_significant.clear();
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
	constexpr void setradix()                          noexcept { _significant.setradix(radix); }
	constexpr void setradix(int _radix)                noexcept { _significant.setradix(_radix); }
	constexpr void setbit(unsigned index, bool v = true) noexcept { _significant.setbit(index, v); }
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
		_significant.setradix(radix);
		// Here we just check for 0 special case
		if (raw == 0) {
			_zero = true;
			_significant.clear();
		}
		else {
			_zero = false;
			_significant.setbits(raw);
		}
	}
	constexpr void setblock(unsigned i, const bt& block) noexcept { _significant.setblock(i, block); }

	// selectors
	inline constexpr bool isnan()                const noexcept { return _nan; }
	inline constexpr bool isinf()                const noexcept { return _inf; }
	inline constexpr bool iszero()               const noexcept { return _zero; }
	inline constexpr bool ispos()                const noexcept { return !_sign; }
	inline constexpr bool isneg()                const noexcept { return _sign; }
	inline constexpr bool sign()                 const noexcept { return _sign; }
	inline constexpr int  scale()                const noexcept { return _scale; }
	inline constexpr int  significantscale()     const noexcept {
		int sigScale = 0;
		for (int i = bfbits - 1; i >= radix; --i) {
			if (_significant.at(static_cast<unsigned>(i))) {
				sigScale = i - radix;
				break;
			}
		}
		return sigScale;
	}
	inline constexpr Significant significant()   const noexcept { return _significant; }
	inline constexpr Significant fraction()      const noexcept { return _significant.fraction(); }
	inline constexpr uint64_t significant_ull()  const noexcept { return _significant.significant_ull(); } // fast path when bfbits <= 64 to get the significant bits out of the representation
	inline constexpr uint64_t fraction_ull()     const noexcept { return _significant.fraction_ull(); }
	inline constexpr bool at(unsigned index)       const noexcept { return _significant.at(index); }
	inline constexpr bool test(unsigned index)     const noexcept { return _significant.at(index); }
	inline constexpr bool any(unsigned index)      const noexcept { return _significant.any(index); }
	inline constexpr bt block(unsigned b)          const noexcept { return _significant.block(b); }

	// helper debug function
	void constexprClassParameters() const {
		std::cout << "-------------------------------------------------------------\n";
		std::cout << "type              : " << typeid(*this).name() << '\n';
		std::cout << "fbits             : " << fbits << '\n';
		std::cout << "operator          : " << op << '\n';
		std::cout << "bitsInByte        : " << bitsInByte << '\n';
		std::cout << "bitsInBlock       : " << bitsInBlock << '\n';
		std::cout << "nrBlocks          : " << nrBlocks << '\n';
		std::cout << "storageMask       : " << to_binary(storageMask) << '\n';

		std::cout << "MSU               : " << MSU << '\n';

		std::cout << "fhbits            : " << fhbits << '\n';
		std::cout << "rbits             : " << rbits << "      rounding bits for addition/subtraction\n";
		std::cout << "abits             : " << abits << "      size of the addend = fbits + rbits\n";
		std::cout << "mbits             : " << mbits << "      size of the multiplier output\n";
		std::cout << "divbits           : " << divbits << "      size of the divider output\n";
		std::cout << "sqrtbits          : " << sqrtbits << "      size of the square root output\n";
		// we transform input operands into the operation's target output size
		// so that everything is aligned correctly before the operation starts.
		std::cout << "bfbits            : " << bfbits << "      bits in the blocksignificant representation\n";
		std::cout << "radix             : " << radix << "      position of the radix point of the ALU operator result\n";
//		std::cout << "encoding          : " << encoding << '\n';
		std::cout << "normalBits        : " << normalBits << "      normal bits to track: metaprogramming trick to remove warnings\n";
		std::cout << "normalFormMask    : " << to_binary(normalFormMask) << "   normalFormMask for small configurations\n";
		std::cout << "significant type  : " << typeid(Significant).name() << '\n';

		std::cout << "ALL_ONES          : " << to_binary(ALL_ONES) << '\n';
		std::cout << "maxbits           : " << maxbits << "        bit to check for overflow: metaprogramming trick\n";
		std::cout << "overflowPattern   : " << to_binary(overflowPattern) << '\n';

		std::cout << std::endl;
	}

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
	void add(blocktriple& lhs, blocktriple& rhs) {
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
		if (lhs.isneg()) lhs._significant.twosComplement();
		if (rhs.isneg()) rhs._significant.twosComplement();

		_significant.add(lhs._significant, rhs._significant);  // do the bit arithmetic manipulation
		_significant.setradix(radix);                          // set the radix interpretation of the output

		if constexpr (_trace_btriple_add) {
			std::cout << "blocksignificant unrounded add: just the significant values\n";
			std::cout << typeid(_significant).name() << '\n';
			std::cout << "lhs significant : " << to_binary(lhs._significant) << " : " << lhs._significant << '\n';
			std::cout << "rhs significant : " << to_binary(rhs._significant) << " : " << rhs._significant << '\n';
			std::cout << "sum significant : " << to_binary(_significant) << " : " << _significant << '\n';
		}

		if (_significant.iszero()) {
			clear();
		}
		else {
			_zero = false;
			if (_significant.test(bfbits-1)) {  // is the result negative?
				_significant.twosComplement();
				_sign = true;
			}
			_scale = scale_of_result;
			// leave 01#.ffff to output processing: this is an overflow condition
			// 001.ffff is a perfect normalized format
			// fix 000.#### denormalized state to normalized
			if (!_significant.test(bfbits-2) && !_significant.test(bfbits-3)) {
				// found a denormalized form to normalize: find MSB
				int msb = _significant.msb(); // zero case has been taken care off above
				int leftShift = static_cast<int>(bfbits) - 3 - msb;
				_significant <<= leftShift;
				_scale -= leftShift;
			}
		}

		if constexpr (_trace_btriple_add) {
			std::cout << "blocktriple normalized add\n";
			std::cout << typeid(*this).name() << '\n';
			std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << "sum : " << to_binary(*this) << " : " << *this << '\n';
		}
	}

	void sub(blocktriple& lhs, blocktriple& rhs) {
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
	void mul(blocktriple& lhs, blocktriple& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = lhs_scale + rhs_scale;

		// avoid copy by directly manipulating the fraction bits of the arguments
		_significant.mul(lhs._significant, rhs._significant);  // do the bit arithmetic manipulation
		_significant.setradix(2*fbits);                        // set the radix interpretation of the output

		if constexpr (_trace_btriple_mul) {
			std::cout << "blocksignificant unrounded mul\n";
			std::cout << typeid(_significant).name() << '\n';
			std::cout << "lhs significant : " << to_binary(lhs._significant) << " : " << lhs._significant << '\n';
			std::cout << "rhs significant : " << to_binary(rhs._significant) << " : " << rhs._significant << '\n';
			std::cout << "mul significant : " << to_binary(_significant) << " : " << _significant << '\n';
		}
		if (_significant.iszero()) {
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
			if (!_significant.test(bfbits - 1) && !_significant.test(bfbits - 2) ) {
				// found a denormalized form, thus need to normalize: find MSB
				int msb = _significant.msb(); // zero case has been taken care off above
				int leftShift = static_cast<int>(bfbits) - 3 - msb;
				_significant <<= leftShift;
				_scale -= leftShift;
			}
		}
		if constexpr (_trace_btriple_mul) {
			std::cout << "blocktriple normalized mul\n";
			std::cout << typeid(*this).name() << '\n';
			std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << "mul : " << to_binary(*this) << " : " << *this << '\n';
		}
	}

	void div(blocktriple& lhs, blocktriple& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = lhs_scale - rhs_scale;

		// avoid copy by directly manipulating the fraction bits of the arguments
		_significant.div(lhs._significant, rhs._significant);
		_significant.setradix(radix);

		if constexpr (_trace_btriple_div) {
			std::cout << "blocksignificant unrounded div\n";
			std::cout << typeid(_significant).name() << '\n';
			std::cout << "lhs significant : " << to_binary(lhs._significant) << " : " << lhs._significant << '\n';
			std::cout << "rhs significant : " << to_binary(rhs._significant) << " : " << rhs._significant << '\n';
			std::cout << "div significant : " << to_binary(_significant) << " : " << _significant << '\n';  // <-- the scale of this representation is not yet set
		}
		if (_significant.iszero()) {
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
			if (!_significant.test(bfbits - 1) && !_significant.test(bfbits - 2)) {
				// found a denormalized form, thus need to normalize: find MSB
				int msb = _significant.msb(); // zero case has been taken care off above
//				std::cout << "div : " << to_binary(*this) << std::endl;
//				std::cout << "msb : " << msb << std::endl;
				int leftShift = static_cast<int>(bfbits) - 2 - msb;
				_significant <<= leftShift;
				_scale -= leftShift;
			}
		}
		if constexpr (_trace_btriple_div) {
			std::cout << "blocktriple normalized div\n";
			std::cout << typeid(*this).name() << '\n';
			std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << "div : " << to_binary(*this) << " : " << *this << '\n';
		}
	}

private:
	// special cases to keep track of
	bool _nan; // most dominant state
	bool _inf; // second most dominant state
	bool _zero;// third most dominant special case

	// the triple (sign, scale, significant)
	bool _sign;
	int  _scale;

protected:
	Significant _significant;

	// helpers

private:
	/// <summary>
	/// round a set of source bits to the present representation.
	/// srcbits is the number of bits of significant in the source representation
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
			// float significant: 24bits : 0bhfff'ffff'ffff'ffff'ffff'ffff; h is hidden, f are fraction bits
			// blocktriple target: 10bits: 0bhfff'ffff'fff    hidden bit is implicit, 10 fraction bits
			//                                           lg'rs
			//                             0b0000'0000'0001'0000'0000'0000; guard mask == 1 << srcbits - fbits - 2: 24 - 10 - 2 = 12
			constexpr unsigned upper = ieee754_parameter<Real>::nbits + 2;
			constexpr int offset = fbits + 2;
			constexpr int fullShift = srcbits - offset;  // srcbits includes the hidden bit, fbits does not
			constexpr unsigned shift = (fullShift < 0 ? 0 : fullShift);
			uint64_t mask = (srcbits < offset ? 0 : (1ull << shift));
//			std::cout << "raw     : " << to_binary(raw, 64, true) << '\n';
//			std::cout << "guard   : " << to_binary(mask, 64, true) << '\n';
			bool guard = (mask & raw);
			mask >>= 1;
//			std::cout << "round   : " << to_binary(mask, 64, true) << '\n';
			bool round = (mask & raw);
			if constexpr (shift > 1 && shift < upper) { // protect against a negative shift
				uint64_t allones(uint64_t(~0));
				mask = uint64_t(allones << (shift - 1));
				mask = ~mask;
			}
			else {
				mask = 0;
			}
//			std::cout << "sticky  : " << to_binary(mask, 64, true) << '\n';
			bool sticky = (mask & raw);

			raw >>= (shift + 1);  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1);
//			std::cout << "raw     : " << to_binary(raw, 64, true) << '\n';

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
//			std::cout << "raw     : " << to_binary(raw, 64, true) << '\n';
			constexpr unsigned shift = fbits - srcbits;
			if constexpr (shift < ieee754_parameter<Real>::nbits) {
				raw <<= shift;
			}
			else {
#if !BIT_CAST_IS_CONSTEXPR
				std::cerr << "round: shift " << shift << " is too large (>= 64)\n";
#endif
			}
//			std::cout << "upshift : " << to_binary(raw, 64, true) << '\n';
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
			_significant.setradix(fbits);
			break;
		case BlockTripleOperator::MUL:
			_significant.setradix(fbits);
			break;
		case BlockTripleOperator::DIV:
			_significant.setradix(2 * fbits);
			break;
		case BlockTripleOperator::SQRT:
			_significant.setradix(2 * fbits);
			break;
		case BlockTripleOperator::REP:
			_significant.setradix(fbits);
			break;
		}
		_significant.setbits(rounded_bits);
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
			_significant.setradix(fbits);
			break;
		case BlockTripleOperator::MUL:
			_significant.setradix(fbits);
			break;
		case BlockTripleOperator::DIV:
			_significant.setradix(2 * fbits);
			break;
		case BlockTripleOperator::SQRT:
			_significant.setradix(2 * fbits);
			break;
		case BlockTripleOperator::REP:
			_significant.setradix(fbits);
			break;
		}
		_significant.setbits(rounded_bits);
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
			if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::snanmask) ||
				rawFraction == (ieee754_parameter<Real>::fmask & (ieee754_parameter<Real>::qnanmask | ieee754_parameter<Real>::snanmask))) {
				// 1.11111111.00000000.......00000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000.......00000001 signalling nan
				// 0.11111111.10000000.......00000001 signalling nan
				// NAN_TYPE_SIGNALLING;
				_nan = true;
				_inf = false; 
				_sign = true; // this is the encoding of a signalling NaN
				_scale = 0;
				return *this;
			}
			if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
				// 1.11111111.10000000.......00000000 quiet nan
				// 0.11111111.10000000.......00000000 quiet nan
				// NAN_TYPE_QUIET);
				_nan = true;
				_inf = false; 
				_sign = false; // this is the encoding of a quiet NaN
				_scale = 0;
				return *this;
			}
			if (rawFraction == 0ull) {
				// 1.11111111.0000000.......000000000 -inf
				// 0.11111111.0000000.......000000000 +inf
				_nan = false;
				_inf = true;
				_sign = s;  // + or - infinity
				_scale = 10000;
				return *this;
			}
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
			std::cerr << "subnormal value TBD\n";
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
			_significant.setbits(rounded_bits);
			switch(op) {
			case BlockTripleOperator::REP:
				_significant.setradix(fbits);
				// std::cout << "rhs = " << rhs << " : significant = " << _significant << '\n';
				break;
			case BlockTripleOperator::ADD:
				_significant.setradix(abits);
				_significant <<= rbits;
				break;
			case BlockTripleOperator::MUL:
				_significant.setradix(2*fbits);
				_significant <<= fbits;
				break;
			case BlockTripleOperator::DIV:
				_significant.setradix(2*fbits);
				_significant <<= fbits;
				break;
			case BlockTripleOperator::SQRT:
				_significant.setradix(2 * fbits);
				_significant <<= fbits;
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
		Real v = Real(_significant);
		v *= std::pow(Real(2.0f), Real(_scale));
		Real s = (_sign ? Real(-1.0) : Real(1.0));
		return s * v;
	}

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned ffbits, BlockTripleOperator oop, typename bbt>
	friend std::istream& operator>> (std::istream& istr, blocktriple<ffbits, oop, bbt>& a);

	// declare as friends to avoid needing a marshalling function to get significant bits out
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

// blocktriple ostream operator
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<fbits, op, bt>& a) {
	if (a.isnan()) {
		if (a.isneg()) {
			ostr << "snan";
		}
		else {
			ostr << "qnan";
		}
	}
	else {
		if (a.isinf()) {
			if (a.isneg()) {
				ostr << "-inf";
			}
			else {
				ostr << "+inf";
			}
		}
		else {
			ostr << double(a);
		}
	}
	return ostr;
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
inline std::istream& operator>> (std::istream& istr, const blocktriple<fbits, op, bt>& a) {
	istr >> a._fraction;
	return istr;
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
inline bool operator==(const blocktriple<fbits, op, bt>& lhs, const blocktriple<fbits, op, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._significant == rhs._significant && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
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
					if (lhs._significant == rhs._significant) return false; // they are the same value
					if (lhs._significant > rhs._significant) {
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
					if (lhs._significant == rhs._significant) return false; // they are the same value
					if (lhs._significant > rhs._significant) {
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
std::string to_binary(const sw::universal::blocktriple<fbits, op, bt>& a, bool nibbleMarker = false) {
	return to_triple(a, nibbleMarker);
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string to_triple(const blocktriple<fbits, op, bt>& a, bool nibbleMarker = true) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ");
	s << std::setw(3) << a._scale << ", ";
	s << to_binary(a._significant, nibbleMarker) << ')';
	return s.str();
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
blocktriple<fbits> abs(const blocktriple<fbits, op, bt>& a) {
	blocktriple<fbits, op, bt> absolute(a);
	absolute.setpos();
	return absolute;
}


}} // namespace sw::universal
