#pragma once
// lns_impl.hpp: implementation of an arbitrary logarithmic number system configuration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/behavior/arithmetic.hpp>
#include <universal/number/lns/lns_fwd.hpp>

namespace sw { namespace universal {
		
	// arithmetic event statistics
	constexpr bool bCollectLnsEventStatistics = false;  // by default, event statistics are disabled
	struct LnsArithmeticStatistics {
		LnsArithmeticStatistics() : conversionEvents{ 0 } {}
		void reset() {
			conversionEvents = 0;
		}
		int conversionEvents;
	};
	inline std::ostream& operator<<(std::ostream& ostr, const LnsArithmeticStatistics& stats) {
		ostr << "Conversions                     : " << stats.conversionEvents << '\n';
		return ostr;
	}
	static LnsArithmeticStatistics lnsStats;

// convert a floating-point value to a specific lns configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline lns<nbits, rbits, bt, xtra...>& convert(const triple<nbits, bt>& v, lns<nbits, rbits, bt, xtra...>& p) {
	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnan();
		return p;
	}
	return p;
}

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline lns<nbits, rbits, bt, xtra...>& minpos(lns<nbits, rbits, bt, xtra...>& lminpos) {
	return lminpos;
}
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>& maxpos(lns<nbits, rbits, bt, xtra...>& lmaxpos) {
	return lmaxpos;
}
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>& minneg(lns<nbits, rbits, bt, xtra...>& lminneg) {
	return lminneg;
}
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>& maxneg(lns<nbits, rbits, bt, xtra...>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in logarithmic form with a sign bit and a fixed-point exponent
// nbits is the total number of bits, rbits represent the rational bit in the fixed-point exponent.
template<unsigned _nbits, unsigned _rbits, typename bt = uint8_t, auto... xtra>
class lns {
	static_assert(_nbits > _rbits, "configuration not supported: not enough integer bits");
	static_assert( sizeof...(xtra) <= 1, "At most one optional extra argument is currently supported" );
	static_assert(_nbits - _rbits < 66, "configuration not supported: the scale of this configuration is > 2^64");
	static_assert(_rbits < 64, "configuration not supported: scaling factor is > 2^64");
public:
	typedef bt BlockType;

	static constexpr unsigned nbits    = _nbits;
	static constexpr unsigned rbits    = _rbits;
	static constexpr Behavior behavior = {xtra...};

	static constexpr double   scaling = double(1ull << rbits);
	static constexpr unsigned bitsInByte = 8ull;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr unsigned MSU = nrBlocks - 1;
	static constexpr bt       MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK = bt(1ull << ((nbits - 1ull) % bitsInBlock));
	static constexpr unsigned MSB_UNIT = (1ull + ((nbits - 2) / bitsInBlock)) - 1ull;
	static constexpr bt       MSB_BIT_MASK = bt(1ull << ((nbits - 2ull) % bitsInBlock));
	static constexpr bt       BLOCK_MSB_MASK = bt(1ull << (bitsInBlock - 1));
	static constexpr bool     SPECIAL_BITS_TOGETHER = (nbits > ((nrBlocks - 1) * bitsInBlock + 1));
	static constexpr bt       MSU_ZERO = MSB_BIT_MASK;
	static constexpr bt       MSU_NAN = SIGN_BIT_MASK | MSU_ZERO;  // only valid when special bits together is true
	static constexpr int64_t  maxShift = static_cast<int64_t>(nbits) - static_cast<int64_t>(rbits) - 2;
	static constexpr unsigned leftShift = (maxShift < 0) ? 0 : maxShift;
	static constexpr int64_t  min_exponent = (maxShift > 0) ? (-(1ll << leftShift)) : 0;
	static constexpr int64_t  max_exponent = (maxShift > 0) ? (1ll << leftShift) - 1 : 0;

	using BlockBinary = blockbinary<nbits, bt, BinaryNumberType::Signed>; // sign + lns exponent
	using ExponentBlockBinary = blockbinary<nbits-1, bt, BinaryNumberType::Signed>;  // just the lns exponent

	/// trivial constructor
	lns() = default;

	// decorated/converting constructors
	constexpr lns(const std::string& stringRep) {
		assign(stringRep);
	}

	template<unsigned srcnbits, unsigned srcrbits, typename srcbt, auto... srcxtra>
	constexpr lns(const lns<srcnbits, srcrbits, srcbt, srcxtra...>& rhs) {
		*this = double(rhs);
	}

	// specific value constructor
	constexpr lns(const SpecificValue code) noexcept
		: _block{} {
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
	constexpr lns(long initial_value)               noexcept { *this = initial_value; }
	constexpr lns(long long initial_value)          noexcept { *this = initial_value; }
	constexpr lns(unsigned char initial_value)      noexcept { *this = initial_value; }
	constexpr lns(unsigned short initial_value)     noexcept { *this = initial_value; }
	constexpr lns(unsigned int initial_value)       noexcept { *this = initial_value; }
	constexpr lns(unsigned long initial_value)      noexcept { *this = initial_value; }
	constexpr lns(unsigned long long initial_value) noexcept { *this = initial_value; }
	constexpr lns(float initial_value)              noexcept : _block{} { *this = initial_value; }
	constexpr lns(double initial_value)             noexcept : _block{} { *this = initial_value; }

	// assignment operators
	constexpr lns& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr lns& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr lns& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr lns& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr lns& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION lns& operator=(float rhs)        noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION lns& operator=(double rhs)       noexcept { return convert_ieee754(rhs); }

	// arithmetic operators
	// prefix operator
	constexpr lns operator-() const noexcept {
		if (isnan() || iszero()) return *this;
		lns negate(*this);
		negate.setbit(nbits - 1, !sign());
		return negate;
	}

	// in-place arithmetic assignment operators
	lns& operator+=(const lns& rhs) {
		double sum{ 0.0 };
		if constexpr (behavior == Behavior::Saturating) {
			sum = double(*this) + double(rhs);  // TODO: native implementation
		}
		else {
			sum = double(*this) + double(rhs);  // TODO: native implementation
		}
		return *this = sum; // <-- saturation happens in the assignment
	}
	lns& operator+=(double rhs) { 
		return operator+=(lns(rhs));
	}
	lns& operator-=(const lns& rhs) { 
		double diff{ 0.0 };
		if constexpr (behavior == Behavior::Saturating) {
			diff = double(*this) - double(rhs);  // TODO: native implementation
		}
		else {
			diff = double(*this) - double(rhs);  // TODO: native implementation
		}
		return *this = diff; // <-- saturation happens in the assignment
	}
	lns& operator-=(double rhs) {
		return operator-=(lns(rhs));
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
		ExponentBlockBinary lexp(_block), rexp(rhs._block); // strip the lns sign bit to yield the exponents
		bool negative = sign() ^ rhs.sign(); // determine sign of result
		if constexpr (behavior == Behavior::Saturating) { // saturating, no infinite
			static constexpr ExponentBlockBinary maxexp(SpecificValue::maxpos), minexp(SpecificValue::maxneg);
			blockbinary<nbits, bt, BinaryNumberType::Signed> maxpos(maxexp), maxneg(minexp); // expand into type of sum
			blockbinary<nbits, bt, BinaryNumberType::Signed> expandedLexp(lexp), expandedRexp(rexp); // expand and sign extend if necessary
			blockbinary<nbits, bt, BinaryNumberType::Signed> sum;

			sum = uradd(expandedLexp, expandedRexp);
			// check if sum is in range
			if (sum >= maxpos) {
				_block = maxpos;
			}
			else if (sum <= maxneg) {
				_block = maxneg;   // == zero encoding
				negative = false;  // ignore lns sign, otherwise this becomes NaN
			}
			else {
				_block.assign(sum); // this might set the lns sign, but we are going to explicitly set it before returning
			}
		}
		else {
			lexp += rexp;
			_block.assign(lexp);
		}
		setsign(negative);
		return *this;
	}
	lns& operator*=(double rhs) { return operator*=(lns(rhs)); }
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

		ExponentBlockBinary lexp(_block), rexp(rhs._block); // strip the lns sign bit to yield the exponents
		bool negative = sign() ^ rhs.sign(); // determine sign of result
		if constexpr (behavior == Behavior::Saturating) { // saturating, no infinite
			static constexpr ExponentBlockBinary maxexp(SpecificValue::maxpos), minexp(SpecificValue::maxneg);
			blockbinary<nbits, bt, BinaryNumberType::Signed> maxpos(maxexp), maxneg(minexp); // expand into type of sum
			blockbinary<nbits, bt, BinaryNumberType::Signed> expandedLexp(lexp), expandedRexp(rexp); // expand and sign extend if necessary
			blockbinary<nbits, bt, BinaryNumberType::Signed> sum;

			sum = ursub(expandedLexp, expandedRexp);
			// check if sum is in range
			if (sum >= maxpos) {
				_block = maxpos;
			}
			else if (sum <= maxneg) {
				_block = maxneg;   // == zero encoding
				negative = false;  // ignore lns sign, otherwise this becomes NaN
			}
			else {
				_block.assign(sum); // this might set the lns sign, but we are going to explicitly set it before returning
			}
		}
		else {
			lexp += rexp;
			_block.assign(lexp);
		}
		setsign(negative);
		return *this;
	}
	lns& operator/=(double rhs) { return operator/=(lns(rhs)); }

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
	constexpr void setnan(bool sign = false)       noexcept { (sign ? clear() : _block.clear()); setbit(nbits - 1); setbit(nbits - 2); }
	constexpr void setinf(bool sign)               noexcept { (sign ? maxneg() : maxpos()); } // TODO: is that what we want?
	constexpr void setsign(bool s = true)          noexcept { setbit(nbits - 1, s); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (i < nbits) {
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			//_block[i / bitsInBlock] = bt((block & null) | mask);
			_block.setblock(blockIndex, bt((block & null) | mask));
		}
		// nop if i is out of range
	}
	constexpr void setbits(uint64_t value) noexcept {
		if constexpr (1 == nrBlocks) {
			_block.setblock(0, value & storageMask);
		}
		else if constexpr (1 < nrBlocks) {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				_block.setblock(i, value & storageMask);
				value >>= bitsInBlock;
			}
		}
		_block.setblock(MSU, static_cast<bt>(_block[MSU] & MSU_MASK)); // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
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
				for (unsigned i = 0; i < nrBlocks - 1; ++i) {
					if (_block[i] != 0) return false;
				}
				return (_block[MSB_UNIT] == MSU_ZERO);  // this will cover the sign != 1 condition
			}
			else {
				for (unsigned i = 0; i < nrBlocks - 2; ++i) {
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
				for (unsigned i = 0; i < nrBlocks - 1; ++i) {
					if (_block[i] != 0) return false;
				}
				return (_block[MSB_UNIT] == MSU_NAN);
			}
			else {
				for (unsigned i = 0; i < nrBlocks - 2; ++i) {
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
	constexpr blockbinary<nbits+2, std::uint32_t, BinaryNumberType::Unsigned> fraction() const noexcept {
		blockbinary<nbits + 2, std::uint32_t, BinaryNumberType::Unsigned> bb{ 0 };
		// TODO: how? and what is the size of the blockbinary? it is much bigger than nbits+2
		assert(false && "lns.fraction() not implemented yet");
		return bb;
	}
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	constexpr bt block(unsigned b) const noexcept {
		if (b < nrBlocks) return _block[b];
		return bt(0); // return 0 when block index out of bounds
	}
// GCC false positive: -Warray-bounds gets confused across deeply-inlined
	// template instantiations and reports blockbinary<64> access on smaller types
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
	constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = int(n % (bitsInBlock >> 2ull));
			bt mask = bt(0xF << (nibbleIndexInWord * 4));
			bt nibblebits = bt(mask & word);
			return uint8_t(nibblebits >> (nibbleIndexInWord * 4));
		}
		return false;
	}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

	explicit constexpr operator int()       const noexcept { return to_signed<int>(); }
	explicit constexpr operator long()      const noexcept { return to_signed<long>(); }
	explicit constexpr operator long long() const noexcept { return to_signed<long long>(); }
	explicit constexpr operator float()     const noexcept { return to_ieee754<float>(); }
	explicit constexpr operator double()    const noexcept { return to_ieee754<double>(); }
	
	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	lns(long double initial_value)                        noexcept { *this = initial_value; }
	CONSTEXPRESSION lns& operator=(long double rhs)       noexcept { return convert_ieee754(rhs); }
	explicit constexpr operator long double()       const noexcept { return to_ieee754<long double>(); }
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
		std::cout << "maxShift              " << maxShift << '\n';
		std::cout << "leftShift             " << leftShift << '\n';
		std::cout << "min_exponent          " << min_exponent << '\n';
		std::cout << "max_exponent          " << max_exponent << '\n';
	}

protected:

	/// <summary>
	/// 1's complement of the encoding. Used internally to create specific bit patterns
	/// </summary>
	/// <returns>reference to this cfloat object</returns>
	constexpr lns& flip() noexcept { // in-place one's complement
		for (unsigned i = 0; i < nrBlocks; ++i) {
			//_block[i] = bt(~_block[i]);
			_block.setblock(i, bt(~_block[i]));
		}
		//_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		_block.setblock(MSU, bt(_block[MSU] & MSU_MASK));
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
		if constexpr (bCollectLnsEventStatistics) ++lnsStats.conversionEvents;
		bool s{ false };
		uint64_t unbiasedExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(v, s, unbiasedExponent, rawFraction, bits);
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

		// check if the value is in the representable range
		// NOTE: this is required to protect the rounding code below, which only works for values between [minpos, maxpos]
		// TODO: this is all incredibly slow as we are creating special values and converting them to Real to compare
		if constexpr (behavior == Behavior::Saturating) {
			constexpr lns maxpos(SpecificValue::maxpos);
			constexpr lns maxneg(SpecificValue::maxneg);
			Real absoluteValue = std::abs(v);
			//std::cout << "maxpos : " << to_binary(maxpos) << " : " << maxpos << '\n';
			if (v > 0 && v >= Real(maxpos)) {
				return *this = maxpos;
			}
			if (v < 0 && v <= Real(maxneg)) {
				return *this = maxneg;
			}
			constexpr lns minpos(SpecificValue::minpos);
			constexpr lns<nbits + 1, rbits + 1, bt, xtra...> halfMinpos(SpecificValue::minpos); // in log space
			//std::cout << "minpos     : " << minpos << '\n';
			//std::cout << "halfMinpos : " << halfMinpos << '\n';
			if (absoluteValue <= Real(halfMinpos)) {
				setzero();
				return *this;
			}
			else if (absoluteValue <= Real(minpos)) {
				return *this = (v > 0 ? minpos : -minpos);
			}
		}

		bool negative = (v < Real(0.0f));
		v = (negative ? -v : v);
		Real logv = std::log2(v);
		if (logv == 0.0) {
			_block.clear();
			_block.setbit(nbits - 1, negative);
			return *this;
		}

		ExponentBlockBinary lnsExponent{ 0 };

		extractFields(logv, s, unbiasedExponent, rawFraction, bits); // use native conversion
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
				for (unsigned i = 0; i < ieee754_parameter<Real>::fbits + 1; ++i) {
					if (rawFraction & 0x01) {
						lnsExponent.setbit(i + shiftLeft);
					}
					rawFraction >>= 1;
				}
				if (s) lnsExponent.twosComplement();
			}
		}

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
		constexpr unsigned minNormalExponent = static_cast<unsigned>(-ieee754_parameter<TargetFloat > ::minNormalExp);
		constexpr unsigned minSubnormalExponent = static_cast<unsigned>(-ieee754_parameter<TargetFloat>::minSubnormalExp);
		static_assert(rbits <= minSubnormalExponent, "lns::to_ieee754: fraction is too small to represent with requested floating-point type");
		TargetFloat multiplier = 0;
		if constexpr (rbits > minNormalExponent) { // value is a subnormal number
			multiplier = ieee754_parameter<TargetFloat>::minSubnormal;
			for (unsigned i = 0; i < minSubnormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		else {
			// the value is a normal number
			multiplier = ieee754_parameter<TargetFloat>::minNormal;
			for (unsigned i = 0; i < minNormalExponent - rbits; ++i) {
				multiplier *= 2.0f; // these are error free multiplies
			}
		}
		// you pop out here with multiplier set to the weight of the starting bit
		ExponentBlockBinary bb(_block);  // strip the sign bit
		bool expNegative = bb.sign();
		if (expNegative) bb.twosComplement();
		// construct the value
		TargetFloat value{ 0.0 };
		unsigned bit = 0;
		for (unsigned b = 0; b < bb.nrBlocks; ++b) {
			BlockType mask = static_cast<BlockType>(1ull);
			BlockType limb = bb[b];
			for (unsigned i = 0; i < bitsInBlock; ++i) {
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

	////////////////////// operators

	/// stream operators

	friend std::ostream& operator<< (std::ostream& ostr, const lns& r) {
		ostr << double(r);
		return ostr;
	}
	friend std::istream& operator>> (std::istream& istr, lns& r) {
		double item;
		istr >> item;
		r = item;
		return istr;
	}

	// lns - logic operators

	friend constexpr bool operator==(const lns& lhs, const lns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		return lhs._block == rhs._block;
	}
	friend constexpr bool operator< (const lns& lhs, const lns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		blockbinary<nbits-1, bt, BinaryNumberType::Signed> l(lhs._block), r(rhs._block); // extract the 2's complement exponent
		bool lhs_is_negative = lhs.sign();
		return (lhs_is_negative != rhs.sign()) ? lhs_is_negative
		                                       : lhs_is_negative ? l > r : l < r;
	}

	friend constexpr bool operator!=(const lns& lhs, const lns& rhs) {
		return !operator==(lhs, rhs);
	}
	friend constexpr bool operator> (const lns& lhs, const lns& rhs) {
		return  operator< (rhs, lhs);
	}
	friend constexpr bool operator<=(const lns& lhs, const lns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		return !operator> (lhs, rhs);
	}
	friend constexpr bool operator>=(const lns& lhs, const lns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		return !operator< (lhs, rhs);
	}

	// lns - literal logic operators

	friend constexpr bool operator==(const lns& lhs, double rhs) { return lhs == lns(rhs); }
	friend constexpr bool operator!=(const lns& lhs, double rhs) { return !operator==(lhs, rhs); }
	friend constexpr bool operator< (const lns& lhs, double rhs) { return lhs < lns(rhs); }
	friend constexpr bool operator> (const lns& lhs, double rhs) { return  operator< (rhs, lhs); }
	friend constexpr bool operator<=(const lns& lhs, double rhs) { return !operator> (lhs, rhs); }
	friend constexpr bool operator>=(const lns& lhs, double rhs) { return !operator< (lhs, rhs); }

	// lns - lns binary arithmetic operators

	friend constexpr lns operator+(const lns& lhs, const lns& rhs) {
		lns sum(lhs);
		sum += rhs;
		return sum;
	}
	friend constexpr lns operator-(const lns& lhs, const lns& rhs) {
		lns diff(lhs);
		diff -= rhs;
		return diff;
	}
	friend constexpr lns operator*(const lns& lhs, const lns& rhs) {
		lns mul(lhs);
		mul *= rhs;
		return mul;
	}
	friend constexpr lns operator/(const lns& lhs, const lns& rhs) {
		lns ratio(lhs);
		ratio /= rhs;
		return ratio;
	}

	// lns - literal binary arithmetic operators

	friend constexpr lns operator+(const lns& lhs, double rhs) {
		lns sum(lhs);
		sum += rhs;
	}
	friend constexpr lns operator-(const lns& lhs, double rhs) {
		lns diff(lhs);
		diff -= rhs;
		return diff;
	}
	friend constexpr lns operator*(const lns& lhs, double rhs) {
		lns mul(lhs);
		mul *= rhs;
		return mul;
	}
	friend constexpr lns operator/(const lns& lhs, double rhs) {
		lns ratio(lhs);
		ratio /= rhs;
		return ratio;
	}

	// literal - lns binary arithmetic operators

	friend constexpr lns operator+(double lhs, const lns& rhs) {
		lns sum(lhs);
		sum += rhs;
		return sum;
	}
	friend constexpr lns operator-(double lhs, const lns& rhs) {
		lns diff(lhs);
		diff -= rhs;
		return diff;
	}
	friend constexpr lns operator*(double lhs, const lns& rhs) {
		lns mul(lhs);
		mul *= rhs;
		return mul;
	}
	friend constexpr lns operator/(double lhs, const lns& rhs) {
		lns ratio(lhs);
		ratio /= rhs;
		return ratio;
	}
};

// return the Unit in the Last Position
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline lns<nbits, rbits, bt, xtra...> ulp(const lns<nbits, rbits, bt, xtra...>& a) {
	lns<nbits, rbits, bt, xtra...> b(a);
	return ++b - a;
}

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
std::string to_binary(const lns<nbits, rbits, bt, xtra...>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	s << (number.sign() ? "1." : "0.");
	if constexpr (nbits - 2 >= rbits) {
		for (int i = static_cast<int>(nbits) - 2; i >= static_cast<int>(rbits); --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && (i - rbits) > 0 && ((i - rbits) % 4) == 0) s << '\'';
		}
	}
	if constexpr (rbits > 0) {
		s << '.';
		for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && i > 0 && (i % 4) == 0) s << '\'';
		}
	}
	return s.str();
}

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
std::string to_triple(const lns<nbits, rbits, bt, xtra...>& v, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	s << (v.sign() ? "(-, " : "(+, ");
	s << v.scale() << ", ";
	s << to_hex(v.fraction(), nibbleMarker) << ')';
	return s.str();
}

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
std::string components(const lns<nbits, rbits, bt, xtra...>& v) {
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
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
constexpr lns<nbits, rbits, bt, xtra...> abs(const lns<nbits, rbits, bt, xtra...>& v) {
	lns<nbits, rbits, bt, xtra...> magnitude(v);
	magnitude.setsign(false);
	return magnitude;
}
// ToDo constexpt frexp
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> frexp(const lns<nbits, rbits, bt, xtra...>& x, int* exp) {
	return lns<nbits, rbits, bt, xtra...>(std::frexp(double(x), exp));
}
// ToDo constexpr ldexp
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> ldexp(const lns<nbits, rbits, bt, xtra...>& x, int exp) {
		return lns<nbits, rbits, bt, xtra...>(std::ldexp(double(x), exp));
}

}} // namespace sw::universal
