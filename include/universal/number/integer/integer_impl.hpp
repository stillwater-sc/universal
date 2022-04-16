#pragma once
// integer_impl.hpp: implementation of a fixed-size arbitrary integer precision number
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>

// supporting types and functions
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/shared/blocktype.hpp>

/*
the integer arithmetic can be configured to:
- throw exceptions on overflow
- throw execptions on arithmetic

you need the exception types defined, but you have the option to throw them
 */
#include <universal/number/integer/exceptions.hpp>

 // composition types used by integer
#include <universal/number/support/decimal.hpp>

namespace sw { namespace universal {

enum class IntegerNumberType {
	IntegerNumber = 0,  // { ...,-3,-2,-1,0,1,2,3,... }
	WholeNumber   = 1,  // {              0,1,2,3,... }
	NaturalNumber = 2   // {                1,2,3,... }
};

// forward references
template<size_t nbits, typename BlockType, IntegerNumberType NumberType> class integer;
template<size_t nbits, typename BlockType, IntegerNumberType NumberType> integer<nbits, BlockType, NumberType> max_int();
template<size_t nbits, typename BlockType, IntegerNumberType NumberType> integer<nbits, BlockType, NumberType> min_int();
template<size_t nbits, typename BlockType, IntegerNumberType NumberType> struct idiv_t;
template<size_t nbits, typename BlockType, IntegerNumberType NumberType> idiv_t<nbits, BlockType, NumberType> idiv(const integer<nbits, BlockType, NumberType>&, const integer<nbits, BlockType, NumberType>&b);

// scale calculate the power of 2 exponent that would capture an approximation of a normalized real value
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline long scale(const integer<nbits, BlockType, NumberType>& i) {
	integer<nbits, BlockType, NumberType> v(i);
	if (i.sign()) { // special case handling
		v.twosComplement();
		if (v == i) {  // special case of 10000..... largest negative number in 2's complement encoding
			return long(nbits - 1);
		}
	}
	// calculate scale
	long scale = 0;
	while (v > 1) {
		++scale;
		v >>= 1;
	}
	return scale;
}

// signed integer conversion
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline constexpr integer<nbits, BlockType, NumberType>& convert(int64_t v, integer<nbits, BlockType, NumberType>& result) {	return result.convert(v); }
// unsigned integer conversion
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline constexpr integer<nbits, BlockType, NumberType>& convert(uint64_t v, integer<nbits, BlockType, NumberType>& result) { return result.convert(v); }

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
bool parse(const std::string& number, integer<nbits, BlockType, NumberType>& v);

// idiv_t for integer<nbits, BlockType, NumberType> to capture quotient and remainder during long division
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
struct idiv_t {
	integer<nbits, BlockType, NumberType> quot; // quotient
	integer<nbits, BlockType, NumberType> rem;  // remainder
};

/*
The rules for detecting overflow in a two's complement sum are simple:
 - If the sum of two positive numbers yields a negative result, the sum has overflowed.
 - If the sum of two negative numbers yields a positive result, the sum has overflowed.
 - Otherwise, the sum has not overflowed.
It is important to note the overflow and carry out can each occur without the other. 
In unsigned numbers, carry out is equivalent to overflow. In two's complement, carry out tells 
you nothing about overflow.

The reason for the rules is that overflow in two's complement occurs, not when a bit is carried out 
out of the left column, but when one is carried into it. That is, when there is a carry into the sign. 
The rules detect this error by examining the sign of the result. A negative and positive added together 
cannot overflow, because the sum is between the addends. Since both of the addends fit within the
allowable range of numbers, and their sum is between them, it must fit as well.

When implementing addition/subtraction on chuncks the overflow condition must be deduced from the 
chunk values. The chunks need to be interpreted as unsigned binary segments.
*/

// integer is an arbitrary fixed-sized 2's complement integer
template<size_t _nbits, typename BlockType = uint8_t, IntegerNumberType NumberType = IntegerNumberType::IntegerNumber>
class integer {
public:
	using bt = BlockType;
	static constexpr size_t nbits = _nbits;
	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr size_t MSU = nrBlocks - 1ull;
	static constexpr bt     ALL_ONES = bt(~0); // block type specific all 1's value
	static constexpr bt     MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr size_t bitsInMSU = bitsInBlock - (nrBlocks * bitsInBlock - nbits);
	static constexpr size_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));

	constexpr integer() noexcept : _block{ 0 } {};

	constexpr integer(const integer&) noexcept = default;
	constexpr integer(integer&&) noexcept = default;

	constexpr integer& operator=(const integer&) noexcept = default;
	constexpr integer& operator=(integer&&) noexcept = default;

	/// Construct a new integer from another, sign extend when necessary, BlockTypes must be the same
	template<size_t srcbits>
	integer(const integer<srcbits, BlockType, NumberType>& a) {
//		static_assert(srcbits > nbits, "Source integer is bigger than target: potential loss of precision"); // TODO: do we want this?
		bitcopy(a);
		if constexpr (srcbits < nbits) {
			if (a.sign()) { // sign extend
				for (unsigned i = srcbits; i < nbits; ++i) {
					setbit(i);
				}
			}
		}
	}

