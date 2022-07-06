#pragma once
// blockbinary.hpp: parameterized blocked binary number system representing a 2's complement binary number
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>

// should be defined by calling environment, just catching it here just in case it is not
#ifndef LONG_DOUBLE_SUPPORT
#pragma message("LONG_DOUBLE_SUPPORT is not defined")
#define LONG_DOUBLE_SUPPORT 0
#endif

namespace sw { namespace universal {

enum class BinaryNumberType {
	Signed = 0,  // { ...,-3,-2,-1,0,1,2,3,... }
	Unsigned = 1 // {              0,1,2,3,... }
};

// forward references
template<size_t nbits, typename BlockType, BinaryNumberType NumberType> class blockbinary;
template<size_t nbits, typename BlockType, BinaryNumberType NumberType> blockbinary<nbits, BlockType, NumberType> twosComplement(const blockbinary<nbits, BlockType, NumberType>&);
template<size_t nbits, typename BlockType, BinaryNumberType NumberType> struct quorem;
template<size_t nbits, typename BlockType, BinaryNumberType NumberType> quorem<nbits, BlockType, NumberType> longdivision(const blockbinary<nbits, BlockType, NumberType>&, const blockbinary<nbits, BlockType, NumberType>&);

// idiv_t for blockbinary<nbits> to capture quotient and remainder during long division
template<size_t nbits, typename BlockType, BinaryNumberType NumberType>
struct quorem {
	int exceptionId;
	blockbinary<nbits, BlockType, NumberType> quo; // quotient
	blockbinary<nbits, BlockType, NumberType> rem;  // remainder
};

// maximum positive 2's complement number: b01111...1111
template<size_t nbits, typename BlockType = uint8_t, BinaryNumberType NumberType>
constexpr blockbinary<nbits, BlockType, NumberType>& maxpos(blockbinary<nbits, BlockType, NumberType>& a) {
	a.clear();
	a.flip();
	if constexpr (NumberType == BinaryNumberType::Signed) {
		a.setbit(nbits - 1, false);
	}
	return a;
}

// maximum negative 2's complement number: b1000...0000
template<size_t nbits, typename BlockType = uint8_t, BinaryNumberType NumberType>
constexpr blockbinary<nbits, BlockType, NumberType>& maxneg(blockbinary<nbits, BlockType, NumberType>& a) {
	a.clear();
	if constexpr (NumberType == BinaryNumberType::Signed) {
		a.setbit(nbits - 1);
	}
	return a;
}

// generate the 2's complement of the block binary number
template<size_t nbits, typename BlockType, BinaryNumberType NumberType>
blockbinary<nbits, BlockType, NumberType> twosComplement(const blockbinary<nbits, BlockType, NumberType>& orig) {
	blockbinary<nbits, BlockType, NumberType> twosC(orig);
	blockbinary<nbits, BlockType, NumberType> plusOne(1);
	twosC.flip();
	twosC += plusOne;
	return twosC;
}

/*
NOTES

for block arithmetic, we need to manage a carry bit.
This disqualifies using uint64_t as a block type as we can't catch the overflow condition
in the same way as the other native types, uint8_t, uint16_t, uint32_t.

We could use a sint64_t and then convert to uint64_t and observe the MSB. Very different 
logic though.
*/

// a block-based binary number configurable to be signed or unsigned. When signed it uses 2's complement encoding
template<size_t _nbits, typename BlockType = uint8_t, BinaryNumberType _NumberType = BinaryNumberType::Signed>
class blockbinary {
public:
	static constexpr size_t nbits = _nbits;
	typedef BlockType bt;
	static constexpr BinaryNumberType NumberType = _NumberType;

	static constexpr size_t   bitsInByte = 8;
	static constexpr size_t   bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t   nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr bt       maxBlockValue = bt(-1);

	static constexpr size_t   MSU = nrBlocks - 1; // MSU == Most Significant Unit
	static constexpr bt       ALL_ONES = bt(~0);
	static constexpr bt       MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK = bt(bt(1) << ((nbits - 1ull) % bitsInBlock));

	static constexpr bool     uniblock64 = (bitsInBlock == 64) && (nrBlocks == 1);
	static_assert(bitsInBlock < 64 || uniblock64, "storage unit for multi-block arithmetic needs to be one of [uint8_t | uint16_t | uint32_t]");

