#pragma once

// Behavioural switches default HERE, beside the code they govern, rather than in the
// dbns.hpp umbrella: including core.hpp directly would otherwise leave them undefined,
// #if would evaluate them as 0, and the switch would silently flip on that path -- the
// trap #1390 hit with POSIT_ENABLE_LITERALS (#1334, #1436).
#if !defined(DBNS_ENABLE_LITERALS)
#define DBNS_ENABLE_LITERALS 1
#endif
#if !defined(DBNS_THROW_ARITHMETIC_EXCEPTION)
#define DBNS_THROW_ARITHMETIC_EXCEPTION 0
#endif
// the fused dot product accumulator (quire, via fdp.hpp) must honor the same
// exception policy as the dbns it accumulates for (#1226)
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
#define QUIRE_THROW_ARITHMETIC_EXCEPTION DBNS_THROW_ARITHMETIC_EXCEPTION
#endif
#include <universal/utility/exceptions_check.hpp>  // reject a throw switch without exceptions (#1925)

// DBNS_TRACE_CONVERSION: the trace statements in convert_ieee754's (a,b) search print,
// so they need <iostream>. They are compiled only when tracing is actually switched on
// -- a preprocessor guard rather than `if constexpr`, because a discarded `if constexpr`
// branch still requires std::cout to be DECLARED, which would keep <iostream> in the
// include graph of every translation unit. Same treatment cfloat got in #1417.
//
// This replaces a local `constexpr bool bDebug = false;` that no caller could reach:
// tracing could not be enabled from outside at all, which is the same defect #1387 found
// in the block layer. Define DBNS_TRACE_CONVERSION=1 before including dbns to switch it
// on.
#if !defined(DBNS_TRACE_CONVERSION)
#define DBNS_TRACE_CONVERSION 0
#endif
#if DBNS_TRACE_CONVERSION
#include <iostream>
#endif

#include <iosfwd>     // std::ostream/std::istream in the stream-operator declarations
#include <universal/utility/icf_array_bounds.hpp>
#include <string>
// dbns_impl.hpp: implementation of a fixed-size, arbitrary configuration 2-base logarithmic number system configuration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <cmath>      // std::exp2, std::log2
#include <limits>
#include <type_traits>

#include <universal/native/ieee754_core.hpp>   // the stream-free half of <universal/native/ieee754.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/internal/abstract/triple.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/behavior/arithmetic.hpp>
#include <universal/number/dbns/dbns_fwd.hpp>
#include <math/constexpr_math.hpp>