	// initializers for native types
	constexpr integer(signed char initial_value)        { *this = initial_value; }
	constexpr integer(short initial_value)              { *this = initial_value; }
	constexpr integer(int initial_value)                { *this = initial_value; }
	constexpr integer(long initial_value)               { *this = initial_value; }
	constexpr integer(long long initial_value)          { *this = initial_value; }
	constexpr integer(char initial_value)               { *this = initial_value; }
	constexpr integer(unsigned short initial_value)     { *this = initial_value; }
	constexpr integer(unsigned int initial_value)       { *this = initial_value; }
	constexpr integer(unsigned long initial_value)      { *this = initial_value; }
	constexpr integer(unsigned long long initial_value) { *this = initial_value; }
	constexpr integer(float initial_value)              { *this = initial_value; }
	constexpr integer(double initial_value)             { *this = initial_value; }
	constexpr integer(long double initial_value)        { *this = initial_value; }

	// specific value constructor
	constexpr integer(const SpecificValue code) noexcept
		: _block{ 0 } {
		switch (code) {
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::infneg:
		case SpecificValue::infpos:
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:
			zero();
			break;
		}
	}

	// access operator for bits
	// this needs a proxy to be able to create l-values
	// bool operator[](const unsigned int i) const //

	// simpler interface for now, using at(i) and set(i)/reset(i)

	// assignment operators for native types
	constexpr integer& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr integer& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr integer& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr integer& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr integer& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr integer& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	constexpr integer& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr integer& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr integer& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr integer& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr integer& operator=(float rhs)              noexcept { return convert_ieee(rhs); }
	constexpr integer& operator=(double rhs)             noexcept { return convert_ieee(rhs); }
#if LONG_DOUBLE_SUPPORT
	constexpr integer& operator=(long double rhs)        noexcept { return convert_ieee(rhs); }
#endif

#ifdef ADAPTER_POSIT_AND_INTEGER
	// convenience assignment operator 
	template<size_t nbits, size_t es>
	integer& operator=(const posit<nbits, es>& rhs) {
		convert_p2i(rhs, *this);
		return *this;
	}
#endif // ADAPTER_POSIT_AND_INTEGER

	// prefix operators
	constexpr integer operator-() const {
		integer negated(*this);
		negated.flip();
		negated += 1;
		return negated;
	}
	// one's complement
	constexpr integer operator~() const {
		integer complement(*this);
		complement.flip();
		return complement;
	}
	// increment
	constexpr integer operator++(int) {
		integer tmp(*this);
		operator++();
		return tmp;
	}
	constexpr integer& operator++() {
		*this += integer(1);
		_block[MSU] = static_cast<bt>(_block[MSU] & MSU_MASK); // assert precondition of properly nulled leading non-bits
		return *this;
	}
	// decrement
	constexpr integer operator--(int) {
		integer tmp(*this);
		operator--();
		return tmp;
	}
	constexpr integer& operator--() {
		*this -= integer(1);
		_block[MSU] = static_cast<bt>(_block[MSU] & MSU_MASK); // assert precondition of properly nulled leading non-bits
		return *this;
	}

	// conversion operators
	explicit operator unsigned short()     const { return to_integer<unsigned short>(); }
	explicit operator unsigned int()       const { return to_integer<unsigned int>(); }
	explicit operator unsigned long()      const { return to_integer<unsigned long>(); }
	explicit operator unsigned long long() const { return to_integer<unsigned long long>(); }
	explicit operator short()              const { return to_integer<short>(); }
	explicit operator int()                const { return to_integer<int>(); }
	explicit operator long()               const { return to_integer<long>(); }
	explicit operator long long()          const { return to_integer<long long>(); }
	explicit operator float()              const { return to_real<float>(); }
	explicit operator double()             const { return to_real<double>(); }
#if LONG_DOUBLE_SUPPORT
	explicit operator long double() const { return to_real<long double>(); }
#endif