	// trivial constructor
	blockbinary() = default;

	/// construct a blockbinary from another: bt must be the same
	template<size_t nnbits>
	blockbinary(const blockbinary<nnbits, BlockType, NumberType>& rhs) { this->assign(rhs); }

	// initializer for long long
	constexpr blockbinary(long long initial_value) noexcept : _block{ 0 } { *this = initial_value; }

	constexpr blockbinary& operator=(long long rhs) noexcept {
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

	// conversion operators
	explicit operator int() const                { return int(to_long_long()); }
	explicit operator long() const               { return long(to_long_long()); }
	explicit operator long long() const          { return to_long_long(); }
	explicit operator unsigned int() const       { return unsigned(to_ull()); }
	explicit operator unsigned long() const      { return (unsigned long)to_ull(); }
	explicit operator unsigned long long() const { return to_ull(); }
	// TODO: these need proper implementations that can convert very large integers to the proper scale afforded by the floating-point formats
	explicit operator float() const              { return float(to_long_long()); }
	explicit operator double() const             { return double(to_long_long()); }

#if LONG_DOUBLE_SUPPORT
	explicit operator long double() const        { return (long double)to_long_long(); }
#endif

	// access operators
	constexpr BlockType& operator[](size_t index) { return _block[index]; }
	constexpr BlockType operator[](size_t index) const { return _block[index]; }

	// prefix operators
	blockbinary operator-() const {
		blockbinary negated(*this);
		blockbinary plusOne(1);
		negated.flip();
		negated += plusOne;
		return negated;
	}
	// one's complement
	blockbinary operator~() const {
		blockbinary complement(*this);
		complement.flip();
		return complement;
	}
	// increment/decrement
	blockbinary operator++(int) {
		blockbinary tmp(*this);
		operator++();
		return tmp;
	}
	blockbinary& operator++() {
		blockbinary increment;
		increment.setbits(0x1);
		*this += increment;
		return *this;
	}
	blockbinary operator--(int) {
		blockbinary tmp(*this);
		operator--();
		return tmp;
	}
	blockbinary& operator--() {
		blockbinary decrement;
		decrement.setbits(0x1);
		return *this -= decrement;
	}
	// logic operators
	blockbinary  operator~() {
		blockbinary<nbits, bt> complement(*this);
		complement.flip();
		return complement;
	}
	// arithmetic operators
	blockbinary& operator+=(const blockbinary& rhs) {
		if constexpr (nrBlocks == 1) {
			_block[0] = static_cast<bt>(_block[0] + rhs.block(0));
			// null any leading bits that fall outside of nbits
			_block[MSU] = static_cast<bt>(MSU_MASK & _block[MSU]);
		}
		else {
			blockbinary sum;
			BlockType* pA = _block;
			BlockType const* pB = rhs._block;
			BlockType* pC = sum._block;
			BlockType* pEnd = pC + nrBlocks;
			std::uint64_t carry = 0;
			while (pC != pEnd) {
				carry += static_cast<std::uint64_t>(*pA) + static_cast<std::uint64_t>(*pB);
				*pC = static_cast<bt>(carry);
				carry >>= bitsInBlock;
				++pA; ++pB; ++pC;
			}
			// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
			BlockType* pLast = pEnd - 1;
			*pLast = static_cast<bt>(MSU_MASK & *pLast);
			*this = sum;
		}
		return *this;
	}
	blockbinary& operator-=(const blockbinary& rhs) {
		return operator+=(sw::universal::twosComplement(rhs));
	}
#define BLOCKBINARY_FAST_MUL
#ifdef BLOCKBINARY_FAST_MUL
	blockbinary& operator*=(const blockbinary& rhs) {
		if constexpr (NumberType == BinaryNumberType::Signed) {
			if constexpr (nrBlocks == 1) {
				_block[0] = static_cast<bt>(_block[0] * rhs.block(0));
			}
			else {
				// is there a better way than upconverting to deal with maxneg in a 2's complement encoding?
				blockbinary<nbits + 1, BlockType, NumberType> base(*this);
				blockbinary<nbits + 1, BlockType, NumberType> multiplicant(rhs);
				bool resultIsNeg = (base.isneg() ^ multiplicant.isneg());
				if (base.isneg()) {
					base.twosComplement();
				}
				if (multiplicant.isneg()) {
					multiplicant.twosComplement();
				}
				clear();
				for (unsigned i = 0; i < static_cast<unsigned>(nrBlocks); ++i) {
					std::uint64_t segment(0);
					for (unsigned j = 0; j < static_cast<unsigned>(nrBlocks); ++j) {
						segment += static_cast<std::uint64_t>(base.block(i)) * static_cast<std::uint64_t>(multiplicant.block(j));

						if (i + j < static_cast<unsigned>(nrBlocks)) {
							segment += _block[i + j];
							_block[i + j] = static_cast<bt>(segment);
							segment >>= bitsInBlock;
						}
					}
				}
				if (resultIsNeg) twosComplement();
			}
		}
		else {  // unsigned
			if constexpr (nrBlocks == 1) {
				_block[0] = static_cast<bt>(_block[0] * rhs.block(0));
			}
			else {
				blockbinary<nbits, BlockType, NumberType> base(*this);
				blockbinary<nbits, BlockType, NumberType> multiplicant(rhs);
				clear();
				for (unsigned i = 0; i < static_cast<unsigned>(nrBlocks); ++i) {
					std::uint64_t segment(0);
					for (unsigned j = 0; j < static_cast<unsigned>(nrBlocks); ++j) {
						segment += static_cast<std::uint64_t>(base.block(i)) * static_cast<std::uint64_t>(multiplicant.block(j));

						if (i + j < static_cast<unsigned>(nrBlocks)) {
							segment += _block[i + j];
							_block[i + j] = static_cast<bt>(segment);
							segment >>= bitsInBlock;
						}
					}
				}
			}
		}
		// null any leading bits that fall outside of nbits
		_block[MSU] = static_cast<bt>(MSU_MASK & _block[MSU]);
		return *this;
	}
#else
	blockbinary& operator*=(const blockbinary& rhs) { // modulo in-place
		blockbinary base(*this);
		blockbinary multiplicant(rhs);
		clear();
		for (size_t i = 0; i < nbits; ++i) {
			if (base.at(i)) {
				operator+=(multiplicant);
			}
			multiplicant <<= 1;
		}
		// since we used operator+=, which enforces the nulling of leading bits
		// we don't need to null here
		return *this;
	}
#endif
	blockbinary& operator/=(const blockbinary& rhs) {
		if constexpr (nbits == (sizeof(BlockType) * 8)) {
			if (rhs.iszero()) {
				*this = 0;
				return *this;
			}
			if constexpr (sizeof(BlockType) == 1) {
				_block[0] = static_cast<bt>(std::int8_t(_block[0]) / std::int8_t(rhs._block[0]));
			}
			else if constexpr (sizeof(BlockType) == 2) {
				_block[0] = static_cast<bt>(std::int16_t(_block[0]) / std::int16_t(rhs._block[0]));
			}
			else if constexpr (sizeof(BlockType) == 4) {
				_block[0] = static_cast<bt>(std::int32_t(_block[0]) / std::int32_t(rhs._block[0]));
			}
			else if constexpr (sizeof(BlockType) == 8) {
				_block[0] = static_cast<bt>(std::int64_t(_block[0]) / std::int64_t(rhs._block[0]));
			}
			_block[0] = static_cast<bt>(MSU_MASK & _block[0]);
		}
		else {
			quorem<nbits, BlockType, NumberType> result = longdivision(*this, rhs);
			*this = result.quo;
		}
		return *this;
	}
	blockbinary& operator%=(const blockbinary& rhs) {
		if constexpr (nbits == (sizeof(BlockType) * 8)) {
			if (rhs.iszero()) {
				*this = 0;
				return *this;
			}
			if constexpr (sizeof(BlockType) == 1) {
				_block[0] = static_cast<bt>(std::int8_t(_block[0]) % std::int8_t(rhs._block[0]));
			}
			else if constexpr (sizeof(BlockType) == 2) {
				_block[0] = static_cast<bt>(std::int16_t(_block[0]) % std::int16_t(rhs._block[0]));
			}
			else if constexpr (sizeof(BlockType) == 4) {
				_block[0] = static_cast<bt>(std::int32_t(_block[0]) % std::int32_t(rhs._block[0]));
			}
			else if constexpr (sizeof(BlockType) == 8) {
				_block[0] = static_cast<bt>(std::int64_t(_block[0]) % std::int64_t(rhs._block[0]));
			}
			_block[0] = static_cast<bt>(MSU_MASK & _block[0]);
		}
		else {
			quorem<nbits, BlockType, NumberType> result = longdivision(*this, rhs);
			*this = result.rem;
		}
		return *this;
	}
	// shift left operator
	blockbinary& operator<<=(int bitsToShift) {
		if (bitsToShift == 0) return *this;
		if (bitsToShift < 0) return operator>>=(-bitsToShift);
		if (bitsToShift > static_cast<int>(nbits)) {
			setzero();
			return *this;
		}
		if (bitsToShift >= static_cast<int>(bitsInBlock)) {
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
			// construct the mask for the upper bits in the block that needs to move to the higher word
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
	// arithmetic shift right operator
	blockbinary& operator>>=(int bitsToShift) {
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
						this->setbit(i);
					}
				}
				else {
					// clean up the blocks we have shifted clean
					bitsToShift += static_cast<int>(blockShift * bitsInBlock);
					for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
						this->setbit(i, false);
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
				this->setbit(i);
			}
		}
		else {
			// clean up the blocks we have shifted clean
			bitsToShift += static_cast<int>(blockShift * bitsInBlock);
			for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
				this->setbit(i, false);
			}
		}

		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}

