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

// compiler specific operators
#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw::universal {

// forward references
template<size_t nbits, typename bt> class blockfraction;
template<size_t nbits, typename bt> constexpr blockfraction<nbits, bt> twosComplement(const blockfraction<nbits, bt>&);
template<size_t nbits, typename bt> struct quorem;
template<size_t nbits, typename bt> quorem<nbits, bt> longdivision(const blockfraction<nbits, bt>&, const blockfraction<nbits, bt>&);

// idiv_t for blockfraction<nbits> to capture quotient and remainder during long division
template<size_t nbits, typename bt>
struct fractionquorem {
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
/// a block-based floating-point fraction of the form  ##.ff---ff
/// for add/sub, expanded to ##.ff---ffaaa
/// for mul, expanded to ##.ff--ffff--ff
/// 
/// NOTE: don't set a default blocktype as this makes the integration more brittle
/// as blocktriple uses the blockfraction as storage class and thus needs to be
/// in sync.
/// </summary>
/// <typeparam name="bt"></typeparam>
template<size_t _nbits, typename bt>
class blockfraction {
public:
	typedef bt BlockType;
	static constexpr size_t nbits = _nbits;
	static constexpr size_t bitsInByte = 8;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint64_t");

	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFul >> (64 - bitsInBlock));
	static constexpr bt maxBlockValue = bt(-1);

	static constexpr size_t MSU = nrBlocks - 1; // MSU == Most Significant Unit
	static constexpr bt ALL_ONES = bt(~0);
	static constexpr bt MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt SIGN_BIT_MASK = bt(bt(1) << ((nbits - 1ull) % bitsInBlock));

	// constructors
	constexpr blockfraction() noexcept : _block{ 0 } {}

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
	explicit operator long double() const        { return (long double)to_long_double(); }

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
	//

