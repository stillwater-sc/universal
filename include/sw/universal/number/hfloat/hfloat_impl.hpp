#pragma once
// hfloat_impl.hpp: implementation of IBM System/360 hexadecimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// IBM System/360 Hexadecimal Floating-Point (1964):
//   Format: [sign(1)] [exponent(es)] [hex_fraction(ndigits*4 bits)]
//   Value:  (-1)^sign * 16^(exponent - bias) * 0.f1f2...fn
//
// Key properties:
//   - No hidden bit (fraction always has explicit leading hex digit)
//   - No NaN, no infinity, no subnormals
//   - Truncation rounding only (never rounds up)
//   - Overflow saturates to maxpos/maxneg
//   - Zero: sign=0, exponent=0, fraction=0
//   - Wobbling precision: 0-3 leading zero bits in MSB hex digit
//
// Standard configurations:
//   Short:    hfloat<6, 7>  = 1+7+24 = 32 bits
//   Long:     hfloat<14, 7> = 1+7+56 = 64 bits
//   Extended: hfloat<28, 7> = 1+7+112 = 120 bits (stored in 128)

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
// hfloat exception structure
#include <universal/number/hfloat/exceptions.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////
// hfloat: IBM System/360 hexadecimal floating-point number
//
// Template parameters:
//   ndigits  - number of hexadecimal fraction digits
//   es       - exponent bits (7 for standard IBM HFP)
//   bt       - block type for storage
//
template<unsigned _ndigits, unsigned _es, typename bt = std::uint32_t>
class hfloat {
public:
	static constexpr unsigned ndigits     = _ndigits;            // hex fraction digits
	static constexpr unsigned es          = _es;                 // exponent bits
	static constexpr unsigned fbits       = ndigits * 4u;        // fraction bits
	static constexpr unsigned nbits       = 1u + es + fbits;     // total bits
	static constexpr int      bias        = (1 << (es - 1));     // exponent bias (64 for es=7)
	static constexpr int      emax        = (1 << es) - 1 - bias; // max unbiased exponent
	static constexpr int      emin        = -bias;                // min unbiased exponent

	typedef bt BlockType;

	static constexpr unsigned bitsInByte  = 8u;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks    = 1u + ((nbits - 1u) / bitsInBlock);
	static constexpr unsigned MSU         = nrBlocks - 1u;

	static constexpr bt ALL_ONES  = bt(~0);
	static constexpr bt MSU_MASK  = (nrBlocks * bitsInBlock == nbits) ? ALL_ONES : bt((1ull << (nbits % bitsInBlock)) - 1u);
	static constexpr bt BLOCK_MASK = bt(~0);

	/// trivial constructor
	hfloat() = default;

	hfloat(const hfloat&) = default;
	hfloat(hfloat&&) = default;

	hfloat& operator=(const hfloat&) = default;
	hfloat& operator=(hfloat&&) = default;

	// specific value constructor
	constexpr hfloat(const SpecificValue code) noexcept : _block{} {
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
			maxpos();  // no infinity in HFP, saturate to maxpos
			break;
		case SpecificValue::infneg:
			maxneg();  // no infinity in HFP, saturate to maxneg
			break;
		case SpecificValue::nar:
		case SpecificValue::qnan:
		case SpecificValue::snan:
			zero();    // no NaN in HFP, map to zero
			break;
		}
	}

	// initializers for native types
	constexpr explicit hfloat(signed char iv)           noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(short iv)                 noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(int iv)                   noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(long iv)                  noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(long long iv)             noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(char iv)                  noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned short iv)        noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned int iv)          noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned long iv)         noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned long long iv)    noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(float iv)                 noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(double iv)                noexcept : _block{} { *this = iv; }

	// assignment operators for native types
	constexpr hfloat& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	// Plain `char` may be signed or unsigned per platform; route through
	// the signed conversion via integer promotion so hfloat(char(-1)) on
	// signed-char targets yields -1, not UCHAR_MAX (CodeRabbit finding
	// from PR #805 applied to the sibling type).
	constexpr hfloat& operator=(char rhs)               noexcept { return convert_signed(static_cast<int>(rhs)); }
	constexpr hfloat& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr hfloat& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	constexpr explicit operator float()           const noexcept { return float(convert_to_double()); }
	constexpr explicit operator double()          const noexcept { return convert_to_double(); }

