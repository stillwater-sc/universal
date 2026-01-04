#pragma once
// blockfraction.hpp: parameterized blocked binary number system representing the bits of the floating-point fraction scaled for the different arithmetic operations {+,-,*,/}
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <cmath> // for std::pow() used in conversions to native IEEE-754 formats values
#include <iostream>
#include <string>
#include <sstream>

#include <universal/internal/blockfraction/blockfraction_fwd.hpp>

namespace sw { namespace universal {

// structure for blockfraction<nbits> to capture quotient and remainder during long division
template<unsigned nbits, typename bt>
struct bfquorem {
	constexpr bfquorem() noexcept : exceptionId{} {} // default constructors
	int exceptionId;
	blockfraction<nbits, bt> quo; // quotient
	blockfraction<nbits, bt> rem; // remainder
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
/// 
/// A blockfraction is by definition an unsigned entity. As arithmetic operators
/// introduce additional bits, the radixpoint is controllable.
/// </summary>
/// <typeparam name="bt"></typeparam>
template<unsigned _nbits, typename bt>
class blockfraction {
public:
	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned bitsInByte = 8;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint64_t");

	static constexpr unsigned nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr uint64_t storageMask = (0xFFFF'FFFF'FFFF'FFFFull >> (64 - bitsInBlock));
	static constexpr uint64_t maxRightShift = ((64 - nbits + 3) > 62) ? 63 : (64 - nbits + 3);
	static constexpr uint64_t fmask = (64 - nbits + 3) > 63 ? 0ull : (0xFFFF'FFFF'FFFF'FFFFull >> maxRightShift);

	static constexpr unsigned MSU = nrBlocks - 1; // MSU == Most Significant Unit
	static constexpr bt ALL_ONES = bt(~0);
	static constexpr bt MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt OVERFLOW_BIT = ~(MSU_MASK >> 1) & MSU_MASK;
	typedef bt BlockType;
	/// NOTE: don't set a default blocktype as this makes the integration more brittle
	/// as blocktriple uses the blocksignificant as storage class and needs to interact
	/// with the client number system, which is also blocked. Using the same blocktype
	/// simplifies the copying of fraction bits from and to the client.
	/// 
	// constructors
	constexpr blockfraction() noexcept : radixPoint{ nbits }, _block{} {}

	// value constructors
//	constexpr blockfraction(signed char rhs) noexcept : radixPoint{ nbits }, _block{} {}
//	constexpr blockfraction(int rhs) noexcept : radixPoint{ nbits }, _block{} {}
	
	// raw bit constructors
	template <size_t... I>
	constexpr blockfraction(const uint64_t raw, int radixPoint, std::index_sequence<I...>) noexcept
	          : radixPoint{ radixPoint }, _block{ static_cast<bt>(storageMask & (raw >> I*bitsInBlock))... } {}

	constexpr blockfraction(const uint64_t raw, int radixPoint) noexcept
	        : blockfraction(raw, radixPoint, std::make_index_sequence<nrBlocks>{}) {}

	constexpr blockfraction(const blockfraction&) noexcept = default;
	constexpr blockfraction(blockfraction&&) noexcept = default;

	constexpr blockfraction& operator=(const blockfraction&) noexcept = default;
	constexpr blockfraction& operator=(blockfraction&&) noexcept = default;

#ifdef NEVER
	// disable the ability to copy different blockfractions to catch any
	// unintended (implicit) copies when working with blockfractions.
	// For performance, the blockfraction is used in-place.
	// Typical design, allocates a blocktriple on the stack, and subsequently
	// uses in add/sub/mul/div/sqrt will directly access the bits of the encapsulated blockfraction.

	/// construct a blockfraction from another: bt must be the same
	template<unsigned nnbits>
	blockfraction(const blockfraction<nnbits, bt>& rhs) { this->assign(rhs); }

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

	// one's complement
	constexpr blockfraction operator~() const noexcept {
		blockfraction complement(*this);
		complement.flip();
		return complement;
	}

	/// logic operators
	constexpr bool anyAfter(unsigned bitIndex) const noexcept {  // TODO: optimize for limbs
		if (bitIndex < nbits) {
			for (unsigned i = 0; i < bitIndex; ++i) if (test(i)) return true;
		}
		return false;
	}

	/// arithmetic operators

	/// <summary>
	/// increment the value by one
	/// </summary>
	/// <returns></returns>
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
	void add(const blockfraction& lhs, const blockfraction& rhs) noexcept {
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
	void sub(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		blockfraction<nbits, bt> b(twosComplementFree(rhs)); 
		add(lhs, b);
	}
	void mul(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		blockfraction<nbits, bt> base(lhs);
		blockfraction<nbits, bt> multiplicant(rhs);
		clear();
		for (unsigned i = 0; i < nbits; ++i) {
			if (base.at(i)) {
				add(*this, multiplicant);
			}
			multiplicant <<= 1;
		}
		// since we used operator+=, which enforces the nulling of leading bits
		// we don't need to null here
	}
	void div(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		blockfraction<nbits, bt> base(lhs);
		blockfraction<nbits, bt> divider(rhs);
		clear();
		unsigned outputRadix = static_cast<unsigned>(lhs.radix());
		unsigned fbits = (outputRadix >> 1);
		for (unsigned i = 0; i <= 2*fbits; ++i) {
//			std::cout << "base    : " << to_binary(base) << " : " << base << '\n';
//			std::cout << "divider : " << to_binary(divider) << " : " << divider << '\n';
			if (divider <= base) {
				base.sub(base, divider);
				this->setbit(outputRadix - i);
			}
			divider >>= 1;
		}
	}

	// multiply a fraction by an integer base
	void scaleByBase(const blockfraction& fraction, const blockfraction& integerBase) noexcept {
		blockfraction<nbits, bt> multiplicant(fraction);
		blockfraction<nbits, bt> base(integerBase);
		clear();
		radixPoint = base.radix();
		for (unsigned i = radixPoint; i < nbits; ++i) {
			if (base.at(i)) {
				add(*this, multiplicant);
			}
			multiplicant <<= 1;
		}
		// since we used operator+=, which enforces the nulling of leading bits
		// we don't need to null here
	}

#ifdef FRACTION_REMAINDER
	// remainder operator
	blockfraction& operator%=(const blockfraction& rhs) noexcept {
		bfquorem<nbits, bt> result = longdivision(*this, rhs);
		*this = result.rem;
		return *this;
	}
#endif

	constexpr blockfraction& operator<<=(int bitsToShift) noexcept {
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
			for (unsigned i = MSU; i > 0; --i) {
				_block[i] <<= bitsToShift;
				// mix in the bits from the right
				bt bits = bt(mask & _block[i - 1]);
				_block[i] |= (bits >> (bitsInBlock - bitsToShift));
			}
		}
		_block[0] <<= bitsToShift;
		return *this;
	}
	constexpr blockfraction& operator>>=(int bitsToShift) noexcept {
		if (bitsToShift == 0) return *this;
		if (bitsToShift < 0) return operator<<=(-bitsToShift);
		if (bitsToShift >= static_cast<int>(nbits)) {
			setzero();
			return *this;
		}

		unsigned blockShift = 0;
		if (bitsToShift >= static_cast<int>(bitsInBlock)) {
			blockShift = bitsToShift / bitsInBlock;
			if (MSU >= blockShift) {
				// shift by blocks
				for (unsigned i = 0; i <= MSU - blockShift; ++i) {
					_block[i] = _block[i + blockShift];
				}
			}
			// adjust the shift
			bitsToShift -= static_cast<int>(blockShift * bitsInBlock);
			if (bitsToShift == 0) {
				// clean up the blocks we have shifted clean
				bitsToShift += static_cast<int>(blockShift * bitsInBlock);
				for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
					this->setbit(i, false); // reset
				}

				return *this;
			}
		}
		if constexpr (MSU > 0) {
			bt mask = ALL_ONES;
			mask >>= (bitsInBlock - bitsToShift); // this is a mask for the lower bits in the block that need to move to the lower word
			for (unsigned i = 0; i < MSU; ++i) {  // TODO: can this be improved? we should not have to work on the upper blocks in case we block shifted
				_block[i] >>= bitsToShift;
				// mix in the bits from the left
				bt bits = bt(mask & _block[i + 1]);
				_block[i] |= (bits << (bitsInBlock - bitsToShift));
			}
		}
		_block[MSU] >>= bitsToShift;

		// clean up the blocks we have shifted clean
		bitsToShift += static_cast<int>(blockShift * bitsInBlock);
		for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
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
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (blockIndex < nrBlocks) {
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[blockIndex] = bt((block & null) | mask);
		}
		// when i is out of bounds, fail silently as no-op
	}
	constexpr void setblock(unsigned b, const bt& block) noexcept {
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
				for (unsigned i = 0; i < nrBlocks; ++i) {
					_block[i] = value & storageMask;
					value >>= bitsInBlock;
				}
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	}
	constexpr blockfraction& flip() noexcept { // in-place one's complement
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}		
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	// in-place 2's complement
	constexpr blockfraction& twosComplement() noexcept {
		blockfraction<nbits, bt> plusOne;
		plusOne.setbit(0);
		flip();
		add(*this, plusOne);
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) if (_block[i] != 0) return false;
		return true;
	}
	constexpr int  radix() const noexcept { return radixPoint; }
	constexpr bool isodd() const noexcept { return _block[0] & 0x1;	}
	constexpr bool iseven() const noexcept { return !isodd(); }
	constexpr bool sign() const noexcept { return test(nbits - 1); }
	constexpr bool isneg() const noexcept { return sign(); }
	constexpr unsigned integer() const noexcept {
		unsigned integerPart{ 0 };
		unsigned bitValue = 0x1;
		for (unsigned i = radixPoint; i < nbits; ++i) {
			if (test(i)) {
				integerPart |= bitValue;
			}
			bitValue <<= 1;
		}
		return integerPart;
	}
	constexpr bool test(unsigned bitIndex) const noexcept { return at(bitIndex); }
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false;
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	// check carry bit in output of the ALU
	constexpr bool checkCarry() const noexcept { return at(nbits - 2); }
	// helpers
	constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			unsigned nibbleIndexInWord = n % (bitsInBlock >> 2);
			bt mask = static_cast<bt>(0x0Fu << (nibbleIndexInWord*4));
			bt nibblebits = static_cast<bt>(mask & word);
			return static_cast<uint8_t>(nibblebits >> static_cast<bt>(nibbleIndexInWord*4));
		}
		return static_cast<uint8_t>(0);  // NOP when nibble index is out of bounds
	}
	constexpr bt block(unsigned b) const noexcept {
		if (b >= nrBlocks) return bt{ 0 };
		return _block[b];
	}
	constexpr blockfraction fraction() const noexcept {
		// return a copy of the fraction with the integer bits removed
		blockfraction fractionBits(*this);
		for (unsigned i = radixPoint; i < nbits; ++i) {
			fractionBits.setbit(i, false);
		}
		return fractionBits;
	}

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