	// arithmetic operators
	integer& operator+=(const integer& rhs) {
		if constexpr (nrBlocks == 1) {
			_block[0] = static_cast<bt>(_block[0] + rhs.block(0));
			// null any leading bits that fall outside of nbits
			_block[MSU] = static_cast<bt>(MSU_MASK & _block[MSU]);
		}
		else {
			integer<nbits, BlockType, NumberType> sum;
			std::uint64_t carry = 0;
			BlockType* pA = _block;
			BlockType const* pB = rhs._block;
			BlockType* pC = sum._block;
			BlockType* pEnd = pC + nrBlocks;
			while (pC != pEnd) {
				carry += static_cast<std::uint64_t>(*pA) + static_cast<std::uint64_t>(*pB);
				*pC = static_cast<bt>(carry);
				if constexpr (bitsInBlock == 64) carry = 0; else carry >>= bitsInBlock;
				++pA; ++pB; ++pC;
			}
			// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
			BlockType* pLast = pEnd - 1;
			*pLast = static_cast<bt>(MSU_MASK & *pLast);
	#if INTEGER_THROW_ARITHMETIC_EXCEPTION
			// TODO: what is the real overflow condition?
			// it is not carry == 1 as  say 1 + -1 sets the carry but is 0
		//		if (carry) throw integer_overflow();
	#endif
			* this = sum;
		}
		return *this;
	}
	integer& operator-=(const integer& rhs) {
		integer twos(rhs);
		operator+=(twos.twosComplement());
		return *this;
	}
	integer& operator*=(const integer& rhs) {
		if constexpr (NumberType == IntegerNumberType::IntegerNumber) {
			if constexpr (nrBlocks == 1) {
				_block[0] = static_cast<bt>(_block[0] * rhs.block(0));
			}
			else {
				// is there a better way than upconverting to deal with maxneg in a 2's complement encoding?
				integer<nbits + 1, BlockType, NumberType> base(*this);
				integer<nbits + 1, BlockType, NumberType> multiplicant(rhs);
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
		else {  // whole and natural numbers are closed under multiplication (modulo)
			if constexpr (nrBlocks == 1) {
				_block[0] = static_cast<bt>(_block[0] * rhs.block(0));
			}
			else {
				integer<nbits, BlockType, NumberType> base(*this);
				integer<nbits, BlockType, NumberType> multiplicant(rhs);
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
	integer& operator/=(const integer& rhs) {
		if constexpr (nbits == (sizeof(BlockType)*8) ) {
			if (rhs._block[0] == 0) {
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				throw integer_divide_by_zero{};
#else
				std::cerr << "integer_divide_by_zero\n";
#endif // INTEGER_THROW_ARITHMETIC_EXCEPTION
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
			idiv_t<nbits, BlockType, NumberType> divresult = idiv<nbits, BlockType, NumberType>(*this, rhs);
			*this = divresult.quot;
		}
		return *this;
	}
	integer& operator%=(const integer& rhs) {
		if constexpr (nbits == (sizeof(BlockType) * 8)) {
			if (rhs._block[0] == 0) {
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				throw integer_divide_by_zero{};
#else
				std::cerr << "integer_divide_by_zero\n";
#endif // INTEGER_THROW_ARITHMETIC_EXCEPTION
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
			idiv_t<nbits, BlockType, NumberType> divresult = idiv<nbits, BlockType, NumberType>(*this, rhs);
			*this = divresult.rem;
		}

		return *this;
	}

	// arithmetic shift right operator
	integer& operator<<=(int bitsToShift) {
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
	integer& operator>>=(int bitsToShift) {
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
	integer& logicShiftRight(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) {
			return operator<<=(-shift);
		}
		if (nbits <= unsigned(shift)) {
			clear();
			return *this;
		}
		integer<nbits, BlockType, NumberType> target;
		for (int i = nbits - 1; i >= shift; --i) {  // TODO: inefficient as it works at the bit level
			target.setbit(static_cast<size_t>(i) - static_cast<size_t>(shift), at(static_cast<size_t>(i)));
		}
		*this = target;
		return *this;
	}
	integer& operator&=(const integer& rhs) {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] &= rhs._block[i];
		}
		_block[MSU] &= MSU_MASK;
		return *this;
	}
	integer& operator|=(const integer& rhs) {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] |= rhs._block[i];
		}
		_block[MSU] &= MSU_MASK;
		return *this;
	}
	integer& operator^=(const integer& rhs) {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] ^= rhs._block[i];
		}
		_block[MSU] &= MSU_MASK;
		return *this;
	}

	// modifiers
	inline constexpr void clear() noexcept { 
		bt* p = _block;
		if constexpr (0 == nrBlocks) {
			return;
		}
		else if constexpr (1 == nrBlocks) {
			*p = bt(0);
		}
		else if constexpr (2 == nrBlocks) {
			*p++ = bt(0);
			*p = bt(0);
		}
		else if constexpr (3 == nrBlocks) {
			*p++ = bt(0);
			*p++ = bt(0);
			*p   = bt(0);
		}
		else if constexpr (4 == nrBlocks) {
			*p++ = bt(0);
			*p++ = bt(0);
			*p++ = bt(0);
			*p   = bt(0);
		}
		else {
			for (size_t i = 0; i < nrBlocks; ++i) {
				*p++ = bt(0);
			}
		}
	}
	inline constexpr void setzero() noexcept { clear(); }
	inline constexpr integer& maxpos() noexcept {
		clear();
		setbit(nbits - 1ull, true);
		flip();
		return *this;
	}
	inline constexpr integer& minpos() noexcept {
		clear();
		setbit(0, true);
		return *this;
	}
	inline constexpr integer& zero() noexcept {
		clear();
		return *this;
	}
	inline constexpr integer& minneg() noexcept {
		clear();
		flip();
		return *this;
	}
	inline constexpr integer& maxneg() noexcept {
		clear();
		setbit(nbits - 1ull, true);
		return *this;
	}
	inline constexpr void setbit(unsigned i, bool v = true) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt((block & null) | mask);
			return;
		}
	}
	inline constexpr void setbyte(unsigned byteIndex, uint8_t data) {
		uint8_t mask = 0x1u;
		unsigned start = byteIndex * 8;
		unsigned end = start + 8;
		for (unsigned i = start; i < end; ++i) {
			setbit(i, (mask & data));
			mask <<= 1;
		}
	}
	inline constexpr void setblock(unsigned i, bt value) noexcept {
		if (i < nrBlocks) _block[i] = value;
	}
	// use un-interpreted raw bits to set the bits of the integer
	inline constexpr integer& setbits(uint64_t raw_bits) noexcept {
		if constexpr (0 == nrBlocks) {
			return *this;
		}
		else if constexpr (1 == nrBlocks) {
			_block[0] = raw_bits & storageMask;
		}
		else if constexpr (2 == nrBlocks) {
			if constexpr (bitsInBlock < 64) {
				_block[0] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[1] = raw_bits & storageMask;
			}
			else {
				_block[0] = raw_bits & storageMask;
				_block[1] = 0;
			}
		}
		else if constexpr (3 == nrBlocks) {
			if constexpr (bitsInBlock < 64) {
				_block[0] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[1] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[2] = raw_bits & storageMask;
			}
			else {
				_block[0] = raw_bits & storageMask;
				_block[1] = 0;
				_block[2] = 0;
			}
		}
		else if constexpr (4 == nrBlocks) {
			if constexpr (bitsInBlock < 64) {
				_block[0] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[1] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[2] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[3] = raw_bits & storageMask;
			}
			else {
				_block[0] = raw_bits & storageMask;
				_block[1] = 0;
				_block[2] = 0;
				_block[3] = 0;
			}
		}
		else {
			if constexpr (bitsInBlock < 64) {
				for (size_t i = 0; i < nrBlocks; ++i) {
					_block[i] = raw_bits & storageMask;
					raw_bits >>= bitsInBlock;
				}
			}
			else {
				_block[0] = raw_bits & storageMask;
				for (size_t i = 1; i < nrBlocks; ++i) {
					_block[i] = 0;
				}
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		return *this;
	}
	inline integer& assign(const std::string& txt) noexcept {
		if (!parse(txt, *this)) {
			std::cerr << "Unable to parse: " << txt << std::endl;
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] = static_cast<BlockType>(MSU_MASK & _block[MSU]);
		return *this;
	}
	// pure bit copy of source integer, no sign extension
	template<size_t src_nbits>
	inline constexpr void bitcopy(const integer<src_nbits, BlockType, NumberType>& src) noexcept {
		clear();
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = src.block(i);
		}
		_block[MSU] = static_cast<bt>(_block[MSU] & MSU_MASK); // assert precondition of properly nulled leading non-bits
	}
	// in-place one's complement
	inline constexpr integer& flip() {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = static_cast<bt>(~_block[i]);
		}
		_block[MSU] = static_cast<bt>(_block[MSU] & MSU_MASK); // assert precondition of properly nulled leading non-bits
		return *this;
	}
	// in-place 2's complement
	inline constexpr integer& twosComplement() {
		flip();
		return ++(*this);
	}