#if LONG_DOUBLE_SUPPORT
	constexpr explicit hfloat(long double iv)           noexcept : _block{} { *this = iv; }
	constexpr hfloat& operator=(long double rhs)        noexcept { return convert_ieee754(double(rhs)); }
	constexpr explicit operator long double()     const noexcept { return (long double)convert_to_double(); }
#endif

	// prefix operators
	constexpr hfloat operator-() const {
		hfloat negated(*this);
		if (!negated.iszero()) {
			negated.setsign(!negated.sign());
		}
		return negated;
	}

	// arithmetic operators
	constexpr hfloat& operator+=(const hfloat& rhs) {
		bool lhs_sign, rhs_sign;
		int lhs_exp, rhs_exp;
		uint64_t lhs_frac, rhs_frac;
		unpack(lhs_sign, lhs_exp, lhs_frac);
		rhs.unpack(rhs_sign, rhs_exp, rhs_frac);

		if (rhs.iszero()) return *this;
		if (iszero()) { *this = rhs; return *this; }

		// align exponents by shifting the smaller-exponent fraction RIGHT
		// by 4*(exp_large - exp_small) bits (hex digit alignment)
		int shift = lhs_exp - rhs_exp;
		uint64_t aligned_lhs = lhs_frac;
		uint64_t aligned_rhs = rhs_frac;
		int result_exp;

		if (shift >= 0) {
			result_exp = lhs_exp;
			aligned_rhs >>= (static_cast<unsigned>(shift) * 4u);
		}
		else {
			result_exp = rhs_exp;
			aligned_lhs >>= (static_cast<unsigned>(-shift) * 4u);
		}

		// add/subtract based on signs
		int64_t result_frac;
		int64_t a = lhs_sign ? -static_cast<int64_t>(aligned_lhs) : static_cast<int64_t>(aligned_lhs);
		int64_t b = rhs_sign ? -static_cast<int64_t>(aligned_rhs) : static_cast<int64_t>(aligned_rhs);
		result_frac = a + b;

		bool result_sign = (result_frac < 0);
		uint64_t abs_frac = static_cast<uint64_t>(result_sign ? -result_frac : result_frac);

		normalize_and_pack(result_sign, result_exp, abs_frac);
		return *this;
	}

	constexpr hfloat& operator-=(const hfloat& rhs) {
		hfloat neg(rhs);
		if (!neg.iszero()) neg.setsign(!neg.sign());
		return operator+=(neg);
	}

	constexpr hfloat& operator*=(const hfloat& rhs) {
		if (iszero() || rhs.iszero()) { setzero(); return *this; }

		bool lhs_sign, rhs_sign;
		int lhs_exp, rhs_exp;
		uint64_t lhs_frac, rhs_frac;
		unpack(lhs_sign, lhs_exp, lhs_frac);
		rhs.unpack(rhs_sign, rhs_exp, rhs_frac);

		bool result_sign = (lhs_sign != rhs_sign);
		int result_exp = lhs_exp + rhs_exp;

#ifdef __SIZEOF_INT128__
		__uint128_t wide = static_cast<__uint128_t>(lhs_frac) * static_cast<__uint128_t>(rhs_frac);
		// The fractions are in 0.f format with fbits fraction bits
		// Product has 2*fbits bits, shift right by fbits to get back to fbits
		uint64_t result_frac = static_cast<uint64_t>(wide >> fbits);
		normalize_and_pack(result_sign, result_exp, result_frac);
#else
		// fallback: delegate through double
		double d = double(*this) * double(rhs);
		*this = d;
#endif
		return *this;
	}

	constexpr hfloat& operator/=(const hfloat& rhs) {
		if (rhs.iszero()) {
#if HFLOAT_THROW_ARITHMETIC_EXCEPTION
			// Throwing in a constant expression is ill-formed; fence the
			// throw under runtime to keep operator/= constexpr-callable.
			// At constant-eval, divide-by-zero falls through to setzero()
			// (IBM HFP has no NaN/inf, so saturation is the closest
			// constexpr-safe behavior).
			if (!std::is_constant_evaluated()) {
				throw hfloat_divide_by_zero();
			}
#endif
			setzero();
			return *this;
		}
		if (iszero()) return *this;

		bool lhs_sign, rhs_sign;
		int lhs_exp, rhs_exp;
		uint64_t lhs_frac, rhs_frac;
		unpack(lhs_sign, lhs_exp, lhs_frac);
		rhs.unpack(rhs_sign, rhs_exp, rhs_frac);

		bool result_sign = (lhs_sign != rhs_sign);
		int result_exp = lhs_exp - rhs_exp;

#ifdef __SIZEOF_INT128__
		// Scale numerator up by fbits for precision
		__uint128_t scaled_num = static_cast<__uint128_t>(lhs_frac) << fbits;
		uint64_t result_frac = static_cast<uint64_t>(scaled_num / static_cast<__uint128_t>(rhs_frac));
		normalize_and_pack(result_sign, result_exp, result_frac);
#else
		double d = double(*this) / double(rhs);
		*this = d;
#endif
		return *this;
	}

	// unary operators
	constexpr hfloat& operator++() {
		*this += hfloat(1);
		return *this;
	}
	constexpr hfloat operator++(int) {
		hfloat tmp(*this);
		operator++();
		return tmp;
	}
	constexpr hfloat& operator--() {
		*this -= hfloat(1);
		return *this;
	}
	constexpr hfloat operator--(int) {
		hfloat tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) _block[i] = bt(0);
	}
	constexpr void setzero() noexcept { clear(); }

	constexpr void setsign(bool negative = true) noexcept {
		setbit(nbits - 1, negative);
	}

	// use un-interpreted raw bits to set the value
	constexpr void setbits(uint64_t value) noexcept {
		clear();
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(value & BLOCK_MASK);
			value >>= bitsInBlock;
		}
		_block[MSU] &= MSU_MASK;
	}

	// create specific number system values of interest
	constexpr hfloat& maxpos() noexcept {
		clear();
		// sign=0, exponent=all 1s, fraction=all 1s
		// exponent field = (1<<es)-1
		unsigned biased_exp = (1u << es) - 1u;
		uint64_t max_frac = (1ull << fbits) - 1u;
		pack(false, static_cast<int>(biased_exp) - bias, max_frac);
		return *this;
	}
	constexpr hfloat& minpos() noexcept {
		clear();
		// sign=0, exponent=0, fraction=0...01
		pack(false, emin, 1);
		return *this;
	}
	constexpr hfloat& zero() noexcept {
		clear();
		return *this;
	}
	constexpr hfloat& minneg() noexcept {
		clear();
		pack(true, emin, 1);
		return *this;
	}
	constexpr hfloat& maxneg() noexcept {
		clear();
		unsigned biased_exp = (1u << es) - 1u;
		uint64_t max_frac = (1ull << fbits) - 1u;
		pack(true, static_cast<int>(biased_exp) - bias, max_frac);
		return *this;
	}

	// selectors
	constexpr bool sign() const noexcept {
		return getbit(nbits - 1);
	}

	constexpr bool iszero() const noexcept {
		// zero when fraction is all zeros (exponent and sign don't matter)
		// In IBM HFP, zero is represented as all-zeros
		for (unsigned i = 0; i < nrBlocks; ++i) {
			bt mask = (i == MSU) ? bt(MSU_MASK & ~(bt(1) << ((nbits - 1) % bitsInBlock))) : BLOCK_MASK;
			if ((_block[i] & mask) != 0) return false;
		}
		return true;
	}

	constexpr bool isone() const noexcept {
		bool s; int e; uint64_t f;
		unpack(s, e, f);
		// 1.0 = 0.1 * 16^1, so e=1, f = 1 << (fbits-4)  (leading hex digit = 1)
		return !s && (e == 1) && (f == (1ull << (fbits - 4)));
	}

	constexpr bool ispos() const noexcept { return !sign(); }
	constexpr bool isneg() const noexcept { return sign(); }

	// IBM HFP has no NaN or infinity
	constexpr bool isinf() const noexcept { return false; }
	constexpr bool isnan() const noexcept { return false; }
	constexpr bool isnan(int) const noexcept { return false; }

	constexpr int scale() const noexcept {
		if (iszero()) return 0;
		bool s; int e; uint64_t f;
		unpack(s, e, f);
		// IBM HFP: value = 0.f * 16^e
		// Scale in terms of powers of 2: scale = 4*e + leading_bit_position_of_f - fbits
		// But conceptually: scale = 4 * (e - bias_already_removed)
		// Find the leading 1 bit of the fraction
		int leading = -1;
		for (int i = static_cast<int>(fbits) - 1; i >= 0; --i) {
			if ((f >> i) & 1) { leading = i; break; }
		}
		if (leading < 0) return 0;
		return 4 * e + leading - static_cast<int>(fbits);
	}

	// convert to string
	std::string str(size_t nrDigits = 0) const {
		if (iszero()) return sign() ? std::string("-0") : std::string("0");

		double d = convert_to_double();
		std::stringstream ss;
		if (nrDigits > 0) {
			ss << std::setprecision(static_cast<int>(nrDigits));
		}
		ss << d;
		return ss.str();
	}

	///////////////////////////////////////////////////////////////////
	// Bit access (public for free functions)
	constexpr bool getbit(unsigned pos) const noexcept {
		if (pos >= nbits) return false;
		unsigned block_idx = pos / bitsInBlock;
		unsigned bit_idx = pos % bitsInBlock;
		return (_block[block_idx] >> bit_idx) & 1;
	}

	///////////////////////////////////////////////////////////////////
	// Unpack into sign, unbiased exponent, and fraction
	constexpr void unpack(bool& s, int& exponent, uint64_t& fraction) const noexcept {
		s = sign();
		if (iszero()) { exponent = 0; fraction = 0; return; }

		// Extract exponent field (es bits)
		unsigned exp_field = 0;
		unsigned expStart = nbits - 2; // MSB of exponent (just below sign)
		for (unsigned i = 0; i < es; ++i) {
			if (getbit(expStart - i)) {
				exp_field |= (1u << (es - 1 - i));
			}
		}
		exponent = static_cast<int>(exp_field) - bias;

		// Extract fraction (fbits bits)
		fraction = 0;
		for (unsigned i = 0; i < fbits; ++i) {
			if (getbit(i)) {
				fraction |= (1ull << i);
			}
		}
	}

