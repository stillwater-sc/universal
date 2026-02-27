#pragma once
// dfloat_impl.hpp: implementation of an IEEE 754-2008 decimal floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

// supporting types and functions
#include <universal/native/ieee754.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// dfloat exception structure
#include <universal/number/dfloat/exceptions.hpp>
// DPD (Densely Packed Decimal) codec
#include <universal/number/dfloat/dpd_codec.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////
// Internal helpers for BID encoding
//
// IEEE 754-2008 decimal format layout:
//   [sign(1)] [combination(5)] [exponent_continuation(w)] [trailing_significand(t)]
//
// Total bits: nbits = 1 + 5 + w + t
// where w = es (exponent continuation bits)
//       t = nbits - 1 - 5 - w
//
// Combination field (5 bits: abcde):
//   ab != 11: exponent MSBs = ab, MSD (most significant digit) = 0cde (0-7)
//   ab == 11 && c != 1: exponent MSBs = cd, MSD = 100e (8 or 9)
//   11110: +/- infinity
//   11111: NaN (quiet or signaling based on trailing significand MSB)
//
// BID encoding: trailing significand stored as binary integer
// DPD encoding: trailing significand stored as densely packed decimal (10-bit declets)

// Compute number of bits needed for a decimal32/64/128 configuration
// decimal32:  ndigits=7,  es=6   -> nbits = 1 + 5 + 6  + 20  = 32
// decimal64:  ndigits=16, es=8   -> nbits = 1 + 5 + 8  + 50  = 64
// decimal128: ndigits=34, es=12  -> nbits = 1 + 5 + 12 + 110 = 128

///////////////////////////////////////////////////////////////////////////////
// power_of_10: constexpr power-of-10 helper using __uint128_t for large values
static constexpr uint64_t _pow10_table[20] = {
	1ull,
	10ull,
	100ull,
	1000ull,
	10000ull,
	100000ull,
	1000000ull,
	10000000ull,
	100000000ull,
	1000000000ull,
	10000000000ull,
	100000000000ull,
	1000000000000ull,
	10000000000000ull,
	100000000000000ull,
	1000000000000000ull,
	10000000000000000ull,
	100000000000000000ull,
	1000000000000000000ull,
	10000000000000000000ull
};

static constexpr uint64_t pow10_64(unsigned n) {
	// uint64_t can hold 10^0 through 10^19; 10^20 overflows
	// In constexpr context, out-of-range access triggers a compile error.
	// At runtime, assert to catch misuse.
	return _pow10_table[n]; // n >= 20 is undefined: array bounds enforced by compiler in constexpr
}

// count decimal digits of a uint64_t
static constexpr unsigned count_decimal_digits(uint64_t v) {
	if (v == 0) return 1;
	unsigned d = 0;
	while (v > 0) { v /= 10; ++d; }
	return d;
}

// constexpr ceil(log2(10^n)) - bits needed to represent 10^n in binary
// This is the number of trailing significand bits for BID encoding
static constexpr unsigned bid_trailing_bits(unsigned n) {
	// 10^n values and their bit widths
	// We compute ceil(log2(10^n)) = floor(log2(10^n - 1)) + 1
	// Using the identity: ceil(n * log2(10)) where log2(10) ≈ 3.321928
	// Approximate with integer arithmetic: ceil(n * 3322 / 1000)
	if (n == 0) return 0;
	return static_cast<unsigned>((static_cast<uint64_t>(n) * 3322u + 999u) / 1000u);
}

// DPD trailing bits: (ndigits-1)/3 declets of 10 bits + remainder
static constexpr unsigned dpd_trailing_bits(unsigned ndigits_minus_1) {
	unsigned full_declets = ndigits_minus_1 / 3;
	unsigned remainder = ndigits_minus_1 % 3;
	unsigned bits = full_declets * 10;
	if (remainder == 1) bits += 4;
	else if (remainder == 2) bits += 7;
	return bits;
}