	// selectors
	inline constexpr bool iszero() const noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			if (_block[i] != 0) return false;
		}
		return true;
	}
	inline constexpr bool ispos()  const noexcept { return *this > 0; }
	inline constexpr bool isneg()  const noexcept { return *this < 0; }
	inline constexpr bool isone()  const noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			if (i == 0) {
				if (_block[0] != BlockType(1u)) return false;
			}
			else {
				if (_block[i] != BlockType(0u)) return false;
			}
		}
		return true;
	}
	inline constexpr bool isodd()  const noexcept  { return bool(_block[0] & 0x01); }
	inline constexpr bool iseven() const noexcept { return !isodd(); }
	inline constexpr bool sign()   const noexcept { return at(nbits - 1); }
	inline constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex < nbits) {
			bt word = _block[bitIndex / bitsInBlock];
			bt mask = bt(1ull << (bitIndex % bitsInBlock));
			return (word & mask);
		}
		return false;
	}
	inline constexpr bool test(unsigned i)  const noexcept { return at(i); }
	inline constexpr bt   block(unsigned i) const noexcept { if (i < nrBlocks) return _block[i]; else return bt(0u); }

	// signed integer conversion
	template<typename SignedInt>
	inline constexpr integer& convert_signed(SignedInt rhs) noexcept {
		clear();
		if (0 == rhs) return *this;
		constexpr size_t argbits = sizeof(rhs);
		int64_t v = rhs;
		unsigned upper = (nbits <= _nbits ? nbits : argbits);
		for (unsigned i = 0; i < upper && v != 0; ++i) {
			if (v & 0x1ull) setbit(i);
			v >>= 1;
		}
		if constexpr (nbits > 64) {
			if (rhs < 0) {	// sign extend if negative
				for (unsigned i = upper; i < nbits; ++i) {
					setbit(i);
				}
			}
		}
		return *this;
	}
	// unsigned integer conversion
	template<typename UnsignedInt>
	inline constexpr integer& convert_unsigned(UnsignedInt rhs) noexcept {
		clear();
		if (0 == rhs) return *this;
		uint64_t v = rhs;
		constexpr size_t argbits = sizeof(rhs);
		unsigned upper = (nbits <= _nbits ? nbits : argbits);
		for (unsigned i = 0; i < upper; ++i) {
			if (v & 0x1ull) setbit(i);
			v >>= 1;
		}
		return *this;
	}
	// native IEEE-754 conversion
	// TODO: currently only supports integer values of 64bits or less
	template<typename Real>
	constexpr integer& convert_ieee(Real rhs) noexcept {
		clear();
		return *this = static_cast<long long>(rhs); // TODO: this clamps the IEEE range to +-2^63
	}

