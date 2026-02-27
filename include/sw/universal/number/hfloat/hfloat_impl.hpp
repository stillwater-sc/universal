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
	explicit hfloat(signed char iv)           noexcept : _block{} { *this = iv; }
	explicit hfloat(short iv)                 noexcept : _block{} { *this = iv; }
	explicit hfloat(int iv)                   noexcept : _block{} { *this = iv; }
	explicit hfloat(long iv)                  noexcept : _block{} { *this = iv; }
	explicit hfloat(long long iv)             noexcept : _block{} { *this = iv; }
	explicit hfloat(char iv)                  noexcept : _block{} { *this = iv; }
	explicit hfloat(unsigned short iv)        noexcept : _block{} { *this = iv; }
	explicit hfloat(unsigned int iv)          noexcept : _block{} { *this = iv; }
	explicit hfloat(unsigned long iv)         noexcept : _block{} { *this = iv; }
	explicit hfloat(unsigned long long iv)    noexcept : _block{} { *this = iv; }
	explicit hfloat(float iv)                 noexcept : _block{} { *this = iv; }
	explicit hfloat(double iv)                noexcept : _block{} { *this = iv; }

	// assignment operators for native types
	hfloat& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	hfloat& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	hfloat& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	hfloat& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	hfloat& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	hfloat& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	hfloat& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	hfloat& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	hfloat& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	hfloat& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	hfloat& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	hfloat& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float()           const noexcept { return float(convert_to_double()); }
	explicit operator double()          const noexcept { return convert_to_double(); }

#if LONG_DOUBLE_SUPPORT
	explicit hfloat(long double iv)           noexcept : _block{} { *this = iv; }
	hfloat& operator=(long double rhs)        noexcept { return convert_ieee754(double(rhs)); }
	explicit operator long double()     const noexcept { return (long double)convert_to_double(); }