protected:
	bt _block[nrBlocks];

	///////////////////////////////////////////////////////////////////
	// Bit manipulation helpers
	constexpr void setbit(unsigned pos, bool value) noexcept {
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
	// Pack sign, unbiased exponent, and fraction
	constexpr void pack(bool s, int exponent, uint64_t fraction) noexcept {
		clear();

		// set sign
		setbit(nbits - 1, s);

		// set exponent field
		unsigned biased_exp = static_cast<unsigned>(exponent + bias);
		unsigned expStart = nbits - 2;
		for (unsigned i = 0; i < es; ++i) {
			setbit(expStart - i, (biased_exp >> (es - 1 - i)) & 1);
		}

		// set fraction field (fbits bits)
		for (unsigned i = 0; i < fbits; ++i) {
			setbit(i, (fraction >> i) & 1);
		}
	}

	///////////////////////////////////////////////////////////////////
	// Normalize: ensure leading hex digit is non-zero, then truncate
	constexpr void normalize_and_pack(bool s, int exponent, uint64_t fraction) noexcept {
		if (fraction == 0) { setzero(); return; }

		// Normalize: shift left until the fraction fits in fbits with a non-zero leading hex digit
		// The leading hex digit occupies bits [fbits-1:fbits-4]
		// We need the fraction to have its MSB within fbits
		while (fraction >= (1ull << fbits)) {
			fraction >>= 4;  // shift right by one hex digit
			exponent++;
		}
		// Shift left until leading hex digit is non-zero
		while (fraction > 0 && fraction < (1ull << (fbits - 4))) {
			fraction <<= 4;  // shift left by one hex digit
			exponent--;
		}

		// Truncate to fbits (IBM HFP truncates, never rounds up)
		fraction &= ((1ull << fbits) - 1);

		// Check overflow/underflow
		if (exponent > emax) {
			// overflow: saturate to maxpos/maxneg
			if (s) maxneg(); else maxpos();
			return;
		}
		if (exponent < emin) {
			// underflow: set to zero
			setzero();
			return;
		}

		pack(s, exponent, fraction);
	}

	///////////////////////////////////////////////////////////////////
	// Conversion helpers

	// Convert IEEE-754 double to hfloat
	//
	// Constexpr-safe rewrite of the original frexp/ldexp implementation:
	// - NaN detected via x != x (only NaN is unequal to itself)
	// - infinity detected by bracketing against numeric_limits::max()
	//   (numeric_limits<double>::max() is constexpr; std::isinf is not)
	// - fabs replaced with `negative ? -rhs : rhs`
	// - frexp/ldexp replaced with raw IEEE 754 field extraction via
	//   sw::bit_cast (constexpr on toolchains exposing std::bit_cast or
	//   __builtin_bit_cast; runtime fallback retains the legacy path).
	//
	// IBM HFP has no NaN and no infinity:
	//   NaN double  -> hfloat zero
	//   +/- inf     -> hfloat maxpos/maxneg (saturation)
	constexpr hfloat& convert_ieee754(double rhs) noexcept {
		if (rhs != rhs) {                       // NaN
			setzero();
			return *this;
		}
		if (rhs == 0.0) {
			setzero();
			return *this;
		}
		constexpr double dbl_max = std::numeric_limits<double>::max();
		if (rhs >  dbl_max) { maxpos(); return *this; }   // +inf
		if (rhs < -dbl_max) { maxneg(); return *this; }   // -inf

		bool negative = (rhs < 0);
		double abs_val = negative ? -rhs : rhs;

		// Reconstruct frexp's (frac, bin_exp) without std::frexp:
		// IEEE 754 double: bias 1023, 52 fraction bits, hidden 1 for normals.
		//   normal: value = (1 + rawFrac/2^52) * 2^(rawExp - 1023)
		//                 = (2^52 + rawFrac) * 2^(rawExp - 1075)
		// frexp returns frac in [0.5, 1) such that value = frac * 2^bin_exp:
		//   frac    = (2^52 + rawFrac) / 2^53
		//   bin_exp = rawExp - 1022
		// So mantissa_with_hidden = (2^52 + rawFrac) and
		//   frac * 2^shift = mantissa_with_hidden * 2^(shift - 53)
		int bin_exp = 0;
		uint64_t mantissa = 0;          // 53-bit significand with hidden bit

		if constexpr (sw::is_bit_cast_constexpr_v) {
			// Constexpr path: extract IEEE 754 fields via bit_cast.
			uint64_t bits = sw::bit_cast<uint64_t>(abs_val);
			uint64_t rawExp  = (bits >> 52) & 0x7FFu;
			uint64_t rawFrac = bits & ((uint64_t(1) << 52) - 1u);
			if (rawExp == 0) {
				// Subnormal double values are far below IBM HFP's smallest
				// representable (16^emin); flush to zero per HFP semantics.
				setzero();
				return *this;
			}
			bin_exp  = static_cast<int>(rawExp) - 1022;
			mantissa = (uint64_t(1) << 52) | rawFrac;
		}
		else {
			// Runtime fallback (legacy path).  Unreachable in constant
			// expressions on toolchains without constexpr bit_cast --
			// the function still compiles but constexpr callers will get
			// a not-a-constant-expression diagnostic at the use site.
			if (!std::is_constant_evaluated()) {
				int e = 0;
				double frac = std::frexp(abs_val, &e);
				bin_exp = e;
				// frac in [0.5, 1) -> mantissa = frac * 2^53 fits in 53 bits
				mantissa = static_cast<uint64_t>(std::ldexp(frac, 53));
			}
			else {
				return *this;  // dead code (constexpr caller would not reach here)
			}
		}

		// Convert binary exponent to base-16 exponent.
		// We want hex_exp = ceil(bin_exp / 4) so 0.f * 16^hex_exp == abs_val
		// with f's leading hex digit non-zero.
		// For bin_exp > 0: ceil(n/4) = (n + 3) / 4
		// For bin_exp <= 0: C integer division truncates toward zero, which
		//   matches ceil for negative numerators.
		int hex_exp;
		if (bin_exp > 0) {
			hex_exp = (bin_exp + 3) / 4;
		}
		else {
			hex_exp = bin_exp / 4;
		}

		// fraction = frac * 2^shift = mantissa * 2^(shift - 53)
		int shift = bin_exp - 4 * hex_exp + static_cast<int>(fbits);
		int mantissa_shift = shift - 53;
		uint64_t fraction = 0;
		if (mantissa_shift >= 0) {
			// mantissa has at most 53 significant bits; shifts up to 11
			// bits stay within uint64_t.
			if (mantissa_shift < 11) {
				fraction = mantissa << mantissa_shift;
			}
			else {
				// Beyond uint64_t headroom: saturate to all-ones in fbits
				// (fbits >= 64 is the hfloat_extended path; the truncate
				// mask below is also no-op for fbits >= 64).
				fraction = (fbits < 64) ? ((uint64_t(1) << fbits) - 1u) : ~uint64_t(0);
			}
		}
		else if (-mantissa_shift < 64) {
			// Shift right truncates low bits -- matches IBM HFP's
			// truncation rounding contract.
			fraction = mantissa >> static_cast<unsigned>(-mantissa_shift);
		}
		// else: fraction stays 0

		// Truncate to fbits.  Mask is no-op when fbits >= 64.
		if constexpr (fbits < 64) {
			fraction &= ((uint64_t(1) << fbits) - 1u);
		}

		normalize_and_pack(negative, hex_exp, fraction);
		return *this;
	}

	// Convert hfloat to IEEE-754 double
	//
	// Constexpr-safe: replaces std::ldexp(d, n) with a power-of-2 scaling
	// loop that multiplies/divides by 2.0 (exactly representable in double).
	// At runtime, dispatches to std::ldexp for performance.  The shift range
	// per template config is bounded (fbits + 4*emax for hfloat_extended is
	// ~360), so the constexpr loop is well-bounded.
	constexpr double convert_to_double() const noexcept {
		if (iszero()) return sign() ? -0.0 : 0.0;

		bool s; int e; uint64_t f;
		unpack(s, e, f);

		// value = 0.f * 16^e = f * 2^(-fbits) * 16^e = f * 2^(4*e - fbits)
		int shift = 4 * e - static_cast<int>(fbits);
		double result = static_cast<double>(f);
		if (std::is_constant_evaluated()) {
			// Power-of-2 scaling: 2.0 and 0.5 are exactly representable
			// in IEEE 754 double, so this introduces no rounding error
			// over the bounded iteration count.
			if (shift > 0) {
				for (int i = 0; i < shift; ++i) result *= 2.0;
			}
			else if (shift < 0) {
				for (int i = 0; i < -shift; ++i) result *= 0.5;
			}
		}
		else {
			result = std::ldexp(result, shift);
		}
		return s ? -result : result;
	}

	constexpr hfloat& convert_signed(int64_t v) noexcept {
		if (v == 0) {
			setzero();
			return *this;
		}
		// Use double conversion for simplicity
		return convert_ieee754(static_cast<double>(v));
	}

	constexpr hfloat& convert_unsigned(uint64_t v) noexcept {
		if (v == 0) {
			setzero();
			return *this;
		}
		return convert_ieee754(static_cast<double>(v));
	}

private:

	// hfloat - hfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend constexpr bool operator==(const hfloat<N, E, B>& lhs, const hfloat<N, E, B>& rhs);

	// hfloat - literal logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend constexpr bool operator==(const hfloat<N, E, B>& lhs, const double rhs);

	// literal - hfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend constexpr bool operator==(const double lhs, const hfloat<N, E, B>& rhs);
};