protected:
	// HELPER methods

	// TODO: conditional NOEXCEPT when we are not throwing arithmetic exceptions?
	template<typename IntType>
	IntType to_integer() const {
		IntType v{ 0 };
		if (*this == 0) return v;
		constexpr unsigned sizeofint = 8 * sizeof(IntType);
		uint64_t mask = 1ull;
		unsigned upper = (nbits < sizeofint ? nbits : sizeofint);
		for (unsigned i = 0; i < upper; ++i) {
			v |= at(i) ? mask : 0;
			mask <<= 1;
		}
		if (sign() && upper < sizeofint) { // sign extend
			for (unsigned i = upper; i < sizeofint; ++i) {
				v |= mask;
				mask <<= 1;
			}
		}
		return v;
	}

	// TODO: enable_if this for native floating-point types only
	template<typename Real>
	constexpr Real to_real() const noexcept {
		Real r = 0.0;
		Real bitValue = static_cast<Real>(1.0);
		for (size_t i = 0; i < nbits; ++i) {
			if (at(i)) r += bitValue;
			bitValue *= static_cast<Real>(2.0);
		}
		return r;
	}

private:
	bt _block[nrBlocks];

	// convert
	template<size_t nnbits, typename BBlockType, IntegerNumberType NNumberType>
	friend std::string convert_to_decimal_string(const integer<nnbits, BBlockType, NNumberType>& value);

	// integer - integer logic comparisons
	template<size_t nnbits, typename BBlockType, IntegerNumberType NNumberType>
	friend bool operator==(const integer<nnbits, BBlockType, NNumberType>& lhs, const integer<nnbits, BBlockType, NNumberType>& rhs);

	// find the most significant bit set
	template<size_t nnbits, typename BBlockType, IntegerNumberType NNumberType>
	friend signed findMsb(const integer<nnbits, BBlockType, NNumberType>& v);
};

////////////////////////    INTEGER functions   /////////////////////////////////

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> abs(const integer<nbits, BlockType, NumberType>& a) {
	integer<nbits, BlockType, NumberType> b(a);
	return (a >= 0 ? b : b.twosComplement());
}

// free function generator to create a 1's complement copy of an integer
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> onesComplement(const integer<nbits, BlockType, NumberType>& value) {
	integer<nbits, BlockType, NumberType> ones(value);
	return ones.flip();
}
// free function generator to create the 2's complement of an integer
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> twosComplement(const integer<nbits, BlockType, NumberType>& value) {
	integer<nbits, BlockType, NumberType> twos(value);
	return twos.twosComplement();;
}

// convert integer to decimal string
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
std::string convert_to_decimal_string(const integer<nbits, BlockType, NumberType>& value) {
	if (value.iszero()) {
		return std::string("0");
	}
	integer<nbits, BlockType, NumberType> number = value.sign() ? twosComplement(value) : value;
	support::decimal partial, multiplier;
	partial.setzero();
	multiplier.setdigit(1);
	// convert integer to decimal by adding and doubling multipliers
	for (unsigned i = 0; i < nbits; ++i) {
		if (number.at(i)) {
			support::add(partial, multiplier);
			// std::cout << partial << std::endl;
		}
		support::add(multiplier, multiplier);
	}
	std::stringstream str;
	if (value.sign()) str << '-';
	for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
		str << (int)*rit;
	}
	return str.str();
}

// findMsb takes an integer<nbits, BlockType, NumberType> reference and returns the 0-based position of the most significant bit, -1 if v == 0
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline signed findMsb(const integer<nbits, BlockType, NumberType>& v) {
	BlockType const* pV = v._block + v.nrBlocks - 1;
	BlockType const* pLast = v._block - 1;
	BlockType BlockMsb = BlockType(BlockType(1u) << (v.bitsInBlock - 1));
	signed msb = static_cast<signed>(v.nbits - 1ull); // the case for an aligned MSB
	unsigned rem = nbits % v.bitsInBlock;
	// we are organized little-endian
	// check if the blocks are aligned with the representation
	if (rem > 0) {
		// the top bits are unaligned: construct the right mask
		BlockType mask = BlockType(BlockType(1u) << (rem - 1u));
		while (mask != 0) {
			if (*pV & mask) return msb;
			--msb;
			mask >>= 1;
		}
		if (msb < 0) return msb;
		--pV;
	}
	// invariant: msb is now aligned with the blocks
	// std::cout << "invariant msb : " << msb << '\n';
	while (pV != pLast) {
		if (*pV != 0) {
			BlockType mask = BlockMsb;
			while (mask != 0) {
				if (*pV & mask) return msb;
				--msb;
				mask >>= 1;
			}
		}
		else {
			msb -= v.bitsInBlock;
		}
		--pV;
	}
	return msb; // == -1 if no significant bit found
}

////////////////////////    INTEGER operators   /////////////////////////////////

// divide integer<nbits, BlockType, NumberType> a and b and return result argument
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
void divide(const integer<nbits, BlockType, NumberType>& a, const integer<nbits, BlockType, NumberType>& b, integer<nbits, BlockType, NumberType>& quotient) {
	if (b == integer<nbits, BlockType, NumberType>(0)) {
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
		throw integer_divide_by_zero{};
#else
		std::cerr << "integer_divide_by_zero\n";
#endif // INTEGER_THROW_ARITHMETIC_EXCEPTION
	}
	idiv_t<nbits, BlockType, NumberType> divresult = idiv<nbits, BlockType, NumberType>(a, b);
	quotient = divresult.quot;
}

