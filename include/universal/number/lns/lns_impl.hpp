#pragma once
// lns_impl.hpp: implementation of an arbitrary logarithmic number system configuration
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {
		
// Forward definitions
template<size_t nbits, size_t rbits, typename bt> class lns;
template<size_t nbits, size_t rbits, typename bt> lns<nbits, rbits, bt> abs(const lns<nbits, rbits, bt>& v);

// convert a floating-point value to a specific lns configuration. Semantically, p = v, return reference to p
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt>& convert(const triple<nbits, bt>& v, lns<nbits, rbits, bt>& p) {
	if (v.iszero()) {
		return p.setnan();
	}
	if (v.isnan() || v.isinf()) {
		return p.setnan();
	}
	return p;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt>& minpos(lns<nbits, rbits, bt>& lminpos) {
	return lminpos;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& maxpos(lns<nbits, rbits, bt>& lmaxpos) {
	return lmaxpos;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& minneg(lns<nbits, rbits, bt>& lminneg) {
	return lminneg;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& maxneg(lns<nbits, rbits, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t _nbits, size_t _rbits, typename bt = uint8_t>
class lns {
public:
	static constexpr size_t   nbits = _nbits;
	static constexpr size_t   rbits = _rbits;
	typedef bt BlockType;
	static constexpr double   scaling = double(1ull << rbits);
	static constexpr size_t   bitsInByte = 8ull;
	static constexpr size_t   bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t   nrBlocks = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr size_t   MSU = nrBlocks - 1;
	static constexpr bt       MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK = bt(1ull << ((nbits - 1ull) % bitsInBlock));
	static constexpr size_t   MSB_UNIT = (1ull + ((nbits - 2) / bitsInBlock)) - 1ull;
	static constexpr bt       MSB_BIT_MASK = bt(1ull << ((nbits - 2ull) % bitsInBlock));
	static constexpr bt       BLOCK_MSB_MASK = bt(1ull << (bitsInBlock - 1));
	static constexpr bool     SPECIAL_BITS_TOGETHER = (nbits > ((nrBlocks - 1) * bitsInBlock + 1));
	static constexpr bt       MSU_ZERO = MSB_BIT_MASK;
	static constexpr bt       MSU_NAN = SIGN_BIT_MASK | MSU_ZERO;  // only valid when special bits together is true

	using BlockBinary = blockbinary<nbits, bt, BinaryNumberType::Signed>; // sign + lns exponent
	using ExponentBlockBinary = blockbinary<nbits-1, bt, BinaryNumberType::Signed>;  // just the lns exponent

	/// trivial constructor
	lns() = default;

	// decorated/converting constructors
	constexpr lns(const std::string& stringRep) {
		assign(stringRep);
	}

	// specific value constructor
	constexpr lns(const SpecificValue code) noexcept
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
		case SpecificValue::infpos:
			setinf(false);
			break;
		case SpecificValue::infneg:
			setinf(true);
			break;
		case SpecificValue::nar: // approximation as lns don't have a NaR
		case SpecificValue::qnan:
		case SpecificValue::snan:
			setnan();
			break;
		}
	}

	constexpr lns(signed char initial_value)        noexcept { *this = initial_value; }
	constexpr lns(short initial_value)              noexcept { *this = initial_value; }
	constexpr lns(int initial_value)                noexcept { *this = initial_value; }
	constexpr lns(long long initial_value)          noexcept { *this = initial_value; }
	constexpr lns(unsigned long long initial_value) noexcept { *this = initial_value; }
	constexpr lns(float initial_value)              noexcept { *this = initial_value; }
	constexpr lns(double initial_value)             noexcept { *this = initial_value; }

	// assignment operators
	constexpr lns& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION lns& operator=(float rhs)        noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION lns& operator=(double rhs)       noexcept { return convert_ieee754(rhs); }

	// arithmetic operators
	// prefix operator
	constexpr lns operator-() const noexcept {
		lns negate(*this);
		negate.setbit(nbits - 1, !sign());
		return negate;
	}

	// in-place arithmetic assignment operators
	lns& operator+=(const lns& rhs) {
		float sum = float(*this) + float(rhs);  // TODO: why floats? because this is a shortcut and we focus on small values
		return *this = sum; 
	}
	lns& operator+=(double rhs) { 
		float sum = float(*this) + float(rhs);  // TODO: why floats? because this is a shortcut and we focus on small values
		return *this = sum;
//		return *this += lns(rhs); 
	}
	lns& operator-=(const lns& rhs) { 
		float diff = float(*this) - float(rhs);  // TODO: why floats? because this is a shortcut and we focus on small values
		return *this = diff;
	}
	lns& operator-=(double rhs) {
		float diff = float(*this) - float(rhs);  // TODO: why floats? because this is a shortcut and we focus on small values
		return *this = diff;
		// return *this -= lns(rhs); 
	}
	lns& operator*=(const lns& rhs) {
		if (isnan()) return *this;
		if (rhs.isnan()) {
			setnan();
			return *this;
		}
		if (iszero()) return *this;
		if (rhs.iszero()) {
			setzero();
			return *this;
		}
		ExponentBlockBinary exp(_block), rhsExp(rhs._block);
		exp += rhsExp;
		bool negative = sign() ^ rhs.sign();
		_block.assign(exp);
		setsign(negative);
		return *this;
	}
	lns& operator*=(double rhs) { return *this *= lns(rhs); }
	lns& operator/=(const lns& rhs) {
		if (isnan()) return *this;
		if (rhs.isnan()) {
			setnan();
			return *this;
		}
		if (rhs.iszero()) {
#if LNS_THROW_ARITHMETIC_EXCEPTION
			throw lns_divide_by_zero();
#else
			setnan();
			return *this;
#endif
		}
		if (iszero()) return *this;

		ExponentBlockBinary exp(_block), rhsExp(rhs._block);
		exp -= rhsExp;
		bool negative = sign() ^ rhs.sign();
		_block.assign(exp);
		setsign(negative);
		return *this;
	}
	lns& operator/=(double rhs) { return *this /= lns(rhs); }

	// prefix/postfix operators
	lns& operator++() {
		++_block;
		return *this;
	}
	lns operator++(int) {
		lns tmp(*this);
		operator++();
		return tmp;
	}
	lns& operator--() {
		--_block;
		return *this;
	}
	lns operator--(int) {
		lns tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	// clear resets all bits
	constexpr void clear()                         noexcept { _block.clear(); }
	constexpr void setzero()                       noexcept { _block.clear(); setbit(nbits - 2, true); }
	constexpr void setnan()                        noexcept { _block.clear(); setbit(nbits - 1); setbit(nbits - 2); }
	constexpr void setinf(bool sign)               noexcept { (sign ? maxneg() : maxpos()); } // TODO: is that what we want?
	constexpr void setsign(bool s = true)          noexcept { setbit(nbits - 1, s); }
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
	
	// create specific number system values of interest
	constexpr lns& maxpos() noexcept {
		// maximum positive value has this bit pattern: 0-01..1-111...111, that is, sign = 0, integer = 01..11, fraction = 11..11
		clear();
		flip();
		setbit(nbits - 1ull, false); // sign = 0
		setbit(nbits - 2ull, false); // msb  = 0
		return *this;
	}
	constexpr lns& minpos() noexcept {
		// minimum positive value has this bit pattern: 0-100-00...01, that is, sign = 0, integer = 10..00, fraction = 00..01
		clear();
		setbit(nbits - 2, true);    // msb  = 1
		setbit(0, true);            // lsb  = 1
		return *this;
	}
	constexpr lns& zero() noexcept {
		// the zero value has this bit pattern: 0-100..00-00..000, sign = 0, msb = 1, rest 0
		clear(); 
		setbit(nbits - 2, true);    // msb = 1
		return *this;
	}
	constexpr lns& minneg() noexcept {
		// minimum negative value has this bit pattern: 1-100-00...01, that is, sign = 1, integer = 10..00, fraction = 00..01
		clear();
		setbit(nbits - 1ull, true); // sign = 1
		setbit(nbits - 2, true);    // msb  = 1
		setbit(0, true);            // lsb  = 1
		return *this;
	}
	constexpr lns& maxneg() noexcept {
		// maximum negative value has this bit pattern: 1-01..1-11..11, that is, sign = 1, integer = 01..1, fraction = 11..11
		clear();
		flip();
		setbit(nbits - 2ull, false); // msb  = 0
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept { // special encoding: 0.1000.0000
		if constexpr (nrBlocks == 1) {
			return (_block[MSB_UNIT] == MSU_ZERO);
		}
		else if constexpr (nrBlocks == 2) {
			if constexpr (SPECIAL_BITS_TOGETHER) {
				return (_block[0] == 0 && _block[1] == MSU_ZERO);
			}
			else {
				return !sign() && _block[0] == MSB_BIT_MASK;
			}
		}
		else {
			if constexpr (SPECIAL_BITS_TOGETHER) {
				for (size_t i = 0; i < nrBlocks - 1; ++i) {
					if (_block[i] != 0) return false;
				}
				return (_block[MSB_UNIT] == MSU_ZERO);  // this will cover the sign != 1 condition
			}
			else {
				for (size_t i = 0; i < nrBlocks - 2; ++i) {
					if (_block[i] != 0) return false;
				}
				return !sign() && _block[MSB_UNIT] == BLOCK_MSB_MASK;
			}
		}
	}
	constexpr bool isneg()  const noexcept { return sign(); }
	constexpr bool ispos()  const noexcept { return !sign(); }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool isnan()  const noexcept { // special encoding
		if constexpr (nrBlocks == 1) {
			return (_block[MSB_UNIT] == MSU_NAN);  // 1.1000.0000 is NaN
		}
		else if constexpr (nrBlocks == 2) {
			if constexpr (SPECIAL_BITS_TOGETHER) {
				return (_block[0] == 0 && _block[1] == MSU_NAN);
			}
			else {
				return sign() && (_block[MSU - 1] == BLOCK_MSB_MASK);
			}
		}
		else {
			if constexpr (SPECIAL_BITS_TOGETHER) {
				for (size_t i = 0; i < nrBlocks - 1; ++i) {
					if (_block[i] != 0) return false;
				}
				return (_block[MSB_UNIT] == MSU_NAN);
			}
			else {
				for (size_t i = 0; i < nrBlocks - 2; ++i) {
					if (_block[i] != 0) return false;
				}
				return sign() && (_block[MSU - 1] == BLOCK_MSB_MASK);
			}
		}
	}
	constexpr bool sign()   const noexcept { 
		return (SIGN_BIT_MASK & _block[MSU]) != 0;
	}
	constexpr int  scale()  const noexcept {
		ExponentBlockBinary exp(_block);
		exp >>= rbits;
		return long(exp);
	}

	constexpr bool at(size_t bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	inline constexpr bt block(size_t b) const noexcept {
		if (b < nrBlocks) return _block[b];
		return bt(0); // return 0 when block index out of bounds
	}

	explicit operator int()       const noexcept { return to_signed<int>(); }
	explicit operator long()      const noexcept { return to_signed<long>(); }
	explicit operator long long() const noexcept { return to_signed<long long>(); }
	explicit operator float()     const noexcept { return to_ieee754<float>(); }
	explicit operator double()    const noexcept { return to_ieee754<double>(); }
	
	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	lns(long double initial_value)                        noexcept { *this = initial_value; }
	CONSTEXPRESSION lns& operator=(long double rhs)       noexcept { return convert_ieee754(rhs); }
	explicit operator long double()                 const noexcept { return to_ieee754<long double>(); }
#endif

	void debugConstexprParameters() {
		std::cout << "constexpr parameters for " << type_tag(*this) << '\n';
		std::cout << "scaling               " << scaling << '\n';
		std::cout << "bitsInByte            " << bitsInByte << '\n';
		std::cout << "bitsInBlock           " << bitsInBlock << '\n';
		std::cout << "nrBlocks              " << nrBlocks << '\n';
		std::cout << "storageMask           " << to_binary(storageMask, bitsInBlock) << '\n';
		std::cout << "MSU                   " << MSU << '\n';
		std::cout << "MSU_MASK              " << to_binary(MSU_MASK, bitsInBlock) << '\n';
		std::cout << "MSB_UNIT              " << MSB_UNIT << '\n';
		std::cout << "SPECIAL_BITS_TOGETHER " << (SPECIAL_BITS_TOGETHER ? "yes" : "no") << '\n';
		std::cout << "SIGN_BIT_MASK         " << to_binary(SIGN_BIT_MASK, bitsInBlock) << '\n';
		std::cout << "MSB_BIT_MASK          " << to_binary(MSB_BIT_MASK, bitsInBlock) << '\n';
		std::cout << "BLOCK_MSB_MASK        " << to_binary(BLOCK_MSB_MASK, bitsInBlock) << '\n';
		std::cout << "MSU_ZERO              " << to_binary(MSU_ZERO, bitsInBlock) << '\n';
		std::cout << "MSU_NAN               " << to_binary(MSU_NAN, bitsInBlock) << '\n';
	}

protected:

	/// <summary>
	/// 1's complement of the encoding. Used internally to create specific bit patterns
	/// </summary>
	/// <returns>reference to this cfloat object</returns>
	constexpr lns& flip() noexcept { // in-place one's complement
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}

	/// <summary>
	/// assign the value of the string representation to the cfloat
	/// </summary>
	/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
	/// <returns>reference to this cfloat</returns>
	/// Clang doesn't support constexpr yet on string manipulations, so we need to make it conditional
	CONSTEXPRESSION lns& assign(const std::string& str) noexcept {
		clear();
		return *this;
	}

	//////////////////////////////////////////////////////
	/// convertion routines from native types

	template<typename SignedInt>
	CONSTEXPRESSION lns& convert_signed(SignedInt v) noexcept {
		return convert_ieee754(double(v));
	}
	template<typename UnsignedInt>
	CONSTEXPRESSION lns& convert_unsigned(UnsignedInt v) noexcept {
		return convert_ieee754(double(v));
	}
	template<typename Real>
	CONSTEXPRESSION lns& convert_ieee754(Real v) noexcept {
		bool s{ false };
		uint64_t unbiasedExponent{ 0 };
		uint64_t rawFraction{ 0 };
		extractFields(v, s, unbiasedExponent, rawFraction);
		if (unbiasedExponent == ieee754_parameter<Real>::eallset) { // nan and inf need to be remapped
			if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::snanmask) ||
				rawFraction == (ieee754_parameter<Real>::fmask & (ieee754_parameter<Real>::qnanmask | ieee754_parameter<Real>::snanmask))) {
				// 1.11111111.00000000.......00000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000.......00000001 signalling nan
				// 0.11111111.10000000.......00000001 signalling nan
				setnan();
				return *this;
			}
			if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
				// 1.11111111.10000000.......00000000 quiet nan
				// 0.11111111.10000000.......00000000 quiet nan
				setnan();
				//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
				return *this;
			}
			if (rawFraction == 0ull) {
				// 1.11111111.0000000.......000000000 -inf
				// 0.11111111.0000000.......000000000 +inf
				setinf(s);
				return *this;
			}
		}
		if (v == 0.0) {
			setzero();
			return *this;
		}

		bool negative = (v < Real(0.0f));
		v = (negative ? -v : v);
		Real logv = std::log2(v);
		if (logv == 0.0) {
			_block.clear();
			_block.setbit(nbits - 1, negative);
			return *this;
		}
		// check if the value is in the representable range

		ExponentBlockBinary lnsExponent{ 0 };

		extractFields(logv, s, unbiasedExponent, rawFraction); // use native conversion
		if (unbiasedExponent > 0) rawFraction |= (1ull << ieee754_parameter<Real>::fbits);
		int radixPoint = ieee754_parameter<Real>::fbits - (static_cast<int>(unbiasedExponent) - ieee754_parameter<Real>::bias);

		// our fixed-point has its radixPoint at rbits
		int shiftRight = radixPoint - int(rbits);
		if (shiftRight > 0) {
			if (shiftRight > 63) {
				// this shift degree would be undefined behavior, but the intended transformation is that we have no bits
				rawFraction = 0;
			}
			else {
				// we need to round the raw bits
				// collect guard, round, and sticky bits
				// this same logic will work for the case where 
				// we only have a guard bit and no round and/or sticky bits
				// because the mask logic will make round and sticky both 0
				// so no need to special case it
				uint64_t mask = (1ull << (shiftRight - 1));
				bool guard = (mask & rawFraction);
				mask >>= 1;
				bool round = (mask & rawFraction);
				if (shiftRight > 1) {
					mask = (0xFFFF'FFFF'FFFF'FFFFull << (shiftRight - 2));
					mask = ~mask;
				}
				else {
					mask = 0;
				}
				bool sticky = (mask & rawFraction);

				rawFraction >>= shiftRight;  // shift out the bits we are rounding away
				bool lsb = (rawFraction & 0x1ul);
				//  ... lsb | guard  round sticky   round
				//       x     0       x     x       down
				//       0     1       0     0       down  round to even
				//       1     1       0     0        up   round to even
				//       x     1       0     1        up
				//       x     1       1     0        up
				//       x     1       1     1        up
				if (guard) {
					if (lsb && (!round && !sticky)) ++rawFraction; // round to even
					if (round || sticky) ++rawFraction;
				}
				rawFraction = (s ? (~rawFraction + 1) : rawFraction); // if negative, map to two's complement
			}
			lnsExponent.setbits(rawFraction);
		}
		else {
			int shiftLeft = -shiftRight;
			if (shiftLeft < (64 - ieee754_parameter<Real>::fbits)) {  // what is the distance between the MSB and 64?
				// no need to round, just shift the bits in place
				rawFraction <<= shiftLeft;
				rawFraction = (s ? (~rawFraction + 1) : rawFraction); // if negative, map to two's complement
				lnsExponent.setbits(rawFraction);
			}
			else {
				// we need to project the bits we have on the fixpnt
				for (size_t i = 0; i < ieee754_parameter<Real>::fbits + 1; ++i) {
					if (rawFraction & 0x01) {
						lnsExponent.setbit(i + shiftLeft);
					}
					rawFraction >>= 1;
				}
				if (s) lnsExponent.twosComplement();
			}
		}
//		std::cout << "lns exponent : " << to_binary(lnsExponent) << " : " << lnsExponent << '\n';
		_block = lnsExponent;
		setsign(negative);
		return *this;
	}

	//////////////////////////////////////////////////////
	/// convertion routines to native types

	template<typename SignedInt>
	typename std::enable_if< std::is_integral<SignedInt>::value&& std::is_signed<SignedInt>::value, SignedInt>::type
		to_signed() const {
		return SignedInt(to_ieee754<double>());
	}
	template<typename UnsignedInt>
	typename std::enable_if< std::is_integral<UnsignedInt>::value&& std::is_unsigned<UnsignedInt>::value, UnsignedInt>::type
		to_unsigned() const {
		return UnsignedInt(to_ieee754<double>());
	}
	template<typename TargetFloat>
	CONSTEXPRESSION TargetFloat to_ieee754() const noexcept {   // TODO: don't use bit math, use proper limb math to speed this up
		// special case handling
		if (isnan()) return TargetFloat(NAN);
		if (iszero()) return TargetFloat(0.0f);
		bool negative = sign(); // cache for later decision
		// pick up the absolute value of the minimum normal and subnormal exponents 
		constexpr size_t minNormalExponent = static_cast<size_t>(-ieee754_parameter<TargetFloat > ::minNormalExp);
		constexpr size_t minSubnormalExponent = static_cast<size_t>(-ieee754_parameter<TargetFloat>::minSubnormalExp);
		static_assert(rbits <= minSubnormalExponent, "lns::to_ieee754: fraction is too small to represent with requested floating-point type");
		TargetFloat multiplier = 0;
		if constexpr (rbits > minNormalExponent) { // value is a subnormal number
			multiplier = ieee754_parameter<TargetFloat>::minSubnormal;
			for (size_t i = 0; i < minSubnormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		else {
			// the value is a normal number
			multiplier = ieee754_parameter<TargetFloat>::minNormal;
			for (size_t i = 0; i < minNormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		// you pop out here with multiplier set to the weight of the starting bit
		ExponentBlockBinary bb(_block);  // strip the sign bit
		bool expNegative = bb.sign();
		if (expNegative) bb.twosComplement();
		// construct the value
		TargetFloat value{ 0.0 };
		size_t bit = 0;
		for (size_t b = 0; b < bb.nrBlocks; ++b) {
			BlockType mask = static_cast<BlockType>(1ull);
			BlockType limb = bb[b];
			for (size_t i = 0; i < bitsInBlock; ++i) {
				if (limb & mask) value += multiplier;
				if (bit == nbits - 2) break; // skip the sign bit of the lns
				++bit;
				mask <<= 1;
				multiplier *= 2.0;
			}
		}
		value = (expNegative ? -value : value);
		value = std::pow(TargetFloat(2.0f), value);
		return (negative ? -value : value);
	}

private:
	BlockBinary _block;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const lns<nnbits, rrbits, nbt>& r);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, lns<nnbits, rrbits, nbt>& r);

	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator==(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator< (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator> (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const lns<nnbits, rrbits, nbt>& v) {
	ostr << double(v);
	return ostr;
}

template<size_t nnbits, size_t rrbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const lns<nnbits, rrbits, nbt>& v) {
	istr >> v._fraction;
	return istr;
}

// lns - logic operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator==(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) {
	using LNS = lns<nnbits, rrbits, nbt>;
	if constexpr (LNS::nrBlocks == 1) {
		return lhs._block[0] == rhs._block[0];
	}
	else if constexpr (LNS::nrBlocks == 2) {
		return (lhs._block[0] == rhs._block[0]) && 
			   (lhs._block[1] == rhs._block[1]);
	}
	else if constexpr (LNS::nrBlocks == 3) {
		return (lhs._block[0] == rhs._block[0]) &&
			   (lhs._block[1] == rhs._block[1]) &&
			   (lhs._block[2] == rhs._block[2]);
	}
	else if constexpr (LNS::nrBlocks == 4) {
		return (lhs._block[0] == rhs._block[0]) &&
			   (lhs._block[1] == rhs._block[1]) &&
			   (lhs._block[2] == rhs._block[2]) &&
			   (lhs._block[3] == rhs._block[3]);
	}
	else {
		for (size_t i = 0; i < LNS::nrBlocks; ++i) {
			if (lhs.block(i) != rhs.block(i)) return false;
		}
		return true;
	}
}
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator< (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) {
	using LNS = lns<nnbits, rrbits, nbt>;
	bool lhsSign = lhs.at(nnbits - 1);
	bool rhsSign = rhs.at(nnbits - 1);
	if (lhsSign) {
		if (rhsSign) {
			LNS l(lhs);
			l.setbit(nnbits - 1, false);
			LNS r(rhs);
			r.setbit(nnbits - 1, false);
		}
		else {
			return true;
		}
	}
	else {
		if (rhsSign) {
			return false;
		}
		else {

		}
	}
	return false; 
}
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator> (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator< (lhs, rhs); }

// lns - literal logic operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator==(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return lhs == lns<nnbits, rrbits, nbt>(rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator< (const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return lhs < lns<nnbits, rrbits, nbt>(rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator> (const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator< (lhs, rhs); }

// lns - lns binary arithmetic operators

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator+(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> sum(lhs);
	sum += rhs;
	return sum;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator-(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> diff(lhs);
	diff -= rhs;
	return diff;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator*(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> mul(lhs);
	mul *= rhs;
	return mul;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator/(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

// lns - literal binary arithmetic operators

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator+(const lns<nbits, rbits, bt>& lhs, double rhs) {
	lns<nbits, rbits, bt> sum(lhs);
	sum += rhs;
	return sum;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator-(const lns<nbits, rbits, bt>& lhs, double rhs) {
	lns<nbits, rbits, bt> diff(lhs);
	diff -= rhs;
	return diff;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator*(const lns<nbits, rbits, bt>& lhs, double rhs) {
	lns<nbits, rbits, bt> mul(lhs);
	mul *= rhs;
	return mul;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator/(const lns<nbits, rbits, bt>& lhs, double rhs) {
	lns<nbits, rbits, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

// literal - lns binary arithmetic operators

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator+(double lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> sum(lhs);
	sum += rhs;
	return sum;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator-(double lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> diff(lhs);
	diff -= rhs;
	return diff;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator*(double lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> mul(lhs);
	mul *= rhs;
	return mul;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator/(double lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<size_t nbits, size_t rbits, typename bt>
inline std::string to_binary(const lns<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	s << (number.sign() ? "1." : "0.");
	if constexpr (nbits - 2 >= rbits) {
		for (int i = static_cast<int>(nbits) - 2; i >= static_cast<int>(rbits); --i) {
			s << (number.at(static_cast<size_t>(i)) ? '1' : '0');
			if ((i - rbits) > 0 && ((i - rbits) % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	if constexpr (rbits > 0) {
		s << '.';
		for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
			s << (number.at(static_cast<size_t>(i)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	return s.str();
}

template<size_t nbits, size_t rbits, typename bt>
inline std::string components(const lns<nbits, rbits, bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

// standard library functions for floating point

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> abs(const lns<nbits, rbits, bt>& v) {
	lns<nbits, rbits, bt> magnitude(v);
	magnitude.setsign(false);
	return magnitude;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> frexp(const lns<nbits, rbits, bt>& x, int* exp) {
	return lns<nbits, rbits, bt>(std::frexp(double(x), exp));
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> ldexp(const lns<nbits, rbits, bt>& x, int exp) {
		return lns<nbits, rbits, bt>(std::ldexp(double(x), exp));
}

}} // namespace sw::universal
