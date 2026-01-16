#pragma once
// dbns_impl.hpp: implementation of a fixed-size, arbitrary configuration 2-base logarithmic number system configuration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/abstract/triple.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/behavior/arithmetic.hpp>
#include <universal/number/dbns/dbns_fwd.hpp>

namespace sw { namespace universal {
		
	// arithmetic event statistics
	constexpr bool bCollectDbnsEventStatistics = false;  // by default, event statistics are disabled
	struct DbnsArithmeticStatistics {
		DbnsArithmeticStatistics() : conversionEvents{ 0 }, exponentOverflowDuringSearch{ 0 }, roundingFailure{ 0 } {}
		void reset() {
			conversionEvents = 0;
			exponentOverflowDuringSearch = 0;
			roundingFailure = 0;
		}
		int conversionEvents;
		int exponentOverflowDuringSearch;
		int roundingFailure;
	};

	inline std::ostream& operator<<(std::ostream& ostr, const DbnsArithmeticStatistics stats) {
		ostr << "Conversions                     : " << stats.conversionEvents << '\n';
		ostr << "Exponent Overflow During Search : " << stats.exponentOverflowDuringSearch << '\n';
		ostr << "Rounding Successes              : " << (stats.conversionEvents - stats.roundingFailure) << '\n';
		ostr << "Rounding Failures               : " << stats.roundingFailure << '\n';
		return ostr;
	}
	static DbnsArithmeticStatistics dbnsStats;

// convert a floating-point value to a specific dbns configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
inline dbns<nbits, fbbits, bt, xtra...>& convert(const triple<nbits, bt>& v, dbns<nbits, fbbits, bt, xtra...>& p) {
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

template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
inline dbns<nbits, fbbits, bt, xtra...>& minpos(dbns<nbits, fbbits, bt, xtra...>& dbns_minpos) {
	dbns<nbits, fbbits, bt, xtra...> a(SpecificValue::minpos);
	dbns_minpos = a;
	return dbns_minpos;
}
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
dbns<nbits, fbbits, bt, xtra...>& maxpos(dbns<nbits, fbbits, bt, xtra...>& dbns_maxpos) {
	dbns<nbits, fbbits, bt, xtra...> a(SpecificValue::maxpos);
	dbns_maxpos = a;
	return dbns_maxpos;
}
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
dbns<nbits, fbbits, bt, xtra...>& minneg(dbns<nbits, fbbits, bt, xtra...>& dbns_minneg) {
	dbns<nbits, fbbits, bt, xtra...> a(SpecificValue::minneg);
	dbns_minneg = a;
	return dbns_minneg;
}
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
dbns<nbits, fbbits, bt, xtra...>& maxneg(dbns<nbits, fbbits, bt, xtra...>& dbns_maxneg) {
	dbns<nbits, fbbits, bt, xtra...> a(SpecificValue::maxneg);
	dbns_maxneg = a;
	return dbns_maxneg;
}

// double-base logarithmic number system: bases 2^-1, and 3
template<unsigned _nbits, unsigned _fbbits, typename bt = uint8_t, auto... xtra>
class dbns {
	static_assert(_nbits > (_fbbits + 1), "configuration not supported: too many first base bits leaving no bits for second base");
	static_assert(_fbbits > 0, "fbbits == 0 is an invalid configuration: need to have two exponent fields to be a double-base number system");
	static_assert(sizeof...(xtra) <= 1, "At most one optional extra argument is currently supported");
	static_assert(_nbits - _fbbits < 66, "configuration not supported: the scale of this configuration is > 2^64");
	static_assert(_fbbits < 64, "configuration not supported: scaling factor is > 2^64");
public:
	typedef bt BlockType;

	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned fbbits = _fbbits;            // first base exponent bits
	static constexpr unsigned sbbits = nbits - fbbits - 1; // second base exponent bits
	static constexpr Behavior behavior = { xtra... };

	static constexpr double   scaling = double(1ull << fbbits);
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
	static constexpr int64_t  maxShift = nbits - fbbits - 2;
	static constexpr unsigned leftShift = (maxShift < 0) ? 0 : maxShift;
	static constexpr int64_t  min_exponent = (maxShift > 0) ? (-(1ll << leftShift)) : 0;
	static constexpr int64_t  max_exponent = (maxShift > 0) ? (1ll << leftShift) - 1 : 0;
	static constexpr int      rightShift = (fbbits == 0 ? 0 : (64 - fbbits));
	static constexpr uint64_t MAX_A = (rightShift > 0 ? (0xFFFF'FFFF'FFFF'FFFFull >> rightShift) : 0ull);
	static constexpr uint64_t FB_MASK = (MAX_A << sbbits);
	static constexpr uint64_t MAX_B   = (0xFFFF'FFFF'FFFF'FFFFull >> (64 - sbbits));
	static constexpr uint64_t SB_MASK = MAX_B;

	// the smallest value with this base set and the assumption that exponents are positive is 0b0.111.0000
	static constexpr double   base0   = 0.5;
	static constexpr double   base1   = 3.0;
	static constexpr double   log2of3 = 1.5849625007211561814537389439478;

	/// trivial constructor
	dbns() = default;

	// decorated/converting constructors
	constexpr dbns(const std::string& stringRep) {
		assign(stringRep);
	}

	// specific value constructor
	constexpr dbns(const SpecificValue code) noexcept
		: _block{0} {
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
		case SpecificValue::nar: // approximation as dbns don't have a NaR
		case SpecificValue::qnan:
		case SpecificValue::snan:
			setnan();
			break;
		}
	}

	constexpr dbns(signed char initial_value)        noexcept { *this = initial_value; }
	constexpr dbns(short initial_value)              noexcept { *this = initial_value; }
	constexpr dbns(int initial_value)                noexcept { *this = initial_value; }
	constexpr dbns(long initial_value)               noexcept { *this = initial_value; }
	constexpr dbns(long long initial_value)          noexcept { *this = initial_value; }
	constexpr dbns(unsigned long long initial_value) noexcept { *this = initial_value; }
	constexpr dbns(float initial_value)              noexcept { *this = initial_value; }
	constexpr dbns(double initial_value)             noexcept { *this = initial_value; }

	// assignment operators
	constexpr dbns& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr dbns& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr dbns& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr dbns& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr dbns& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr dbns& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION dbns& operator=(float rhs)        noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION dbns& operator=(double rhs)       noexcept { return convert_ieee754(rhs); }

	// arithmetic operators
	// prefix operator
	constexpr dbns operator-() const noexcept {
		if (isnan() || iszero()) return *this;
		dbns negate(*this);
		negate.setbit(nbits - 1, !sign());
		return negate;
	}

	// in-place arithmetic assignment operators
	dbns& operator+=(const dbns& rhs) {
		double sum{ 0.0 };
		if constexpr (behavior == Behavior::Saturating) {
			sum = double(*this) + double(rhs);  // TODO: native implementation
		}
		else {
			sum = double(*this) + double(rhs);  // TODO: native implementation
		}
		return *this = sum; // <-- saturation happens in the assignment
	}
	dbns& operator+=(double rhs) { 
		return operator+=(dbns(rhs));
	}
	dbns& operator-=(const dbns& rhs) { 
		double diff{ 0.0 };
		if constexpr (behavior == Behavior::Saturating) {
			diff = double(*this) - double(rhs);  // TODO: native implementation
		}
		else {
			diff = double(*this) - double(rhs);  // TODO: native implementation
		}
		return *this = diff; // <-- saturation happens in the assignment
	}
	dbns& operator-=(double rhs) {
		return operator-=(dbns(rhs));
	}
	dbns& operator*=(const dbns& rhs) {
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
#if defined(NATIVE_DBNS_ARITHMETIC)
		bool negative = sign() ^ rhs.sign(); // determine sign of result
		uint32_t a = extractExponent(0) + rhs.extractExponent(0);
		uint32_t b = extractExponent(1) + rhs.extractExponent(1);
		if constexpr (behavior == Behavior::Saturating) { // saturating, no infinite
			clear();
			if (a > MAX_A || b > MAX_B) {
				// try to project the value back into valid pairs
				// the approximations of unity looks like (3, -2), (8,-5), (19,-12), (84,-53),... 
				// they grow too fast and in a rather irregular manner. There are more 
				// subtle number theoretic considerations, but the ones outlined above 
				// should be sufficient to figure out a good solution to the problem.
				// 2^3*3^-2 = 0.888  2^-3*3^2 = 1.125
				// 2^8*3^-5 = 1.053  2^-8*3^5 = 0.949
				unsigned first[] = { 3, 8, 19, 84 };
				unsigned second[] = { 2, 5, 12, 53 };
				bool unableToAdjust{ true };
				for (unsigned i = 0; i < 4; ++i) {
					unsigned adjusted_a = a - first[i];
					unsigned adjusted_b = b - second[i];
					if (adjusted_a <= MAX_A && adjusted_b <= MAX_B) {
						setexponent(0, adjusted_a);
						setexponent(1, adjusted_b);
						unableToAdjust = false;
					}
				}
				if (unableToAdjust){
					if (a > b) {
						setexponent(0, MAX_A);
						setexponent(1, 0);
						negative = false; // we need to avoid nan(ind)
					}
					else {
						setexponent(0, 0);
						setexponent(1, MAX_B);
					}
				}
			}
			else {
				setexponent(0, a);
				setexponent(1, b);
			}
		}
		else {
			static_assert(true, "multi-limb TBD");
		}
		setsign(negative);
		if (isnan()) setzero(); // if the arithmetic ends up in the nan encoding, set the value to zero
#else
		// marshall through double value
		* this = double(*this) * double(rhs);
#endif
		return *this;
	}
	dbns& operator*=(double rhs) { return operator*=(dbns(rhs)); }
	dbns& operator/=(const dbns& rhs) {
		if (isnan()) return *this;
		if (rhs.isnan()) {
			setnan();
			return *this;
		}
		if (rhs.iszero()) {
#if DBNS_THROW_ARITHMETIC_EXCEPTION
			throw dbns_divide_by_zero();
#else
			setnan();
			return *this;
#endif
		}
		if (iszero()) return *this;

#if defined(NATIVE_DBNS_ARITHMETIC)
		// this simple code doesn't work because of modulo underflow when the right hand side exponent is bigger than the left hand side
		bool negative = sign() ^ rhs.sign(); // determine sign of result
		uint32_t e0 = extractExponent(0) - rhs.extractExponent(0);
		uint32_t e1 = extractExponent(1) - rhs.extractExponent(1);
		if constexpr (behavior == Behavior::Saturating) { // saturating, no infinite
			clear();
			setexponent(0, e0);
			setexponent(1, e1);
		}
		else {
			static_assert(true, "multi-limb TBD");
		}
		setsign(negative);
#else
		// marshall through double value
		* this = double(*this) / double(rhs);
#endif
		return *this;
	}
	dbns& operator/=(double rhs) { return operator/=(dbns(rhs)); }

	// prefix/postfix operators
	dbns& operator++() {
		if constexpr (1 == nrBlocks) {
			++_block[0];
		}
		else {
			if (_block[0] == storageMask) {
				_block[0] = 0;
				for (unsigned i = 1; i < nrBlocks; ++i) {
					if (_block[i] < storageMask) {
						++_block[i];
						break;
					}
				}
			}
			else {
				++_block[0];
			}
		}
		return *this;
	}
	dbns operator++(int) {
		dbns tmp(*this);
		operator++();
		return tmp;
	}
	dbns& operator--() {
		if constexpr (1 == nrBlocks) {
			--_block[0];
		}
		else {
			if (_block[0] == 0) {
				_block[0] = storageMask;
				for (unsigned i = 1; i < nrBlocks; ++i) {
					if (_block[i] > 0) { // execute the borrow
						--_block[i];
						break;
					}
					else {
						_block[i] = storageMask;
					}
				}
			}
			else {
				--_block[0];
			}
		}
		return *this;
	}
	dbns operator--(int) {
		dbns tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	// clear resets all bits
	constexpr void clear()                         noexcept {
		if constexpr (1 == nrBlocks) {
			_block[0] = 0;
		}
		else if constexpr (2 == nrBlocks) {
			_block[0] = 0;
			_block[1] = 0;
		}
		else {
			for (unsigned i = 0; i < nrBlocks; ++i) _block[i] = 0;
		}
	}
	constexpr void setzero()                       noexcept { zero(); }
	constexpr void setnan(bool sign = true)        noexcept { zero(); setbit(nbits - 1, sign); } // to be consistent with IEEE-754 to have either quiet or signalling NaNs
	constexpr void setinf(bool sign = false)       noexcept { (sign ? maxneg() : maxpos()); } // TODO: is that what we want?
	constexpr void setsign(bool s = true)          noexcept { setbit(nbits - 1, s); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (i < nbits) {
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[blockIndex] = bt((block & null) | mask);
		}
		// nop if i is out of range
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
	constexpr void setexponent(int base, uint32_t exponentBits) noexcept {
		if constexpr (1 == nrBlocks) {
			if (base == 0) {
				_block[MSU] &= ~FB_MASK;
				exponentBits &= MAX_A; // lob off any bits outside the field width
				exponentBits <<= sbbits; // shift them in place
				_block[MSU] |= (exponentBits & FB_MASK); 
			}
			else if (base == 1) {
				_block[MSU] &= ~SB_MASK;
				_block[MSU] |= (exponentBits & SB_MASK);
			}
		}
		else {
			uint32_t mask = 0x1;
			if (0 == base) {
				for (unsigned i = sbbits; i < nbits - 1; ++i) {
					setbit(i, (mask & exponentBits) != 0);
					mask <<= 1;
				}
			}
			else {
				for (unsigned i = 0; i < sbbits; ++i) {
					setbit(i, (mask & exponentBits) != 0);
					mask <<= 1;
				}
			}
		}
	}
	// create specific number system values of interest
	constexpr dbns& maxpos() noexcept {
		// maximum positive value has this bit pattern: 0-00..00-11...11, that is, sign = 0, first base = 00..00, second base = 11..11
		clear();
		for (unsigned i = 0; i < sbbits; ++i) {
			setbit(i, true);
		}
		return *this;
	}
	constexpr dbns& minpos() noexcept {
		// minimum positive value has this bit pattern: 0-11...11-00...00, that is, sign = 0, first base = 11..10, second base = 00..00
		clear();
		flip();
		setbit(nbits - 1, false);
		for (unsigned i = 0; i < sbbits; ++i) {
			setbit(i, false);
		}
		return *this;
	}
	constexpr dbns& zero() noexcept {
		// the zero value has this bit pattern: 0-11..11-00..000, sign = 0, fbbits all 1, rest 0
		clear();
		if constexpr (1 == nrBlocks) {
			setbits(FB_MASK);
		}
		else {
			for (unsigned i = sbbits; i < nbits - 1; ++i) {
				setbit(i, true);
			}
		}
		return *this;
	}
	constexpr dbns& minneg() noexcept {
		// minimum negative value has this bit pattern: 1-11...10-00...00, that is, sign = 0, first base = 11..10, second base = 00..00
		minpos();
		setbit(nbits - 1ull, true);
		return *this;
	}
	constexpr dbns& maxneg() noexcept {
		// maximum negative value has this bit pattern: 1-00..00-11...11, that is, sign = 0, first base = 00..00, second base = 11..11
		maxpos();
		setbit(nbits - 1ull, true); // sign = 1
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept { // special encoding: 0.11..11.0000
		if constexpr (1 == nrBlocks) {
			if (!at(nbits - 1) && ((_block[MSU] & FB_MASK) == FB_MASK) && ((_block[MSU] & SB_MASK) == 0)) return true; else return false;
		}
		else {
			for (unsigned i = 0; i < sbbits; ++i) {
				if (at(i)) return false;
			}
			for (unsigned i = sbbits; i < nbits - 1; ++i) {
				if (!at(i)) return false;
			}
			// zero is sign bit is off, nan is sign bit is on
			return !at(nbits - 1);
		}
	}
	constexpr bool isneg()  const noexcept { return sign(); }
	constexpr bool ispos()  const noexcept { return !sign(); }
	constexpr bool isinf()  const noexcept { return false; }  // TODO: no inf in dbns: shall we set it to maxpos?
	constexpr bool isnan()  const noexcept { // special encoding
		// 1.1111.0000 is NaN
		for (unsigned i = 0; i < sbbits; ++i) {
			if (at(i)) return false;
		}
		for (unsigned i = sbbits; i < nbits - 1; ++i) {
			if (!at(i)) return false;
		}
		// zero is sign bit is off, nan is sign bit is on
		return at(nbits - 1);
	}
	constexpr bool sign()   const noexcept { 
		return (SIGN_BIT_MASK & _block[MSU]) != 0;
	}
	constexpr int  scale()  const noexcept {
		using std::log2;
		// Scale needs to work for all potential bases
		// we shouldn't go through double conversion as doubles do not
		// have enough dynamic range for dbns configs, so
		// we should go through the exponent calculation directly
		uint32_t e0 = extractExponent(0);
		uint32_t e1 = extractExponent(1);
		return static_cast<int>(e0 + e1 * log2of3);
	}
	// fraction returns 0
	constexpr uint64_t fraction() const noexcept { return 0; }
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	constexpr bt   block(unsigned b) const noexcept {
		if (b < nrBlocks) return _block[b];
		return bt(0); // return 0 when block index out of bounds
	}
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
	constexpr uint32_t extractExponent(int base) const noexcept { // we return a 32bit exponent
		if constexpr (1 == nrBlocks) {
			uint64_t bits = static_cast<uint64_t>(_block[MSU]);
			if (base == 0) {
				bits &= FB_MASK;
				bits >>= sbbits; // normalize the value
			}
			else if (base == 1) {
				bits &= SB_MASK; // value is already normalized
			}
			return static_cast<uint32_t>(bits);
		}
		else {
			uint64_t bits{ 0 };
			if (0 == base) {
				for (unsigned i = sbbits; i < nbits - 1; ++i) {
					bits |= (at(i) ? (1 << (i - sbbits)) : 0);
				}
			}
			else {
				for (unsigned i = 0; i < sbbits; ++i) {
					bits |= (at(i) ? (1 << i) : 0);
				}
			}
			return static_cast<uint32_t>(bits);
		}
	}
	explicit operator int()       const noexcept { return to_signed<int>(); }
	explicit operator long()      const noexcept { return to_signed<long>(); }
	explicit operator long long() const noexcept { return to_signed<long long>(); }
	explicit operator float()     const noexcept { return to_ieee754<float>(); }
	explicit operator double()    const noexcept { return to_ieee754<double>(); }
	
	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	explicit operator long double()                 const noexcept { return to_ieee754<long double>(); }
	dbns(long double initial_value)                        noexcept { *this = initial_value; }
	dbns& operator=(long double rhs)       noexcept { return convert_ieee754(rhs); }
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
		std::cout << "FB_MASK               " << to_binary(FB_MASK, bitsInBlock) << '\n';
		std::cout << "SB_MASK               " << to_binary(SB_MASK, bitsInBlock) << '\n';
	}

protected:

	/// <summary>
	/// 1's complement of the encoding. Used internally to create specific bit patterns
	/// </summary>
	/// <returns>reference to this cfloat object</returns>
	constexpr dbns& flip() noexcept { // in-place one's complement
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}
		_block[MSU] = (_block[MSU] & MSU_MASK); // assert precondition of properly nulled leading non-bits
		return *this;
	}

	/// <summary>
	/// assign the value of the string representation to the cfloat
	/// </summary>
	/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
	/// <returns>reference to this cfloat</returns>
	/// Clang doesn't support constexpr yet on string manipulations, so we need to make it conditional
	CONSTEXPRESSION dbns& assign(const std::string& str) noexcept {
		clear();
		return *this;
	}

	//////////////////////////////////////////////////////
	/// convertion routines from native types

	template<typename SignedInt>
	CONSTEXPRESSION dbns& convert_signed(SignedInt v) noexcept {
		return convert_ieee754(double(v));
	}
	template<typename UnsignedInt>
	CONSTEXPRESSION dbns& convert_unsigned(UnsignedInt v) noexcept {
		return convert_ieee754(double(v));
	}
	template<typename Real>
	CONSTEXPRESSION dbns& convert_ieee754(Real v) noexcept {
		if constexpr (bCollectDbnsEventStatistics) ++dbnsStats.conversionEvents;
		using std::abs;
		using std::log2;
		using std::pow;
		using std::round;
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

		// it is too expensive to check if the value is in the representable range
		// the search below will end up at 0 or maxpos

		// we search for the a and b in v = 2^a * 3^b, with both a and b positive
		// in our representation we have 0.5^a * 3^b, which would be equivalent
		// to a being negative
		// 
		// v = 2^a * 3^b =>
		// v = 2^(a + b*log2of3) =>
		// scale of v = (a + b*log2of3)
		// we use this relationship to search among the second base exponents 
		// and find a first base exponent that minimizes the error
		// between the result and the value we are trying to approximate.
		constexpr bool bDebug = false;
		double scale = log2(abs(v)); 
		if constexpr (bDebug) std::cout << "scale : " << scale << '\n';
		double lowestError = 1.0e10;
		constexpr int kNotFound = std::numeric_limits<int>::max();
		int best_a = kNotFound;
		int best_b = kNotFound;
		for (int b = 0; b <= static_cast<int>(SB_MASK); ++b) {
			int a = static_cast<int>(round((scale - b * log2of3))); // find the first base exponent that is closest to the value
			if (a > 0 || a > static_cast<int>(MAX_A)) {
				if constexpr (bCollectDbnsEventStatistics) ++dbnsStats.exponentOverflowDuringSearch;
				continue;
			}
			double err = abs(scale - (a + b * log2of3));
			if constexpr (bDebug) {
				double fb = pow(2.0, a);
				double sb = pow(3.0, b);
				double value = fb * sb;
				std::cout << "a : " << a << " b : " << b << " err : " << err << " fb : " << fb << " sb : " << sb << " value : " << value << '\n';
			}
			if (err < lowestError) {
				lowestError = err;
				best_a = a;
				best_b = b;
			}
		}
		if constexpr (bDebug) std::cout << "best a : " << best_a << " best b : " << best_b << " lowest err : " << lowestError << '\n';
		clear();

		// If the search produced no candidate, avoid using sentinel values in the
		// adjustment logic below. Fall back to the existing saturating behavior.
		if (best_a == kNotFound || best_b == kNotFound) {
			if constexpr (bCollectDbnsEventStatistics) ++dbnsStats.roundingFailure;
			setexponent(0, 0);
			setexponent(1, MAX_B);
			setsign(s);
			// avoid assigning to nan(ind)
			if (isnan()) setzero();
			return *this;
		}

		assert(best_b >= 0); // second exponent is negative
		int a = -best_a;
		int b = best_b;
		if (a < 0 || a > static_cast<int>(MAX_A) || b > static_cast<int>(MAX_B)) {
			// try to project the value back into valid pairs
			// the approximations of unity looks like (8,-5), (19,-12), (84,-53),... 
			// they grow too fast and in a rather irregular manner. There are more 
			// subtle number theoretic considerations, but the ones outlined above 
			// should be sufficient to figure out a good solution to the problem.
			// 2^3*3^-2 = 0.888  2^-3*3^2 = 1.125
			// 2^8*3^-5 = 1.053  2^-8*3^5 = 0.949
			// multiplier   0.5, 1.5, 0.6, 0.889, 1.125, 0.949, 1.053.....
			int first[]  = { 1, 1, -1, 3, -3, 5, -5, 8, -8, 19, -19, 84, -84 };
			int second[] = { 0, 1, -1, 2, -2, 3, -3, 5, -5, 12, -12, 53, -53 };
			bool unableToAdjust{ true };
			for (unsigned i = 0; i < 13; ++i) {
				int adjusted_a = a - first[i];
				int adjusted_b = b - second[i];
				if (adjusted_a >= 0 && adjusted_a < static_cast<int>(MAX_A) && adjusted_b >= 0 && adjusted_b < static_cast<int>(MAX_B)) {
					setexponent(0, static_cast<unsigned>(adjusted_a));
					setexponent(1, static_cast<unsigned>(adjusted_b));
					setsign(s);
					unableToAdjust = false;
					break;
				}
			}
			if (unableToAdjust) {
				if constexpr (bCollectDbnsEventStatistics) ++dbnsStats.roundingFailure;
				//if (a > b) {
				if (best_a < 0 && best_b >= 0) {
					setexponent(0, MAX_A);
					setexponent(1, 0);
					setsign(false); // we need to avoid nan(ind)
				}
				else {   // we have maxed out
					setexponent(0, 0);
					setexponent(1, MAX_B);
					setsign(s);
				}
			}
		}
		else {
			a <<= sbbits;
			_block[MSU] = static_cast<bt>(static_cast<bt>(s ? SIGN_BIT_MASK : 0u) | static_cast<bt>(a) | static_cast<bt>(b));
		}
		// avoid assigning to nan(ind)
		if (isnan()) setzero();
		return *this;
	}

	//////////////////////////////////////////////////////
	/// convertion routines to native types

	template<typename Real>
	Real ipow(Real base, uint64_t exp) const noexcept {
		Real result(1.0f);
		for (;;) {
			if (exp & 0x1) result *= base;
			exp >>= 1;
			if (exp == 0) break;
			base *= base;
		}
		return result;
	}

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
		int signValue = (sign() ? -1 : 1); // cache for later decision
		// pick up the absolute value of the minimum normal and subnormal exponents 
		//constexpr unsigned minNormalExponent = static_cast<unsigned>(-ieee754_parameter<TargetFloat > ::minNormalExp);
		constexpr unsigned minSubnormalExponent = static_cast<unsigned>(-ieee754_parameter<TargetFloat>::minSubnormalExp);
		static_assert(fbbits <= minSubnormalExponent, "dbns::to_ieee754: fraction is too small to represent with requested floating-point type");

		TargetFloat dim1, dim2;
		dim1 = ipow(TargetFloat(base0), extractExponent(0));
		dim2 = ipow(TargetFloat(base1), extractExponent(1));
		return static_cast<TargetFloat>(signValue) * dim1 * dim2;
	}

private:
	BlockType _block[nrBlocks];

	////////////////////// operators

	// stream operators

	friend std::ostream& operator<< (std::ostream& ostr, const dbns& r) {
		ostr << double(r);
		return ostr;
	}
	friend std::istream& operator>> (std::istream& istr, dbns& r) {
		double d;
		istr >> d;
		r = d;
		return istr;
	}

	// dbns - logic operators

	friend constexpr bool operator==(const dbns& lhs, const dbns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		if constexpr (1 == nrBlocks) {
			return lhs._block[0] == rhs._block[0];
		}
		else if constexpr (2 == nrBlocks) {
			return lhs._block[0] == rhs._block[0] && lhs._block[1] == rhs._block[1];
		}
		else {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				if (lhs._block[i] != rhs._block[i]) return false;
			}
			return true;
		}
	}
	friend constexpr bool operator< (const dbns& lhs, const dbns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		blockbinary<nbits-1, bt, BinaryNumberType::Signed> l(lhs._block), r(rhs._block); // extract the 2's complement exponent
		bool lhs_is_negative = lhs.sign();
		return (lhs_is_negative != rhs.sign()) ? lhs_is_negative
		                                       : lhs_is_negative ? l > r : l < r;
	}

	friend constexpr bool operator!=(const dbns& lhs, const dbns& rhs) {
		return !operator==(lhs, rhs);
	}
	friend constexpr bool operator> (const dbns& lhs, const dbns& rhs) {
		return  operator< (rhs, lhs);
	}
	friend constexpr bool operator<=(const dbns& lhs, const dbns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		return !operator> (lhs, rhs);
	}
	friend constexpr bool operator>=(const dbns& lhs, const dbns& rhs) {
		if (lhs.isnan() || rhs.isnan()) return false;
		return !operator< (lhs, rhs);
	}
	// dbns - literal logic operators

	friend constexpr bool operator==(const dbns& lhs, double rhs) { return lhs == dbns(rhs); }
	friend constexpr bool operator!=(const dbns& lhs, double rhs) { return !operator==(lhs, rhs); }
	friend constexpr bool operator< (const dbns& lhs, double rhs) { return lhs < dbns(rhs); }
	friend constexpr bool operator> (const dbns& lhs, double rhs) { return  operator< (rhs, lhs); }
	friend constexpr bool operator<=(const dbns& lhs, double rhs) { return !operator> (lhs, rhs); }
	friend constexpr bool operator>=(const dbns& lhs, double rhs) { return !operator< (lhs, rhs); }

	// dbns - dbns binary arithmetic operators

	friend constexpr dbns operator+(const dbns& lhs, const dbns& rhs) {
		dbns sum(lhs);
		sum += rhs;
		return sum;
	}
	friend constexpr dbns operator-(const dbns& lhs, const dbns& rhs) {
		dbns diff(lhs);
		diff -= rhs;
		return diff;
	}
	friend constexpr dbns operator*(const dbns& lhs, const dbns& rhs) {
		dbns mul(lhs);
		mul *= rhs;
		return mul;
	}
	friend constexpr dbns operator/(const dbns& lhs, const dbns& rhs) {
		dbns ratio(lhs);
		ratio /= rhs;
		return ratio;
	}

	// dbns - literal binary arithmetic operators

	friend constexpr dbns operator+(const dbns& lhs, double rhs) {
		dbns sum(lhs);
		sum += rhs;
	}
	friend constexpr dbns operator-(const dbns& lhs, double rhs) {
		dbns diff(lhs);
		diff -= rhs;
		return diff;
	}
	friend constexpr dbns operator*(const dbns& lhs, double rhs) {
		dbns mul(lhs);
		mul *= rhs;
		return mul;
	}
	friend constexpr dbns operator/(const dbns& lhs, double rhs) {
		dbns ratio(lhs);
		ratio /= rhs;
		return ratio;
	}

	// literal - dbns binary arithmetic operators

	friend constexpr dbns operator+(double lhs, const dbns& rhs) {
		dbns sum(lhs);
		sum += rhs;
		return sum;
	}
	friend constexpr dbns operator-(double lhs, const dbns& rhs) {
		dbns diff(lhs);
		diff -= rhs;
		return diff;
	}
	friend constexpr dbns operator*(double lhs, const dbns& rhs) {
		dbns mul(lhs);
		mul *= rhs;
		return mul;
	}
	friend constexpr dbns operator/(double lhs, const dbns& rhs) {
		dbns ratio(lhs);
		ratio /= rhs;
		return ratio;
	}
};

// return the Unit in the Last Position
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
inline dbns<nbits, fbbits, bt, xtra...> ulp(const dbns<nbits, fbbits, bt, xtra...>& a) {
	dbns<nbits, fbbits, bt, xtra...> b(a);
	return ++b - a;
}

template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
std::string to_binary(const dbns<nbits, fbbits, bt, xtra...>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	s << (number.sign() ? "1." : "0.");
	// first base exponent bits
	constexpr int lsbFirstBase = static_cast<int>(nbits - fbbits - 1);
	if constexpr (nbits - 2 >= fbbits) {
		for (int i = static_cast<int>(nbits) - 2; i >= lsbFirstBase; --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if ((i - fbbits) > 0 && ((i - fbbits) % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	if constexpr (lsbFirstBase > 0) {
		s << '.';
		for (int i = lsbFirstBase - 1; i >= 0; --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	return s.str();
}

// standard library functions for floating point

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
constexpr dbns<nbits, fbbits, bt, xtra...> abs(const dbns<nbits, fbbits, bt, xtra...>& v) {
	dbns<nbits, fbbits, bt, xtra...> magnitude(v);
	magnitude.setsign(false);
	return magnitude;
}
// ToDo constexpt frexp
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
dbns<nbits, fbbits, bt, xtra...> frexp(const dbns<nbits, fbbits, bt, xtra...>& x, int* exp) {
	return dbns<nbits, fbbits, bt, xtra...>(std::frexp(double(x), exp));
}
// ToDo constexpr ldexp
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
dbns<nbits, fbbits, bt, xtra...> ldexp(const dbns<nbits, fbbits, bt, xtra...>& x, int exp) {
		return dbns<nbits, fbbits, bt, xtra...>(std::ldexp(double(x), exp));
}

}} // namespace sw::universal