///////////////////////////////////////////////////////////////////////////////
// dfloat: IEEE 754-2008 decimal floating-point number
//
// Template parameters:
//   ndigits  - number of decimal precision digits (p)
//   es       - exponent continuation bits (w)
//   Encoding - BID or DPD
//   bt       - block type for storage
//
template<unsigned _ndigits, unsigned _es, DecimalEncoding _Encoding = DecimalEncoding::BID, typename bt = std::uint32_t>
class dfloat {
public:
	static constexpr unsigned ndigits  = _ndigits;             // precision in decimal digits (p)
	static constexpr unsigned es       = _es;                  // exponent continuation bits (w)
	static constexpr DecimalEncoding encoding = _Encoding;
	static constexpr unsigned combBits = 5u;                   // combination field bits
	static constexpr unsigned t        = (encoding == DecimalEncoding::BID)
		? bid_trailing_bits(ndigits - 1)
		: dpd_trailing_bits(ndigits - 1);
	static constexpr unsigned nbits    = 1u + combBits + es + t;
	static constexpr int      bias     = (3 << (es - 1)) + static_cast<int>(ndigits) - 2;
	static constexpr int      emax     = (3 << es) - 1 - bias;   // max biased exponent
	static constexpr int      emin     = -bias;                    // min biased exponent

	// Current implementation uses uint64_t for significand arithmetic,
	// which can represent up to 10^19 (19 digits). Configurations with
	// ndigits > 19 (e.g., decimal128 with 34 digits) require wider
	// significand types and are not yet supported.
	static_assert(ndigits <= 19,
		"dfloat: ndigits > 19 exceeds uint64_t significand range; "
		"decimal128 (34 digits) requires __uint128_t significand support (not yet implemented)");

	typedef bt BlockType;

	static constexpr unsigned bitsInByte  = 8u;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks    = 1u + ((nbits - 1u) / bitsInBlock);
	static constexpr unsigned MSU         = nrBlocks - 1u;

	// storage mask for the MSU (most significant unit)
	static constexpr bt ALL_ONES  = bt(~0);
	static constexpr bt MSU_MASK  = (nrBlocks * bitsInBlock == nbits) ? ALL_ONES : bt((1ull << (nbits % bitsInBlock)) - 1u);
	static constexpr bt BLOCK_MASK = bt(~0);

	/// trivial constructor
	dfloat() = default;

	dfloat(const dfloat&) = default;
	dfloat(dfloat&&) = default;

	dfloat& operator=(const dfloat&) = default;
	dfloat& operator=(dfloat&&) = default;

	// converting constructors
	constexpr dfloat(const std::string& stringRep) : _block{} { assign(stringRep); }

