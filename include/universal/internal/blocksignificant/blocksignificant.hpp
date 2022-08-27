#pragma once
// blocksignificant.hpp: parameterized blocked binary number system representing the bits of the floating-point significant scaled for the different arithmetic operations {+,-,*,/}
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>
#include <cmath> // for std::pow() used in conversions to native IEEE-754 formats values

// should be defined by calling environment, just catching it here just in case it is not
#ifndef LONG_DOUBLE_SUPPORT
#pragma message("LONG_DOUBLE_SUPPORT is not defined")
#define LONG_DOUBLE_SUPPORT 0
#endif

/*
   The fraction bits in a floating-point representation benefit from different
   representations for different operators:
   - for addition and subtraction, a 2's complement encoding is best, 
   - for multiplication, a simple 1's complement encoding is best
   - for division
   - for square root
   a blocksignificant type will be marked by its encoding to enable direct code paths.
   By encoding it in the type, we won't be able to dynamically go between types,
   but that is ok as the blocksignificant is a composition type that gets used
   by the ephemeral blocktriple type, which is set up for each floating-point
   operation, used, and then discarded. 

   The last piece of information we need to manage for blocksignificants is where
   the radix point is. For add/sub it is at a fixed location, nbits - 3, and
   for multiplication and division is transforms from the input values to the
   output values. The blocksignificant operators, add, sub, mul, div, sqrt manage
   this radix point transformation. Fundamentally, the actual bits of the 
   blocksignificant are used as a binary encoded integer. The encoding interpretation
   and the placement of the radix point, are directed by the aggregating class,
   such as blocktriple.
 */
