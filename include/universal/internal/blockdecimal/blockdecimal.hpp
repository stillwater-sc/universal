#pragma once
// blockdecimal.hpp: parameterized blocked decimal number system representing a fixed-sized decimal integer number
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {

/*
NOTES

for block arithmetic, we need to manage a carry bit.
This disqualifies using uint64_t as a block type as we can't catch the overflow condition
in the same way as the other native types, uint8_t, uint16_t, uint32_t.

We could use a sint64_t and then convert to uint64_t and observe the MSB. Very different 
logic though.
*/

template<unsigned _ndigits>
class blockdecimal {
	static constexpr unsigned ndigits = _ndigits;
public:

};

#ifdef LATER
// a block-based decimal number configurable to be signed or unsigned. When signed it uses 2's complement encoding
template<unsigned _ndigits>
class blockdecimal {
public:
	static constexpr unsigned ndigits = _ndigits;
	typedef bt std::uint8_t;

	static constexpr unsigned bitsInByte = 8;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks = (0 == nbits ? 1 : (1ull + ((nbits - 1ull) / bitsInBlock)));
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr bt       maxBlockValue = bt(-1);

	static constexpr unsigned MSU = nrBlocks - 1; // MSU == Most Significant Unit
	static constexpr bt       ALL_ONES = bt(~0);
	static constexpr unsigned maxShift = (0 == nbits ? 0 : (nrBlocks* bitsInBlock - nbits)); // protect the shift that is >= sizeof(bt)
	static constexpr bt       MSU_MASK = (0 == nbits ? bt(0) : (ALL_ONES >> maxShift));      // the other side of this protection
	static constexpr bt       SIGN_BIT_MASK = (0 == nbits ? bt(0) : (bt(bt(1) << ((nbits - 1ull) % bitsInBlock))));

	static constexpr bool     uniblock64 = (bitsInBlock == 64) && (nrBlocks == 1);

	/// trivial constructor
	blockdecimal() = default;

	/// construct a blockdecimal from another: bt must be the same
	template<unsigned nnbits>
	blockdecimal(const blockdecimal<nnbits, BlockType, NumberType>& rhs) { this->assign(rhs); }

	// specific value constructor
	constexpr blockdecimal(const SpecificValue code) : _block{} {
		switch (code) {
		case SpecificValue::infpos:
		case SpecificValue::maxpos:
//			maxpos();
			break;
		case SpecificValue::minpos:
//			minpos();
			break;
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
//			minneg();
			break;
		case SpecificValue::infneg:
		case SpecificValue::maxneg:
//			maxneg();
			break;
		}
	}

	// initializer for long long
	constexpr blockdecimal(long long initial_value) noexcept : _block{} { *this = initial_value; }

	constexpr blockdecimal& operator=(long long rhs) noexcept {
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

	// limb access operators
	constexpr BlockType& operator[](unsigned index) { return _block[index]; }
	constexpr BlockType operator[](unsigned index) const { return _block[index]; }

	// prefix operators
	blockdecimal operator-() const {
		blockdecimal negated(*this);
		blockdecimal plusOne(1);
		negated.flip();
		negated += plusOne;
		return negated;
	}
	// one's complement
	blockdecimal operator~() const {
		blockdecimal complement(*this);
		complement.flip();
		return complement;
	}
	// increment/decrement
	blockdecimal operator++(int) {
		blockdecimal tmp(*this);
		operator++();
		return tmp;
	}
	blockdecimal& operator++() {
		blockdecimal increment;
		increment.setbits(0x1);
		*this += increment;
		return *this;
	}
	blockdecimal operator--(int) {
		blockdecimal tmp(*this);
		operator--();
		return tmp;
	}
	blockdecimal& operator--() {
		blockdecimal decrement;
		decrement.setbits(0x1);
		return *this -= decrement;
	}
	// logic operators
	blockdecimal  operator~() {
		blockdecimal<nbits, bt> complement(*this);
		complement.flip();
		return complement;
	}
	// arithmetic operators
	blockdecimal& operator+=(const blockdecimal& rhs) {
		if constexpr (nrBlocks == 1) {
			_block[0] = static_cast<bt>(_block[0] + rhs.block(0));
			// null any leading bits that fall outside of nbits
			_block[MSU] = static_cast<bt>(MSU_MASK & _block[MSU]);
		}
		else {
			blockdecimal sum;
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
	blockdecimal& operator-=(const blockdecimal& rhs) {
		return operator+=(sw::universal::twosComplement(rhs));
	}
#define BLOCKBINARY_FAST_MUL
#ifdef BLOCKBINARY_FAST_MUL
	blockdecimal& operator*=(const blockdecimal& rhs) {
		if constexpr (NumberType == BinaryNumberType::Signed) {
			if constexpr (nrBlocks == 1) {
				_block[0] = static_cast<bt>(_block[0] * rhs.block(0));
			}
			else {
				// is there a better way than upconverting to deal with maxneg in a 2's complement encoding?
				blockdecimal<nbits + 1, BlockType, NumberType> base(*this);
				blockdecimal<nbits + 1, BlockType, NumberType> multiplicant(rhs);
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
				blockdecimal base(*this);
				blockdecimal multiplicant(rhs);
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
	blockdecimal& operator*=(const blockdecimal& rhs) { // modulo in-place
		blockdecimal base(*this);
		blockdecimal multiplicant(rhs);
		clear();
		for (unsigned i = 0; i < nbits; ++i) {
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
	blockdecimal& operator/=(const blockdecimal& rhs) {
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
	blockdecimal& operator%=(const blockdecimal& rhs) {
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
	
	///////////////////////////////////////////////////////////////////
	///              logic operators

	blockdecimal& operator|=(const blockdecimal& rhs) noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] |= rhs._block[i];
		}
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	blockdecimal& operator&=(const blockdecimal& rhs) noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] &= rhs._block[i];
		}
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	blockdecimal& operator^=(const blockdecimal& rhs) noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] ^= rhs._block[i];
		}
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}

	// shift left operator
	blockdecimal& operator<<=(int bitsToShift) {
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
	// arithmetic shift right operator
	blockdecimal& operator>>=(int bitsToShift) {
		if (bitsToShift == 0) return *this;
		if (bitsToShift < 0) return operator<<=(-bitsToShift);
		if (bitsToShift >= static_cast<int>(nbits)) {
			setzero();
			return *this;
		}
		bool signext = sign();
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
				// fix up the leading zeros if we have a negative number
				if (signext) {
					// bitsToShift is guaranteed to be less than nbits
					bitsToShift += static_cast<int>(blockShift * bitsInBlock);
					for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
						this->setbit(i);
					}
				}
				else {
					// clean up the blocks we have shifted clean
					bitsToShift += static_cast<int>(blockShift * bitsInBlock);
					for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
						this->setbit(i, false);
					}
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

		// fix up the leading zeros if we have a negative number
		if (signext) {
			// bitsToShift is guaranteed to be less than nbits
			bitsToShift += static_cast<int>(blockShift * bitsInBlock);
			for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
				this->setbit(i);
			}
		}
		else {
			// clean up the blocks we have shifted clean
			bitsToShift += static_cast<int>(blockShift * bitsInBlock);
			for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
				this->setbit(i, false);
			}
		}

		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}

	///////////////////////////////////////////////////////////////////
	///                  modifiers

	constexpr void clear() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(0ull);
		}
	}
	constexpr void setzero() noexcept { clear(); }
	constexpr void set() noexcept { // set all bits to 1
		if constexpr (nrBlocks > 1) {
			for (unsigned i = 0; i < nrBlocks - 1; ++i) {
				_block[i] = ALL_ONES;
			}
		}
		_block[MSU] = ALL_ONES & MSU_MASK;
	}
	constexpr void reset() noexcept { clear(); } // set all bits to 0
	constexpr void set(unsigned i) noexcept {	setbit(i, true); }
	constexpr void reset(unsigned i) noexcept { setbit(i, false); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (blockIndex < nrBlocks) {
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[blockIndex] = bt((block & null) | mask);
		}
		// nop if blockIndex is out of range
	}
	constexpr void setbits(uint64_t value) noexcept {
		if constexpr (1 == nrBlocks) {
			_block[0] = value & storageMask;
		}
		else if constexpr (1 < nrBlocks) {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				_block[i] = value & storageMask;
				value >>= bitsInBlock;
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	}
	constexpr void setblock(unsigned b, const bt& block) noexcept {
		if (b < nrBlocks) _block[b] = block; // nop if b is out of range
	}	
	constexpr blockdecimal& flip() noexcept { // in-place one's complement
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}		
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	constexpr blockdecimal& twosComplement() noexcept { // in-place 2's complement
		blockdecimal plusOne(1);
		if constexpr (NumberType == BinaryNumberType::Signed) {
			flip();
		}
		else {
			static_assert(NumberType == BinaryNumberType::Signed, "calling in-place 2's complement on an unsigned blockdecimal"); // should this be allowed?
		}
		return *this += plusOne;
	}

	// minimum positive value of the blockdecimal configuration
	constexpr blockdecimal& minpos() noexcept {
		// minpos = 0000....00001
		clear();
		setbit(0, true);
		return *this;
	}
	// maximum positive value of the blockdecimal configuration
	constexpr blockdecimal& maxpos() noexcept {
		if constexpr (NumberType == BinaryNumberType::Signed) {
			// maxpos = 01111....1111
			clear();
			flip();
			setbit(nbits - 1, false);
		}
		else {
			// maxpos = 11111....1111
			clear();
			flip();
		}
		return *this;
	}
	// zero
	constexpr blockdecimal& zero() noexcept {
		clear();
		return *this;
	}
	// minimum negative value of the blockdecimal configuration
	constexpr blockdecimal& minneg() noexcept {
		if constexpr (NumberType == BinaryNumberType::Signed) {
			// minneg = 11111....11111
			clear();
			flip();
		}
		else {
			// minneg = 00000....00000
			clear();
		}
		return *this;
	}
	// maximum negative value of the blockdecimal configuration
	constexpr blockdecimal& maxneg() noexcept {
		if constexpr (NumberType == BinaryNumberType::Signed) {
			// maxneg = 10000....0000
			clear();
			setbit(nbits - 1);
		}
		else {
			// maxneg = 00000....00000
			clear();
		}
				
		return *this;
	}

	// selectors
	constexpr bool sign() const noexcept { return _block[MSU] & SIGN_BIT_MASK; }
	constexpr bool ispos() const noexcept { return !sign(); }
	constexpr bool isneg() const noexcept { return sign(); }
	constexpr bool iszero() const noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) if (_block[i] != 0) return false;
		return true;
	}
	constexpr bool isodd() const noexcept { return _block[0] & 0x1;	}
	constexpr bool iseven() const noexcept { return !isodd(); }

	constexpr bool all() const noexcept {
		if constexpr (nrBlocks > 1) for (unsigned i = 0; i < nrBlocks - 1; ++i) if (_block[i] != ALL_ONES) return false;
		if (_block[MSU] != MSU_MASK) return false;
		return true;
	}
	constexpr bool any() const noexcept {
		if constexpr (nrBlocks > 1) for (unsigned i = 0; i < nrBlocks - 1; ++i) if (_block[i] || ALL_ONES) return true;
		if (_block[MSU] || MSU_MASK) return true;
		return false;
	}
	constexpr bool anyAfter(unsigned bitIndex) const noexcept {  // TODO: optimize for limbs
		if (bitIndex < nbits) {
			for (unsigned i = 0; i < bitIndex; ++i) if (test(i)) return true;
		}
		return false;
	}

	constexpr bool none() const noexcept {
		if constexpr (nrBlocks > 1) for (unsigned i = 0; i < nrBlocks - 1; ++i) if (_block[i] != 0) return false;
		if (_block[MSU] & MSU_MASK) return false;
		return true;
	}
	constexpr unsigned count() const noexcept { // TODO: optimize for limbs
		unsigned nrOnes = 0;
		for (unsigned i = 0; i < nbits; ++i) {
			if (test(i)) ++nrOnes;
		}
		return nrOnes;
	}
	constexpr bool test(unsigned bitIndex) const noexcept { return at(bitIndex); }
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		unsigned blockIndex = bitIndex / bitsInBlock;
		bt limb = _block[blockIndex];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (limb & mask);
	}
	constexpr uint8_t nibble(unsigned n) const noexcept {
		uint8_t retval{ 0 };
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			unsigned nibbleIndexInWord = n % (bitsInBlock >> 2);
			bt mask = static_cast<bt>(0x0Fu << (nibbleIndexInWord*4));
			bt nibblebits = static_cast<bt>(mask & word);
			retval = static_cast<uint8_t>(nibblebits >> static_cast<bt>(nibbleIndexInWord*4));
		}
		else { // nop when nibble index out of bounds
			retval = 0;
		}
		return retval;
	}
	constexpr bt block(unsigned b) const noexcept {
		if (b < nrBlocks) return _block[b]; 
		return bt(0); // return 0 when block index out of bounds
	}

	// copy a value over from one blockdecimal to this blockdecimal
	// blockdecimal is a 2's complement encoding, so we sign-extend by default
	template<unsigned srcbits>
	blockdecimal<nbits, bt>& assign(const blockdecimal<srcbits, bt>& rhs) {
		clear();
		// since bt is the same, we can simply copy the blocks in
		unsigned minNrBlocks = (this->nrBlocks < rhs.nrBlocks) ? this->nrBlocks : rhs.nrBlocks;
		for (unsigned i = 0; i < minNrBlocks; ++i) {
			_block[i] = rhs.block(i);
		}
		if constexpr (nbits > srcbits) { // check if we need to sign extend
			if (rhs.sign()) {
				for (unsigned i = srcbits; i < nbits; ++i) { // TODO: replace bit-oriented sequence with block
					setbit(i);
				}
			}
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
		return *this;
	}

	// copy a value over from one blockdecimal to this without sign-extending the value
	// blockdecimal is a 2's complement encoding, so we sign-extend by default
	// for fraction/significent encodings, we need to turn off sign-extending.
	template<unsigned srcbits>
	blockdecimal<nbits, bt>& assignWithoutSignExtend(const blockdecimal<srcbits, bt>& rhs) {
		clear();
		// since bt is the same, we can simply copy the blocks in
		unsigned minNrBlocks = (this->nrBlocks < rhs.nrBlocks) ? this->nrBlocks : rhs.nrBlocks;
		for (unsigned i = 0; i < minNrBlocks; ++i) {
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
	bool roundingMode(unsigned targetLsb) const {
		bool lsb = at(targetLsb);
		bool guard = (targetLsb == 0 ? false : at(targetLsb - 1));
		bool round = (targetLsb > 1 ? at(targetLsb - 2) : false);
		bool sticky =(targetLsb < 3 ? false : any(targetLsb - 3));
		bool tie = guard && !round && !sticky;
		return (lsb && tie) || (guard && !tie);
	}
	bool any(unsigned msb) const {
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

private:
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// integer - integer logic comparisons
	template<unsigned N, typename B, BinaryNumberType T>
	friend bool operator==(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs);
	template<unsigned N, typename B, BinaryNumberType T>
	friend bool operator!=(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs);
	// the other logic operators are defined in terms of arithmetic terms

	template<unsigned N, typename B, BinaryNumberType T>
	friend std::ostream& operator<<(std::ostream& ostr, const blockdecimal<N, B, T>& v);
};

// Generate a type tag for blockdecimal
template<unsigned N, typename B, BinaryNumberType T>
std::string type_tag(const blockdecimal<N, B, T>& = {}) {
	std::stringstream str;
	str << "blockdecimal<"
		<< std::setw(4) << N << ", "
		<< typeid(B).name() << ", "
		<< typeid(T).name() << '>';
	return str.str();
}

//////////////////////////////////////////////////////////////////////////////////
// logic operators

template<unsigned N, typename B, BinaryNumberType T>
inline bool operator==(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs) {
	for (unsigned i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<unsigned N, typename B, BinaryNumberType T>
inline bool operator!=(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned N, typename B, BinaryNumberType T>
inline bool operator<(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs) {
	if (lhs.ispos() && rhs.isneg()) return false; // need to filter out possible overflow conditions
	if (lhs.isneg() && rhs.ispos()) return true;  // need to filter out possible underflow conditions
	if (lhs == rhs) return false; // so the maxneg logic works
	blockdecimal<N, B, T> mneg; maxneg<N, B>(mneg);
	if (rhs == mneg) return false; // special case: nothing is smaller than maximum negative
	blockdecimal<N, B, T> diff = lhs - rhs;
	return diff.isneg();
}
template<unsigned N, typename B, BinaryNumberType T>
inline bool operator<=(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs) {
	return (lhs < rhs || lhs == rhs);
}
template<unsigned N, typename B, BinaryNumberType T>
inline bool operator>(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs) {
	return !(lhs <= rhs);
}
template<unsigned N, typename B, BinaryNumberType T>
inline bool operator>=(const blockdecimal<N, B, T>& lhs, const blockdecimal<N, B, T>& rhs) {
	return !(lhs < rhs);
}
///////////////////////////////////////////////////////////////////////////////
// binary operators

template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N, B, T> operator+(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<N, B, T> c(a);
	return c += b;
}
template<unsigned N, typename B, BinaryNumberType T >
inline blockdecimal<N, B, T> operator-(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<N, B, T> c(a);
	return c -= b;
}
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N, B, T> operator*(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<N, B, T> c(a);
	return c *= b;
}
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N, B, T> operator/(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<N, B, T> c(a);
	return c /= b;
}
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N, B, T> operator%(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<N, B, T> c(a);
	return c %= b;
}

template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N, B, T> operator<<(const blockdecimal<N, B, T>& a, const long b) {
	blockdecimal<N, B, T> c(a);
	return c <<= b;
}
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N, B, T> operator>>(const blockdecimal<N, B, T>& a, const long b) {
	blockdecimal<N, B, T> c(a);
	return c >>= b;
}

// divide a by b and return both quotient and remainder
template<unsigned N, typename B, BinaryNumberType T>
quorem<N, B, T> longdivision(const blockdecimal<N, B, T>& _a, const blockdecimal<N, B, T>& _b) {
	using BlockBinary = blockdecimal<N + 1, B, T>;
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
			result.quo.setbit(static_cast<unsigned>(i));
		}
		else {
			result.quo.setbit(static_cast<unsigned>(i), false);
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

// unrounded addition, returns a blockdecimal that is of size nbits+1
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N + 1, B, T> uradd(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<N + 1, B, T> result(a);
	return result += blockdecimal<N + 1, B, T>(b);
}

// unrounded subtraction, returns a blockdecimal that is of size nbits+1
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<N + 1, B, T> ursub(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<N + 1, B, T> result(a);
	return result -= blockdecimal<N + 1, B, T>(b);
}

#define TRACE_URMUL 0
// unrounded multiplication, returns a blockdecimal that is of size 2*nbits
// using brute-force sign-extending of operands to yield correct sign-extended result for 2*nbits 2's complement.
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<2*N, B, T> urmul(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	using BlockBinary = blockdecimal<2 * N, B, T>;
	BlockBinary result(0);
	if (a.iszero() || b.iszero()) return result;

	// compute the result
	BlockBinary signextended_a(a);
	BlockBinary multiplicant(b);
#if TRACE_URMUL
	std::cout << "    " << to_binary(a) << " * " << to_binary(b) << std::endl;
	std::cout << std::setw(3) << 0 << ' ' << to_binary(multiplicant) << ' ' << to_binary(result) << std::endl;
#endif
	for (unsigned i = 0; i < 2*N; ++i) {
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
	//blockdecimal<2 * nbits, bt> clipped(result);
	// since we used operator+=, which enforces the nulling of leading bits
	// we don't need to null here
	return result;
}

// unrounded multiplication, returns a blockdecimal that is of size 2*nbits
// using nbits modulo arithmetic with final sign
template<unsigned N, typename B, BinaryNumberType T>
inline blockdecimal<2 * N, B, T> urmul2(const blockdecimal<N, B, T>& a, const blockdecimal<N, B, T>& b) {
	blockdecimal<2 * N, B, T> result(0);
	if (a.iszero() || b.iszero()) return result;

	// compute the result
	bool result_sign = a.sign() ^ b.sign();
	// normalize both arguments to positive in new size
	blockdecimal<N + 1, B, T> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockdecimal<N + 1, B, T> b_new(b);
	if (a.sign()) a_new.twosComplement();
	if (b.sign()) b_new.twosComplement();
	blockdecimal<2*N, B, T> multiplicant(b_new);

#if TRACE_URMUL
	std::cout << "    " << a_new << " * " << b_new << std::endl;
	std::cout << std::setw(3) << 0 << ' ' << multiplicant << ' ' << result << std::endl;
#endif
	for (unsigned i = 0; i < (N+1); ++i) {
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
// unrounded division, returns a blockdecimal that is of size 2*nbits
template<unsigned nbits, unsigned roundingBits, typename B, BinaryNumberType T>
inline blockdecimal<2 * nbits + roundingBits, B, T> urdiv(const blockdecimal<nbits, B, T>& a, const blockdecimal<nbits, B, T>& b) {
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
	blockdecimal<nbits + 1, B, T> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockdecimal<nbits + 1, B, T> b_new(b);
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
	blockdecimal<2 * nbits + roundingBits + 1, B, T> decimator(a_new);
	blockdecimal<2 * nbits + roundingBits + 1, B, T> subtractand(b_new); // prepare the subtractand
	blockdecimal<2 * nbits + roundingBits + 1, B, T> result;

	constexpr unsigned msp = nbits + roundingBits; // msp = most significant position
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
			result.setbit(static_cast<unsigned>(i));
		}
		else {
			result.setbit(static_cast<unsigned>(i), false);
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
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_binary(const blockdecimal<nbits, BlockType, NumberType>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (unsigned i = 0; i < nbits; ++i) {
		unsigned bitIndex = nbits - 1ull - i;
		s << (number.at(bitIndex) ? '1' : '0');
		if (bitIndex > 0 && (bitIndex % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}

// local helper to display the contents of a byte array
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_hex(const blockdecimal<nbits, BlockType, NumberType>& number, bool wordMarker = true) {
	static constexpr unsigned bitsInByte = 8;
	static constexpr unsigned bitsInBlock = sizeof(BlockType) * bitsInByte;
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

// ostream operator
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::ostream& operator<<(std::ostream& ostr, const blockdecimal<nbits, BlockType, NumberType>& number) {
	return ostr << number.to_long_long(); // TODO: add an decimal converter
}
#endif

}} // namespace sw::universal