#endif

	// prefix operators
	hfloat operator-() const {
		hfloat negated(*this);
		if (!negated.iszero()) {
			negated.setsign(!negated.sign());
		}
		return negated;
	}

	// arithmetic operators
	hfloat& operator+=(const hfloat& rhs) {
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

	hfloat& operator-=(const hfloat& rhs) {
		hfloat neg(rhs);
		if (!neg.iszero()) neg.setsign(!neg.sign());
		return operator+=(neg);
	}

	hfloat& operator*=(const hfloat& rhs) {
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
#else
		// fallback: delegate through double
		double d = double(*this) * double(rhs);
		*this = d;
		return *this;
#endif

		normalize_and_pack(result_sign, result_exp, result_frac);
		return *this;
	}

	hfloat& operator/=(const hfloat& rhs) {
		if (rhs.iszero()) {
#if HFLOAT_THROW_ARITHMETIC_EXCEPTION
			throw hfloat_divide_by_zero();
#else
			setzero();
			return *this;
#endif
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
#else
		double d = double(*this) / double(rhs);
		*this = d;
		return *this;
#endif

		normalize_and_pack(result_sign, result_exp, result_frac);
		return *this;
	}

	// unary operators
	hfloat& operator++() {
		*this += hfloat(1);
		return *this;
	}
	hfloat operator++(int) {
		hfloat tmp(*this);
		operator++();
		return tmp;
	}
	hfloat& operator--() {
		*this -= hfloat(1);
		return *this;
	}
	hfloat operator--(int) {
		hfloat tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	void clear() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) _block[i] = bt(0);
	}
	void setzero() noexcept { clear(); }

	void setsign(bool negative = true) noexcept {
		setbit(nbits - 1, negative);
	}

	// use un-interpreted raw bits to set the value
	inline void setbits(uint64_t value) noexcept {
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
	bool sign() const noexcept {
		return getbit(nbits - 1);
	}

	bool iszero() const noexcept {
		// zero when fraction is all zeros (exponent and sign don't matter)
		// In IBM HFP, zero is represented as all-zeros
		for (unsigned i = 0; i < nrBlocks; ++i) {
			bt mask = (i == MSU) ? bt(MSU_MASK & ~(bt(1) << ((nbits - 1) % bitsInBlock))) : BLOCK_MASK;
			if ((_block[i] & mask) != 0) return false;
		}
		return true;
	}

	bool isone() const noexcept {
		bool s; int e; uint64_t f;
		unpack(s, e, f);
		// 1.0 = 0.1 * 16^1, so e=1, f = 1 << (fbits-4)  (leading hex digit = 1)
		return !s && (e == 1) && (f == (1ull << (fbits - 4)));
	}

	bool ispos() const noexcept { return !sign(); }
	bool isneg() const noexcept { return sign(); }

	// IBM HFP has no NaN or infinity
	bool isinf() const noexcept { return false; }
	bool isnan() const noexcept { return false; }
	bool isnan(int) const noexcept { return false; }

	int scale() const noexcept {
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
	bool getbit(unsigned pos) const noexcept {
		if (pos >= nbits) return false;
		unsigned block_idx = pos / bitsInBlock;
		unsigned bit_idx = pos % bitsInBlock;
		return (_block[block_idx] >> bit_idx) & 1;
	}

	///////////////////////////////////////////////////////////////////
	// Unpack into sign, unbiased exponent, and fraction
	void unpack(bool& s, int& exponent, uint64_t& fraction) const noexcept {
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
	void normalize_and_pack(bool s, int exponent, uint64_t fraction) noexcept {
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
	hfloat& convert_ieee754(double rhs) noexcept {
		if (std::isnan(rhs) || rhs == 0.0) {
			setzero();
			return *this;
		}
		if (std::isinf(rhs)) {
			if (rhs > 0) maxpos(); else maxneg();
			return *this;
		}

		bool negative = (rhs < 0);
		double abs_val = std::fabs(rhs);

		// Convert to hex floating-point: value = 0.f * 16^e
		// First get binary exponent
		int bin_exp;
		double frac = std::frexp(abs_val, &bin_exp);
		// frac is in [0.5, 1.0), bin_exp is such that abs_val = frac * 2^bin_exp

		// Convert to base-16 exponent
		// We need hex_exp = ceil(bin_exp / 4) so the fraction 0.f fits in [1/16, 1)
		// For positive bin_exp: ceil(n/4) = (n+3)/4
		// For negative bin_exp: ceil(n/4) in C integer arithmetic
		int hex_exp;
		if (bin_exp > 0) {
			hex_exp = (bin_exp + 3) / 4;
		}
		else {
			// For bin_exp <= 0: ceil(bin_exp / 4)
			// C integer division truncates toward zero, which is ceil for negative values
			hex_exp = bin_exp / 4;
			// But we need to ensure we don't undershoot
		}

		// Compute fraction: abs_val / 16^hex_exp, then scale to fbits
		// fraction = abs_val * 16^(-hex_exp) * 2^fbits
		// = frac * 2^bin_exp * 16^(-hex_exp) * 2^fbits
		// = frac * 2^(bin_exp - 4*hex_exp + fbits)
		int shift = bin_exp - 4 * hex_exp + static_cast<int>(fbits);
		uint64_t fraction;
		if (shift >= 0 && shift < 64) {
			fraction = static_cast<uint64_t>(std::ldexp(frac, shift));
		}
		else if (shift >= 64) {
			fraction = (1ull << fbits) - 1; // saturate
		}
		else {
			fraction = 0;
		}

		// Truncate to fbits (IBM HFP truncation rounding)
		fraction &= ((1ull << fbits) - 1);

		normalize_and_pack(negative, hex_exp, fraction);
		return *this;
	}

	// Convert hfloat to IEEE-754 double
	double convert_to_double() const noexcept {
		if (iszero()) return sign() ? -0.0 : 0.0;

		bool s; int e; uint64_t f;
		unpack(s, e, f);

		// value = 0.f * 16^e = f * 2^(-fbits) * 16^e = f * 2^(4*e - fbits)
		double result = std::ldexp(static_cast<double>(f), 4 * e - static_cast<int>(fbits));
		return s ? -result : result;
	}

	hfloat& convert_signed(int64_t v) noexcept {
		if (v == 0) {
			setzero();
			return *this;
		}
		// Use double conversion for simplicity
		return convert_ieee754(static_cast<double>(v));
	}

	hfloat& convert_unsigned(uint64_t v) noexcept {
		if (v == 0) {
			setzero();
			return *this;
		}
		return convert_ieee754(static_cast<double>(v));
	}

private:

	// hfloat - hfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend bool operator==(const hfloat<N, E, B>& lhs, const hfloat<N, E, B>& rhs);

	// hfloat - literal logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend bool operator==(const hfloat<N, E, B>& lhs, const double rhs);

	// literal - hfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend bool operator==(const double lhs, const hfloat<N, E, B>& rhs);
};


////////////////////////    helper functions   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_binary(const hfloat<ndigits, es, BlockType>& number, bool nibbleMarker = false) {
	using Hfloat = hfloat<ndigits, es, BlockType>;
	std::stringstream s;

	// sign bit
	s << (number.sign() ? '1' : '0') << '.';

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


////////////////////////    HFLOAT functions   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> abs(const hfloat<ndigits, es, BlockType>& a) {
	hfloat<ndigits, es, BlockType> result(a);
	result.setsign(false);
	return result;
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> fabs(hfloat<ndigits, es, BlockType> a) {
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
inline bool operator==(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	if (lhs.iszero() && rhs.iszero()) return true;
	// compare through double
	return double(lhs) == double(rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator!=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator< (const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return double(lhs) < double(rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator> (const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator<=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator>=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - literal binary logic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator==(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator==(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator!=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator< (const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator<(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator> (const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (hfloat<ndigits, es, BlockType>(rhs), lhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator<=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator>=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - hfloat binary logic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator==(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator==(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator!=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator< (const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator<(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator> (const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator<=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline bool operator>=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - hfloat binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator+(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator-(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator*(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator/(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - literal binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator+(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator+(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator-(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator-(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator*(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator*(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator/(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator/(lhs, hfloat<ndigits, es, BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - hfloat binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator+(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator+(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator-(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator-(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator*(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator*(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline hfloat<ndigits, es, BlockType> operator/(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator/(hfloat<ndigits, es, BlockType>(lhs), rhs);
}

}} // namespace sw::universal