namespace sw { namespace universal {

// Encoding of the blocksignificant
enum class BitEncoding {
	Flex,        // placeholder for flexible use cases
	Ones,        // 1's complement encoding
	Twos         // 2's complement encoding
};

// forward references
template<size_t nbits, typename bt> class blocksignificant;
template<size_t nbits, typename bt> constexpr blocksignificant<nbits, bt> twosComplementFree(const blocksignificant<nbits, bt>&) noexcept;
template<size_t nbits, typename bt> struct bfquorem;
template<size_t nbits, typename bt> bfquorem<nbits, bt> longdivision(const blocksignificant<nbits, bt>&, const blocksignificant<nbits, bt>&);

// idiv_t for blocksignificant<nbits> to capture quotient and remainder during long division
template<size_t nbits, typename bt>
struct bfquorem {
	constexpr bfquorem() noexcept : exceptionId{} {} // default constructors
	int exceptionId;
	blocksignificant<nbits, bt> quo; // quotient
	blocksignificant<nbits, bt> rem; // remainder
};

/*
NOTE 1
   For block arithmetic, we need to manage a carry bit.
This disqualifies using uint64_t as a block type as we can't catch the overflow condition
in the same way as the other native types, uint8_t, uint16_t, uint32_t.
   We could use a sint64_t and then convert to uint64_t and observe the MSB. 
That requires different logic though. 
TODO: the highest performance for 32bits and up would be to have a uint64_t base type
for which we need asm to get the carry bit logic to work.

TODO: are there mechanisms where we can use SIMD for vector operations?
If there are, then doing something with more fitting and smaller base types might
yield more concurrency and thus more throughput for that ISA.


NOTE 2
adding two block triples of nbits would yield a result of nbits+1. To implement a
fast use of blocksignificant storage complicates this relationship. 

Standardizing the blocktriple add to take two arguments of nbits, and product a result
of nbits+1, makes sense in the abstract pipeline as the triple would gain one bit of
accuracy. Any subsequent use would need to make a decision whether to round or not.
If we go to a quire, we wouldn't round, if we reassign it to a source precision, we would.

What is the required API of blocksignificant to support that semantic?
*/



/// <summary>
/// a block-based floating-point significant 
/// for add/sub  in 2's complement of the form  ##h.fffff
/// for mul      in sign-magnitude form expanded to 0'00001.fffff
/// for div      in sign-magnitude form expanded to 00000'00001'fffff
/// 
/// NOTE: don't set a default blocktype as this makes the integration more brittle
/// as blocktriple uses the blocksignificant as storage class and needs to interact
/// with the client number system, which also is blocked. Using the same blocktype
/// simplifies the copying of exponent and fraction bits from and to the client.
/// </summary>
/// <typeparam name="bt"></typeparam>
template<size_t _nbits, typename bt>
class blocksignificant {
public:
	typedef bt BlockType;
	static constexpr size_t nbits = _nbits;
	static constexpr size_t bitsInByte = 8;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint64_t");

	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr uint64_t storageMask = (0xFFFF'FFFF'FFFF'FFFFull >> (64 - bitsInBlock));
	static constexpr uint64_t maxRightShift = ((64 - nbits + 3) > 62) ? 63 : (64 - nbits + 3);
	static constexpr uint64_t fmask = (64 - nbits + 3) > 63 ? 0ull : (0xFFFF'FFFF'FFFF'FFFFull >> maxRightShift);

	static constexpr size_t MSU = nrBlocks - 1; // MSU == Most Significant Unit
	static constexpr bt ALL_ONES = bt(~0);
	static constexpr bt MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt OVERFLOW_BIT = ~(MSU_MASK >> 1) & MSU_MASK;

	// constructors
	constexpr blocksignificant() noexcept : radixPoint{ nbits }, encoding{ BitEncoding::Flex }, _block { 0 } {}

	template <size_t... I>
	constexpr blocksignificant(const uint64_t raw, int radixPoint, std::index_sequence<I...>) noexcept
	          : radixPoint{ radixPoint }, encoding{ BitEncoding::Flex }
		  , _block{ static_cast<bt>(storageMask & (raw >> I*bitsInBlock))... } {}

	constexpr blocksignificant(const uint64_t raw, int radixPoint) noexcept
	        : blocksignificant(raw, radixPoint, std::make_index_sequence<nrBlocks>{}) {}

	constexpr blocksignificant(const blocksignificant&) noexcept = default;
	constexpr blocksignificant(blocksignificant&&) noexcept = default;

	constexpr blocksignificant& operator=(const blocksignificant&) noexcept = default;
	constexpr blocksignificant& operator=(blocksignificant&&) noexcept = default;

#ifdef NEVER
	// disable the ability to copy different blocksignificants to catch any
	// unintended (implicit) copies when working with blocksignificants.
	// For performance, the blocksignificant is used in-place.
	// Typical design, allocates a blocktriple on the stack, and subsequently
	// uses in add/sub/mul/div/sqrt will directly access the bits of the encapsulated blocksignificant.

	/// construct a blocksignificant from another: bt must be the same
	template<size_t nnbits>
	blocksignificant(const blocksignificant<nnbits, bt>& rhs) { this->assign(rhs); }

	// blocksignificant cannot have decorated constructors or assignment
	// as blocksignificant does not have all the information to interpret a value
	// So by design, the class interface does not interact with values
	constexpr blocksignificant(long long initial_value) noexcept : _block{ 0 } { *this = initial_value; }

	constexpr blocksignificant& operator=(long long rhs) noexcept {
		if constexpr (1 < nrBlocks) {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				_block[i] = rhs & storageMask;
				rhs >>= bitsInBlock;
			}
			// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
			_block[MSU] &= MSU_MASK;
		} 
		else if constexpr (1 == nrBlocks) {
			_block[0] = rhs & storageMask;
			// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
			_block[MSU] &= MSU_MASK;
		}
		return *this;
	}
#endif

	/// explicit conversion operators
	explicit constexpr operator float() const noexcept { return float(to_float()); }
	explicit constexpr operator double() const noexcept { return double(to_double()); }

#if LONG_DOUBLE_SUPPORT
	explicit constexpr operator long double() const noexcept { return (long double)to_long_double(); }
	constexpr long double to_long_double() const noexcept {
		return (long double)to_double();
	}
#endif

	/// prefix operators
	//
	// 
	// one's complement
	constexpr blocksignificant operator~() const noexcept {
		blocksignificant complement(*this);
		complement.flip();
		return complement;
	}

	/// logic operators
	// none

	/// arithmetic operators
	// none

	constexpr void increment() noexcept {
		bool carry = true;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			// cast up so we can test for overflow
			uint64_t l = uint64_t(_block[i]);
			uint64_t s = l + (carry ? uint64_t(1) : uint64_t(0));
			carry = (s > ALL_ONES);
			_block[i] = bt(s);
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
	}
	/// <summary>
	/// add two fractions of the form 00h.fffff, that is, radix point at nbits-3
	/// In this encoding, all valid values are encapsulated
	/// </summary>
	/// <param name="lhs">nbits of fraction in the form 00h.ffff</param>
	/// <param name="rhs">nbits of fraction in the form 00h.ffff</param>
	void add(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		bool carry = false;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			// cast up so we can test for overflow
			uint64_t l = uint64_t(lhs._block[i]);
			uint64_t r = uint64_t(rhs._block[i]);
			uint64_t s = l + r + (carry ? uint64_t(1) : uint64_t(0));
			carry = (s > ALL_ONES);
			_block[i] = bt(s);
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
	}
	void sub(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		blocksignificant<nbits, bt> b(twosComplementFree(rhs)); 
		add(lhs, b);
	}
	void mul(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		blocksignificant<nbits, bt> base(lhs);
		blocksignificant<nbits, bt> multiplicant(rhs);
		clear();
		for (size_t i = 0; i < nbits; ++i) {
			if (base.at(i)) {
				add(*this, multiplicant);
			}
			multiplicant <<= 1;
		}
		// since we used operator+=, which enforces the nulling of leading bits
		// we don't need to null here
	}
	void div(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		blocksignificant<nbits, bt> base(lhs);
		blocksignificant<nbits, bt> divider(rhs);
		clear();
		size_t outputRadix = static_cast<size_t>(lhs.radix());
		size_t fbits = (outputRadix >> 1);
		for (size_t i = 0; i <= 2*fbits; ++i) {
//			std::cout << "base    : " << to_binary(base) << " : " << base << '\n';
//			std::cout << "divider : " << to_binary(divider) << " : " << divider << '\n';
			if (divider <= base) {
				base.sub(base, divider);
				this->setbit(outputRadix - i);
			}
			divider >>= 1;
		}
	}

#ifdef FRACTION_REMAINDER
	// remainder operator
	blocksignificant& operator%=(const blocksignificant& rhs) noexcept {
		bfquorem<nbits, bt> result = longdivision(*this, rhs);
		*this = result.rem;
		return *this;
	}
#endif

	constexpr blocksignificant& operator<<=(int bitsToShift) noexcept {
		if (bitsToShift == 0) return *this;
		if (bitsToShift < 0) return operator>>=(-bitsToShift);
		if (bitsToShift > long(nbits)) bitsToShift = nbits; // clip to max
		if (bitsToShift >= long(bitsInBlock)) {
			int blockShift = bitsToShift / static_cast<int>(bitsInBlock);
			for (int i = static_cast<int>(MSU); i >= blockShift; --i) {
				_block[i] = _block[i - blockShift];
			}
			for (int i = blockShift - 1; i >= 0; --i) {
				_block[i] = bt(0);
			}
			// adjust the shift
			bitsToShift -= static_cast<int>(blockShift * bitsInBlock);
			if (bitsToShift == 0) return *this;
		}
		if constexpr (MSU > 0) {
			// construct the mask for the upper bits in the block that need to move to the higher word
			bt mask = 0xFFFFFFFFFFFFFFFF << (bitsInBlock - bitsToShift);
			for (size_t i = MSU; i > 0; --i) {
				_block[i] <<= bitsToShift;
				// mix in the bits from the right
				bt bits = bt(mask & _block[i - 1]);
				_block[i] |= (bits >> (bitsInBlock - bitsToShift));
			}
		}
		_block[0] <<= bitsToShift;
		return *this;
	}
	constexpr blocksignificant& operator>>=(int bitsToShift) noexcept {
		if (bitsToShift == 0) return *this;
		if (bitsToShift < 0) return operator<<=(-bitsToShift);
		if (bitsToShift >= static_cast<int>(nbits)) {
			setzero();
			return *this;
		}

		size_t blockShift = 0;
		if (bitsToShift >= static_cast<int>(bitsInBlock)) {
			blockShift = bitsToShift / bitsInBlock;
			if (MSU >= blockShift) {
				// shift by blocks
				for (size_t i = 0; i <= MSU - blockShift; ++i) {
					_block[i] = _block[i + blockShift];
				}
			}
			// adjust the shift
			bitsToShift -= static_cast<int>(blockShift * bitsInBlock);
			if (bitsToShift == 0) {
				// clean up the blocks we have shifted clean
				bitsToShift += static_cast<int>(blockShift * bitsInBlock);
				for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
					this->setbit(i, false); // reset
				}

				return *this;
			}
		}
		if constexpr (MSU > 0) {
			bt mask = ALL_ONES;
			mask >>= (bitsInBlock - bitsToShift); // this is a mask for the lower bits in the block that need to move to the lower word
			for (size_t i = 0; i < MSU; ++i) {  // TODO: can this be improved? we should not have to work on the upper blocks in case we block shifted
				_block[i] >>= bitsToShift;
				// mix in the bits from the left
				bt bits = bt(mask & _block[i + 1]);
				_block[i] |= (bits << (bitsInBlock - bitsToShift));
			}
		}
		_block[MSU] >>= bitsToShift;

		// clean up the blocks we have shifted clean
		bitsToShift += static_cast<int>(blockShift * bitsInBlock);
		for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
			this->setbit(i, false); // reset
		}

		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}