////////////////////////    helper functions   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_binary(const hfloat<ndigits, es, BlockType>& number, bool nibbleMarker = false) {
	using Hfloat = hfloat<ndigits, es, BlockType>;
	std::stringstream s;

	// sign bit
	s << "0b" << (number.sign() ? '1' : '0') << '.';

	// exponent field (es bits)
	unsigned expStart = Hfloat::nbits - 2;
	for (unsigned i = 0; i < es; ++i) {
		s << (number.getbit(expStart - i) ? '1' : '0');
	}
	s << '.';

	// fraction field (fbits bits, show in hex-digit groups)
	for (int i = static_cast<int>(Hfloat::fbits) - 1; i >= 0; --i) {
		s << (number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4 == 0)) s << '\'';
	}

	return s.str();
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_hex(const hfloat<ndigits, es, BlockType>& number) {
	std::stringstream s;

	s << (number.sign() ? '-' : '+');
	s << "0x0.";

	// extract hex digits from fraction
	bool sign_val; int exp_val; uint64_t frac_val;
	number.unpack(sign_val, exp_val, frac_val);

	for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
		unsigned hex_digit = (frac_val >> (i * 4)) & 0xF;
		s << "0123456789ABCDEF"[hex_digit];
	}
	s << " * 16^" << (exp_val);

	return s.str();
}