#include <universal/internal/bit_manipulation.hpp>

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

	// defined out-of-line in number/dbns/iostream.hpp so this header needs no
	// <iostream> (#1334). Include that header to stream the statistics.
	std::ostream& operator<<(std::ostream& ostr, const DbnsArithmeticStatistics stats);
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
	// the exponent fields are carried in 32 bits by setexponent() and extractExponent(), and
	// conversion searches them in 64-bit arithmetic (#1933)
	static_assert(_fbbits <= 32, "configuration not supported: the first base exponent field is wider than 32 bits");
	static_assert(_nbits - _fbbits - 1 <= 32, "configuration not supported: the second base exponent field is wider than 32 bits");
	// conversion searches min(2^sbbits, 2^fbbits / log2(3)) exponent pairs: one field of at most
	// 20 bits keeps that below about a million, where two 32-bit fields would be 2.7 billion
	static_assert(_fbbits <= 20 || _nbits - _fbbits - 1 <= 20, "configuration not supported: with both exponent fields wider than 20 bits a conversion would search billions of exponent pairs");
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
	CONSTEXPRESSION dbns& operator+=(const dbns& rhs) {
		double sum{ 0.0 };
		if constexpr (behavior == Behavior::Saturating) {
			sum = double(*this) + double(rhs);  // TODO: native implementation
		}
		else {
			sum = double(*this) + double(rhs);  // TODO: native implementation
		}
		return *this = sum; // <-- saturation happens in the assignment
	}
	CONSTEXPRESSION dbns& operator+=(double rhs) {
		return operator+=(dbns(rhs));
	}
	CONSTEXPRESSION dbns& operator-=(const dbns& rhs) {
		double diff{ 0.0 };
		if constexpr (behavior == Behavior::Saturating) {
			diff = double(*this) - double(rhs);  // TODO: native implementation
		}
		else {
			diff = double(*this) - double(rhs);  // TODO: native implementation
		}
		return *this = diff; // <-- saturation happens in the assignment
	}
	CONSTEXPRESSION dbns& operator-=(double rhs) {
		return operator-=(dbns(rhs));
	}
	CONSTEXPRESSION dbns& operator*=(const dbns& rhs) {
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
		// The exact product is (-1)^(s1^s2) * 0.5^(a1+a2) * 3^(b1+b2). It is stored as is when that
		// pair is an encoding, and rounded to the nearest one otherwise, by the same rule as a
		// conversion (#1940). This used to project an out-of-range pair back with approximations
		// of unity, which changed the value by up to 11%.
		assign_pair(static_cast<long long>(extractExponent(0)) + static_cast<long long>(rhs.extractExponent(0)),
		            static_cast<long long>(extractExponent(1)) + static_cast<long long>(rhs.extractExponent(1)),
		            sign() != rhs.sign());
#else
		// marshall through double value
		* this = double(*this) * double(rhs);
#endif
		return *this;
	}
	CONSTEXPRESSION dbns& operator*=(double rhs) { return operator*=(dbns(rhs)); }
	CONSTEXPRESSION dbns& operator/=(const dbns& rhs) {
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
		// The exact quotient is (-1)^(s1^s2) * 0.5^(a1-a2) * 3^(b1-b2), in signed arithmetic: a
		// divisor with the larger exponents gives a negative one, which wrapped in uint32_t and
		// has no encoding anyway, so such a quotient rounds to the nearest encoding (#1940).
		assign_pair(static_cast<long long>(extractExponent(0)) - static_cast<long long>(rhs.extractExponent(0)),
		            static_cast<long long>(extractExponent(1)) - static_cast<long long>(rhs.extractExponent(1)),
		            sign() != rhs.sign());
#else
		// marshall through double value
		* this = double(*this) / double(rhs);
#endif
		return *this;
	}
	CONSTEXPRESSION dbns& operator/=(double rhs) { return operator/=(dbns(rhs)); }

	// prefix/postfix operators
	constexpr dbns& operator++() {
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
	constexpr dbns operator++(int) {
		dbns tmp(*this);
		operator++();
		return tmp;
	}
	constexpr dbns& operator--() {
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
	constexpr dbns operator--(int) {
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
			bt null = bit_clear_mask<bt>(i, bitsInBlock);
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
				// in 64 bits: a single uint64_t block holds fields wider than 32 bits together, and
				// shifting a uint32_t by sbbits lost the top of the field, or was undefined at a
				// 32-bit second field (CodeRabbit on PR #1939)
				uint64_t field = static_cast<uint64_t>(exponentBits) & MAX_A;   // lob off any bits outside the field width
				field <<= sbbits;                                                  // shift them in place
				_block[MSU] |= static_cast<bt>(field & FB_MASK);
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
		// minimum positive value has this bit pattern: 0-11...10-00...00, that is, sign = 0,
		// first base = 11..10, second base = 00..00: 0.5^(MAX_A - 1). 0-11...11-00...00 is the
		// zero encoding, which this used to produce, so minpos was 0 and minneg NaN (#1933).
		clear();
		flip();
		setbit(nbits - 1, false);
		for (unsigned i = 0; i < sbbits; ++i) {
			setbit(i, false);
		}
		setbit(sbbits, false);
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
	// The binary scale of the value: floor(log2(|v|)).
	//
	// A dbns encodes (-1)^s * base0^e0 * base1^e1 with base0 = 0.5 and base1 = 3, so its
	// log2 is -e0 + e1*log2(3). This returned +e0 + e1*log2(3): the base-0.5 exponent
	// contributes NEGATIVELY and the sign was dropped, so every value with a non-zero e0
	// reported the wrong scale -- 2 for 1.5, whose scale is 0, and +1 for 0.5, whose
	// scale is -1 (#1582).
	//
	// operator< below already had it right: it documents the value as (-1)^s * 0.5^a * 3^b
	// and orders on M(x) = -a + b*log2of3. convert_ieee754 says the same thing in as many
	// words -- "in our representation we have 0.5^a * 3^b, which would be equivalent to a
	// being negative" -- and only accepts a <= 0, storing its magnitude. This function was
	// the one place that disagreed.
	//
	// The exponents are combined in the log domain rather than by converting to double,
	// because a dbns configuration can range far outside what a double can hold.
	//
	// int64_t, not int: the second-base exponent field can be 32 bits wide, so
	// dbns<34,1,uint64_t> reaches e1 = 4294967295 and a scale near 6.81e9. That overflows
	// an int, and a float-to-int conversion out of range is undefined behaviour -- it
	// returned -2147483648 in practice. Both exponents come back from extractExponent as
	// uint32_t, so the result is bounded by about +/-6.81e9 and int64_t covers the whole
	// range with room to spare. takum and efloat already return int64_t from scale() for
	// the same reason.
	constexpr int64_t scale() const noexcept {
		if (iszero() || isnan()) return 0;   // no binary scale to report
		const double e0 = static_cast<double>(extractExponent(0));
		const double e1 = static_cast<double>(extractExponent(1));
		const double log2v = -e0 + e1 * log2of3;
		// floor, not truncation: a static_cast rounds toward zero, which is wrong for
		// every negative scale -- it reported 0 for values in (0.5, 1). std::floor is not
		// constexpr before C++23, so the adjustment is done by hand to keep scale()
		// usable in a constant expression.
		int64_t s = static_cast<int64_t>(log2v);
		if (log2v < 0.0 && static_cast<double>(s) != log2v) --s;
		return s;
	}
	// A dbns has no fraction field: the value is entirely carried by the two base
	// exponents. This accessor exists so generic code that probes for .fraction() on an
	// arbitrary number type still compiles, and always answers 0. It is deliberately NOT
	// reported by components() or to_triple(), which used to print this constant as
	// though it meant something (#1582).
	constexpr uint64_t fraction() const noexcept { return 0; }
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		// in bounds: bitIndex < nbits => index <= nrBlocks-1. The pragma silences a
		// GCC -fipa-icf false positive; see utility/icf_array_bounds.hpp.
		UNIVERSAL_ICF_ARRAY_BOUNDS_PUSH
		bt word = _block[bitIndex / bitsInBlock];
		UNIVERSAL_ICF_ARRAY_BOUNDS_POP
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
					bits |= (at(i) ? (1ull << (i - sbbits)) : 0ull);
				}
			}
			else {
				for (unsigned i = 0; i < sbbits; ++i) {
					bits |= (at(i) ? (1ull << i) : 0ull);
				}
			}
			return static_cast<uint32_t>(bits);
		}
	}
	explicit CONSTEXPRESSION operator int()       const noexcept { return to_signed<int>(); }
	explicit CONSTEXPRESSION operator long()      const noexcept { return to_signed<long>(); }
	explicit CONSTEXPRESSION operator long long() const noexcept { return to_signed<long long>(); }
	explicit CONSTEXPRESSION operator float()     const noexcept { return to_ieee754<float>(); }
	explicit CONSTEXPRESSION operator double()    const noexcept { return to_ieee754<double>(); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	explicit CONSTEXPRESSION operator long double() const noexcept { return to_ieee754<long double>(); }
	CONSTEXPRESSION dbns(long double initial_value)       noexcept { *this = initial_value; }
	CONSTEXPRESSION dbns& operator=(long double rhs)      noexcept { return convert_ieee754(rhs); }
#endif

	// defined out-of-line in number/dbns/debug.hpp so this header needs no <iostream>
	// (#1334). Include that header to call it -- it is an opt-in debug facility, not
	// something the arithmetic reaches on its own, so the failure mode of forgetting it
	// is a link error naming the missing function. Same shape as blocktriple's
	// constexprClassParameters() in #1388.
	void debugConstexprParameters();

	// normalize: decompose dbns value into a blocktriple<fbbits, REP> for quire accumulation.
	// DBNS stores values as (-1)^sign * 2^a * 3^b; materializing to linear domain is inherently
	// approximate, so the double intermediary is acceptable here.
	// Guard: MAX_B * log2(3) must fit in binary64 exponent range (1023) to avoid frexp(inf).
	template<typename TargetBlockType = bt>
	void normalize(blocktriple<fbbits, BlockTripleOperator::REP, TargetBlockType>& tgt) const {
		// log2(3) ~= 1.585; use rational approximation 1585/1000 for compile-time check
		static_assert(MAX_B * 1585 / 1000 + MAX_A <= 1023,
			"dbns configuration exceeds binary64 range: double(*this) would overflow");
		if (iszero()) { tgt.setzero(); return; }
		if (isnan())  { tgt.setnan();  return; }
		double v = double(*this);
		tgt.setnormal();
		tgt.setsign(v < 0);
		int e;
		double frac = std::frexp(std::abs(v), &e);
		tgt.setscale(e - 1);
		// frac is in [0.5, 1.0); shift to get fbbits+1 bits of significand
		uint64_t sig = static_cast<uint64_t>(std::ldexp(frac, static_cast<int>(fbbits) + 1));
		tgt.setbits(sig);
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
	/// string assignment is not implemented yet: the dbns is cleared to zero and the string is ignored
	/// </summary>
	/// <param name="str">ignored</param>
	/// <returns>reference to this dbns, now zero</returns>
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
		if (!std::is_constant_evaluated()) {
			if constexpr (bCollectDbnsEventStatistics) ++dbnsStats.conversionEvents;
		}
		bool s{ false };
		uint64_t unbiasedExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(v, s, unbiasedExponent, rawFraction, bits);
		if (unbiasedExponent == ieee754_parameter<Real>::eallset) { // nan and inf need to be remapped
			// IEEE-754: exponent all ones with a ZERO fraction is infinity, and with ANY
			// non-zero fraction is a NaN, whatever the payload.  This used to compare the
			// fraction for equality against three specific payloads, so every other one --
			// including the canonical signalling 0x1 -- missed all three and fell through to
			// the numeric path below, which turned a NaN into a finite value or an infinity.
			// Same defect as cfloat's, same origin; issue #1303.
			if (rawFraction == 0ull) {
				setinf(s);
				return *this;
			}
			// dbns has a single NaN encoding, so the quiet / signalling distinction the
			// source carries in its fraction MSB has nowhere to go and is not preserved.
			setnan();
			return *this;
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
		// Use sw::math::constexpr_math::log2 (not std::log2 which isn't
		// constexpr until C++26) so this whole conversion is evaluable at
		// compile time. cm only has float/double overloads; for long double
		// route through double at compile time (constexpr literal long-double
		// inputs are bounded to values the literal grammar can represent), and
		// use std::log2 at runtime to preserve the long-double exponent range.
		Real abs_v = (v < Real(0) ? Real(-v) : v);
		double scale;
		if constexpr (std::is_same_v<Real, float> || std::is_same_v<Real, double>) {
			scale = static_cast<double>(sw::math::constexpr_math::log2(abs_v));
		} else {
			if (std::is_constant_evaluated()) {
				scale = static_cast<double>(sw::math::constexpr_math::log2(static_cast<double>(abs_v)));
			} else {
				scale = static_cast<double>(std::log2(abs_v));
			}
		}
#if DBNS_TRACE_CONVERSION
		// the !is_constant_evaluated() guard keeps convert_ieee754 usable in a constexpr
		// context with tracing on -- std::cout is not constant-evaluable. Same idiom the
		// statistics counters below already use.
		if (!std::is_constant_evaluated()) std::cout << "scale : " << scale << '\n';
#endif
		return assign_log2(scale, s);
	}

	// The encoding nearest 2^scale, with the given sign, in the logarithmic domain: the one rounding
	// rule that conversion from a native type and the native multiply and divide share (#1940).
	// scale is log2 of the magnitude; below half of minpos the result is zero, and beyond maxpos it
	// saturates to maxpos.
	constexpr dbns& assign_log2(double scale, bool negative) noexcept {
		// The search walks the second base exponents b and, for each, takes the first base
		// exponent nearest scale - b*log2(3), clamped to the encodable range: the encoding
		// stores 0.5^a with a in [0, MAX_A], so the base-2 exponent is -a. The pair nearest in
		// log2 wins. The pair (MAX_A, 0) is reserved for zero, so it is replaced by minpos's
		// (MAX_A - 1, 0) (#1933). Every candidate is a valid encoding, so the result is the
		// nearest representable value in the logarithmic domain. This used to admit any a and
		// then try to project an out-of-range pair back, which failed for a third of all values
		// and sent them to the zero pattern or to a far neighbour.
		// In 64-bit integers: a field may be 32 bits wide, and dbns<34,1>'s MAX_B of 2^32 - 1 cast
		// to int was -1, so this loop did not run at all.
		// Only a window of b needs searching. With a clamped to [-MAX_A, 0], a b below
		// scale/log2(3) has a = 0 and an error that shrinks as b grows, and a b above
		// (scale + MAX_A)/log2(3) has a = -MAX_A and an error that grows with b, so the nearest
		// pair has b within that window, give or take one. The window is at most MAX_A/log2(3) + 4
		// wide and is cut to [0, MAX_B], so the search costs min(MAX_B + 1, MAX_A/log2(3) + 4)
		// steps rather than MAX_B + 1: 3 for dbns<34,1>, 32 for dbns<32,26>.
		// When both fields are wide that is still large: dbns<48,20>'s window is 660k, which made a
		// conversion take 1.6 ms (#1973). Inside the window, where a is not clamped, the error of
		// b is the distance from scale - b*log2(3) to the nearest integer, a linear function
		// mod 1, whose minimum over a range of b argmin_frac finds in O(log) steps. Windows above
		// exhaustive_search_limit take that path; the smaller ones keep the scan.
		const long long maxA = static_cast<long long>(MAX_A);
		const long long maxB = static_cast<long long>(MAX_B);
		double lowestError = 1.0e300;
		long long best_a = 0;   // the base-2 exponent, -a in the encoding
		long long best_b = 0;
		long long bLo = floor_ll(scale / log2of3) - 1;
		long long bHi = floor_ll((scale + static_cast<double>(maxA)) / log2of3) + 2;
		if (bLo < 0) bLo = 0;
		if (bLo > maxB) bLo = maxB;
		if (bHi < 0) bHi = 0;
		if (bHi > maxB) bHi = maxB;
		if (bLo > bHi) bLo = bHi;   // the value is beyond the range: the window collapses onto its edge
		// the candidate b, scored as the scan scores it; a tie keeps the smaller b when the
		// candidates come in ascending order, as they do in the scan
		auto consider = [&](long long b) {
			// constexpr-safe round-to-nearest-int for finite double in long long range
			double r = scale - static_cast<double>(b) * log2of3;
			long long a = static_cast<long long>(r >= 0.0 ? r + 0.5 : r - 0.5);
			if (a > 0) a = 0;
			if (a < -maxA) a = -maxA;
			if (a == -maxA && b == 0) a = -(maxA - 1);   // (MAX_A, 0) is the zero encoding
			double diff = scale - (static_cast<double>(a) + static_cast<double>(b) * log2of3);
			double err = (diff < 0.0 ? -diff : diff);
#if DBNS_TRACE_CONVERSION
			if (!std::is_constant_evaluated()) {
				double fb = sw::math::constexpr_math::exp2(static_cast<double>(a));
				double sb = sw::math::constexpr_math::pow(3.0, static_cast<double>(b));
				double value = fb * sb;
				std::cout << "a : " << a << " b : " << b << " err : " << err << " fb : " << fb << " sb : " << sb << " value : " << value << '\n';
			}
#endif
			if (err < lowestError || (err == lowestError && b < best_b)) {
				lowestError = err;
				best_a = a;
				best_b = b;
			}
		};
		if (bHi - bLo + 1 <= exhaustive_search_limit) {
			for (long long b = bLo; b <= bHi; ++b) consider(b);
		}
		else {
			// a is unclamped, -MAX_A <= round(r) <= 0, for r = scale - b*log2(3) in
			// [-MAX_A - 1/2, 1/2): b in [i0, i1]. Outside that the error grows away from the
			// interior, so its edges and the window's edges are scored directly, with a margin
			// for rounding.
			long long i0 = floor_ll((scale - 0.5) / log2of3) + 1;
			long long i1 = floor_ll((scale + static_cast<double>(maxA) + 0.5) / log2of3);
			if (i0 < bLo) i0 = bLo;
			if (i1 > bHi) i1 = bHi;
			auto considerNear = [&](long long b) {
				for (long long c = b - 2; c <= b + 2; ++c) if (c >= bLo && c <= bHi) consider(c);
			};
			considerNear(bLo);
			if (i0 <= i1) {
				// r(i0 + x) = r(i0) - x*log2(3): its distance to the integers is the smaller of
				// frac(r) and frac(-r), and each is a linear function mod 1 of x
				const long long n = i1 - i0 + 1;
				const double r0 = scale - static_cast<double>(i0) * log2of3;
				const long long xUp   = argmin_frac(n, frac_part(-log2of3), frac_part(r0));    // r just above an integer
				const long long xDown = argmin_frac(n, frac_part(log2of3), frac_part(-r0));    // r just below one
				const long long first  = (xUp < xDown) ? xUp : xDown;
				const long long second = (xUp < xDown) ? xDown : xUp;
				considerNear(i0);
				considerNear(i0 + first);
				considerNear(i0 + second);
				considerNear(i1);
			}
			considerNear(bHi);
		}
#if DBNS_TRACE_CONVERSION
		if (!std::is_constant_evaluated()) std::cout << "best a : " << best_a << " best b : " << best_b << " lowest err : " << lowestError << '\n';
#endif
		clear();
		// below half of minpos, 2^-(MAX_A - 1), the nearest value is zero
		if (scale < -static_cast<double>(maxA - 1) - 1.0) {
			setzero();
			return *this;
		}
		// through the field setters, which handle any number of blocks. This wrote
		// (sign | a << sbbits | b) into the most significant block alone, which is the whole
		// encoding only when it fits one block: dbns<16,5> with the default uint8_t blocks
		// truncated the exponents into the top byte and never wrote the low one, so 1.5
		// became 1.4e122, and every dbns wider than its block type converted wrongly (#1933)
		setexponent(0, static_cast<unsigned>(-best_a));
		setexponent(1, static_cast<unsigned>(best_b));
		setsign(negative);
		return *this;
	}

	// windows of b up to this size are scanned; wider ones are searched with argmin_frac
	static constexpr long long exhaustive_search_limit = 4096;

	// floor and fractional part of a finite double well inside long long's range, constexpr
	static constexpr long long floor_ll(double x) noexcept {
		long long i = static_cast<long long>(x);
		return (static_cast<double>(i) > x) ? i - 1 : i;
	}
	static constexpr double frac_part(double x) noexcept {
		const double f = x - static_cast<double>(floor_ll(x));
		return (f < 0.0 || f >= 1.0) ? 0.0 : f;
	}

	// The x in [0, n) that minimizes frac(beta + x*gamma), for beta and gamma in [0, 1), in
	// O(log n) steps rather than n (#1973). f(x) = frac(beta + x*gamma) rises in runs and drops
	// at each wrap past an integer, so a minimum sits at a run's start:
	//   gamma <= 1/2: at x = 0, or at x_k = ceil((k - beta)/gamma), the first x after the k-th
	//     wrap, k = 1..K with K = floor(beta + (n-1)*gamma). f(x_k) = gamma * frac((beta - k)/gamma),
	//     which over k is the same problem again, with gamma' = frac(-1/gamma) and K <= n*gamma
	//     + 1 terms.
	//   gamma > 1/2: f falls by g = 1 - gamma at each step, so the minimum sits at a run's end:
	//     x = n - 1, or x_k = floor((beta + k)/g), k = 0..K-1, with K = ceil(n*g - beta).
	//     f(x_k) = g * frac((beta + k)/g): gamma' = frac(1/g), and K <= n*g + 1 terms.
	// Either way the count at least halves. The index found is rounded back through a
	// division, so its neighbours are checked as well; the caller scores the final candidates
	// with the exact rule of the scan.
	static constexpr long long argmin_frac(long long n, double gamma, double beta) noexcept {
		constexpr long long small = 32;
		auto f = [&](long long x) { return frac_part(beta + static_cast<double>(x) * gamma); };
		auto better = [&](long long& best, long long x) { if (x >= 0 && x < n && f(x) < f(best)) best = x; };
		if (n <= small) {
			long long best = 0;
			for (long long x = 1; x < n; ++x) better(best, x);
			return best;
		}
		if (gamma <= 0.5) {
			long long best = 0;
			const long long K = floor_ll(beta + static_cast<double>(n - 1) * gamma);
			auto xk = [&](long long k) {
				const double q = (static_cast<double>(k) - beta) / gamma;
				long long x = floor_ll(q);
				if (static_cast<double>(x) < q) ++x;   // ceil
				return x;
			};
			if (K <= small) {
				for (long long k = 1; k <= K; ++k) { const long long x = xk(k); for (long long c = x - 1; c <= x + 1; ++c) better(best, c); }
				return best;
			}
			const long long k = argmin_frac(K, frac_part(-1.0 / gamma), frac_part((beta - 1.0) / gamma)) + 1;
			const long long x = xk(k);
			for (long long c = x - 1; c <= x + 1; ++c) better(best, c);
			return best;
		}
		const double g = 1.0 - gamma;
		long long best = n - 1;
		const double kq = static_cast<double>(n) * g - beta;
		long long K = floor_ll(kq);
		if (static_cast<double>(K) < kq) ++K;   // ceil
		if (K < 0) K = 0;
		auto xk = [&](long long k) { return floor_ll((beta + static_cast<double>(k)) / g); };
		if (K <= small) {
			for (long long k = 0; k < K; ++k) { const long long x = xk(k); for (long long c = x - 1; c <= x + 1; ++c) better(best, c); }
			return best;
		}
		const long long k = argmin_frac(K, frac_part(1.0 / g), frac_part(beta / g));
		const long long x = xk(k);
		for (long long c = x - 1; c <= x + 1; ++c) better(best, c);
		return best;
	}

	// log2 of 0.5^a * 3^b, for exponents of any size: log2(3) is split into a 20-bit head, whose
	// product with b is exact for |b| below 2^33, and the tail
	static constexpr double pair_log2(long long a, long long b) noexcept {
		constexpr double hi = 1.5849609375;           // 20 bits of log2(3)
		constexpr double lo = log2of3 - hi;
		return (static_cast<double>(b) * hi - static_cast<double>(a)) + static_cast<double>(b) * lo;
	}

	// the value (-1)^negative * 0.5^a * 3^b: stored exactly when (a, b) is an encoding, which the
	// zero pattern (MAX_A, 0) is not, and rounded to the nearest encoding otherwise
	constexpr dbns& assign_pair(long long a, long long b, bool negative) noexcept {
		const long long maxA = static_cast<long long>(MAX_A);
		const long long maxB = static_cast<long long>(MAX_B);
		if (a >= 0 && a <= maxA && b >= 0 && b <= maxB && !(a == maxA && b == 0)) {
			clear();
			setexponent(0, static_cast<unsigned>(a));
			setexponent(1, static_cast<unsigned>(b));
			setsign(negative);
			return *this;
		}
		return assign_log2(pair_log2(a, b), negative);
	}

	//////////////////////////////////////////////////////
	/// convertion routines to native types

	template<typename Real>
	constexpr Real ipow(Real base, uint64_t exp) const noexcept {
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
	CONSTEXPRESSION typename std::enable_if< std::is_integral<SignedInt>::value&& std::is_signed<SignedInt>::value, SignedInt>::type
		to_signed() const {
		return SignedInt(to_ieee754<double>());
	}
	template<typename UnsignedInt>
	CONSTEXPRESSION typename std::enable_if< std::is_integral<UnsignedInt>::value&& std::is_unsigned<UnsignedInt>::value, UnsignedInt>::type
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

		const uint64_t a = extractExponent(0);
		const uint64_t b = extractExponent(1);
		// 0.5^a and 3^b each stay in the normal range for these exponents: their product rounds
		// once, as it always has
		constexpr uint64_t maxHalvings  = static_cast<uint64_t>(-(std::numeric_limits<TargetFloat>::min_exponent - 1));
		constexpr uint64_t maxTriplings = static_cast<uint64_t>(static_cast<double>(std::numeric_limits<TargetFloat>::max_exponent - 1) / log2of3);
		if (a <= maxHalvings && b <= maxTriplings) {
			TargetFloat dim1 = ipow(TargetFloat(base0), a);
			TargetFloat dim2 = ipow(TargetFloat(base1), b);
			return static_cast<TargetFloat>(signValue) * dim1 * dim2;
		}
		// Otherwise one factor leaves the range while the value need not: in dbns<16,5>,
		// 3^647 * 0.5^31 is 2^994, but 3^647 alone overflowed and the value decoded to inf, and
		// with both fields wide 0.5^a * 3^b came out 0 * inf = NaN (#1933). So the value is taken
		// as 2^L, L = b * log2(3) - a. log2(3) is split into a 20-bit head, whose product with a
		// 32-bit b is exact, and a tail, so that L keeps its fraction when b * log2(3) is large.
		constexpr double log2of3hi = 1.5849609375;                           // 0x1.95c0p+0, 20 bits
		constexpr double log2of3lo = log2of3 - log2of3hi;                      // the rest
		const double L = (static_cast<double>(b) * log2of3hi - static_cast<double>(a)) + static_cast<double>(b) * log2of3lo;
		TargetFloat magnitude;
		if (L > static_cast<double>(std::numeric_limits<TargetFloat>::max_exponent)) {
			magnitude = std::numeric_limits<TargetFloat>::infinity();
		}
		else if (L < static_cast<double>(std::numeric_limits<TargetFloat>::min_exponent - std::numeric_limits<TargetFloat>::digits - 1)) {
			magnitude = TargetFloat(0);
		}
		else if (std::is_constant_evaluated()) {
			// 2^L = 2^F * 2^N with F in [0, 1): 2^F from the constexpr exp2, which is double only,
			// and 2^N exactly in the target type, so a long double keeps its wider range at compile
			// time as it does at run time (CodeRabbit on PR #1939)
			double Nd = static_cast<double>(static_cast<long long>(L));
			if (Nd > L) Nd -= 1.0;
			const long long N = static_cast<long long>(Nd);
			// The scaling runs in the wider of double and the target, in steps that stay normal
			// there, and the result is rounded to the target once: a float step of 2^-512 would
			// underflow to zero before the product, and a subnormal float result came out 0
			// (CodeRabbit on PR #1939)
			using Wide = std::conditional_t<(sizeof(TargetFloat) > sizeof(double)), TargetFloat, double>;
			Wide m = static_cast<Wide>(sw::math::constexpr_math::exp2(L - Nd));
			long long n = N;
			while (n > 0) { long long k = (n > 512) ? 512 : n; m *= ipow(Wide(2), static_cast<uint64_t>(k)); n -= k; }
			while (n < 0) { long long k = (-n > 512) ? 512 : -n; m *= ipow(Wide(0.5), static_cast<uint64_t>(k)); n += k; }
			magnitude = static_cast<TargetFloat>(m);
		}
		else {
			// in the wider of double and the target: L rounded to float would cost a float's
			// precision of 97.4 in the exponent, and a long double keeps its wider range
			using Wide = std::conditional_t<(sizeof(TargetFloat) > sizeof(double)), TargetFloat, double>;
			magnitude = static_cast<TargetFloat>(std::exp2(static_cast<Wide>(L)));
		}
		return static_cast<TargetFloat>(signValue) * magnitude;
	}

private:
	BlockType _block[nrBlocks];

	////////////////////// operators

	// stream operators: operator<< and operator>> are defined in
	// number/dbns/iostream.hpp (#1334). They were in-class friend DEFINITIONS, which
	// pin <iostream> into every consumer of this header -- the same trap blockdigit and
	// floatcascade had. They need no private access (they go through the public double
	// conversion and assignment), so they are plain namespace-scope templates there
	// rather than friends here.

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
		// Total ordering on dbns sign-magnitude (-1)^s * 0.5^a * 3^b without
		// going through to_ieee754: marshalling via double would collapse
		// distinct very-large or very-small values that exceed double's range.
		// Compare in the log domain: M(x) = -a + b * log2of3 strictly orders
		// all (a,b) pairs because log2of3 is irrational. Sign and zero handled
		// up front.
		const bool lhs_neg = lhs.sign();
		const bool rhs_neg = rhs.sign();
		if (lhs_neg != rhs_neg) return lhs_neg;        // negative < positive
		const bool lhs_zero = lhs.iszero();
		const bool rhs_zero = rhs.iszero();
		if (lhs_zero && rhs_zero) return false;
		if (lhs_zero) return !rhs_neg;                  // 0 < positive
		if (rhs_zero) return lhs_neg;                   // negative < 0
		const double ma = -static_cast<double>(lhs.extractExponent(0))
		                + static_cast<double>(lhs.extractExponent(1)) * log2of3;
		const double mb = -static_cast<double>(rhs.extractExponent(0))
		                + static_cast<double>(rhs.extractExponent(1)) * log2of3;
		// for positives x < y iff M(x) < M(y); for negatives, larger magnitude
		// is smaller, so x < y iff M(x) > M(y).
		return lhs_neg ? (ma > mb) : (ma < mb);
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

// to_binary() moved to manipulators.hpp in #1334: it formats through a std::stringstream,
// which is what keeps it out of the core.

// standard library functions for floating point

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
constexpr dbns<nbits, fbbits, bt, xtra...> abs(const dbns<nbits, fbbits, bt, xtra...>& v) {
	dbns<nbits, fbbits, bt, xtra...> magnitude(v);
	magnitude.setsign(false);
	return magnitude;
}
/// Square root via double conversion (native implementation TODO)
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
dbns<nbits, fbbits, bt, xtra...> sqrt(const dbns<nbits, fbbits, bt, xtra...>& v) {
	return dbns<nbits, fbbits, bt, xtra...>(std::sqrt(double(v)));
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