	// modifiers
	 // clear a block binary number
	constexpr void clear() noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(0ull);
		}
	}
	constexpr void setzero() noexcept { clear(); }
	constexpr void setbit(size_t i, bool v = true) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt((block & null) | mask);
		}
		// nop if i is out of range
	}
	constexpr void setbits(uint64_t value) noexcept {
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
	constexpr void setblock(size_t b, const bt& block) noexcept {
		if (b < nrBlocks) _block[b] = block; // nop if b is out of range
	}	
	constexpr blockbinary& flip() noexcept { // in-place one's complement
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}		
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	constexpr blockbinary& twosComplement() noexcept { // in-place 2's complement
		blockbinary<nbits, bt> plusOne(1);
		flip();
		return *this += plusOne;
	}

	// selectors
	constexpr bool sign() const noexcept { return _block[MSU] & SIGN_BIT_MASK; }
	constexpr bool ispos() const noexcept { return !sign(); }
	constexpr bool isneg() const noexcept { return sign(); }
	constexpr bool iszero() const noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) if (_block[i] != 0) return false;
		return true;
	}
	constexpr bool isallones() const noexcept {
		if constexpr (nrBlocks > 1) for (size_t i = 0; i < nrBlocks-1; ++i) if (_block[i] != ALL_ONES) return false;
		if (_block[MSU] != MSU_MASK) return false;
		return true;
	}
	constexpr bool isodd() const noexcept { return _block[0] & 0x1;	}
	constexpr bool iseven() const noexcept { return !isodd(); }
	constexpr bool test(size_t bitIndex) const noexcept { return at(bitIndex); }
	constexpr bool at(size_t bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	constexpr uint8_t nibble(size_t n) const noexcept {
		uint8_t retval{ 0 };
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			size_t nibbleIndexInWord = n % (bitsInBlock >> 2);
			bt mask = static_cast<bt>(0x0Fu << (nibbleIndexInWord*4));
			bt nibblebits = static_cast<bt>(mask & word);
			retval = static_cast<uint8_t>(nibblebits >> static_cast<bt>(nibbleIndexInWord*4));
		}
		else { // nop when nibble index out of bounds
			retval = 0;
		}
		return retval;
	}
	constexpr bt block(size_t b) const noexcept {
		if (b < nrBlocks) return _block[b]; 
		return bt(0); // return 0 when block index out of bounds
	}

	// copy a value over from one blockbinary to this blockbinary
	// blockbinary is a 2's complement encoding, so we sign-extend by default
	template<size_t srcbits>
	blockbinary<nbits, bt>& assign(const blockbinary<srcbits, bt>& rhs) {
		clear();
		// since bt is the same, we can simply copy the blocks in
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

	// copy a value over from one blockbinary to this without sign-extending the value
	// blockbinary is a 2's complement encoding, so we sign-extend by default
	// for fraction/significent encodings, we need to turn off sign-extending.
	template<size_t srcbits>
	blockbinary<nbits, bt>& assignWithoutSignExtend(const blockbinary<srcbits, bt>& rhs) {
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
	int msb() const noexcept {
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
	int64_t to_long_long() const {
		constexpr unsigned sizeoflonglong = 8 * sizeof(long long);
		int64_t ll{ 0 };
		int64_t mask{ 1 };
		unsigned upper = (nbits < sizeoflonglong ? nbits : sizeoflonglong);
		for (unsigned i = 0; i < upper; ++i) {
			ll |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < sizeoflonglong) { // sign extend
			for (unsigned i = upper; i < sizeoflonglong; ++i) {
				ll |= mask;
				mask <<= 1;
			}
		}
		return ll;
	}
	uint64_t to_ull() const {
		uint64_t ull{ 0 };
		uint64_t mask{ 1 };
		uint32_t msb = nbits < 64 ? nbits : 64;
		for (uint32_t i = 0; i < msb; ++i) {
			ull |= at(i) ? mask : 0;
			mask <<= 1;
		}
		return ull;
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

private:
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// integer - integer logic comparisons
	template<size_t N, typename B, BinaryNumberType T>
	friend bool operator==(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs);
	template<size_t N, typename B, BinaryNumberType T>
	friend bool operator!=(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs);
	// the other logic operators are defined in terms of arithmetic terms

	template<size_t N, typename B, BinaryNumberType T>
	friend std::ostream& operator<<(std::ostream& ostr, const blockbinary<N, B, T>& v);
};

// Generate a type tag for blockbinary
template<size_t N, typename B, BinaryNumberType T>
std::string type_tag(const blockbinary<N, B, T>& v) {
	std::stringstream str;
	if (v.isneg()) str << ' '; // remove 'unreferenced formal parameter warning from compilation log
	str << "blockbinary<"
		<< std::setw(4) << N << ", "
		<< typeid(B).name() << ", "
		<< typeid(T).name() << '>';
	return str.str();
}

//////////////////////////////////////////////////////////////////////////////////
// logic operators

template<size_t N, typename B, BinaryNumberType T>
inline bool operator==(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs) {
	for (size_t i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<size_t N, typename B, BinaryNumberType T>
inline bool operator!=(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t N, typename B, BinaryNumberType T>
inline bool operator<(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs) {
	if (lhs.ispos() && rhs.isneg()) return false; // need to filter out possible overflow conditions
	if (lhs.isneg() && rhs.ispos()) return true;  // need to filter out possible underflow conditions
	if (lhs == rhs) return false; // so the maxneg logic works
	blockbinary<N, B> mneg; maxneg<N, B>(mneg);
	if (rhs == mneg) return false; // special case: nothing is smaller than maximum negative
	blockbinary<N, B> diff = lhs - rhs;
	return diff.isneg();
}
template<size_t N, typename B, BinaryNumberType T>
inline bool operator<=(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs) {
	return (lhs < rhs || lhs == rhs);
}
template<size_t N, typename B, BinaryNumberType T>
inline bool operator>(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs) {
	return !(lhs <= rhs);
}
template<size_t N, typename B, BinaryNumberType T>
inline bool operator>=(const blockbinary<N, B, T>& lhs, const blockbinary<N, B, T>& rhs) {
	return !(lhs < rhs);
}
///////////////////////////////////////////////////////////////////////////////
// binary operators

template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N, B, T> operator+(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<N, B, T> c(a);
	return c += b;
}
template<size_t N, typename B, BinaryNumberType T >
inline blockbinary<N, B, T> operator-(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<N, B, T> c(a);
	return c -= b;
}
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N, B, T> operator*(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<N, B, T> c(a);
	return c *= b;
}
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N, B, T> operator/(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<N, B, T> c(a);
	return c /= b;
}
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N, B, T> operator%(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<N, B, T> c(a);
	return c %= b;
}

template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N, B, T> operator<<(const blockbinary<N, B, T>& a, const long b) {
	blockbinary<N, B, T> c(a);
	return c <<= b;
}
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N, B, T> operator>>(const blockbinary<N, B, T>& a, const long b) {
	blockbinary<N, B, T> c(a);
	return c >>= b;
}

// divide a by b and return both quotient and remainder
template<size_t N, typename B, BinaryNumberType T>
quorem<N, B, T> longdivision(const blockbinary<N, B, T>& _a, const blockbinary<N, B, T>& _b) {
	using BlockBinary = blockbinary<N + 1, B, T>;
	quorem<N, B, T> result = { 0, 0, 0 };
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
	BlockBinary a(_a);
	BlockBinary b(_b);
	if (a_sign) a.twosComplement();
	if (b_sign) b.twosComplement();

	if (a < b) { // optimization for integer numbers
		result.rem = _a; // a % b = a when a / b = 0
		return result;   // a / b = 0 when b > a
	}
	// initialize the long division
	BlockBinary accumulator = a;
	// prepare the subtractand
	BlockBinary subtractand = b;
	int msb_b = b.msb();
	int msb_a = a.msb();
	int shift = msb_a - msb_b;
	subtractand <<= shift;
	// long division
	for (int i = shift; i >= 0; --i) {
		if (subtractand <= accumulator) {
			accumulator -= subtractand;
			result.quo.setbit(static_cast<size_t>(i));
		}
		else {
			result.quo.setbit(static_cast<size_t>(i), false);
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

// unrounded addition, returns a blockbinary that is of size nbits+1
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N + 1, B, T> uradd(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<N + 1, B, T> result(a);
	return result += blockbinary<N + 1, B, T>(b);
}

// unrounded subtraction, returns a blockbinary that is of size nbits+1
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<N + 1, B, T> ursub(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<N + 1, B, T> result(a);
	return result -= blockbinary<N + 1, B, T>(b);
}

#define TRACE_URMUL 0
// unrounded multiplication, returns a blockbinary that is of size 2*nbits
// using brute-force sign-extending of operands to yield correct sign-extended result for 2*nbits 2's complement.
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<2*N, B, T> urmul(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	using BlockBinary = blockbinary<2 * N, B, T>;
	BlockBinary result(0);
	if (a.iszero() || b.iszero()) return result;

	// compute the result
	BlockBinary signextended_a(a);
	BlockBinary multiplicant(b);
#if TRACE_URMUL
	std::cout << "    " << to_binary(a) << " * " << to_binary(b) << std::endl;
	std::cout << std::setw(3) << 0 << ' ' << to_binary(multiplicant) << ' ' << to_binary(result) << std::endl;
#endif
	for (size_t i = 0; i < 2*N; ++i) {
		if (signextended_a.at(i)) {
			result += multiplicant;
		}
		multiplicant <<= 1;
#if TRACE_URMUL
		std::cout << std::setw(3) << i << ' ' << to_binary(multiplicant) << ' ' << to_binary(result) << std::endl;
#endif

	}
#if TRACE_URMUL
	std::cout << "fnl " << to_binary(result) << std::endl;
#endif
	//blockbinary<2 * nbits, bt> clipped(result);
	// since we used operator+=, which enforces the nulling of leading bits
	// we don't need to null here
	return result;
}

// unrounded multiplication, returns a blockbinary that is of size 2*nbits
// using nbits modulo arithmetic with final sign
template<size_t N, typename B, BinaryNumberType T>
inline blockbinary<2 * N, B, T> urmul2(const blockbinary<N, B, T>& a, const blockbinary<N, B, T>& b) {
	blockbinary<2 * N, B, T> result(0);
	if (a.iszero() || b.iszero()) return result;

	// compute the result
	bool result_sign = a.sign() ^ b.sign();
	// normalize both arguments to positive in new size
	blockbinary<N + 1, B, T> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockbinary<N + 1, B, T> b_new(b);
	if (a.sign()) a_new.twosComplement();
	if (b.sign()) b_new.twosComplement();
	blockbinary<2*N, B, T> multiplicant(b_new);

#if TRACE_URMUL
	std::cout << "    " << a_new << " * " << b_new << std::endl;
	std::cout << std::setw(3) << 0 << ' ' << multiplicant << ' ' << result << std::endl;
#endif
	for (size_t i = 0; i < (N+1); ++i) {
		if (a_new.at(i)) {
			result += multiplicant;  // if multiplicant is not the same size as result, the assignment will get sign-extended if the MSB is true, this is not correct because we are assuming unsigned binaries in this loop
		}
		multiplicant <<= 1;
#if TRACE_URMUL
		std::cout << std::setw(3) << i << ' ' << multiplicant << ' ' << result << std::endl;
#endif
	}
	if (result_sign) result.twosComplement();
#if TRACE_URMUL
	std::cout << "fnl " << result << std::endl;
#endif
	return result;
}

#define TRACE_DIV 0
// unrounded division, returns a blockbinary that is of size 2*nbits
template<size_t nbits, size_t roundingBits, typename B, BinaryNumberType T>
inline blockbinary<2 * nbits + roundingBits, B, T> urdiv(const blockbinary<nbits, B, T>& a, const blockbinary<nbits, B, T>& b) {
	if (b.iszero()) {
		// division by zero
		throw "urdiv divide by zero";
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = a.sign();
	bool b_sign = b.sign();
	bool result_negative = (a_sign ^ b_sign);

	// normalize both arguments to positive, which requires expansion by 1-bit to deal with maxneg
	blockbinary<nbits + 1, B, T> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockbinary<nbits + 1, B, T> b_new(b);
#if TRACE_DIV
	std::cout << "a " << to_binary(a_new) << '\n';
	std::cout << "b " << to_binary(b_new) << '\n';
#endif
	if (a_sign) a_new.twosComplement();
	if (b_sign) b_new.twosComplement();
#if TRACE_DIV
	std::cout << "a " << to_binary(a_new) << '\n';
	std::cout << "b " << to_binary(b_new) << '\n';
#endif

	// initialize the long division
	blockbinary<2 * nbits + roundingBits + 1, B, T> decimator(a_new);
	blockbinary<2 * nbits + roundingBits + 1, B, T> subtractand(b_new); // prepare the subtractand
	blockbinary<2 * nbits + roundingBits + 1, B, T> result;

	constexpr size_t msp = nbits + roundingBits; // msp = most significant position
	decimator <<= msp; // scale the decimator to the largest possible positive value

	int msb_b = subtractand.msb();
	int msb_a = decimator.msb();
	int shift = msb_a - msb_b;
	subtractand <<= shift;
	int offset = msb_a - static_cast<int>(msp);  // msb of the result
	int scale  = shift - static_cast<int>(msp);  // scale of the result quotient

#if TRACE_DIV
	std::cout << "  " << to_binary(decimator, true)   << " msp  : " << msp << '\n';
	std::cout << "- " << to_binary(subtractand, true) << " shift: " << shift << '\n';
#endif
	// long division
	for (int i = msb_a; i >= 0; --i) {

		if (subtractand <= decimator) {
			decimator -= subtractand;
			result.setbit(static_cast<size_t>(i));
		}
		else {
			result.setbit(static_cast<size_t>(i), false);
		}
		subtractand >>= 1;

#if TRACE_DIV
		std::cout << "  " << to_binary(decimator, true) << "  current quotient: " << to_binary(result, true) << '\n';
		std::cout << "- " << to_binary(subtractand, true) << '\n';
#endif
	}
	result <<= (scale - offset);
#if TRACE_DIV
	std::cout << "  " << "scaled result: " << to_binary(result, true) << " scale : " << scale << " offset : " << offset << '\n';
#endif
	if (result_negative) result.twosComplement();
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the storage
template<size_t nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_binary(const blockbinary<nbits, BlockType, NumberType>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = int(nbits - 1); i >= 0; --i) {
		s << (number.at(size_t(i)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}

// local helper to display the contents of a byte array
template<size_t nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_hex(const blockbinary<nbits, BlockType, NumberType>& number, bool wordMarker = true) {
	static constexpr size_t bitsInByte = 8;
	static constexpr size_t bitsInBlock = sizeof(BlockType) * bitsInByte;
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
template<size_t nbits, typename BlockType, BinaryNumberType NumberType>
std::ostream& operator<<(std::ostream& ostr, const blockbinary<nbits, BlockType, NumberType>& number) {
	return ostr << number.to_long_long(); // TODO: add an decimal converter
}


}} // namespace sw::universal