		blockfraction<nbits, bt> tmp(*this);
		int bit = static_cast<int>(nbits - 1);
		int shift = static_cast<int>(nbits - 1 - radixPoint);

		// process the value above the radix
		unsigned bitValue = 1ull << shift;
		for (; bit >= radixPoint; --bit) {
			if (tmp.test(static_cast<unsigned>(bit))) d += static_cast<double>(bitValue);
			bitValue >>= 1;
		}
		// process the value below the radix
		double v = std::pow(2.0, -double(radixPoint));
		for (unsigned fbit = 0; fbit < static_cast<unsigned>(radixPoint); ++fbit) {
			if (tmp.test(fbit)) d += v;
			v *= 2.0;
		}

//		if constexpr (nbits > 49) { // check if we can represent this value with a native normal double with 52 fraction bits => nbits <= (52 - 3)
//			std::cerr << "to_double() will yield inaccurate result since blockfraction has more precision than native IEEE-754 double\n";
//		}

		return d;
	}

	// determine the rounding direction for round-to-even: returns true if we need to round up, false if we need to truncate
	// Function argument is the bit position of the LSB of the target number.
	constexpr bool roundingDirection(unsigned targetLsb) const noexcept {
		bool lsb    = at(targetLsb);
		bool guard  = (targetLsb == 0 ? false : at(targetLsb - 1));
		bool round  = (targetLsb <= 1 ? false : at(targetLsb - 2));
		bool sticky = (targetLsb <= 2 ? false : any(targetLsb - 3));
		bool tie = guard && !round && !sticky;
		return (lsb && tie) || (guard && !tie);
	}
	constexpr bool any(unsigned msb) const noexcept {
		msb = (msb > nbits - 1 ? nbits - 1 : msb);
		unsigned topBlock = msb / bitsInBlock;
		bt mask = bt(ALL_ONES >> (bitsInBlock - 1 - (msb % bitsInBlock)));
		for (unsigned i = 0; i < topBlock; ++i) {
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

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// integer - integer logic comparisons
	friend constexpr bool operator==(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		for (unsigned i = 0; i < lhs.nrBlocks; ++i) {
			if (lhs._block[i] != rhs._block[i]) {
				return false;
			}
		}
		return true;
	}

	friend constexpr bool operator!=(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		return !operator==(lhs, rhs);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// logic operators

	friend constexpr bool operator<(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		blockfraction diff;
		diff.sub(lhs, rhs);
		return diff.isneg();
	}

	friend constexpr bool operator<=(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		return (lhs < rhs || lhs == rhs);
	}

	friend constexpr bool operator>(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		return !(lhs <= rhs);
	}

	friend constexpr bool operator>=(const blockfraction& lhs, const blockfraction& rhs) noexcept {
		return !(lhs < rhs);
	}

	///////////////////////////////////////////////////////////////////////////////
	// binary operators

	friend constexpr  blockfraction operator<<(const blockfraction& a, const long b) noexcept {
		blockfraction c(a);
		return c <<= b;
	}

	friend constexpr  blockfraction operator>>(const blockfraction& a, const long b) noexcept {
		blockfraction c(a);
		return c >>= b;
	}


	// ostream operator
	friend std::ostream& operator<<(std::ostream& ostr, const blockfraction& v) {
		return ostr << double(v);
	}
};

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the blockfraction: 00h.ffff
// by design, the radix point is at nbits-3
template<unsigned nbits, typename bt>
std::string to_binary(const blockfraction<nbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = nbits - 1; i >= 0; --i) {
		s << (number.at(unsigned(i)) ? '1' : '0');
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
template<unsigned nbits, typename bt>
std::string to_hex(const blockfraction<nbits, bt>& number, bool wordMarker = true) {
	static constexpr unsigned bitsInByte = 8;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	int nrNibbles = int(1 + ((nbits - 1) >> 2));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = number.nibble(static_cast<unsigned>(n));
		ss << hexChar[nibble];
		if (wordMarker && n > 0 && ((n * 4ll) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

///////////////////////////////////////////////////////////////////////////////
// specialty binary operators


#define TRACE_DIV 0
// unrounded division, returns a blockfraction that is of size 2*nbits
template<unsigned nbits, unsigned roundingBits, typename bt>
blockfraction<2 * nbits + roundingBits, bt> urdiv(const blockfraction<nbits, bt>& a, const blockfraction<nbits, bt>& b, blockfraction<roundingBits, bt>& r) {
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
	blockfraction<nbits + 1, bt> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockfraction<nbits + 1, bt> b_new(b);
	if (a_sign) a_new.twoscomplement();
	if (b_sign) b_new.twoscomplement();

	// initialize the long division
	blockfraction<2 * nbits + roundingBits, bt> decimator(a_new);
	blockfraction<2 * nbits + roundingBits, bt> subtractand(b_new); // prepare the subtractand
	blockfraction<2 * nbits + roundingBits, bt> result;

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
			result.set(static_cast<unsigned>(i));
		}
		else {
			result.reset(static_cast<unsigned>(i));
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
template<unsigned nbits, typename bt>
constexpr blockfraction<nbits, bt> twosComplementFree(const blockfraction<nbits, bt>& a) noexcept {
	blockfraction<nbits, bt> b(a);
	return b.twosComplement();
}

}} // namespace sw::universal