	/// <summary>
	/// add two fractions of the form 0h.fffff, that is, radix point at nbits-2
	/// by design, the carry gets lopped off
	/// </summary>
	/// <param name="lhs">nbits of fraction in the form 0h.ffff</param>
	/// <param name="rhs">nbits of fraction in the form 0h.ffff</param>
	void add(const blockfraction<nbits, bt>& lhs, const blockfraction<nbits, bt>& rhs) {
		bool carry = false;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			// cast up so we can test for overflow
			uint64_t l = uint64_t(lhs._block[i]);
			uint64_t r = uint64_t(rhs._block[i]);
			uint64_t s = l + r + (carry ? uint64_t(1) : uint64_t(0));
			carry = (s > maxBlockValue);
			_block[i] = bt(s);
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
	}
	/// <summary>
	/// add two fractions of the form 0h.ffff and produce a result of the form 0hf.ffff
	/// </summary>
	/// <param name="lhs">nbits-1 of fraction in the form 0h.ffff</param>
	/// <param name="rhs">nbits-1 of fraction in the form 0h.ffff</param>
	void uradd(const blockfraction<nbits-1, bt>& lhs, const blockfraction<nbits-1, bt>& rhs) {
		bool carry = false;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			// cast up so we can test for overflow
			uint64_t l = uint64_t(lhs._block[i]);
			uint64_t r = uint64_t(rhs._block[i]);
			uint64_t s = l + r + (carry ? uint64_t(1) : uint64_t(0));
			carry = (s > maxBlockValue);
			_block[i] = bt(s);
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
	}
	void sub(const blockfraction<nbits, bt>& lhs, blockfraction<nbits, bt>& rhs) {
		add(lhs, rhs.twosComplement());
	}
	void mul(const blockfraction<nbits, bt>& lhs, const blockfraction<nbits, bt>& rhs) {
		blockfraction<nbits, bt> base(lhs);
		blockfraction<nbits, bt> multiplicant(rhs);
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
	// division operator
	void div(const blockfraction<nbits, bt>& lhs, const blockfraction<nbits, bt>& rhs) {
//		quorem<nbits, bt> result = longdivision(*this, rhs);
//		*this = result.quo;
	}

#ifdef FRACTION_REMAINDER
	// remainder operator
	blockfraction& operator%=(const blockfraction& rhs) {
//		quorem<nbits, bt> result = longdivision(*this, rhs);
//		*this = result.rem;
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
		bool signext = sign();
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
				// fix up the leading zeros if we have a negative number
				if (signext) {
					// bitsToShift is guaranteed to be less than nbits
					bitsToShift += static_cast<int>(blockShift * bitsInBlock);
					for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
						this->setBit(i);
					}
				}
				else {
					// clean up the blocks we have shifted clean
					bitsToShift += static_cast<int>(blockShift * bitsInBlock);
					for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
						this->setBit(i, false);
					}
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

		// fix up the leading zeros if we have a negative number
		if (signext) {
			// bitsToShift is guaranteed to be less than nbits
			bitsToShift += static_cast<int>(blockShift * bitsInBlock);
			for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
				this->setBit(i);
			}
		}
		else {
			// clean up the blocks we have shifted clean
			bitsToShift += static_cast<int>(blockShift * bitsInBlock);
			for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
				this->setBit(i, false);
			}
		}

		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}

	// modifiers
	 // clear a block binary number
	inline constexpr void clear() noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(0ull);
		}
	}
	inline constexpr void setzero() noexcept { clear(); }
	inline constexpr void setBit(size_t i, bool v = true) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt((block & null) | mask);
		}
		// when i is out of bounds, fail silently as no-op
	}
	inline constexpr void setBlock(size_t b, const bt& block) noexcept {
		if (b < nrBlocks) _block[b] = block;
		// when b is out of bounds, fail silently as no-op
	}
	inline constexpr void setBits(uint64_t value) noexcept {
		if constexpr (1 == nrBlocks) {
			_block[0] = value & storageMask;
		}
		else if constexpr (1 < nrBlocks) {
			for (size_t i = 0; i < nrBlocks; ++i) {
				_block[i] = value & storageMask;
				value >>= bitsInBlock;
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
		blockfraction<nbits, bt> plusOne;
		plusOne.setBit(0);
		flip();
		add(*this, plusOne);
		return *this;
	}

	// selectors
	inline constexpr bool sign() const noexcept { return _block[MSU] & SIGN_BIT_MASK; }
	inline constexpr bool ispos() const noexcept { return !sign(); }
	inline constexpr bool isneg() const noexcept { return sign(); }
	inline constexpr bool iszero() const noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) if (_block[i] != 0) return false;
		return true;
	}
	inline constexpr bool isodd() const noexcept { return _block[0] & 0x1;	}
	inline constexpr bool iseven() const noexcept { return !isodd(); }
	inline constexpr bool test(size_t bitIndex) const {
		return at(bitIndex);
	}
	inline constexpr bool at(size_t bitIndex) const {
		if (bitIndex < nbits) {
			bt word = _block[bitIndex / bitsInBlock];
			bt mask = bt(1ull << (bitIndex % bitsInBlock));
			return (word & mask);
		}
		throw "bit index out of bounds";
	}
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
	inline constexpr bt block(size_t b) const {
		if (b >= nrBlocks) throw "block index out of bounds";
		return _block[b];
	}

	// copy a value over from one blockfraction to this blockfraction
	// blockfraction is a 2's complement encoding, so we sign-extend by default
	template<size_t srcbits>
	inline blockfraction<nbits, bt>& assign(const blockfraction<srcbits, bt>& rhs) {
		clear();
		// since bt is the same, we can simply copy the blocks in
		size_t minNrBlocks = (this->nrBlocks < rhs.nrBlocks) ? this->nrBlocks : rhs.nrBlocks;
		for (size_t i = 0; i < minNrBlocks; ++i) {
			_block[i] = rhs.block(i);
		}
		if constexpr (nbits > srcbits) { // check if we need to sign extend
			if (rhs.sign()) {
				for (size_t i = srcbits; i < nbits; ++i) { // TODO: replace bit-oriented sequence with block
					setBit(i);
				}
			}
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}

#ifdef DEPRECATED
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
#endif

	// conversion to native types
	inline constexpr float to_float() const noexcept {
		float f{ 0.0f };
		// nbits in the form 0h.fffff in 2's complement, so check if we are negative and fix that first
		blockfraction<nbits, bt> tmp(*this);
		if (test(nbits - 1)) tmp.twosComplement();
		if (test(nbits - 2)) f = 1.0f;  // check hidden bit
		// enumerate from the smallest bit position and add and increment value
		if (nbits < 21) { // check if we can represent this value with a native normal float with 23 fraction bits => nbits <= (23 - 2)
			float v = std::pow(0.5f, float(nbits - 2));
			for (size_t i = 0; i < nbits - 2u; ++i) {
				if (test(i)) f += v;
				v *= 2.0;
			}
		}
		else {
			std::cerr << "to_float() will yield inaccurate result since blockfraction has more precision than native IEEE-754 double\n";
		}
		return f;
	}
	inline constexpr double to_double() const noexcept {
		double d{ 0.0 };
		// nbits in the form 0h.fffff in 2's complement, so check if we are negative and fix that first
		blockfraction<nbits, bt> tmp(*this);
		if (test(nbits - 1)) tmp.twosComplement();
		if (test(nbits - 2)) d = 1.0;  // check hidden bit
		// enumerate from the smallest bit position and add and increment value
		if (nbits < 51) { // check if we can represent this value with a native normal double with 52 fraction bits => nbits <= (52 - 2)
			double v = std::pow(0.5, double(nbits - 2));
			for (size_t i = 0; i < nbits - 2u; ++i) {
				if (test(i)) d += v;
				v *= 2.0;
			}
		}
		else {
			std::cerr << "to_double() will yield inaccurate result since blockfraction has more precision than native IEEE-754 double\n";
		}
		return d;
	}
	inline constexpr long double to_long_double() const noexcept {
		return (long double)to_double();
	}
	// determine the rounding mode: result needs to be rounded up if true
	bool roundingMode(size_t targetLsb) const {
		bool lsb = at(targetLsb);
		bool guard = (targetLsb == 0 ? false : at(targetLsb - 1));
		bool round = (targetLsb > 1 ? at(targetLsb - 2) : false);
		bool sticky =(targetLsb < 3 ? false : any(targetLsb - 3));
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
	bt _block[nrBlocks];

private:
	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// integer - integer logic comparisons
	template<size_t N, typename B>
	friend bool operator==(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs);
	template<size_t N, typename B>
	friend bool operator!=(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs);
	// the other logic operators are defined in terms of arithmetic terms

	template<size_t nnbits, typename Bbt>
	friend std::ostream& operator<<(std::ostream& ostr, const blockfraction<nnbits, Bbt>& v);
};

//////////////////////////////////////////////////////////////////////////////////
// logic operators

template<size_t N, typename B>
inline bool operator==(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs) {
	for (size_t i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<size_t N, typename B>
inline bool operator!=(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t N, typename B>
inline bool operator<(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs) {
	if (lhs.ispos() && rhs.isneg()) return false; // need to filter out possible overflow conditions
	if (lhs.isneg() && rhs.ispos()) return true;  // need to filter out possible underflow conditions
	if (lhs == rhs) return false; // so the maxneg logic works
	blockfraction<N, B> mneg; maxneg<N, B>(mneg);
	if (rhs == mneg) return false; // special case: nothing is smaller than maximum negative
	blockfraction<N, B> diff = lhs - rhs;
	return diff.isneg();
}
template<size_t N, typename B>
inline bool operator<=(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs) {
	return (lhs < rhs || lhs == rhs);
}
template<size_t N, typename B>
inline bool operator>(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs) {
	return !(lhs <= rhs);
}
template<size_t N, typename B>
inline bool operator>=(const blockfraction<N, B>& lhs, const blockfraction<N, B>& rhs) {
	return !(lhs < rhs);
}

///////////////////////////////////////////////////////////////////////////////
// binary operators

template<size_t nbits, typename bt>
inline blockfraction<nbits, bt> operator<<(const blockfraction<nbits, bt>& a, const long b) {
	blockfraction<nbits, bt> c(a);
	return c <<= b;
}
template<size_t nbits, typename bt>
inline blockfraction<nbits, bt> operator>>(const blockfraction<nbits, bt>& a, const long b) {
	blockfraction<nbits, bt> c(a);
	return c >>= b;
}

// divide a by b and return both quotient and remainder
template<size_t nbits, typename bt>
quorem<nbits, bt> longdivision(const blockfraction<nbits, bt>& _a, const blockfraction<nbits, bt>& _b) {
	quorem<nbits, bt> result = { 0, 0, 0 };
	if (_b.iszero()) {
		result.exceptionId = 1; // division by zero
		return result;
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = _a.sign();
	bool b_sign = _b.sign();
	bool result_negative = (a_sign ^ b_sign);
	// normalize both arguments to positive, which requires expansion by 1-bit to deal with maxneg
	blockfraction<nbits + 1, bt> a(_a);
	blockfraction<nbits + 1, bt> b(_b);
	if (a_sign) a.twoscomplement();
	if (b_sign) b.twoscomplement();

	if (a < b) { // optimization for integer numbers
		result.rem = _a; // a % b = a when a / b = 0
		return result;   // a / b = 0 when b > a
	}
	// initialize the long division
	blockfraction<nbits + 1, bt> accumulator = a;
	// prepare the subtractand
	blockfraction<nbits + 1, bt> subtractand = b;
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
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// specialty binary operators


#define TRACE_DIV 0
// unrounded division, returns a blockfraction that is of size 2*nbits
template<size_t nbits, size_t roundingBits, typename bt>
inline blockfraction<2 * nbits + roundingBits, bt> urdiv(const blockfraction<nbits, bt>& a, const blockfraction<nbits, bt>& b, blockfraction<roundingBits, bt>& r) {
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
	if (result_negative) result.twoscomplement();
	r.assign(result); // copy the lowest bits which represent the bits on which we need to apply the rounding test
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the storage
template<size_t nbits, typename bt>
std::string to_binary(const blockfraction<nbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << 'b';
	for (int i = int(nbits - 1); i >= 0; --i) {
		s << (number.at(size_t(i)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}

// local helper to display the contents of a byte array
template<size_t nbits, typename bt>
std::string to_hex(const blockfraction<nbits, bt>& number, bool wordMarker = true) {
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

// ostream operator
template<size_t nbits, typename bt>
std::ostream& operator<<(std::ostream& ostr, const blockfraction<nbits, bt>& number) {
	return ostr << to_binary(number);
}

// free function generator of the 2's complement of a blockfraction
template<size_t nbits, typename bt> 
inline constexpr blockfraction<nbits, bt> twosComplement(const blockfraction<nbits, bt>& a) {
	blockfraction<nbits, bt> b(a);
	return b.twosComplement();
}

} // namespace sw::universal