// calculate remainder of integer<nbits, BlockType, NumberType> a and b and return result argument
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
void remainder(const integer<nbits, BlockType, NumberType>& a, const integer<nbits, BlockType, NumberType>& b, integer<nbits, BlockType, NumberType>& remainder) {
	if (b == integer<nbits, BlockType, NumberType>(0)) {
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
		throw integer_divide_by_zero{};
#else
		std::cerr << "integer_divide_by_zero\n";
#endif // INTEGER_THROW_ARITHMETIC_EXCEPTION
	}
	idiv_t<nbits, BlockType, NumberType> divresult = idiv<nbits, BlockType, NumberType>(a, b);
	remainder = divresult.rem;
}

// divide integer<nbits, BlockType, NumberType> a and b and return result argument
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
idiv_t<nbits, BlockType, NumberType> idiv(const integer<nbits, BlockType, NumberType>& _a, const integer<nbits, BlockType, NumberType>& _b) {
	if (_b.iszero()) {
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
		throw integer_divide_by_zero{};
#else
		std::cerr << "integer_divide_by_zero\n";
#endif // INTEGER_THROW_ARITHMETIC_EXCEPTION
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_negative = _a.sign();
	bool b_negative = _b.sign();
	bool result_negative = (a_negative ^ b_negative);
	integer<nbits + 1, BlockType, NumberType> a; a.bitcopy(a_negative ? -_a : _a);
	integer<nbits + 1, BlockType, NumberType> b; b.bitcopy(b_negative ? -_b : _b);
	idiv_t<nbits, BlockType, NumberType> divresult;
	if (a < b) {
		divresult.rem = _a; // a % b = a when a / b = 0
		return divresult; // a / b = 0 when b > a
	}
	// initialize the long division
	integer<nbits + 1, BlockType, NumberType> accumulator = a;
	// prepare the subtractand
	integer<nbits + 1, BlockType, NumberType> subtractand = b;
	int msb_b = findMsb(b);
	int msb_a = findMsb(a);
	int shift = msb_a - msb_b;
	subtractand <<= shift;
	// long division
	for (int i = shift; i >= 0; --i) {
		if (subtractand <= accumulator) {
			accumulator -= subtractand;
			divresult.quot.setbit(static_cast<size_t>(i));
		}
		else {
			divresult.quot.setbit(static_cast<size_t>(i), false);
		}
		subtractand >>= 1;
//		std::cout << "i = " << i << " subtractand : " << subtractand << '\n';
	}
	if (result_negative) {  // take 2's complement
		divresult.quot.flip();
		divresult.quot += 1;
	} 
	if (_a.isneg()) {
		divresult.rem = -accumulator;
	}
	else {
		divresult.rem = accumulator;
	}

	return divresult;
}

/// stream operators

// read a integer ASCII format and make a binary integer out of it
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
bool parse(const std::string& number, integer<nbits, BlockType, NumberType>& value) {
	bool bSuccess = false;
	value.clear();
	// check if the txt is an integer form: [0123456789]+
	std::regex decimal_regex("^[-+]*[0-9]+");
	std::regex octal_regex("^[-+]*0[1-7][0-7]*$");
	std::regex hex_regex("^[-+]*0[xX][0-9a-fA-F']+");
	// setup associative array to map chars to nibbles
	std::map<char, int> charLookup{
		{ '0', 0 },
		{ '1', 1 },
		{ '2', 2 },
		{ '3', 3 },
		{ '4', 4 },
		{ '5', 5 },
		{ '6', 6 },
		{ '7', 7 },
		{ '8', 8 },
		{ '9', 9 },
		{ 'a', 10 },
		{ 'b', 11 },
		{ 'c', 12 },
		{ 'd', 13 },
		{ 'e', 14 },
		{ 'f', 15 },
		{ 'A', 10 },
		{ 'B', 11 },
		{ 'C', 12 },
		{ 'D', 13 },
		{ 'E', 14 },
		{ 'F', 15 },
	};
	if (std::regex_match(number, octal_regex)) {
		std::cout << "found an octal representation\n";
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			std::cout << "char = " << *r << std::endl;
		}
		bSuccess = false; // TODO
	}
	else if (std::regex_match(number, hex_regex)) {
		//std::cout << "found a hexadecimal representation\n";
		// each char is a nibble
		int maxByteIndex = nbits / 8;
		int byte = 0;
		int byteIndex = 0;
		bool odd = false;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend() && byteIndex < maxByteIndex;
			++r) {
			if (*r == '\'') {
				// ignore
			}
			else if (*r == 'x' || *r == 'X') {
				if (odd) {
					// complete the most significant byte
					value.setbyte(static_cast<size_t>(byteIndex), static_cast<uint8_t>(byte));
				}
				// check that we have [-+]0[xX] format
				++r;
				if (r != number.rend()) {
					if (*r == '0') {
						// check if we have a sign
						++r;
						if (r == number.rend()) {
							// no sign, thus by definition positive
							bSuccess = true;
						}
						else if (*r == '+') {
							// optional positive sign, no further action necessary
							bSuccess = true;
						}
						else if (*r == '-') {
							// negative sign, invert
							value = -value;
							bSuccess = true;
						}
						else {
							// the regex will have filtered this out
							bSuccess = false;
						}
					}
					else {
						// we didn't find the obligatory '0', the regex should have filtered this out
						bSuccess = false;
					}
				}
				else {
					// we are missing the obligatory '0', the regex should have filtered this out
					bSuccess = false;
				}
				// we have reached the end of our parse
				break;
			}
			else {
				if (odd) {
					byte += charLookup.at(*r) << 4;
					value.setbyte(static_cast<size_t>(byteIndex), static_cast<uint8_t>(byte));
					++byteIndex;
				}
				else {
					byte = charLookup.at(*r);
				}
				odd = !odd;
			}
		}
	}
	else if (std::regex_match(number, decimal_regex)) {
		//std::cout << "found a decimal integer representation\n";
		integer<nbits, BlockType, NumberType> scale = 1;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			if (*r == '-') {
				value = -value;
			}
			else if (*r == '+') {
				break;
			}
			else {
				integer<nbits, BlockType, NumberType> digit = charLookup.at(*r);
				value += scale * digit;
				scale *= 10;
			}
		}
		bSuccess = true;
	}

	return bSuccess;
}

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
std::string to_string(const integer<nbits, BlockType, NumberType>& n) {
	return convert_to_decimal_string(n);
}

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
std::string convert_to_string(std::ios_base::fmtflags flags, const integer<nbits, BlockType, NumberType>& n) {
	using IntegerBase = integer<nbits, BlockType, NumberType>;
	using Integer = integer<nbits+1, BlockType, NumberType>;  // got to be 1 bigger to be able to represent maxneg in 2's complement form

	// set the base of the target number system to convert to
	int base = 10;
	if ((flags & std::ios_base::oct) == std::ios_base::oct) base = 8;
	if ((flags & std::ios_base::hex) == std::ios_base::hex) base = 16;

	std::string result;
	if (base == 8 || base == 16) {
		if (n.sign()) return std::string("negative value: ignored");

		BlockType shift = base == 8 ? 3 : 4;
		BlockType mask = static_cast<BlockType>((1u << shift) - 1);
		Integer t(n);
		result.assign(nbits / shift + ((nbits % shift) ? 1 : 0), '0');
		std::string::difference_type pos = result.size() - 1;
		for (size_t i = 0; i < nbits / static_cast<size_t>(shift); ++i) {
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9' - 1;
			result[pos--] = c;
			t >>= shift;
		}
		if (nbits % shift) {
			mask = static_cast<BlockType>((1u << (nbits % shift)) - 1);
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9';
			result[pos] = c;
		}
		//
		// Get rid of leading zeros:
		//
		std::string::size_type n = result.find_first_not_of('0');
		if (!result.empty() && (n == std::string::npos))
			n = result.size() - 1;
		result.erase(0, n);
		if (flags & std::ios_base::showbase) {
			const char* pp = base == 8 ? "0" : "0x";
			result.insert(static_cast<std::string::size_type>(0), pp);
		}
	}
	else {
		result.assign(nbits / 3 + 1u, '0');
		std::string::difference_type pos = result.size() - 1;
		Integer t(n);
		if (t.sign()) t.twosComplement();  // TODO: how to deal with maxneg which has no positive representation in 2's complement?

		Integer block10;
		unsigned digits_in_block10 = 2;
		if constexpr (IntegerBase::bitsInBlock == 8) {
			block10 = 100;
			digits_in_block10 = 2;
		}
		else if constexpr (IntegerBase::bitsInBlock == 16) {
			block10 = 10'000;
			digits_in_block10 = 4;
		}
		else if constexpr (IntegerBase::bitsInBlock == 32) {
			block10 = 1'000'000'000;
			digits_in_block10 = 9;
		}
		else if constexpr (IntegerBase::bitsInBlock == 64) {
			block10 = 1'000'000'000'000'000'000;
			digits_in_block10 = 18;
		}

		while (!t.iszero()) {

			Integer t2 = t / block10;
			Integer r  = t % block10;
//			std::cout << "t  " << long(t) << '\n';
//			std::cout << "t2 " << long(t2) << '\n';
//			std::cout << "r  " << long(r) << '\n';
			t = t2;
			BlockType v = r.block(0);
//			std::cout << "v  " << uint32_t(v) << '\n';
			for (unsigned i = 0; i < digits_in_block10; ++i) {
//				std::cout << i << " " << (v / 10) << " " << (v % 10) << '\n';
				char c = '0' + v % 10;
				v /= 10;
				result[pos] = c;
//				std::cout << result << '\n';
				if (pos-- == 0)
					break;
			}
		}

		std::string::size_type firstDigit = result.find_first_not_of('0');
		result.erase(0, firstDigit);
		if (result.empty())
			result = "0";
		if (n.isneg())
			result.insert(static_cast<std::string::size_type>(0), 1, '-');
		else if (flags & std::ios_base::showpos)
			result.insert(static_cast<std::string::size_type>(0), 1, '+');
	}
	return result;
}

