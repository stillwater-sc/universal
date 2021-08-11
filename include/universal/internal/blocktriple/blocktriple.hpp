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

// Forward definitions
template<size_t fbits, typename bt> class blocktriple;
template<size_t fbits, typename bt> blocktriple<fbits, bt> abs(const blocktriple<fbits, bt>& v);

template<size_t fbits, typename bt>
blocktriple<fbits, bt>& convert(unsigned long long uint, blocktriple<fbits, bt>& tgt) {
	return tgt;
}

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


/// <summary>
/// Generalized blocktriple representing a (sign, scale, significant) with unrounded arithmetic
/// </summary>
/// <typeparam name="fractionbits">number of fraction bits in the significant</typeparam>
/// <typeparam name="bt">block type: one of [uint8_t, uint16_t, uint32_t, uint64_t]</typeparam>
template<size_t fractionbits, typename bt = uint32_t> 
class blocktriple {
public:
	static constexpr size_t nbits = fractionbits;  // a convenience and consistency alias
	static constexpr size_t fbits = fractionbits;
	typedef bt BlockType;
	static constexpr size_t bfbits = fbits + 3;
	// to maximize performance, can we make the default blocktype a uint64_t?
	// storage unit for block arithmetic needs to be uin32_t until we can figure out 
	// how to manage carry propagation on uint64_t using intrinsics/assembly code
	using Frac = sw::universal::blockfraction<bfbits, bt>;

	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr size_t storageMask = (0xFFFF'FFFF'FFFF'FFFFull >> (64ull - bitsInBlock));

	static constexpr size_t MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken

	static constexpr size_t fhbits = fbits + 1;            // size of all bits
	static constexpr size_t abits = fhbits + 3ull;         // size of the addend
	static constexpr size_t mbits = 2ull * fhbits;         // size of the multiplier output
	static constexpr size_t divbits = 3ull * fbits + 4ull; // size of the divider output
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
		_sign{ false }, _scale{ 0 } {} // _significant uses default constructor

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
	constexpr blocktriple(long double iv)        noexcept { *this = iv; }

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
	constexpr blocktriple& operator=(long double rhs)        noexcept { return *this = double(rhs); };
	
	// align the blocktriple
	inline constexpr void align(int rightShift) noexcept {
		_scale += rightShift;
		_significant >>= rightShift;
	}

	// apply a 2's complement recoding of the fraction bits
	inline constexpr void twosComplement() noexcept {
		_significant.twosComplement();
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
	constexpr void setbit(size_t index, bool v = true) noexcept { _significant.setbit(index, v); }
	/// <summary>
	/// set the bits of the significant, given raw fraction bits. only works for bfbits < 64
	/// </summary>
	/// <param name="raw">fraction bits</param>
	/// <returns></returns>
	constexpr void setbits(uint64_t raw) noexcept {
		// do not clear the nan/inf/zero booleans: caller must manage
		_significant.setbits(raw);
	}
	constexpr void setblock(size_t i, const bt& block) {
		_significant.setblock(i, block);
	}

	// selectors
	inline constexpr bool isnan()       const noexcept { return _nan; }
	inline constexpr bool isinf()       const noexcept { return _inf; }
	inline constexpr bool iszero()      const noexcept { return _zero; }
	inline constexpr bool ispos()       const noexcept { return !_sign; }
	inline constexpr bool isneg()       const noexcept { return _sign; }
	inline constexpr bool sign()        const noexcept { return _sign; }
	inline constexpr int  scale()       const noexcept { return _scale; }
	inline constexpr Frac significant() const noexcept { return _significant; }
	// specialty function to offer a fast path to get the fraction bits out of the representation
	// to convert to a target number system: only valid for bfbits <= 64
	inline constexpr uint64_t fraction_ull() const noexcept{ return _significant.fraction_ull(); }
	// fraction bit accessors
	inline constexpr bool at(size_t index)   const noexcept { return _significant.at(index); }
	inline constexpr bool test(size_t index) const noexcept { return _significant.at(index); }

	// conversion operators
	explicit operator float()       const noexcept { return to_float(); }
	explicit operator double()      const noexcept { return to_double(); }
	explicit operator long double() const noexcept { return to_long_double(); }

	/////////////////////////////////////////////////////////////
	// ALU operators

	/// <summary>
	/// add two real numbers with nbits fraction bits yielding an nbits unrounded sum
	/// To avoid fraction bit copies, the input requirements are pushed to the
	/// calling environment to prepare the correct storage
	/// </summary>
	/// <param name="lhs">ephemeral blocktriple<abits> that may get modified</param>
	/// <param name="rhs">ephemeral blocktriple<abits> that may get modified</param>
	/// <param name="result">unrounded sum</param>
	void add(blocktriple<nbits, bt>& lhs, blocktriple<nbits, bt>& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = std::max(lhs_scale, rhs_scale);

		// avoid copy by directly manipulating the fraction bits of the arguments
		int expDiff = lhs_scale - rhs_scale;
		if (expDiff < 0) {
			lhs.align(-expDiff);
		}
		else if (expDiff > 0) {
			rhs.align(expDiff);
		}
		if (lhs.isneg()) lhs._significant.twosComplement();
		if (rhs.isneg()) rhs._significant.twosComplement();

		_significant.add(lhs._significant, rhs._significant);

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
			if (_significant.test(bfbits-1)) {  // is the result negative
				_significant.twosComplement();
				_sign = true;
			}
			_scale = scale_of_result;
			if (_significant.test(bfbits-2)) { // test for carry
				_scale += 1;
				_significant >>= 1; // TODO: do we need to round on bits shifted away?
			}
			else if (_significant.test(bfbits - 3)) { // check for the hidden bit
				// ready to go
			}
			else {
				// found a denormalized form, thus need to normalize: find MSB
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

	/// <summary>
	/// multiply two real numbers with <nbits> fraction bits yielding an <2*nbits> unrounded result
	/// To avoid fraction bit copies, the input requirements are pushed to the
	/// calling environment to prepare the correct storage
	/// </summary>
	/// <param name="lhs">ephemeral blocktriple<mbits> that may get modified</param>
	/// <param name="rhs">ephemeral blocktriple<mbits> that may get modified</param>
	/// <param name="result">unrounded sum</param>
	void mul(blocktriple<nbits, bt>& lhs, blocktriple<nbits, bt>& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = lhs_scale + rhs_scale;

		// avoid copy by directly manipulating the fraction bits of the arguments
		_significant.mul(lhs._significant, rhs._significant);

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
			std::cout << "sum : " << to_binary(*this) << " : " << *this << '\n';
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

public:
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
		_significant.setbits(rounded_bits);
		return *this;
	}

	CONSTEXPRESSION inline blocktriple& convert_float(float rhs) noexcept {
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
			if (raw == 1ul || raw == 0x0040'0001ul) {
				// 1.11111111.00000000000000000000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000000000000000001 signalling nan
				// 0.11111111.10000000000000000000001 signalling nan
				// NAN_TYPE_SIGNALLING;
				_nan = true;
				_inf = true; // this is the encoding of a signalling NaN
				return *this;
			}
			if (raw == 0x0040'0000ul) {
				// 1.11111111.10000000000000000000000 quiet nan
				// 0.11111111.10000000000000000000000 quiet nan
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
		// normal number, not zero
		_nan = false;
		_inf = false;
		_zero = false;
		_sign = s;
		_scale = static_cast<int>(raw_exp) - 127;
		uint32_t rounded_bits = round<24, uint32_t>(raw);
		_significant.setbits(rounded_bits);
		return *this;
	}
	CONSTEXPRESSION inline blocktriple& convert_double(double rhs) noexcept { // TODO: deal with subnormals and inf
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
		_significant.setbits(rounded_bits);
		return *this;
	}

	double      to_float() const {
		if (_zero) return 0.0;
		float v = float(_significant);
		v *= std::pow(2.0f, float(_scale));
		return (_sign ? -v : v);
	}
	double      to_double() const {  // TODO: this needs a native, correctly rounded version
		if (_zero) return 0.0;
		double v = double(_significant);
		v *= std::pow(2.0, double(_scale));
		return (_sign ? -v : v);
	}
	long double to_long_double() const {
		return (long double)(to_double());
	}

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const blocktriple<nnbits, bbt>& a);
	template<size_t nnbits, typename bbt>
	friend std::istream& operator>> (std::istream& istr, blocktriple<nnbits, bbt>& a);

	// declare as friends to avoid needing a marshalling function to get significant bits out
	template<size_t nnbits, typename bbt>
	friend std::string to_binary(const blocktriple<nnbits, bbt>&, bool);
	template<size_t nnbits, typename bbt>
	friend std::string to_triple(const blocktriple<nnbits, bbt>&, bool);

	// logic operators
	template<size_t nnbits, typename bbt>
	friend bool operator==(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator!=(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator< (const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator> (const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator<=(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator>=(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
};

////////////////////// operators
template<size_t nbits, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<nbits, bt>& a) {
	if (a._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << double(a);
	}
	return ostr;
}

template<size_t nbits, typename bt>
inline std::istream& operator>> (std::istream& istr, const blocktriple<nbits, bt>& a) {
	istr >> a._fraction;
	return istr;
}

template<size_t nbits, typename bt>
inline bool operator==(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._significant == rhs._significant && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
template<size_t nbits, typename bt>
inline bool operator!=(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, typename bt>
inline bool operator< (const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) {
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
template<size_t nbits, typename bt>
inline bool operator> (const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, typename bt>
inline bool operator<=(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, typename bt>
inline bool operator>=(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return !operator< (lhs, rhs); }


////////////////////////////////// string conversion functions //////////////////////////////

template<size_t nbits, typename bt>
std::string to_binary(const sw::universal::blocktriple<nbits, bt>& a, bool nibbleMarker = true) {
	return to_triple(a, nibbleMarker);
}

template<size_t nbits, typename bt>
std::string to_triple(const blocktriple<nbits, bt>& a, bool nibbleMarker = true) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ");
	s << std::setw(3) << a._scale << ", ";
	s << to_binary(a._significant, nibbleMarker) << ')';
	return s.str();
}

template<size_t nbits, typename bt>
blocktriple<nbits> abs(const blocktriple<nbits, bt>& a) {
	blocktriple<nbits> absolute(a);
	absolute.setpos();
	return absolute;
}

}  // namespace sw::universal