	// modifiers
	 // clear a block binary number
	constexpr void clear() noexcept {
		*this = {};
	}
	constexpr void setzero() noexcept { clear(); }
	constexpr void setradix(int radix) noexcept { radixPoint = radix; }
	constexpr void setbit(size_t i, bool v = true) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt((block & null) | mask);
		}
		// when i is out of bounds, fail silently as no-op
	}
	constexpr void setblock(size_t b, const bt& block) noexcept {
		if (b < nrBlocks) _block[b] = block;
		// when b is out of bounds, fail silently as no-op
	}
	constexpr void setbits(uint64_t value) noexcept {
		// radixPoint needs to be set, either using the constructor or the setradix() function
		if constexpr (1 == nrBlocks) {
			_block[0] = value & storageMask;
		}
		else if constexpr (1 < nrBlocks) {
			if constexpr (bitsInBlock == 64) {
				// just set the highest bits with the value provided
				_block[MSU] = value;
			}
			else {
				for (size_t i = 0; i < nrBlocks; ++i) {
					_block[i] = value & storageMask;
					value >>= bitsInBlock;
				}
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	}
	constexpr blocksignificant& flip() noexcept { // in-place one's complement
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}		
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	// in-place 2's complement
	constexpr blocksignificant& twosComplement() noexcept {
		blocksignificant<nbits, bt> plusOne;
		plusOne.setbit(0);
		flip();
		add(*this, plusOne);
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) if (_block[i] != 0) return false;
		return true;
	}
	constexpr int  radix() const noexcept { return radixPoint; }
	constexpr bool isodd() const noexcept { return _block[0] & 0x1;	}
	constexpr bool iseven() const noexcept { return !isodd(); }
	constexpr bool sign() const noexcept { return test(nbits - 1); }
	constexpr bool isneg() const noexcept { return sign(); }
	constexpr bool test(size_t bitIndex) const noexcept { return at(bitIndex); }
	constexpr bool at(size_t bitIndex) const noexcept {
		if (bitIndex >= nbits) return false;
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	// check carry bit in output of the ALU
	constexpr bool checkCarry() const noexcept { return at(nbits - 2); }
	// helpers
	constexpr uint8_t nibble(size_t n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			size_t nibbleIndexInWord = n % (bitsInBlock >> 2);
			bt mask = static_cast<bt>(0x0Fu << (nibbleIndexInWord*4));
			bt nibblebits = static_cast<bt>(mask & word);
			return static_cast<uint8_t>(nibblebits >> static_cast<bt>(nibbleIndexInWord*4));
		}
		throw "nibble index out of bounds";
	}
	constexpr bt block(size_t b) const noexcept {
		if (b >= nrBlocks) return bt{ 0 };
		return _block[b];
	}
	constexpr blocksignificant fraction() const noexcept {
		// return a copy of the significant with the integer bits removed
		blocksignificant fractionBits(*this);
		fractionBits.setbit(static_cast<size_t>(radixPoint), false);
		return fractionBits;
	}
	constexpr uint64_t fraction_ull() const noexcept {
		uint64_t raw = significant_ull();
		// remove the non-fraction bits
		uint64_t fractionBits = (0xFFFF'FFFF'FFFF'FFFFull >> (64 - radixPoint));
		raw &= fractionBits;
		return raw;
	}
	template <size_t... I>
	constexpr uint64_t significant_ull(std::index_sequence<I...> = {}) const noexcept {
		uint64_t raw{};
		raw = _block[MSU];
		raw &= MSU_MASK;
		if constexpr (sizeof...(I) == 0) {
			if constexpr (bitsInBlock < 64 && nrBlocks > 1) {
				return blocksignificant::significant_ull(std::make_index_sequence<MSU>{});
			}
			else { // if bitsInBlock < 64, take top 64bits and ignore the rest
				return raw;
			}
		}
		else {
			return ((raw <<= bitsInBlock,
			         raw |= _block[MSU - 1 - I]), ...);
		}
	}
#ifdef DEPRECATED
	// copy a value over from one blocksignificant to this blocksignificant
	// blocksignificant is a 2's complement encoding, so we sign-extend by default
	template<size_t srcbits>
	inline blocksignificant<nbits, bt>& assign(const blocksignificant<srcbits, bt>& rhs) noexcept {
		clear();
		// since bt is the same, we can directly copy the blocks in
		size_t minNrBlocks = (this->nrBlocks < rhs.nrBlocks) ? this->nrBlocks : rhs.nrBlocks;
		for (size_t i = 0; i < minNrBlocks; ++i) {
			_block[i] = rhs.block(i);
		}
		if constexpr (nbits > srcbits) { // check if we need to sign extend
			if (rhs.sign()) {
				for (size_t i = srcbits; i < nbits; ++i) { // TODO: replace bit-oriented sequence with block
					setbit(i);
				}
			}
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}

	// copy a value over from one blocksignificant to this without sign-extending the value
	// blocksignificant is a 2's complement encoding, so we sign-extend by default
	// for fraction/significent encodings, we need to turn off sign-extending.
	template<size_t srcbits>
	inline blocksignificant<nbits, bt>& assignWithoutSignExtend(const blocksignificant<srcbits, bt>& rhs) noexcept {
		clear();
		// since bt is the same, we can simply copy the blocks in
		size_t minNrBlocks = (this->nrBlocks < rhs.nrBlocks) ? this->nrBlocks : rhs.nrBlocks;
		for (size_t i = 0; i < minNrBlocks; ++i) {
			_block[i] = rhs.block(i);
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}
#endif
	// return the position of the most significant bit, -1 if v == 0
	constexpr int msb() const noexcept {
		for (int i = int(MSU); i >= 0; --i) {
			if (_block[i] != 0) {
				bt mask = (bt(1u) << (bitsInBlock-1));
				for (int j = bitsInBlock - 1; j >= 0; --j) {
					if (_block[i] & mask) {
						return i * static_cast<int>(bitsInBlock) + j;
					}
					mask >>= 1;
				}
			}
		}
		return -1; // no significant bit found, all bits are zero
	}

	// conversion to native types
	constexpr float to_float() const noexcept {
		return float(to_double());
	}
	constexpr double to_double() const noexcept {
		double d{ 0.0 };
		double s{ 1.0 };
		blocksignificant<nbits, bt> tmp(*this);
		int bit = static_cast<int>(nbits - 1);
		int shift = static_cast<int>(nbits - 1 - radixPoint);

		// special case preprocessing for 2's complement encodings
//		if (encoding == BitEncoding::Twos) {
			// nbits in the target form 00h.fffff, check msb and if set take 2's complement
			if (test(static_cast<size_t>(bit--))) {
				tmp.twosComplement();
				s = -1.0;
			}
			--shift; // and remove the MSB from the value computation
//		}

		// process the value above the radix
		size_t bitValue = 1ull << shift;
		for (; bit >= radixPoint; --bit) {
			if (tmp.test(static_cast<size_t>(bit))) d += static_cast<double>(bitValue);
			bitValue >>= 1;
		}
		// process the value below the radix
		double v = std::pow(2.0, -double(radixPoint));
		for (size_t fbit = 0; fbit < static_cast<size_t>(radixPoint); ++fbit) {
			if (tmp.test(fbit)) d += v;
			v *= 2.0;
		}

//		if constexpr (nbits > 49) { // check if we can represent this value with a native normal double with 52 fraction bits => nbits <= (52 - 3)
//			std::cerr << "to_double() will yield inaccurate result since blocksignificant has more precision than native IEEE-754 double\n";
//		}

		return s * d;
	}

	// determine the rounding direction for round-to-even: returns true if we need to round up, false if we need to truncate
	// Function argument is the bit position of the LSB of the target number.
	constexpr bool roundingDirection(size_t targetLsb) const noexcept {
		bool lsb    = at(targetLsb);
		bool guard  = (targetLsb == 0 ? false : at(targetLsb - 1));
		bool round  = (targetLsb <= 1 ? false : at(targetLsb - 2));
		bool sticky = (targetLsb <= 2 ? false : any(targetLsb - 3));
		bool tie = guard && !round && !sticky;
		return (lsb && tie) || (guard && !tie);
	}
	constexpr bool any(size_t msb) const noexcept {
		msb = (msb > nbits - 1 ? nbits - 1 : msb);
		size_t topBlock = msb / bitsInBlock;
		bt mask = bt(ALL_ONES >> (bitsInBlock - 1 - (msb % bitsInBlock)));
		for (size_t i = 0; i < topBlock; ++i) {
			if (_block[i] > 0) return true;
		}
		// process the partial block
		if (_block[topBlock] & mask) return true;
		return false;
	}

protected:
	// HELPER methods
	// none

public:
	int radixPoint;
	BitEncoding encoding;
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// integer - integer logic comparisons
	friend constexpr bool operator==(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		for (size_t i = 0; i < lhs.nrBlocks; ++i) {
			if (lhs._block[i] != rhs._block[i]) {
				return false;
			}
		}
		return true;
	}

	friend constexpr bool operator!=(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		return !operator==(lhs, rhs);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// logic operators

	friend constexpr bool operator<(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		blocksignificant diff;
		diff.sub(lhs, rhs);
		return diff.isneg();
	}

	friend constexpr bool operator<=(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		return (lhs < rhs || lhs == rhs);
	}

	friend constexpr bool operator>(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		return !(lhs <= rhs);
	}

	friend constexpr bool operator>=(const blocksignificant& lhs, const blocksignificant& rhs) noexcept {
		return !(lhs < rhs);
	}

	///////////////////////////////////////////////////////////////////////////////
	// binary operators

	friend constexpr  blocksignificant operator<<(const blocksignificant& a, const long b) noexcept {
		blocksignificant c(a);
		return c <<= b;
	}

	friend constexpr  blocksignificant operator>>(const blocksignificant& a, const long b) noexcept {
		blocksignificant c(a);
		return c >>= b;
	}


	// ostream operator
	friend std::ostream& operator<<(std::ostream& ostr, const blocksignificant& v) {
		return ostr << double(v);
	}
};

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the blocksignificant: 00h.ffff
// by design, the radix point is at nbits-3
template<size_t nbits, typename bt>
std::string to_binary(const blocksignificant<nbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = nbits - 1; i >= 0; --i) {
		s << (number.at(size_t(i)) ? '1' : '0');
		if (i == number.radix()) {
			s << '.';
		}
		else {
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	return s.str();
}

// local helper to display the contents of a byte array
template<size_t nbits, typename bt>
std::string to_hex(const blocksignificant<nbits, bt>& number, bool wordMarker = true) {
	static constexpr size_t bitsInByte = 8;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	int nrNibbles = int(1 + ((nbits - 1) >> 2));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = number.nibble(static_cast<size_t>(n));
		ss << hexChar[nibble];
		if (wordMarker && n > 0 && ((n * 4ll) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

// divide a by b and return both quotient and remainder
template<size_t nbits, typename bt>
bfquorem<nbits, bt> longdivision(const blocksignificant<nbits, bt>& _a, const blocksignificant<nbits, bt>& _b)  {
	bfquorem<nbits, bt> result;
	if (_b.iszero()) {
		result.exceptionId = 1; // division by zero
		return result;
	}
/*
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = _a.sign();
	bool b_sign = _b.sign();
	bool result_negative = (a_sign ^ b_sign);
	// normalize both arguments to positive, which requires expansion by 1-bit to deal with maxneg
	blocksignificant<nbits + 1, bt> a(_a);
	blocksignificant<nbits + 1, bt> b(_b);
	if (a_sign) a.twosComplement();
	if (b_sign) b.twosComplement();

	if (a < b) { // optimization for integer numbers
		result.rem = _a; // a % b = a when a / b = 0
		return result;   // a / b = 0 when b > a
	}
	// initialize the long division
	blocksignificant<nbits + 1, bt> decimator = a;
	// prepare the subtractand
	blocksignificant<nbits + 1, bt> subtractand = b;
	int msb_b = b.msb();
	int msb_a = a.msb();
	int shift = msb_a - msb_b;
	subtractand <<= shift;
	// long division
	for (int i = shift; i >= 0; --i) {
		if (subtractand <= decimator) {
			decimator -= subtractand;
			result.quo.set(static_cast<size_t>(i));
		}
		else {
			result.quo.reset(static_cast<size_t>(i));
		}
		subtractand >>= 1;
	}
	if (result_negative) {  // take 2's complement
		result.quo.flip();
		result.quo += 1;
	}
	if (_a.isneg()) {
		result.rem = -decimator;
	}
	else {
		result.rem = decimator;
	}
*/
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// specialty binary operators


#define TRACE_DIV 0
// unrounded division, returns a blocksignificant that is of size 2*nbits
template<size_t nbits, size_t roundingBits, typename bt>
inline blocksignificant<2 * nbits + roundingBits, bt> urdiv(const blocksignificant<nbits, bt>& a, const blocksignificant<nbits, bt>& b, blocksignificant<roundingBits, bt>& r) {
	if (b.iszero()) {
		// division by zero
		throw "urdiv divide by zero";
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = a.sign();
	bool b_sign = b.sign();
	bool result_negative = (a_sign ^ b_sign);

	// normalize both arguments to positive in new size
	blocksignificant<nbits + 1, bt> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blocksignificant<nbits + 1, bt> b_new(b);
	if (a_sign) a_new.twoscomplement();
	if (b_sign) b_new.twoscomplement();

	// initialize the long division
	blocksignificant<2 * nbits + roundingBits, bt> decimator(a_new);
	blocksignificant<2 * nbits + roundingBits, bt> subtractand(b_new); // prepare the subtractand
	blocksignificant<2 * nbits + roundingBits, bt> result;

	int msp = nbits + roundingBits - 1; // msp = most significant position
	decimator <<= msp; // scale the decimator to the largest possible positive value

	int msb_b = subtractand.msb();
	int msb_a = decimator.msb();
	int shift = msb_a - msb_b;
	int scale = shift - msp;   // scale of the result quotient
	subtractand <<= shift;

#if TRACE_DIV
	std::cout << "  " << to_binary(decimator) << std::endl;
	std::cout << "- " << to_binary(subtractand) << " shift: " << shift << std::endl;
#endif
	// long division
	for (int i = msb_a; i >= 0; --i) {

		if (subtractand <= decimator) {
			decimator -= subtractand;
			result.set(static_cast<size_t>(i));
		}
		else {
			result.reset(static_cast<size_t>(i));
		}
		subtractand >>= 1;

#if TRACE_DIV
		std::cout << "  " << to_binary(decimator) << ' ' << to_binary(result) << std::endl;
		std::cout << "- " << to_binary(subtractand) << std::endl;
#endif
	}
	result <<= scale;
	if (result_negative) result.twosComplement();
	r.assign(result); // copy the lowest bits which represent the bits on which we need to apply the rounding test
	return result;
}

// free function generator of the 2's complement of a blocksignificant
template<size_t nbits, typename bt>
inline constexpr blocksignificant<nbits, bt> twosComplementFree(const blocksignificant<nbits, bt>& a) noexcept {
	blocksignificant<nbits, bt> b(a);
	return b.twosComplement();
}

}} // namespace sw::universal