	// specific value constructor
	constexpr dfloat(const SpecificValue code) noexcept : _block{} {
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
		case SpecificValue::nar:
		case SpecificValue::qnan:
			setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	// initializers for native types
	explicit dfloat(signed char iv)           noexcept : _block{} { *this = iv; }
	explicit dfloat(short iv)                 noexcept : _block{} { *this = iv; }
	explicit dfloat(int iv)                   noexcept : _block{} { *this = iv; }
	explicit dfloat(long iv)                  noexcept : _block{} { *this = iv; }
	explicit dfloat(long long iv)             noexcept : _block{} { *this = iv; }
	explicit dfloat(char iv)                  noexcept : _block{} { *this = iv; }
	explicit dfloat(unsigned short iv)        noexcept : _block{} { *this = iv; }
	explicit dfloat(unsigned int iv)          noexcept : _block{} { *this = iv; }
	explicit dfloat(unsigned long iv)         noexcept : _block{} { *this = iv; }
	explicit dfloat(unsigned long long iv)    noexcept : _block{} { *this = iv; }
	explicit dfloat(float iv)                 noexcept : _block{} { *this = iv; }
	explicit dfloat(double iv)                noexcept : _block{} { *this = iv; }

	// assignment operators for native types
	dfloat& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	dfloat& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	dfloat& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	dfloat& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	dfloat& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	dfloat& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	dfloat& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	dfloat& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	dfloat& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float()           const noexcept { return float(convert_to_double()); }
	explicit operator double()          const noexcept { return convert_to_double(); }

#if LONG_DOUBLE_SUPPORT
	explicit dfloat(long double iv)           noexcept : _block{} { *this = iv; }
	dfloat& operator=(long double rhs)        noexcept { return convert_ieee754(double(rhs)); }
	explicit operator long double()     const noexcept { return (long double)convert_to_double(); }
#endif

	// prefix operators
	dfloat operator-() const {
		dfloat negated(*this);
		if (!negated.iszero()) {
			negated.setsign(!negated.sign());
		}
		return negated;
	}

	// arithmetic operators
	dfloat& operator+=(const dfloat& rhs) {
		// unpack both operands
		bool lhs_sign, rhs_sign;
		int lhs_exp, rhs_exp;
		uint64_t lhs_sig, rhs_sig;
		unpack(lhs_sign, lhs_exp, lhs_sig);
		rhs.unpack(rhs_sign, rhs_exp, rhs_sig);

		// handle special values
		if (isnan() || rhs.isnan()) { setnan(NAN_TYPE_QUIET); return *this; }
		if (isinf() && rhs.isinf()) {
			if (lhs_sign != rhs_sign) { setnan(NAN_TYPE_QUIET); return *this; } // inf + (-inf) = NaN
			return *this; // same sign inf
		}
		if (isinf()) return *this;
		if (rhs.isinf()) { *this = rhs; return *this; }
		if (rhs.iszero()) return *this;
		if (iszero()) { *this = rhs; return *this; }

		// align exponents by scaling the higher-exponent significand UP
		// result exponent = min(lhs_exp, rhs_exp)
		int shift = lhs_exp - rhs_exp;
		int result_exp;
		uint64_t aligned_lhs = lhs_sig;
		uint64_t aligned_rhs = rhs_sig;
		if (shift >= 0) {
			result_exp = rhs_exp;
			// scale lhs up by 10^shift
			for (int i = 0; i < shift; ++i) aligned_lhs *= 10;
		}
		else {
			result_exp = lhs_exp;
			// scale rhs up by 10^(-shift)
			for (int i = 0; i < -shift; ++i) aligned_rhs *= 10;
		}

		int64_t a = lhs_sign ? -static_cast<int64_t>(aligned_lhs) : static_cast<int64_t>(aligned_lhs);
		int64_t b = rhs_sign ? -static_cast<int64_t>(aligned_rhs) : static_cast<int64_t>(aligned_rhs);
		int64_t result_sig = a + b;

		bool result_sign = (result_sig < 0);
		uint64_t abs_sig = static_cast<uint64_t>(result_sign ? -result_sig : result_sig);

		// normalize to ndigits precision
		normalize_and_pack(result_sign, result_exp, abs_sig);
		return *this;
	}
	dfloat& operator-=(const dfloat& rhs) {
		dfloat neg(rhs);
		if (!neg.iszero()) neg.setsign(!neg.sign());
		return operator+=(neg);
	}
	dfloat& operator*=(const dfloat& rhs) {
		bool lhs_sign, rhs_sign;
		int lhs_exp, rhs_exp;
		uint64_t lhs_sig, rhs_sig;
		unpack(lhs_sign, lhs_exp, lhs_sig);
		rhs.unpack(rhs_sign, rhs_exp, rhs_sig);

		// handle special values
		if (isnan() || rhs.isnan()) { setnan(NAN_TYPE_QUIET); return *this; }
		if (isinf() || rhs.isinf()) {
			if (iszero() || rhs.iszero()) { setnan(NAN_TYPE_QUIET); return *this; } // 0 * inf = NaN
			setinf(lhs_sign != rhs_sign);
			return *this;
		}
		if (iszero() || rhs.iszero()) { setzero(); return *this; }

		bool result_sign = (lhs_sign != rhs_sign);
		int result_exp = lhs_exp + rhs_exp;

		// use __uint128_t for the wide multiply if available
#ifdef __SIZEOF_INT128__
		__uint128_t wide = static_cast<__uint128_t>(lhs_sig) * static_cast<__uint128_t>(rhs_sig);
		// reduce to fit in uint64_t by dividing by powers of 10
		while (wide > UINT64_MAX) {
			wide /= 10;
			result_exp++;
		}
		uint64_t result_sig = static_cast<uint64_t>(wide);
#else
		// fallback: delegate through double for smaller configs
		double d = double(*this) * double(rhs);
		*this = d;
		return *this;
#endif

		normalize_and_pack(result_sign, result_exp, result_sig);
		return *this;
	}
	dfloat& operator/=(const dfloat& rhs) {
		bool lhs_sign, rhs_sign;
		int lhs_exp, rhs_exp;
		uint64_t lhs_sig, rhs_sig;
		unpack(lhs_sign, lhs_exp, lhs_sig);
		rhs.unpack(rhs_sign, rhs_exp, rhs_sig);

		// handle special values
		if (isnan() || rhs.isnan()) { setnan(NAN_TYPE_QUIET); return *this; }
		if (isinf() && rhs.isinf()) { setnan(NAN_TYPE_QUIET); return *this; }
		if (rhs.iszero()) {
#if DFLOAT_THROW_ARITHMETIC_EXCEPTION
			throw dfloat_divide_by_zero();
#else
			if (iszero()) { setnan(NAN_TYPE_QUIET); return *this; } // 0/0
			setinf(lhs_sign != rhs_sign);
			return *this;
#endif
		}
		if (iszero()) { setzero(); return *this; }
		if (isinf()) { setsign(lhs_sign != rhs_sign); return *this; }

		bool result_sign = (lhs_sign != rhs_sign);
		int result_exp = lhs_exp - rhs_exp;

		// scale numerator up by 10^ndigits for precision
#ifdef __SIZEOF_INT128__
		__uint128_t scaled_num = static_cast<__uint128_t>(lhs_sig) * static_cast<__uint128_t>(pow10_64(ndigits));
		__uint128_t q = scaled_num / static_cast<__uint128_t>(rhs_sig);
		result_exp -= static_cast<int>(ndigits);
		while (q > UINT64_MAX) {
			q /= 10;
			result_exp++;
		}
		uint64_t result_sig = static_cast<uint64_t>(q);
#else
		double d = double(*this) / double(rhs);
		*this = d;
		return *this;
#endif

		normalize_and_pack(result_sign, result_exp, result_sig);
		return *this;
	}

	// unary operators
	dfloat& operator++() {
		*this += dfloat(1);
		return *this;
	}
	dfloat operator++(int) {
		dfloat tmp(*this);
		operator++();
		return tmp;
	}
	dfloat& operator--() {
		*this -= dfloat(1);
		return *this;
	}
	dfloat operator--(int) {
		dfloat tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	void clear() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) _block[i] = bt(0);
	}
	void setzero() noexcept { clear(); }

