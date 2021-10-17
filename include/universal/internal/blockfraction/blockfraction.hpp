#pragma once
// blockfraction.hpp: parameterized blocked binary number system representing a floating-point fraction including leading 1 bit
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
   a blockfraction type will be marked by its encoding to enable direct code paths.
   By encoding it in the type, we won't be able to dynamically go between types,
   but that is ok as the blockfraction is a composition type that gets used
   by the ephemeral blocktriple type, which is set up for each floating-point
   operation, used, and then discarded. 

   The last piece of information we need to manage for blockfractions is where
   the radix point is. For add/sub it is at a fixed location, nbits - 3, and
   for multiplication and division is transforms from the input values to the
   output values. The blockfraction operators, add, sub, mul, div, sqrt manage
   this radix point transformation. Fundamentally, the actual bits of the 
   blockfraction are used as a binary encoded integer. The encoding interpretation
   and the placement of the radix point, are directed by the aggregating class,
   such as blocktriple.
 */
namespace sw::universal {

	// Encoding of the BlockFraction
	enum class BitEncoding {
		Flex,        // placeholder for flexible use cases
		Ones,        // 1's complement encoding
		Twos         // 2's complement encoding
	};

// forward references
template<size_t nbits, typename bt, BitEncoding encoding> class blockfraction;
template<size_t nbits, typename bt, BitEncoding encoding> constexpr blockfraction<nbits, bt, encoding> twosComplement(const blockfraction<nbits, bt, encoding>&);
template<size_t nbits, typename bt, BitEncoding encoding> struct bfquorem;
template<size_t nbits, typename bt, BitEncoding encoding> bfquorem<nbits, bt, encoding> longdivision(const blockfraction<nbits, bt, encoding>&, const blockfraction<nbits, bt, encoding>&);

// idiv_t for blockfraction<nbits> to capture quotient and remainder during long division
template<size_t nbits, typename bt, BitEncoding encoding>
struct bfquorem {
	bfquorem() {} // default constructors
	int exceptionId;
	blockfraction<nbits, bt, encoding> quo; // quotient
	blockfraction<nbits, bt, encoding> rem; // remainder
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
fast use of blockfraction storage complicates this relationship. 

Standardizing the blocktriple add to take two arguments of nbits, and product a result
of nbits+1, makes sense in the abstract pipeline as the triple would gain one bit of
accuracy. Any subsequent use would need to make a decision whether to round or not.
If we go to a quire, we wouldn't round, if we reassign it to a source precision, we would.

What is the required API of blockfraction to support that semantic?
*/



/// <summary>
/// a block-based floating-point fraction 
/// for add/sub  in 2's complement of the form  ##h.fffff
/// for mul      in sign-magnitude form expanded to 0'00001.fffff
/// for div      in sign-magnitude form expanded to 00000'00001'fffff
/// 
/// NOTE: don't set a default blocktype as this makes the integration more brittle
/// as blocktriple uses the blockfraction as storage class and needs to interact
/// with the client number system, which also is blocked. Using the same blocktype
/// simplifies the copying of exponent and fraction bits from and to the client.
/// </summary>
/// <typeparam name="bt"></typeparam>
template<size_t _nbits, typename bt, BitEncoding _encoding>
class blockfraction {
public:
	typedef bt BlockType;
	static constexpr size_t nbits = _nbits;
	static constexpr BitEncoding encoding = _encoding;
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
	constexpr blockfraction() noexcept : radixPoint{ nbits }, _block { 0 } {}
	constexpr blockfraction(uint64_t raw, int radixPoint) noexcept : radixPoint{ radixPoint }, _block { 0 } {
		if constexpr (1 == nrBlocks) {
			_block[0] = static_cast<bt>(storageMask & raw);;
		}
		else if constexpr (2 == nrBlocks) {
			_block[0] = static_cast<bt>(storageMask & raw);
			_block[1] = static_cast<bt>(storageMask & (raw >> bitsInBlock));
		}
		else if constexpr (3 == nrBlocks) {
			_block[0] = static_cast<bt>(storageMask & raw);
			_block[1] = static_cast<bt>(storageMask & (raw >> bitsInBlock));
			_block[2] = static_cast<bt>(storageMask & (raw >> 2*bitsInBlock));
		}
		else if constexpr (4 == nrBlocks) {
			_block[0] = static_cast<bt>(storageMask & raw);
			_block[1] = static_cast<bt>(storageMask & (raw >> bitsInBlock));
			_block[2] = static_cast<bt>(storageMask & (raw >> 2 * bitsInBlock));
			_block[3] = static_cast<bt>(storageMask & (raw >> 3 * bitsInBlock));
		}
		else {
			for (size_t i = 0; i < nrBlocks; ++i) {
				_block[i] = static_cast<bt>(storageMask & (raw >> i * bitsInBlock));
			}
		}
	}

