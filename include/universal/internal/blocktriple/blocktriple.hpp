#pragma once
// blocktriple.hpp: definition of a (sign, scale, significant) representation of a generic floating-point value
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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
#if !defined(BIT_CAST_SUPPORT)
#pragma message("BIT_CAST_SUPPORT is not defined")
#define BIT_CAST_SUPPORT 0
#endif
#if !defined(CONSTEXPRESSION)
#define CONSTEXPRESSION
#endif

// dependent types for stand-alone use of this class
#include <universal/native/integers.hpp> // to_binary(uint64_t)
#include <universal/native/ieee754.hpp>
#include <universal/native/subnormal.hpp>
#include <universal/native/bit_functions.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
// blocktriple operation trace options
#include <universal/internal/blocktriple/trace_constants.hpp>

namespace sw::universal {

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
	blockfraction = 00h.ffffeee <- three bits before radix point, fraction bits plus 3 rounding bits
	size_t bfbits = fbits + 3;

	for multiply
	size_t bfbits = 2*fhbits;
	*/

 // operator specialization tag for blocktriple
enum class BlockTripleOperator { ADD, MUL, DIV, SQRT, REPRESENTATION };

// Forward definitions
template<size_t fbits, BlockTripleOperator op, typename bt> class blocktriple;
template<size_t fbits, BlockTripleOperator op, typename bt> blocktriple<fbits, op, bt> abs(const blocktriple<fbits, op, bt>& v);

template<size_t fbits, BlockTripleOperator op, typename bt>
blocktriple<fbits, op, bt>& convert(unsigned long long uint, blocktriple<fbits, op, bt>& tgt) {
	return tgt;
}

// Generate a type tag for this type: blocktriple<fbits, operator, unsigned int>
template<size_t fbits, BlockTripleOperator op, typename bt>
std::string type_tag(const blocktriple<fbits, op, bt>& v) {
	std::stringstream s;
	s << "blocktriple<"
		<< fbits << ", ";
	switch (op) {
	case BlockTripleOperator::REPRESENTATION:
		s << "BlockTripleOperator::REPRESENTATION, ";
		break;
	case BlockTripleOperator::ADD:
		s << "BlockTripleOperator::ADD, ";
		break;
	case BlockTripleOperator::MUL:
		s << "BlockTripleOperator::MUL, ";
		break;
	case BlockTripleOperator::DIV:
		s << "BlockTripleOperator::DIV, ";
		break;
	case BlockTripleOperator::SQRT:
		s << "BlockTripleOperator::SQRT, ";
		break;
	default:
		s << "unknown operator, ";
	}
	s << typeid(bt).name() << '>';
	if (v.iszero()) s << ' ';
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
/// <typeparam name="fractionbits">number of fraction bits in the significant</typeparam>
/// <typeparam name="bt">block type: one of [uint8_t, uint16_t, uint32_t, uint64_t]</typeparam>
template<size_t fractionbits, BlockTripleOperator _op = BlockTripleOperator::ADD, typename bt = uint32_t>
class blocktriple {
public:
	static constexpr size_t nbits = fractionbits;  // a convenience and consistency alias
	static constexpr size_t fbits = fractionbits;
	typedef bt BlockType;
	static constexpr BlockTripleOperator op = _op;

	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr size_t storageMask = (0xFFFF'FFFF'FFFF'FFFFull >> (64ull - bitsInBlock));

	static constexpr size_t MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken

	static constexpr size_t fhbits = fbits + 1;            // size of all bits
	static constexpr size_t abits = fbits + 3ull;          // size of the addend
	static constexpr size_t mbits = 2ull * fhbits;         // size of the multiplier output
	static constexpr size_t divbits = 3ull * fbits + 4ull; // size of the divider output
	static constexpr size_t sqrtbits = 2ull * fhbits;      // size of the square root output
	// we transform input operands into the operation's target output size
	// so that everything is aligned correctly before the operation starts.
	static constexpr size_t bfbits =
		(op == BlockTripleOperator::ADD ? abits :
			(op == BlockTripleOperator::MUL ? mbits :
				(op == BlockTripleOperator::DIV ? divbits :
					(op == BlockTripleOperator::SQRT ? sqrtbits : fhbits))));  // REPRESENTATION is the fall through condition
	// radix point of the OUTPUT of an operator
	static constexpr int radix =
		(op == BlockTripleOperator::ADD ? static_cast<int>(fbits) :
			(op == BlockTripleOperator::MUL ? static_cast<int>(2*fbits) :
				(op == BlockTripleOperator::DIV ? static_cast<int>(fbits) :
					(op == BlockTripleOperator::SQRT ? static_cast<int>(sqrtbits) : static_cast<int>(fbits)))));  // REPRESENTATION is the fall through condition
	static constexpr BitEncoding encoding =
		(op == BlockTripleOperator::ADD ? BitEncoding::Twos :
			(op == BlockTripleOperator::MUL ? BitEncoding::Ones :
				(op == BlockTripleOperator::DIV ? BitEncoding::Ones :
					(op == BlockTripleOperator::SQRT ? BitEncoding::Ones : BitEncoding::Ones))));
	static constexpr size_t normalBits = (bfbits < 64 ? bfbits : 64);
	static constexpr size_t normalFormMask = (normalBits == 64) ? 0xFFFF'FFFF'FFFF'FFFFull : (~(0xFFFF'FFFF'FFFF'FFFFull << (normalBits - 1)));
	// to maximize performance, can we make the default blocktype a uint64_t?
	// storage unit for block arithmetic needs to be uin32_t until we can figure out 
	// how to manage carry propagation on uint64_t using intrinsics/assembly code
	using Frac = sw::universal::blockfraction<bfbits, bt, encoding>;

	static constexpr bt ALL_ONES = bt(~0);
	// generate the special case overflow pattern mask when representation is nbits + 1 < 64
	static constexpr size_t maxbits = (nbits + 1) < 63 ? (nbits + 1) : 63;
	static constexpr size_t overflowPattern = (maxbits < 63) ? (1ull << maxbits) : 0ull; // overflow of 1.11111 to 10.0000

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
	constexpr blocktriple& operator=(float rhs)              noexcept { return convert_float(rhs); }
	constexpr blocktriple& operator=(double rhs)             noexcept { return convert_double(rhs); }


	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	CONSTEXPRESSION blocktriple(long double iv)				noexcept { *this = iv; }
	CONSTEXPRESSION blocktriple& operator=(long double rhs) noexcept { return *this = (long double)(rhs); };
	explicit operator long double() const noexcept { return to_native<long double>(); }
#endif

	// operators
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

	/// <summary>
	/// roundingDecision returns a pair<bool, size_t> to direct the rounding and right shift
	/// </summary>
	/// <param name="adjustment">adjustment for subnormals </param>
	/// <returns>std::pair<bool, size_t> of rounding direction (up is true, down is false), and the right shift</returns>
	constexpr std::pair<bool, size_t> roundingDecision(int adjustment = 0) const noexcept {
		// preconditions: blocktriple is in 1's complement form, and not a denorm
		// this implies that the scale of the significant is 0 or 1
		size_t significantScale = static_cast<size_t>(significantscale());
		// find the shift that gets us to the lsb
		size_t shift = significantScale + static_cast<size_t>(radix) - fbits;
#ifdef PERFORMANCE_OPTIMIZATION
		if constexpr (bfbits < 65) {
			uint64_t fracbits = _significant.get_ull(); // get all the bits, including the integer bits
			//  ... lsb | guard  round sticky   round
			//       x     0       x     x       down
			//       0     1       0     0       down  round to even
			//       1     1       0     0        up   round to even
			//       x     1       0     1        up
			uint64_t mask = (1ull << (shift + adjustment);
			bool lsb = fracbits & mask;
			mask >>= 1;
			bool guard = fracbits & mask;
			mask >>= 1;
			bool round = fracbits & mask;
			if (shift < 2) {
				mask = 0xFFFF'FFFF'FFFF'FFFFull;
			}
			else {
				mask = 0xFFFF'FFFF'FFFF'FFFFull << (shift - 2);
			}
			mask = ~mask;
			//				std::cout << "fracbits    : " << to_binary(fracbits) << std::endl;
			//				std::cout << "sticky mask : " << to_binary(mask) << std::endl;
			bool sticky = fracbits & mask;
			roundup = (guard && (lsb || (round || sticky)));
		}
		else {
			roundup = _significant.roundingMode(shift);
		}
#endif
		bool roundup = _significant.roundingMode(shift + adjustment);
		return std::pair<bool, size_t>(roundup, shift + adjustment);
	}
	// apply a 2's complement recoding of the fraction bits
	inline constexpr blocktriple& twosComplement() noexcept {
		_significant.twosComplement();
		return *this;
	}

	// modifiers
	constexpr void clear() noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		_sign = false;
		_scale = 0;
		_significant.clear();
	}
	constexpr void setzero(bool sign = false) noexcept {
		clear();
		_sign = sign;
	}
	constexpr void setnan(bool sign = true) noexcept {
		clear();
		_nan = true;
		_inf = false;
		_zero = false;
		_sign = sign;   // if true, signalling NaN, otherwise quiet
	}
	constexpr void setinf(bool sign = true) noexcept {
		clear();
		_inf = true;
		_zero = false;
		_sign = sign;
	}
	constexpr void setpos() noexcept { _sign = false; }
	constexpr void setnormal() noexcept {
		_nan = false;
		_inf = false;
		_zero = false;
	}
	constexpr void setsign(bool s) noexcept { _sign = s; }
	constexpr void setscale(int scale) noexcept { _scale = scale; }
	constexpr void setradix(int _radix) noexcept { _significant.setradix(_radix); }
	constexpr void setbit(size_t index, bool v = true) noexcept { _significant.setbit(index, v); }
	/// <summary>
	/// set the bits of the significant, given raw fraction bits. only works for bfbits < 64
	/// </summary>
	/// <param name="raw">raw bit pattern representing the fraction bits</param>
	/// <returns></returns>
	constexpr void setbits(uint64_t raw) noexcept {
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
	constexpr void setblock(size_t i, const bt& block) {
		_significant.setblock(i, block);
	}

	// selectors
	inline constexpr bool isnan()            const noexcept { return _nan; }
	inline constexpr bool isinf()            const noexcept { return _inf; }
	inline constexpr bool iszero()           const noexcept { return _zero; }
	inline constexpr bool ispos()            const noexcept { return !_sign; }
	inline constexpr bool isneg()            const noexcept { return _sign; }
	inline constexpr bool sign()             const noexcept { return _sign; }
	inline constexpr int  scale()            const noexcept { return _scale; }
	inline constexpr int  significantscale() const noexcept {
		int sigScale = 0;
		for (int i = bfbits - 1; i >= radix; --i) {
			if (_significant.at(static_cast<size_t>(i))) {
				sigScale = i - radix;
				break;
			}
		}
		return sigScale;
	}
	inline constexpr Frac significant()      const noexcept { return _significant; }
	// specialty function to offer a fast path to get the fraction bits out of the representation
	// to convert to a target number system: only valid for bfbits <= 64
	inline constexpr uint64_t fraction_ull() const noexcept { return _significant.fraction_ull(); }
	inline constexpr uint64_t get_ull()      const noexcept { return _significant.get_ull(); }
	// fraction bit accessors
	inline constexpr bool at(size_t index)   const noexcept { return _significant.at(index); }
	inline constexpr bool test(size_t index) const noexcept { return _significant.at(index); }

	// conversion operators
	explicit operator float()       const noexcept { return to_native<float>(); }
	explicit operator double()      const noexcept { return to_native<double>(); }


	/////////////////////////////////////////////////////////////
	// ALU operators

	/// <summary>
	/// add two fixed-point numbers with fbits fraction bits 
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
		int expDiff = lhs_scale - rhs_scale;
		if (expDiff < 0) {
			lhs >>= -expDiff;
		}
		else if (expDiff > 0) {
			rhs >>= expDiff;
		}
		if (lhs.isneg()) lhs._significant.twosComplement();
		if (rhs.isneg()) rhs._significant.twosComplement();

		_significant.add(lhs._significant, rhs._significant);  // do the bit arithmetic manipulation
		_significant.setradix(radix);                          // set the radix interpretation of the output

		if constexpr (_trace_btriple_add) {
			std::cout << "blockfraction unrounded add\n";
			std::cout << typeid(lhs._significant).name() << '\n';
			std::cout << "lhs significant : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs significant : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << typeid(_significant).name() << '\n';
			std::cout << "sum significant : " << to_binary(*this) << " : " << *this << '\n';
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
//				std::cout << "sum : " << to_binary(*this) << std::endl;
//				std::cout << "msb : " << msb << std::endl;
				int leftShift = static_cast<int>(bfbits) - 3 - msb;
				_significant <<= leftShift;
				_scale -= leftShift;
			}
		}
		if constexpr (_trace_btriple_add) {
			std::cout << "blocktriple normalized add\n";
			std::cout << typeid(lhs).name() << '\n';
			std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << typeid(*this).name() << '\n';
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
			std::cout << "blockfraction unrounded mul\n";
			std::cout << typeid(lhs._significant).name() << '\n';
			std::cout << "lhs significant : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs significant : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << typeid(_significant).name() << '\n';
			std::cout << "mul significant : " << to_binary(*this) << " : " << *this << '\n';  // <-- the scale of this representation is not yet set
		}
		if (_significant.iszero()) {
			clear();
		}
		else {
			_zero = false;
			_scale = scale_of_result;
			_sign = (lhs.sign() == rhs.sign()) ? false : true;
			if (_significant.test(bfbits - 1)) { // test for carry
				bool roundup = _significant.test(1) && _significant.test(0);
				_scale += 1;
				_significant >>= 1;
				if (roundup) _significant.increment();
			}
			else if (_significant.test(bfbits - 2)) {
//				all good, found a normal form
			}
			else {
				// found a denormalized form, thus need to normalize: find MSB
				int msb = _significant.msb(); // zero case has been taken care off above
//				std::cout << "mul : " << to_binary(*this) << std::endl;
//				std::cout << "msb : " << msb << std::endl;
				int leftShift = static_cast<int>(bfbits) - 3 - msb;
				_significant <<= leftShift;
				_scale -= leftShift;
			}
		}
		if constexpr (_trace_btriple_mul) {
			std::cout << "blocktriple normalized mul\n";
			std::cout << typeid(lhs).name() << '\n';
			std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << typeid(*this).name() << '\n';
			std::cout << "mul : " << to_binary(*this) << " : " << *this << '\n';
		}
	}