	void setinf(bool negative = true) noexcept {
		clear();
		// combination field = 11110 -> bits: sign | 11110 | 0...0
		// set sign
		setbit(nbits - 1, negative);
		// set combination field bits to 11110
		unsigned combStart = nbits - 2; // MSB of combination
		setbit(combStart,     true);   // a = 1
		setbit(combStart - 1, true);   // b = 1
		setbit(combStart - 2, true);   // c = 1
		setbit(combStart - 3, true);   // d = 1
		setbit(combStart - 4, false);  // e = 0
	}

	void setnan(int NaNType = NAN_TYPE_SIGNALLING) noexcept {
		clear();
		// combination field = 11111
		unsigned combStart = nbits - 2;
		setbit(combStart,     true);
		setbit(combStart - 1, true);
		setbit(combStart - 2, true);
		setbit(combStart - 3, true);
		setbit(combStart - 4, true);
		if (NaNType == NAN_TYPE_QUIET) {
			// set MSB of trailing significand for quiet NaN
			if (t > 0) setbit(t - 1, true);
		}
	}

	void setsign(bool negative = true) noexcept {
		setbit(nbits - 1, negative);
	}

	// use un-interpreted raw bits to set the value of the dfloat
	inline void setbits(uint64_t value) noexcept {
		clear();
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(value & BLOCK_MASK);
			value >>= bitsInBlock;
		}
		// mask MSU
		_block[MSU] &= MSU_MASK;
	}

	// create specific number system values of interest
	constexpr dfloat& maxpos() noexcept {
		// maxpos: sign=0, max exponent, max significand = 10^ndigits - 1
		clear();
		// Pack: sign=0, exponent = emax, significand = 10^ndigits - 1
		pack(false, emax, pow10_64(ndigits) - 1);
		return *this;
	}
	constexpr dfloat& minpos() noexcept {
		// minpos: sign=0, min exponent, significand = 1
		clear();
		pack(false, emin, 1);
		return *this;
	}
	constexpr dfloat& zero() noexcept {
		clear();
		return *this;
	}
	constexpr dfloat& minneg() noexcept {
		clear();
		pack(true, emin, 1);
		return *this;
	}
	constexpr dfloat& maxneg() noexcept {
		clear();
		pack(true, emax, pow10_64(ndigits) - 1);
		return *this;
	}

	dfloat& assign(const std::string& txt) {
		// TODO: implement decimal string parsing
		clear();
		return *this;
	}

	// selectors
	bool sign() const noexcept {
		return getbit(nbits - 1);
	}

	bool iszero() const noexcept {
		// zero when all bits are 0 (positive zero)
		// or when sign=1 and rest are 0 (negative zero)
		for (unsigned i = 0; i < MSU; ++i) {
			if (_block[i] != 0) return false;
		}
		// check MSU ignoring sign bit
		bt msu_no_sign = _block[MSU] & bt(MSU_MASK >> 1);
		return (msu_no_sign == 0);
	}

	bool isone() const noexcept {
		bool s; int e; uint64_t sig;
		unpack(s, e, sig);
		return !s && (sig == 1) && (e == 0);
	}

	bool ispos() const noexcept { return !sign(); }
	bool isneg() const noexcept { return sign(); }

	bool isinf() const noexcept {
		// combination field == 11110
		unsigned combStart = nbits - 2;
		return getbit(combStart) && getbit(combStart - 1) &&
		       getbit(combStart - 2) && getbit(combStart - 3) &&
		       !getbit(combStart - 4);
	}

	bool isnan() const noexcept {
		// combination field == 11111
		unsigned combStart = nbits - 2;
		return getbit(combStart) && getbit(combStart - 1) &&
		       getbit(combStart - 2) && getbit(combStart - 3) &&
		       getbit(combStart - 4);
	}

	bool isnan(int NaNType) const noexcept {
		if (!isnan()) return false;
		if (NaNType == NAN_TYPE_QUIET) {
			return (t > 0) ? getbit(t - 1) : true;
		}
		else {
			return (t > 0) ? !getbit(t - 1) : true;
		}
	}

	int scale() const noexcept {
		if (iszero() || isinf() || isnan()) return 0;
		bool s; int e; uint64_t sig;
		unpack(s, e, sig);
		// scale in powers of 10
		return e + static_cast<int>(count_decimal_digits(sig)) - 1;
	}

	// convert to string containing digits number of digits
	std::string str(size_t nrDigits = 0) const {
		if (isnan()) return std::string("nan");
		if (isinf()) return sign() ? std::string("-inf") : std::string("inf");
		if (iszero()) return sign() ? std::string("-0") : std::string("0");

		bool s; int e; uint64_t sig;
		unpack(s, e, sig);

		// value = (-1)^s * sig * 10^e
		std::string digits = std::to_string(sig);
		int num_digits = static_cast<int>(digits.size());
		int decimal_pos = num_digits + e; // position of decimal point from left

		std::string result;
		if (s) result = "-";

		if (decimal_pos <= 0) {
			// value < 1: 0.000...digits
			result += "0.";
			for (int i = 0; i < -decimal_pos; ++i) result += '0';
			result += digits;
		}
		else if (decimal_pos >= num_digits) {
			// integer value
			result += digits;
			for (int i = 0; i < decimal_pos - num_digits; ++i) result += '0';
			result += ".0";
		}
		else {
			// mixed: some digits before and after decimal
			result += digits.substr(0, static_cast<size_t>(decimal_pos));
			result += '.';
			result += digits.substr(static_cast<size_t>(decimal_pos));
		}

		return result;
	}

	///////////////////////////////////////////////////////////////////
	// Bit access (public for free functions like to_binary, color_print)
	bool getbit(unsigned pos) const noexcept {
		if (pos >= nbits) return false;
		unsigned block_idx = pos / bitsInBlock;
		unsigned bit_idx = pos % bitsInBlock;
		return (_block[block_idx] >> bit_idx) & 1;
	}

	///////////////////////////////////////////////////////////////////
	// Unpacking / Packing helpers (public for testing)

	// Unpack the dfloat into sign, unbiased exponent, and significand integer
	void unpack(bool& s, int& exponent, uint64_t& significand) const noexcept {
		s = sign();
		if (iszero()) { exponent = 0; significand = 0; return; }
		if (isinf() || isnan()) { exponent = 0; significand = 0; return; }

		// Extract combination field (5 bits)
		unsigned combStart = nbits - 2;
		bool a = getbit(combStart);
		bool b = getbit(combStart - 1);
		bool c = getbit(combStart - 2);
		bool d = getbit(combStart - 3);
		bool e_bit = getbit(combStart - 4);

		unsigned exp_msbs;
		unsigned msd; // most significant digit

		if (!(a && b)) {
			// ab != 11: exp MSBs = ab, MSD = 0cde
			exp_msbs = (a ? 2u : 0u) + (b ? 1u : 0u);
			msd = (c ? 4u : 0u) + (d ? 2u : 0u) + (e_bit ? 1u : 0u);
		}
		else {
			// ab == 11, c determines large digit vs special
			// cd are exp MSBs, MSD = 100e (digit 8 or 9)
			exp_msbs = (c ? 2u : 0u) + (d ? 1u : 0u);
			msd = 8u + (e_bit ? 1u : 0u);
		}

		// Extract exponent continuation (es bits after combination field)
		unsigned exp_cont = 0;
		unsigned bitpos = nbits - 1 - 1 - combBits; // first bit of exponent continuation
		for (unsigned i = 0; i < es; ++i) {
			if (getbit(bitpos - i)) {
				exp_cont |= (1u << (es - 1 - i));
			}
		}

		unsigned biased_exp = (exp_msbs << es) | exp_cont;
		exponent = static_cast<int>(biased_exp) - bias;

		// Extract trailing significand (t bits) as binary integer
		uint64_t trailing = 0;
		for (unsigned i = 0; i < t; ++i) {
			if (getbit(i)) {
				trailing |= (1ull << i);
			}
		}

		// Full significand = MSD * 10^(ndigits-1) + trailing_significand_value
		if constexpr (encoding == DecimalEncoding::BID) {
			// In BID, trailing is a binary integer representing the lower (ndigits-1) digits
			significand = static_cast<uint64_t>(msd) * pow10_64(ndigits - 1) + trailing;
		}
		else {
			// DPD: decode declets from trailing bits
			significand = dpd_decode_trailing(msd, trailing);
		}
	}