// native semantic representation: radix-16, delegates to to_hex
template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_native(const hfloat<ndigits, es, BlockType>& v, bool = false) {
	return to_hex(v);
}

////////////////////////    HFLOAT functions   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> abs(const hfloat<ndigits, es, BlockType>& a) {
	hfloat<ndigits, es, BlockType> result(a);
	result.setsign(false);
	return result;
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> fabs(hfloat<ndigits, es, BlockType> a) {
	a.setsign(false);
	return a;
}


////////////////////////  stream operators   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const hfloat<ndigits, es, BlockType>& i) {
	std::stringstream ss;
	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));
	return ostr << ss.str();
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::istream& operator>>(std::istream& istr, hfloat<ndigits, es, BlockType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an hfloat value\n";
	}
	return istr;
}

////////////////// string operators

template<unsigned ndigits, unsigned es, typename BlockType>
bool parse(const std::string& number, hfloat<ndigits, es, BlockType>& value) {
	bool bSuccess = false;
	// TODO: implement hex string parsing
	return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - hfloat binary logic operators

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator==(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	if (lhs.iszero() && rhs.iszero()) return true;
	// compare through double (constexpr after convert_to_double promotion)
	return double(lhs) == double(rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator!=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator< (const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return double(lhs) < double(rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator> (const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator<=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator>=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - literal binary logic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator==(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator==(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator!=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator< (const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator<(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator> (const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (hfloat<ndigits, es, BlockType>(rhs), lhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator<=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator>=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - hfloat binary logic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator==(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator==(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator!=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator< (const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator<(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator> (const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator<=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator>=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - hfloat binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator+(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator-(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator*(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator/(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - literal binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator+(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator+(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator-(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator-(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator*(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator*(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator/(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator/(lhs, hfloat<ndigits, es, BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - hfloat binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator+(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator+(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator-(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator-(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator*(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator*(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator/(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator/(hfloat<ndigits, es, BlockType>(lhs), rhs);
}

}} // namespace sw::universal