	void div(blocktriple& lhs, blocktriple& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = lhs_scale + rhs_scale;

		// avoid copy by directly manipulating the fraction bits of the arguments
		_significant.mul(lhs._significant, rhs._significant);

		if constexpr (_trace_btriple_div) {
			std::cout << "blockfraction unrounded div\n";
			std::cout << typeid(lhs._significant).name() << '\n';
			std::cout << "lhs significant : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs significant : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << typeid(_significant).name() << '\n';
			std::cout << "div significant : " << to_binary(*this) << " : " << *this << '\n';  // <-- the scale of this representation is not yet set
		}
		if (_significant.iszero()) {
			clear();
		}
		else {
			_zero = false;
			_scale = scale_of_result;
			if (_significant.test(bfbits - 1)) { // test for carry
				_scale += 1;
				_significant >>= 2; // TODO: do we need to round on bits shifted away?
			}
			else if (_significant.test(bfbits - 2)) { // check for the hidden bit
				_significant >>= 1;
			}
			else {
				// found a denormalized form, thus need to normalize: find MSB
				int msb = _significant.msb(); // zero case has been taken care off above
//				std::cout << "div : " << to_binary(*this) << std::endl;
//				std::cout << "msb : " << msb << std::endl;
				int leftShift = static_cast<int>(bfbits) - 3 - msb;
				_significant <<= leftShift;
				_scale -= leftShift;
			}
		}
		if constexpr (_trace_btriple_div) {
			std::cout << "blocktriple normalized div\n";
			std::cout << typeid(lhs).name() << '\n';
			std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << typeid(*this).name() << '\n';
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
	Frac _significant;

	// helpers

private:
	/// <summary>
/// round a set of source bits to the present representation.
/// srcbits is the number of bits of significant in the source representation
/// round<> is intended only for rounding raw IEEE-754 bits
/// </summary>
/// <typeparam name="StorageType">type of incoming bits</typeparam>
/// <param name="raw">the raw unrounded bits</param>
/// <returns></returns>
	template<size_t srcbits, typename StorageType>
	constexpr StorageType round(StorageType raw) noexcept {
		if constexpr (nbits < srcbits) {
			// round to even: lsb guard round sticky
			// collect guard, round, and sticky bits
			// this same logic will work for the case where
			// we only have a guard bit and no round and/or sticky bits
			// because the mask logic will make round and sticky both 0

			// example: rounding the bits of a float to our nbits 
			// float significant: 24bits : 0bhfff'ffff'ffff'ffff'ffff'ffff; h is hidden, f is fraction bit
			// blocktriple target: 10bits: 0bhfff'ffff'fff    hidden bit is implicit, 10 fraction bits
			//                                           lg'rs
			//                             0b0000'0000'0001'0000'0000'0000; guard mask == 1 << srcbits - nbits - 2: 24 - 10 - 2 = 12
			constexpr uint32_t upper = 8 * sizeof(StorageType) + 2;
			constexpr uint32_t shift = srcbits - nbits - 2ull;  // srcbits includes the hidden bit, nbits does not
			StorageType mask = (StorageType{ 1ull } << shift);
//			std::cout << "raw   : " << to_binary(raw, sizeof(StorageType)*8, true) << '\n';
//			std::cout << "guard : " << to_binary(mask, sizeof(StorageType) * 8, true) << '\n';
			bool guard = (mask & raw);
			mask >>= 1;
//			std::cout << "round : " << to_binary(mask, sizeof(StorageType) * 8, true) << '\n';
			bool round = (mask & raw);
			if constexpr (shift > 1 && shift < upper) { // protect against a negative shift
				StorageType allones(StorageType(~0));
				mask = StorageType(allones << (shift - 1));
				mask = ~mask;
			}
			else {
				mask = 0;
			}
//			std::cout << "sticky: " << to_binary(mask, sizeof(StorageType) * 8, true) << '\n';
			bool sticky = (mask & raw);

			raw >>= (shift + 1);  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1);
//			std::cout << "raw   : " << to_binary(raw, sizeof(StorageType) * 8, true) << '\n';

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
			constexpr size_t shift = nbits - srcbits;
			if constexpr (shift < sizeof(StorageType)) {
				raw <<= shift;
			}
			else {
#if !BIT_CAST_SUPPORT
				std::cerr << "round: shift " << shift << " is too large (>= " << sizeof(StorageType) << ")\n";
#endif
			}
		}
//		std::cout << "final : " << to_binary(raw, sizeof(StorageType) * 8, true) << '\n';
		return static_cast<StorageType>(raw);
	}

	template<typename Ty>
	constexpr inline blocktriple& convert_unsigned_integer(const Ty& rhs) noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = false;
		uint64_t raw = static_cast<uint64_t>(rhs);
		_scale = static_cast<int>(findMostSignificantBit(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr size_t sizeInBits = 8 * sizeof(Ty);
		uint64_t shift = sizeInBits - int64_t(_scale) - 1;
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
			_significant.setradix(3 * fbits);
			break;
		case BlockTripleOperator::SQRT:
			_significant.setradix(2 * fbits);
			break;
		case BlockTripleOperator::REPRESENTATION:
			_significant.setradix(fbits);
			break;
		}
		_significant.setbits(rounded_bits);
		return *this;
	}
	template<typename Ty>
	constexpr inline blocktriple& convert_signed_integer(const Ty& rhs) noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = (rhs < 0);
		uint64_t raw = static_cast<uint64_t>(_sign ? -rhs : rhs);
		_scale = static_cast<int>(findMostSignificantBit(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr size_t sizeInBits = 8 * sizeof(Ty);
		uint64_t shift = sizeInBits - int64_t(_scale) - 1;
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
			_significant.setradix(3*fbits);
			break;
		case BlockTripleOperator::SQRT:
			_significant.setradix(2*fbits);
			break;
		case BlockTripleOperator::REPRESENTATION:
			_significant.setradix(fbits);
			break;
		}
		_significant.setbits(rounded_bits);
		return *this;
	}

	inline CONSTEXPRESSION blocktriple& convert_float(float rhs) noexcept {   // TODO: deal with subnormals and inf
#if BIT_CAST_SUPPORT
		// normal number
		uint32_t bc = std::bit_cast<uint32_t>(rhs);
		bool s = (0x8000'0000ul & bc);
		uint32_t raw_exp = static_cast<uint32_t>((0x7F80'0000ul & bc) >> 23);
		uint32_t raw = (1ul << 23) | (0x007F'FFFFul & bc);
#else // !BIT_CAST_SUPPORT
		float_decoder decoder;
		decoder.f = rhs;
		bool s = decoder.parts.sign ? true : false;
		uint32_t raw_exp = decoder.parts.exponent;
		uint32_t raw = (1ul << 23) | decoder.parts.fraction;
#endif // !BIT_CAST_SUPPORT

		// special case handling
		if (raw_exp == 0xFFu) { // special cases
			if (raw == 1ul || raw == 0x00C0'0001ul) {
				// 1.11111111.00000000000000000000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000000000000000001 signalling nan
				// 0.11111111.10000000000000000000001 signalling nan
				// NAN_TYPE_SIGNALLING;
				_nan = true;
				_inf = false; 
				_sign = true; // this is the encoding of a signalling NaN
				return *this;
			}
			if (raw == 0x00C0'0000ul) {
				// 1.11111111.10000000000000000000000 quiet nan
				// 0.11111111.10000000000000000000000 quiet nan
				// NAN_TYPE_QUIET);
				_nan = true;
				_inf = false; 
				_sign = false; // this is the encoding of a quiet NaN
				return *this;
			}
			if (raw == 0ul) {
				// 1.11111111.00000000000000000000000 -inf
				// 0.11111111.00000000000000000000000 +inf
				_nan = false;
				_inf = true;
				_sign = s;  // + or - infinity
				return *this;
			}
		}
		if (rhs == 0.0f) { // IEEE rule: this is valid for + and - 0.0
			_nan = false;
			_inf = false;
			_zero = true;
			_sign = s;
			return *this;
		}
		// normal number, not zero
		_nan = false;
		_inf = false;
		_zero = false;
		_sign = s;
		_scale = static_cast<int>(raw_exp) - 127;
		uint32_t rounded_bits = round<24, uint32_t>(raw);
		_significant.setradix(fbits); // round maps the fraction bits to the radix at fbits
		_significant.setbits(rounded_bits);
		return *this;
	}

	inline CONSTEXPRESSION blocktriple& convert_double(double rhs) noexcept { // TODO: deal with subnormals and inf
#if BIT_CAST_SUPPORT
		uint64_t bc = std::bit_cast<uint64_t>(rhs);
		bool s = (0x8000'0000'0000'0000ull & bc);
		uint32_t raw_exp = static_cast<uint32_t>((0x7FF0'0000'0000'0000ull & bc) >> 52);
		uint64_t raw = (1ull << 52) | (0x000F'FFFF'FFFF'FFFFull & bc);
#else
		double_decoder decoder;
		decoder.d = rhs;
		bool s = decoder.parts.sign ? true : false;
		uint64_t raw_exp = decoder.parts.exponent;
		uint64_t raw = (1ull << 52) | decoder.parts.fraction;
#endif // !BIT_CAST_SUPPORT

		// special case handling
		if (raw_exp == 0x7FFu) { // special cases
			if (raw == 1ul || raw == 0x0040'0001ul) {
				// 1.111'1111'1111.0000000000...0000000001 signalling nan
				// 0.111'1111'1111.0000000000...0000000001 signalling nan
				// MSVC
				// 1.111'1111'1111.1000000000...0000000001 signalling nan
				// 0.111'1111'1111.1000000000...0000000001 signalling nan
				// NAN_TYPE_SIGNALLING;
				_nan = true;
				_inf = true; // this is the encoding of a signalling NaN
				return *this;
	}
			if (raw == 0x0008'0000'0000'0000ull) {
				// 1.111'1111'1111.1000000000...0000000000 quiet nan
				// 0.111'1111'1111.1000000000...0000000000 quiet nan
				// NAN_TYPE_QUIET);
				_nan = true;
				_inf = false; // this is the encoding of a quiet NaN
				return *this;
			}
			if (raw == 0ul) {
				// 1.11111111.00000000000000000000000 -inf
				// 0.11111111.00000000000000000000000 +inf
				_nan = false;
				_inf = true;
				_sign = s;  // + or - infinity
				return *this;
			}
		}
		if (rhs == 0.0f) { // IEEE rule: this is valid for + and - 0.0
			_nan = false;
			_inf = false;
			_zero = true;
			_sign = s;
			return *this;
		}
		// normal number
		_nan = false;
		_inf = false;
		_zero = false;
		_sign = s;
		_scale = static_cast<int>(raw_exp) - 1023;
		uint64_t rounded_bits = round<53, uint64_t>(raw); // round manipulates _scale if needed
		_significant.setradix(fbits); // round maps the fraction bits to the radix at fbits
		_significant.setbits(rounded_bits);
		return *this;
	}

	template<typename Real>
	constexpr Real to_native() const noexcept {
		if (_nan) { if (_sign) return std::numeric_limits<Real>::signaling_NaN(); else return std::numeric_limits<Real>::quiet_NaN(); }
		if (_inf) { if (_sign) return -std::numeric_limits<Real>::infinity(); else return std::numeric_limits<Real>::infinity(); }
		if (_zero) { if (_sign) return -Real(0.0f); else return Real(0.0f); }
		Real v = Real(_significant);
		v *= std::pow(Real(2.0f), Real(_scale));
		return (_sign ? -v : v);
	}

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend std::istream& operator>> (std::istream& istr, blocktriple<nnbits, oop, bbt>& a);

	// declare as friends to avoid needing a marshalling function to get significant bits out
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend std::string to_binary(const blocktriple<nnbits, oop, bbt>&, bool);
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend std::string to_triple(const blocktriple<nnbits, oop, bbt>&, bool);

	// logic operators
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend bool operator==(const blocktriple<nnbits, oop, bbt>& lhs, const blocktriple<nnbits, oop, bbt>& rhs);
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend bool operator!=(const blocktriple<nnbits, oop, bbt>& lhs, const blocktriple<nnbits, oop, bbt>& rhs);
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend bool operator< (const blocktriple<nnbits, oop, bbt>& lhs, const blocktriple<nnbits, oop, bbt>& rhs);
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend bool operator> (const blocktriple<nnbits, oop, bbt>& lhs, const blocktriple<nnbits, oop, bbt>& rhs);
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend bool operator<=(const blocktriple<nnbits, oop, bbt>& lhs, const blocktriple<nnbits, oop, bbt>& rhs);
	template<size_t nnbits, BlockTripleOperator oop, typename bbt>
	friend bool operator>=(const blocktriple<nnbits, oop, bbt>& lhs, const blocktriple<nnbits, oop, bbt>& rhs);
};

////////////////////// operators

// BlockTripleOperator ostream operator
inline std::ostream& operator<<(std::ostream& ostr, const BlockTripleOperator& op) {
	switch (op) {
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
	case BlockTripleOperator::REPRESENTATION:
		ostr << "REP";
		break;
	default:
		ostr << "NOP";
	}
	return ostr;
}

// blocktriple ostream operator
template<size_t nbits, BlockTripleOperator op, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<nbits, op, bt>& a) {
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

template<size_t nbits, BlockTripleOperator op, typename bt>
inline std::istream& operator>> (std::istream& istr, const blocktriple<nbits, op, bt>& a) {
	istr >> a._fraction;
	return istr;
}

template<size_t nbits, BlockTripleOperator op, typename bt>
inline bool operator==(const blocktriple<nbits, op, bt>& lhs, const blocktriple<nbits, op, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._significant == rhs._significant && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
template<size_t nbits, BlockTripleOperator op, typename bt>
inline bool operator!=(const blocktriple<nbits, op, bt>& lhs, const blocktriple<nbits, op, bt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, BlockTripleOperator op, typename bt>
inline bool operator< (const blocktriple<nbits, op, bt>& lhs, const blocktriple<nbits, op, bt>& rhs) {
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
template<size_t nbits, BlockTripleOperator op, typename bt>
inline bool operator> (const blocktriple<nbits, op, bt>& lhs, const blocktriple<nbits, op, bt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, BlockTripleOperator op, typename bt>
inline bool operator<=(const blocktriple<nbits, op, bt>& lhs, const blocktriple<nbits, op, bt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, BlockTripleOperator op, typename bt>
inline bool operator>=(const blocktriple<nbits, op, bt>& lhs, const blocktriple<nbits, op, bt>& rhs) { return !operator< (lhs, rhs); }


////////////////////////////////// string conversion functions //////////////////////////////

template<size_t nbits, BlockTripleOperator op, typename bt>
std::string to_binary(const sw::universal::blocktriple<nbits, op, bt>& a, bool nibbleMarker = true) {
	return to_triple(a, nibbleMarker);
}

template<size_t nbits, BlockTripleOperator op, typename bt>
std::string to_triple(const blocktriple<nbits, op, bt>& a, bool nibbleMarker = true) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ");
	s << std::setw(3) << a._scale << ", ";
	s << to_binary(a._significant, nibbleMarker) << ')';
	return s.str();
}

template<size_t nbits, BlockTripleOperator op, typename bt>
blocktriple<nbits> abs(const blocktriple<nbits, op, bt>& a) {
	blocktriple<nbits> absolute(a);
	absolute.setpos();
	return absolute;
}


}  // namespace sw::universal