protected:
	bt _block[nrBlocks];

	///////////////////////////////////////////////////////////////////
	// Bit manipulation helpers
	void setbit(unsigned pos, bool value) noexcept {
		if (pos >= nbits) return;
		unsigned block_idx = pos / bitsInBlock;
		unsigned bit_idx = pos % bitsInBlock;
		if (value) {
			_block[block_idx] |= bt(1ull << bit_idx);
		}
		else {
			_block[block_idx] &= bt(~(1ull << bit_idx));
		}
	}

	///////////////////////////////////////////////////////////////////
	// Pack sign, unbiased exponent, and significand into the dfloat encoding
	constexpr void pack(bool s, int exponent, uint64_t significand) noexcept {
		clear();
		if (significand == 0) return; // zero

		// Determine MSD and trailing
		unsigned msd = static_cast<unsigned>(significand / pow10_64(ndigits - 1));
		uint64_t trailing;
		if constexpr (encoding == DecimalEncoding::BID) {
			trailing = significand % pow10_64(ndigits - 1);
		}
		else {
			trailing = dpd_encode_trailing(significand);
		}

		unsigned biased_exp = static_cast<unsigned>(exponent + bias);

		// Encode sign
		setbit(nbits - 1, s);

		// Encode combination field
		unsigned exp_msbs = (biased_exp >> es) & 0x3u;
		unsigned combStart = nbits - 2;

		if (msd < 8) {
			// ab = exp_msbs, cde = msd
			setbit(combStart,     (exp_msbs >> 1) & 1);
			setbit(combStart - 1, exp_msbs & 1);
			setbit(combStart - 2, (msd >> 2) & 1);
			setbit(combStart - 3, (msd >> 1) & 1);
			setbit(combStart - 4, msd & 1);
		}
		else {
			// ab = 11, cd = exp_msbs, e = msd & 1
			setbit(combStart,     true);
			setbit(combStart - 1, true);
			setbit(combStart - 2, (exp_msbs >> 1) & 1);
			setbit(combStart - 3, exp_msbs & 1);
			setbit(combStart - 4, msd & 1);
		}

		// Encode exponent continuation (es bits)
		unsigned exp_cont = biased_exp & ((1u << es) - 1u);
		unsigned bitpos = nbits - 1 - 1 - combBits;
		for (unsigned i = 0; i < es; ++i) {
			setbit(bitpos - i, (exp_cont >> (es - 1 - i)) & 1);
		}

		// Encode trailing significand (t bits)
		for (unsigned i = 0; i < t; ++i) {
			setbit(i, (trailing >> i) & 1);
		}
	}

	///////////////////////////////////////////////////////////////////
	// Normalize significand to ndigits and pack
	void normalize_and_pack(bool s, int exponent, uint64_t significand) noexcept {
		if (significand == 0) { setzero(); if (s) setsign(true); return; }

		// Normalize: ensure significand has exactly ndigits digits
		unsigned digits = count_decimal_digits(significand);
		while (digits > ndigits) {
			significand /= 10;
			exponent++;
			digits--;
		}
		// No need to scale up - smaller significands are valid

		// Check for overflow/underflow
		if (exponent > emax) {
			setinf(s);
			return;
		}
		if (exponent < emin) {
			// underflow to zero
			setzero();
			if (s) setsign(true);
			return;
		}

		pack(s, exponent, significand);
	}

	///////////////////////////////////////////////////////////////////
	// DPD encode/decode helpers
	static uint64_t dpd_decode_trailing(unsigned msd, uint64_t trailing) noexcept {
		// Decode DPD trailing bits to get the lower (ndigits-1) decimal digits
		uint64_t lower = dpd_decode_significand(trailing, ndigits);
		return static_cast<uint64_t>(msd) * pow10_64(ndigits - 1) + lower;
	}
	static uint64_t dpd_encode_trailing(uint64_t significand) noexcept {
		// Encode the trailing (ndigits-1) decimal digits into DPD format
		return dpd_encode_significand(significand, ndigits);
	}

	///////////////////////////////////////////////////////////////////
	// Conversion helpers

	// Convert native IEEE-754 double to dfloat
	dfloat& convert_ieee754(double rhs) noexcept {
		if (std::isnan(rhs)) {
			setnan(NAN_TYPE_QUIET);
			return *this;
		}
		if (std::isinf(rhs)) {
			setinf(rhs < 0);
			return *this;
		}
		if (rhs == 0.0) {
			setzero();
			if (std::signbit(rhs)) setsign(true);
			return *this;
		}

		bool negative = (rhs < 0);
		double abs_val = std::fabs(rhs);

		// Convert to decimal significand and exponent
		// value = significand * 10^exponent where significand is an integer with ndigits digits
		int dec_exp = 0;
		if (abs_val != 0.0) {
			dec_exp = static_cast<int>(std::floor(std::log10(abs_val)));
		}

		// Scale to get ndigits significant digits
		int target_exp = dec_exp - static_cast<int>(ndigits) + 1;
		double scaled = abs_val / std::pow(10.0, static_cast<double>(target_exp));
		uint64_t significand = static_cast<uint64_t>(std::round(scaled));

		// Adjust if rounding pushed us to ndigits+1 digits
		if (significand >= pow10_64(ndigits)) {
			significand /= 10;
			target_exp++;
		}
		// Remove trailing zeros
		while (significand > 0 && (significand % 10) == 0) {
			significand /= 10;
			target_exp++;
		}

		normalize_and_pack(negative, target_exp, significand);
		return *this;
	}

	// Convert dfloat to native IEEE-754 double
	double convert_to_double() const noexcept {
		if (isnan()) return std::numeric_limits<double>::quiet_NaN();
		if (isinf()) return sign() ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
		if (iszero()) return sign() ? -0.0 : 0.0;

		bool s; int e; uint64_t sig;
		unpack(s, e, sig);

		// value = (-1)^s * sig * 10^e
		double result = static_cast<double>(sig) * std::pow(10.0, static_cast<double>(e));
		return s ? -result : result;
	}

	dfloat& convert_signed(int64_t v) noexcept {
		if (0 == v) {
			setzero();
			return *this;
		}
		bool negative = (v < 0);
		uint64_t abs_v = static_cast<uint64_t>(negative ? -v : v);

		// Remove trailing zeros
		int exponent = 0;
		while (abs_v > 0 && (abs_v % 10) == 0) {
			abs_v /= 10;
			exponent++;
		}

		normalize_and_pack(negative, exponent, abs_v);
		return *this;
	}

	dfloat& convert_unsigned(uint64_t v) noexcept {
		if (0 == v) {
			setzero();
			return *this;
		}

		int exponent = 0;
		while (v > 0 && (v % 10) == 0) {
			v /= 10;
			exponent++;
		}

		normalize_and_pack(false, exponent, v);
		return *this;
	}