	blockfraction(const blockfraction&) noexcept = default;
	blockfraction(blockfraction&&) noexcept = default;

	blockfraction& operator=(const blockfraction&) noexcept = default;
	blockfraction& operator=(blockfraction&&) noexcept = default;

#ifdef NEVER
	// disable the ability to copy different blockfractions to catch any
	// unintended (implicit) copies when working with blockfractions.
	// For performance, the blockfraction must be used in-place.
	// Typical design, allocates a blocktriple on the stack, and subsequently
	// uses in add/sub/mul/div/sqrt will directly access the encapsulated blockfraction.

	/// construct a blockfraction from another: bt must be the same
	template<size_t nnbits>
	blockfraction(const blockfraction<nnbits, bt>& rhs) {
		this->assign(rhs);
	}

	// blockfraction cannot have decorated constructors or assignment
	// as blockfraction does not have all the information to interpret a value
	// So by design, the class interface does not interact with values
	constexpr blockfraction(long long initial_value) noexcept : _block{ 0 } { *this = initial_value; }

	constexpr blockfraction& operator=(long long rhs) noexcept {
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

	/// conversion operators
	explicit operator float() const              { return float(to_float()); }
	explicit operator double() const             { return double(to_double()); }
#if LONG_DOUBLE_SUPPORT
	explicit operator long double() const        { return (long double)to_long_double(); }
#endif

	/// prefix operators
	//
	// 
	// one's complement
	blockfraction operator~() const {
		blockfraction complement(*this);
		complement.flip();
		return complement;
	}

	/// logic operators
	// none

	/// arithmetic operators
	// none

	void increment() {
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
	void add(const blockfraction& lhs, const blockfraction& rhs) {
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
	void sub(const blockfraction& lhs, blockfraction& rhs) {
		add(lhs, rhs.twosComplement());
	}
	void mul(const blockfraction& lhs, const blockfraction& rhs) {
		blockfraction<nbits, bt, encoding> base(lhs);
		blockfraction<nbits, bt, encoding> multiplicant(rhs);
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
	void div(const blockfraction& lhs, const blockfraction& rhs) {
		bfquorem<nbits, bt, encoding> result = longdivision(*this, rhs);
		*this = result.quo;
	}

#ifdef FRACTION_REMAINDER
	// remainder operator
	blockfraction& operator%=(const blockfraction& rhs) {
		bfquorem<nbits, bt, encoding> result = longdivision(*this, rhs);
		*this = result.rem;
		return *this;
	}
#endif

	// shift left operator
	blockfraction& operator<<=(int bitsToShift) {
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

	// shift right operator
	blockfraction& operator>>=(int bitsToShift) {
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
	inline constexpr void clear() noexcept {
		if constexpr (1 == nrBlocks) {
			_block[0] = 0;
		}
		else if constexpr (2 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
		}
		else if constexpr (3 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
			_block[2] = 0;
		}
		else if constexpr (4 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
			_block[2] = 0;
			_block[3] = 0;
		}
		else if constexpr (5 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
			_block[2] = 0;
			_block[3] = 0;
			_block[4] = 0;
		}
		else if constexpr (6 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
			_block[2] = 0;
			_block[3] = 0;
			_block[4] = 0;
			_block[5] = 0;
		}
		else if constexpr (7 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
			_block[2] = 0;
			_block[3] = 0;
			_block[4] = 0;
			_block[5] = 0;
			_block[6] = 0;
		}
		else if constexpr (8 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
			_block[2] = 0;
			_block[3] = 0;
			_block[4] = 0;
			_block[5] = 0;
			_block[6] = 0;
			_block[7] = 0;
		}
		else {
			for (size_t i = 0; i < nrBlocks; ++i) {
				_block[i] = static_cast<bt>(0ull);
			}
		}
	}
	inline constexpr void setzero() noexcept { clear(); }
	inline constexpr void setradix(int radix) { radixPoint = radix; }
	inline constexpr void setbit(size_t i, bool v = true) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt((block & null) | mask);
		}
		// when i is out of bounds, fail silently as no-op
	}
	inline constexpr void setblock(size_t b, const bt& block) noexcept {
		if (b < nrBlocks) _block[b] = block;
		// when b is out of bounds, fail silently as no-op
	}
	inline constexpr void setbits(uint64_t value) noexcept {
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
	inline constexpr blockfraction& flip() noexcept { // in-place one's complement
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}		
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	// in-place 2's complement
	inline constexpr blockfraction& twosComplement() noexcept {
		blockfraction<nbits, bt, encoding> plusOne;
		plusOne.setbit(0);
		flip();
		add(*this, plusOne);
		return *this;
	}

	// selectors
	inline constexpr bool iszero() const noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) if (_block[i] != 0) return false;
		return true;
	}
	inline constexpr int  radix() const { return radixPoint; }
	inline constexpr bool isodd() const noexcept { return _block[0] & 0x1;	}
	inline constexpr bool iseven() const noexcept { return !isodd(); }
	inline constexpr bool sign() const { return false; } // dummy to unify the API with other number systems in Universal 
	inline constexpr bool test(size_t bitIndex) const noexcept { return at(bitIndex); }
	inline constexpr bool at(size_t bitIndex) const noexcept {
		if (bitIndex >= nbits) return false;
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	// check carry bit in output of the ALU
	inline constexpr bool checkCarry() const noexcept { return at(nbits - 2); }
	// helpers
	inline constexpr uint8_t nibble(size_t n) const {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			size_t nibbleIndexInWord = n % (bitsInBlock >> 2);
			bt mask = static_cast<bt>(0x0Fu << (nibbleIndexInWord*4));
			bt nibblebits = static_cast<bt>(mask & word);
			return static_cast<uint8_t>(nibblebits >> static_cast<bt>(nibbleIndexInWord*4));
		}
		throw "nibble index out of bounds";
	}
	inline constexpr bt block(size_t b) const noexcept {
		if (b >= nrBlocks) return bt{ 0 };
		return _block[b];
	}
	inline constexpr uint64_t fraction_ull() const noexcept {
		uint64_t raw = get_ull();
		// remove the non-fraction bits
		uint64_t fractionBits = (0xFFFF'FFFF'FFFF'FFFFull >> (64 - radixPoint));
		raw &= fractionBits;
		return raw;
	}
	inline constexpr uint64_t get_ull() const noexcept {
		uint64_t raw{ 0 };
		if constexpr (bitsInBlock < 64) {
			if constexpr (1 == nrBlocks) {
				raw = _block[MSU];
				raw &= MSU_MASK;
			}
			else if constexpr (2 == nrBlocks) {
				raw = _block[MSU];
				raw &= MSU_MASK;
				raw <<= bitsInBlock;
				raw |= _block[0];
			}
			else if constexpr (3 == nrBlocks) {
				raw = _block[MSU];
				raw &= MSU_MASK;
				raw <<= bitsInBlock;
				raw |= _block[1];
				raw <<= bitsInBlock;
				raw |= _block[0];
			}
			else if constexpr (4 == nrBlocks) {
				raw = _block[MSU];
				raw &= MSU_MASK;
				raw <<= bitsInBlock;
				raw |= _block[2];
				raw <<= bitsInBlock;
				raw |= _block[1];
				raw <<= bitsInBlock;
				raw |= _block[0];
			}
			else {
				raw = _block[MSU];
				raw &= MSU_MASK;
				for (int i = MSU - 1; i >= 0; --i) {
					raw <<= bitsInBlock;
					raw |= _block[i];
				}
			}
		}
		else { // take top 64bits and ignore the rest
			raw = _block[MSU];
			raw &= MSU_MASK;
		}
		return raw;
	}
#ifdef DEPRECATED
	// copy a value over from one blockfraction to this blockfraction
	// blockfraction is a 2's complement encoding, so we sign-extend by default
	template<size_t srcbits>
	inline blockfraction<nbits, bt>& assign(const blockfraction<srcbits, bt>& rhs) {
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

	// copy a value over from one blockfraction to this without sign-extending the value
	// blockfraction is a 2's complement encoding, so we sign-extend by default
	// for fraction/significent encodings, we need to turn off sign-extending.
	template<size_t srcbits>
	inline blockfraction<nbits, bt>& assignWithoutSignExtend(const blockfraction<srcbits, bt>& rhs) {
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
	inline int msb() const noexcept {
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
	inline constexpr float to_float() const noexcept {
		return float(to_double());
	}
	inline constexpr double to_double() const noexcept {
		double d{ 0.0 };
		blockfraction<nbits, bt, encoding> tmp(*this);
		int bit = static_cast<int>(nbits - 1);
		int shift = static_cast<int>(nbits - 1 - radixPoint);

		// special case preprocessing for 2's complement encodings
		if constexpr (encoding == BitEncoding::Twos) {
			// nbits in the target form 00h.fffff, check msb and if set take 2's complement
			if (test(static_cast<size_t>(bit--))) tmp.twosComplement();
			--shift; // and remove the MSB from the value computation
		}

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
//			std::cerr << "to_double() will yield inaccurate result since blockfraction has more precision than native IEEE-754 double\n";
//		}

		return d;
	}
	inline constexpr long double to_long_double() const noexcept {
		return (long double)to_double();
	}

	// determine the rounding mode: up if true, down if false
	// fraction bits:    bbbbbbbbbbb
	// target LSB   :         |
	// guard        :          |
	// round        :           |
	// sticky       :            ---
	bool roundingMode(size_t targetLsb) const {
		bool lsb    = at(targetLsb);
		bool guard  = (targetLsb == 0 ? false : at(targetLsb - 1));
		bool round  = (targetLsb <= 1 ? false : at(targetLsb - 2));
		bool sticky = (targetLsb <= 2 ? false : any(targetLsb - 3));
		bool tie = guard && !round && !sticky;
		return (lsb && tie) || (guard && !tie);
	}
	bool any(size_t msb) const {
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
	bt _block[nrBlocks];

private:
	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// integer - integer logic comparisons
	template<size_t N, typename B, BitEncoding C>
	friend bool operator==(const blockfraction<N,B,C>& lhs, const blockfraction<N, B, C>& rhs);
	template<size_t N, typename B, BitEncoding C>
	friend bool operator!=(const blockfraction<N, B, C>& lhs, const blockfraction<N, B, C>& rhs);
	// the other logic operators are defined in terms of arithmetic terms

	template<size_t N, typename B, BitEncoding C>
	friend std::ostream& operator<<(std::ostream& ostr, const blockfraction<N, B, C>& v);
};

//////////////////////////////////////////////////////////////////////////////////
// stream operators

// ostream operator
template<size_t nbits, typename bt, BitEncoding encoding>
std::ostream& operator<<(std::ostream& ostr, const blockfraction<nbits, bt, encoding>& number) {
	return ostr << double(number);
}

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the blockfraction: 00h.ffff
// by design, the radix point is at nbits-3
template<size_t nbits, typename bt, BitEncoding encoding>
std::string to_binary(const blockfraction<nbits, bt, encoding>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
#ifdef DEPRECATED
	int i = nbits - 1;
	s << (number.at(size_t(i--)) ? '1' : '0'); // sign indicator of 2's complement
	s << (number.at(size_t(i--)) ? '1' : '0'); // overflow indicator to trigger right shift
	s << (number.at(size_t(i--)) ? '1' : '0'); // the hidden bit
	s << '.';
	for (; i >= 0; --i) {
		s << (number.at(size_t(i)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
#endif
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
template<size_t nbits, typename bt, BitEncoding encoding>
std::string to_hex(const blockfraction<nbits, bt, encoding>& number, bool wordMarker = true) {
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

//////////////////////////////////////////////////////////////////////////////////
// logic operators

template<size_t N, typename B, BitEncoding C>
inline bool operator==(const blockfraction<N, B, C>& lhs, const blockfraction<N, B, C>& rhs) {
	for (size_t i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<size_t N, typename B, BitEncoding C>
inline bool operator!=(const blockfraction<N, B, C>& lhs, const blockfraction<N, B, C>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t N, typename B, BitEncoding C>
inline bool operator<(const blockfraction<N, B, C>& lhs, const blockfraction<N, B, C>& rhs) {
	if (lhs.ispos() && rhs.isneg()) return false; // need to filter out possible overflow conditions
	if (lhs.isneg() && rhs.ispos()) return true;  // need to filter out possible underflow conditions
	if (lhs == rhs) return false; // so the maxneg logic works
	blockfraction<N, B, C> mneg; maxneg<N, B>(mneg);
	if (rhs == mneg) return false; // special case: nothing is smaller than maximum negative
	blockfraction<N, B, C> diff = lhs - rhs;
	return diff.isneg();
}
template<size_t N, typename B, BitEncoding C>
inline bool operator<=(const blockfraction<N, B, C>& lhs, const blockfraction<N, B, C>& rhs) {
	return (lhs < rhs || lhs == rhs);
}
template<size_t N, typename B, BitEncoding C>
inline bool operator>(const blockfraction<N, B, C>& lhs, const blockfraction<N, B, C>& rhs) {
	return !(lhs <= rhs);
}
template<size_t N, typename B, BitEncoding C>
inline bool operator>=(const blockfraction<N, B, C>& lhs, const blockfraction<N, B, C>& rhs) {
	return !(lhs < rhs);
}

///////////////////////////////////////////////////////////////////////////////
// binary operators

template<size_t nbits, typename bt, BitEncoding encoding>
inline blockfraction<nbits, bt, encoding> operator<<(const blockfraction<nbits, bt, encoding>& a, const long b) {
	blockfraction<nbits, bt, encoding> c(a);
	return c <<= b;
}
template<size_t nbits, typename bt, BitEncoding encoding>
inline blockfraction<nbits, bt, encoding> operator>>(const blockfraction<nbits, bt, encoding>& a, const long b) {
	blockfraction<nbits, bt, encoding> c(a);
	return c >>= b;
}

// divide a by b and return both quotient and remainder
template<size_t nbits, typename bt, BitEncoding encoding>
bfquorem<nbits, bt, encoding> longdivision(const blockfraction<nbits, bt, encoding>& _a, const blockfraction<nbits, bt, encoding>& _b) {
	bfquorem<nbits, bt, encoding> result;
	if (_b.iszero()) {
		result.exceptionId = 1; // division by zero
		return result;
	}
#ifdef LATER
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = _a.sign();
	bool b_sign = _b.sign();
	bool result_negative = (a_sign ^ b_sign);
	// normalize both arguments to positive, which requires expansion by 1-bit to deal with maxneg
	blockfraction<nbits + 1, bt, encoding> a(_a);
	blockfraction<nbits + 1, bt, encoding> b(_b);
	if (a_sign) a.twosComplement();
	if (b_sign) b.twosComplement();

	if (a < b) { // optimization for integer numbers
		result.rem = _a; // a % b = a when a / b = 0
		return result;   // a / b = 0 when b > a
	}
	// initialize the long division
	blockfraction<nbits + 1, bt, encoding> accumulator = a;
	// prepare the subtractand
	blockfraction<nbits + 1, bt, encoding> subtractand = b;
	int msb_b = b.msb();
	int msb_a = a.msb();
	int shift = msb_a - msb_b;
	subtractand <<= shift;
	// long division
	for (int i = shift; i >= 0; --i) {
		if (subtractand <= accumulator) {
			accumulator -= subtractand;
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
		result.rem = -accumulator;
	}
	else {
		result.rem = accumulator;
	}
#endif
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// specialty binary operators


#define TRACE_DIV 0
// unrounded division, returns a blockfraction that is of size 2*nbits
template<size_t nbits, size_t roundingBits, typename bt, BitEncoding encoding>
inline blockfraction<2 * nbits + roundingBits, bt, encoding> urdiv(const blockfraction<nbits, bt, encoding>& a, const blockfraction<nbits, bt, encoding>& b, blockfraction<roundingBits, bt, encoding>& r) {
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
	blockfraction<nbits + 1, bt, encoding> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockfraction<nbits + 1, bt, encoding> b_new(b);
	if (a_sign) a_new.twoscomplement();
	if (b_sign) b_new.twoscomplement();

	// initialize the long division
	blockfraction<2 * nbits + roundingBits, bt, encoding> decimator(a_new);
	blockfraction<2 * nbits + roundingBits, bt, encoding> subtractand(b_new); // prepare the subtractand
	blockfraction<2 * nbits + roundingBits, bt, encoding> result;

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

// free function generator of the 2's complement of a blockfraction
template<size_t nbits, typename bt, BitEncoding encoding>
inline constexpr blockfraction<nbits, bt, encoding> twosComplement(const blockfraction<nbits, bt, encoding>& a) {
	blockfraction<nbits, bt, encoding> b(a);
	return b.twosComplement();
}

} // namespace sw::universal