#ifdef OLD
// generate an integer format ASCII format
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline std::ostream& operator<<(std::ostream& ostr, const integer<nbits, BlockType, NumberType>& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << convert_to_decimal_string(i);

	return ostr << ss.str();
}
#else 
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline std::ostream& operator<<(std::ostream& ostr, const integer<nbits, BlockType, NumberType>& i) {
	std::streamsize prec = ostr.precision();
	std::string s = convert_to_string(ostr.flags(), i);
	std::streamsize width = ostr.width();
	if (width > static_cast<std::streamsize>(s.size())) {
		char fill = ostr.fill();
		if ((ostr.flags() & std::ios_base::left) == std::ios_base::left)
			s.append(static_cast<std::string::size_type>(width - s.size()), fill);
		else
			s.insert(static_cast<std::string::size_type>(0), static_cast<std::string::size_type>(width - s.size()), fill);
	}
	return ostr << s;
}
#endif
// read an ASCII integer format
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline std::istream& operator>>(std::istream& istr, integer<nbits, BlockType, NumberType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string to_binary(const integer<nbits, BlockType, NumberType>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = nbits - 1; i >= 0; --i) {
		s << (number.at(static_cast<size_t>(i)) ? "1" : "0");
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// integer - integer binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator==(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	for (unsigned i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) return false;
	}
	return true;
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator!=(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator< (const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	bool lhs_is_negative = lhs.sign();
	bool rhs_is_negative = rhs.sign();
	if (lhs_is_negative && !rhs_is_negative) return true;
	if (rhs_is_negative && !lhs_is_negative) return false;
	// arguments have the same sign
	integer<nbits, BlockType, NumberType> diff(0);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
	// we need to catch and ignore the exception
	try {
		diff = (lhs - rhs);
	}
	catch (const integer_overflow& e) {
		// all good as the arithmetic is modulo
		const char* p = e.what();
		if (p) --p;
	}
#else 
	diff = (lhs - rhs);
#endif
	return diff.sign();
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator> (const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator< (rhs, lhs);
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator<=(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return !operator> (lhs, rhs);
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator>=(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// integer - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<size_t nbits, typename BlockType, IntegerNumberType NumberType, typename IntType>
inline bool operator==(const integer<nbits, BlockType, NumberType>& lhs, IntType rhs) {
	return operator==(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType, typename IntType>
inline bool operator!=(const integer<nbits, BlockType, NumberType>& lhs, IntType rhs) {
	return !operator==(lhs, rhs);
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType, typename IntType>
inline bool operator< (const integer<nbits, BlockType, NumberType>& lhs, IntType rhs) {
	return operator<(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType, typename IntType>
inline bool operator> (const integer<nbits, BlockType, NumberType>& lhs, IntType rhs) {
	return operator< (integer<nbits, BlockType, NumberType>(rhs), lhs);
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType, typename IntType>
inline bool operator<=(const integer<nbits, BlockType, NumberType>& lhs, IntType rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<size_t nbits, typename BlockType, IntegerNumberType NumberType, typename IntType>
inline bool operator>=(const integer<nbits, BlockType, NumberType>& lhs, IntType rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - integer binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<typename IntType, size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator==(IntType lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator==(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
template<typename IntType, size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator!=(IntType lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return !operator==(lhs, rhs);
}
template<typename IntType, size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator< (IntType lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator<(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
template<typename IntType, size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator> (IntType lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator< (rhs, lhs);
}
template<typename IntType, size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator<=(IntType lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<typename IntType, size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline bool operator>=(IntType lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator<<(const integer<nbits, BlockType, NumberType>& lhs, int shift) {
	integer<nbits, BlockType, NumberType> shifted(lhs);
	return (shifted <<= shift);
}

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator>>(const integer<nbits, BlockType, NumberType>& lhs, int shift) {
	integer<nbits, BlockType, NumberType> shifted(lhs);
	return (shifted >>= shift);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// integer - integer binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator+(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator-(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator*(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator/(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> ratio(lhs);
	ratio /= rhs;
	return ratio;
}
// BINARY REMAINDER
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator%(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> ratio(lhs);
	ratio %= rhs;
	return ratio;
}
// BINARY BIT-WISE AND
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator&(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> bitwise(lhs);
	bitwise &= rhs;
	return bitwise;
}
// BINARY BIT-WISE OR
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator|(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> bitwise(lhs);
	bitwise |= rhs;
	return bitwise;
}
// BINARY BIT-WISE XOR
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator^(const integer<nbits, BlockType, NumberType>& lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	integer<nbits, BlockType, NumberType> bitwise(lhs);
	bitwise ^= rhs;
	return bitwise;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// integer - literal binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator+(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator+(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
// BINARY SUBTRACTION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator-(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator-(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
// BINARY MULTIPLICATION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator*(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator*(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
// BINARY DIVISION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator/(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator/(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
// BINARY REMAINDER
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator%(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator%(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
// BINARY BIT-WISE AND
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator&(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator&(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
// BINARY BIT-WISE OR
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator|(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator|(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
// BINARY BIT-WISE XOR
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator^(const integer<nbits, BlockType, NumberType>& lhs, long long rhs) {
	return operator^(lhs, integer<nbits, BlockType, NumberType>(rhs));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - integer binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator+(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator+(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
// BINARY SUBTRACTION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator-(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator-(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator*(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator*(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
// BINARY DIVISION
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator/(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator/(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
// BINARY REMAINDER
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator%(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator%(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
// BINARY BIT-WISE AND
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator&(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator&(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
// BINARY BIT-WISE OR
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator|(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator|(integer<nbits, BlockType, NumberType>(lhs), rhs);
}
// BINARY BIT-WISE XOR
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline integer<nbits, BlockType, NumberType> operator^(long long lhs, const integer<nbits, BlockType, NumberType>& rhs) {
	return operator^(integer<nbits, BlockType, NumberType>(lhs), rhs);
}

}} // namespace sw::universal