private:

	// dfloat - dfloat logic comparisons
	template<unsigned N, unsigned E, DecimalEncoding Enc, typename B>
	friend bool operator==(const dfloat<N, E, Enc, B>& lhs, const dfloat<N, E, Enc, B>& rhs);

	// dfloat - literal logic comparisons
	template<unsigned N, unsigned E, DecimalEncoding Enc, typename B>
	friend bool operator==(const dfloat<N, E, Enc, B>& lhs, const double rhs);

	// literal - dfloat logic comparisons
	template<unsigned N, unsigned E, DecimalEncoding Enc, typename B>
	friend bool operator==(const double lhs, const dfloat<N, E, Enc, B>& rhs);
};


////////////////////////    helper functions   /////////////////////////////////

// divide dfloat a and b and return result argument
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
void divide(const dfloat<ndigits, es, Encoding, BlockType>& a, const dfloat<ndigits, es, Encoding, BlockType>& b, dfloat<ndigits, es, Encoding, BlockType>& quotient) {
	quotient = a;
	quotient /= b;
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline std::string to_binary(const dfloat<ndigits, es, Encoding, BlockType>& number, bool nibbleMarker = false) {
	using Dfloat = dfloat<ndigits, es, Encoding, BlockType>;
	std::stringstream s;

	// sign bit
	s << (number.sign() ? '1' : '0') << '.';

	// combination field (5 bits)
	unsigned combStart = Dfloat::nbits - 2;
	for (unsigned i = 0; i < Dfloat::combBits; ++i) {
		s << (number.getbit(combStart - i) ? '1' : '0');
	}
	s << '.';

	// exponent continuation (es bits)
	unsigned expStart = Dfloat::nbits - 1 - 1 - Dfloat::combBits;
	for (unsigned i = 0; i < es; ++i) {
		s << (number.getbit(expStart - i) ? '1' : '0');
	}
	s << '.';

	// trailing significand (t bits, MSB first)
	for (int i = static_cast<int>(Dfloat::t) - 1; i >= 0; --i) {
		s << (number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4 == 0)) s << '\'';
	}

	return s.str();
}

////////////////////////    DFLOAT functions   /////////////////////////////////

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> abs(const dfloat<ndigits, es, Encoding, BlockType>& a) {
	dfloat<ndigits, es, Encoding, BlockType> result(a);
	result.setsign(false);
	return result;
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> fabs(dfloat<ndigits, es, Encoding, BlockType> a) {
	a.setsign(false);
	return a;
}


////////////////////////  stream operators   /////////////////////////////////

// generate a dfloat format ASCII format
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const dfloat<ndigits, es, Encoding, BlockType>& i) {
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));

	return ostr << ss.str();
}

// read an ASCII dfloat format
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline std::istream& operator>>(std::istream& istr, dfloat<ndigits, es, Encoding, BlockType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a dfloat value\n";
	}
	return istr;
}

////////////////// string operators

// read a dfloat ASCII format and make a dfloat out of it
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
bool parse(const std::string& number, dfloat<ndigits, es, Encoding, BlockType>& value) {
	bool bSuccess = false;
	// TODO: implement decimal string parsing
	return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - dfloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator==(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	// NaN != anything (including itself)
	if (lhs.isnan() || rhs.isnan()) return false;
	// both zero (ignoring sign)
	if (lhs.iszero() && rhs.iszero()) return true;
	// compare unpacked values
	bool ls, rs; int le, re; uint64_t lsig, rsig;
	lhs.unpack(ls, le, lsig);
	rhs.unpack(rs, re, rsig);
	return (ls == rs) && (le == re) && (lsig == rsig);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator!=(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator< (const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	// NaN is unordered
	if (lhs.isnan() || rhs.isnan()) return false;
	// handle infinities
	if (lhs.isinf() && rhs.isinf()) {
		return lhs.sign() && !rhs.sign(); // -inf < +inf
	}
	if (lhs.isinf()) return lhs.sign();  // -inf < anything
	if (rhs.isinf()) return !rhs.sign(); // anything < +inf

	// compare through double for simplicity
	return double(lhs) < double(rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator> (const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator<=(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator>=(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - literal binary logic operators
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator==(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator==(lhs, dfloat<ndigits, es, Encoding, BlockType>(rhs));
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator!=(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator< (const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator<(lhs, dfloat<ndigits, es, Encoding, BlockType>(rhs));
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator> (const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator< (dfloat<ndigits, es, Encoding, BlockType>(rhs), lhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator<=(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator>=(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dfloat binary logic operators
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator==(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator==(dfloat<ndigits, es, Encoding, BlockType>(lhs), rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator!=(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator< (const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator<(dfloat<ndigits, es, Encoding, BlockType>(lhs), rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator> (const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator<=(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator>=(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - dfloat binary arithmetic operators
// BINARY ADDITION
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator+(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	dfloat<ndigits, es, Encoding, BlockType> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator-(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	dfloat<ndigits, es, Encoding, BlockType> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator*(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	dfloat<ndigits, es, Encoding, BlockType> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator/(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	dfloat<ndigits, es, Encoding, BlockType> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - literal binary arithmetic operators
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator+(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator+(lhs, dfloat<ndigits, es, Encoding, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator-(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator-(lhs, dfloat<ndigits, es, Encoding, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator*(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator*(lhs, dfloat<ndigits, es, Encoding, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator/(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const double rhs) {
	return operator/(lhs, dfloat<ndigits, es, Encoding, BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dfloat binary arithmetic operators
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator+(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator+(dfloat<ndigits, es, Encoding, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator-(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator-(dfloat<ndigits, es, Encoding, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator*(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator*(dfloat<ndigits, es, Encoding, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline dfloat<ndigits, es, Encoding, BlockType> operator/(const double lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	return operator/(dfloat<ndigits, es, Encoding, BlockType>(lhs), rhs);
}

}} // namespace sw::universal
