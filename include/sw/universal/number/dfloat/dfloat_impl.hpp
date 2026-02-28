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
// blockbinary for encoding storage and significand arithmetic
#include <universal/internal/blockbinary/blockbinary.hpp>

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
// power_of_10: constexpr power-of-10 helpers
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

	// Significand arithmetic type: blockbinary with enough bits for any ndigits
	// Signed is required because blockbinary::longdivision() requires it.
	// The sign bit is unused headroom since significands are always >= 0.
	static constexpr unsigned sig_bits = 4 * ndigits + 8;
	using significand_t = blockbinary<sig_bits, bt, BinaryNumberType::Signed>;

	// Wide significand for overflow-free multiplication
	using wide_significand_t = blockbinary<2 * sig_bits, bt, BinaryNumberType::Signed>;

	// Helper: power of 10 returning significand_t
	static significand_t pow10_s(unsigned n) {
		significand_t result(1);
		significand_t ten(10);
		for (unsigned i = 0; i < n; ++i) result *= ten;
		return result;
	}

	// Helper: count decimal digits of a significand_t
	static unsigned count_digits_s(const significand_t& v) {
		if (v.iszero()) return 1;
		unsigned count = 0;
		significand_t tmp(v);
		significand_t ten(10);
		while (!tmp.iszero()) { tmp /= ten; ++count; }
		return count;
	}

	// Helper: significand_t to string
	static std::string sig_to_string(const significand_t& v) {
		return to_decimal(v);
	}

	typedef bt BlockType;

	// Encoding storage type: blockbinary with Unsigned encoding
	using encoding_t = blockbinary<nbits, bt, BinaryNumberType::Unsigned>;

	/// trivial constructor
	dfloat() = default;

	dfloat(const dfloat&) = default;
	dfloat(dfloat&&) = default;

	dfloat& operator=(const dfloat&) = default;
	dfloat& operator=(dfloat&&) = default;

	// converting constructors
	constexpr dfloat(const std::string& stringRep) { clear(); assign(stringRep); }

	// specific value constructor
	constexpr dfloat(const SpecificValue code) noexcept {
		clear();
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
	explicit dfloat(signed char iv)           noexcept { clear(); *this = iv; }
	explicit dfloat(short iv)                 noexcept { clear(); *this = iv; }
	explicit dfloat(int iv)                   noexcept { clear(); *this = iv; }
	explicit dfloat(long iv)                  noexcept { clear(); *this = iv; }
	explicit dfloat(long long iv)             noexcept { clear(); *this = iv; }
	explicit dfloat(char iv)                  noexcept { clear(); *this = iv; }
	explicit dfloat(unsigned short iv)        noexcept { clear(); *this = iv; }
	explicit dfloat(unsigned int iv)          noexcept { clear(); *this = iv; }
	explicit dfloat(unsigned long iv)         noexcept { clear(); *this = iv; }
	explicit dfloat(unsigned long long iv)    noexcept { clear(); *this = iv; }
	explicit dfloat(float iv)                 noexcept { clear(); *this = iv; }
	explicit dfloat(double iv)                noexcept { clear(); *this = iv; }

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
	explicit dfloat(long double iv)           noexcept { clear(); *this = iv; }
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
		significand_t lhs_sig, rhs_sig;
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
		int abs_shift = (shift >= 0) ? shift : -shift;

		// When the magnitude difference exceeds the precision, the smaller
		// operand cannot contribute any digits to the result -- short-circuit.
		if (abs_shift >= static_cast<int>(ndigits)) {
			if (shift > 0) return *this;       // lhs dominates
			*this = rhs; return *this;         // rhs dominates
		}

		int result_exp;
		bool result_sign;
		significand_t abs_sig;

		// Unified path using blockbinary significand_t
		significand_t aligned_lhs(lhs_sig);
		significand_t aligned_rhs(rhs_sig);
		significand_t ten(10);

		if (shift >= 0) {
			result_exp = rhs_exp;
			for (int i = 0; i < shift; ++i) aligned_lhs *= ten;
		}
		else {
			result_exp = lhs_exp;
			for (int i = 0; i < -shift; ++i) aligned_rhs *= ten;
		}

		if (lhs_sign == rhs_sign) {
			abs_sig = aligned_lhs + aligned_rhs;
			result_sign = lhs_sign;
		}
		else {
			if (aligned_lhs >= aligned_rhs) {
				abs_sig = aligned_lhs - aligned_rhs;
				result_sign = lhs_sign;
			}
			else {
				abs_sig = aligned_rhs - aligned_lhs;
				result_sign = rhs_sign;
			}
		}

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
		significand_t lhs_sig, rhs_sig;
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

		// Wide multiplication: urmul returns blockbinary<2*sig_bits>
		wide_significand_t wide = urmul(lhs_sig, rhs_sig);
		wide_significand_t ten_w(10);

		// Count digits in wide result and trim to ndigits
		// Use a helper to count digits of the wide result
		unsigned wd = 0;
		{
			wide_significand_t tmp(wide);
			if (tmp.iszero()) { wd = 1; }
			else { while (!tmp.iszero()) { tmp /= ten_w; ++wd; } }
		}
		while (wd > ndigits) {
			wide /= ten_w;
			result_exp++;
			wd--;
		}

		// Truncate wide result to significand_t
		significand_t result_sig;
		result_sig.assign(wide);

		normalize_and_pack(result_sign, result_exp, result_sig);
		return *this;
	}
	dfloat& operator/=(const dfloat& rhs) {
		bool lhs_sign, rhs_sign;
		int lhs_exp, rhs_exp;
		significand_t lhs_sig, rhs_sig;
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

		// Unified iterative long division using blockbinary
		significand_t remainder(lhs_sig);
		significand_t quotient(0);
		significand_t ten(10);
		for (unsigned i = 0; i < ndigits; ++i) {
			remainder *= ten;
			quotient = quotient * ten + remainder / rhs_sig;
			remainder = remainder % rhs_sig;
		}
		result_exp -= static_cast<int>(ndigits);

		normalize_and_pack(result_sign, result_exp, quotient);
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
		_encoding.clear();
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
		_encoding.setbits(value);
	}

	// create specific number system values of interest
	dfloat& maxpos() noexcept {
		clear();
		significand_t max_sig = pow10_s(ndigits) - significand_t(1);
		pack(false, emax, max_sig);
		return *this;
	}
	dfloat& minpos() noexcept {
		clear();
		pack(false, emin, significand_t(1));
		return *this;
	}
	dfloat& zero() noexcept {
		clear();
		return *this;
	}
	dfloat& minneg() noexcept {
		clear();
		pack(true, emin, significand_t(1));
		return *this;
	}
	dfloat& maxneg() noexcept {
		clear();
		significand_t max_sig = pow10_s(ndigits) - significand_t(1);
		pack(true, emax, max_sig);
		return *this;
	}

	dfloat& assign(const std::string& txt) {
		clear();
		if (txt.empty()) return *this;

		// Skip leading whitespace
		size_t pos = 0;
		while (pos < txt.size() && std::isspace(static_cast<unsigned char>(txt[pos]))) ++pos;
		if (pos >= txt.size()) return *this;

		// Check for sign
		bool negative = false;
		if (txt[pos] == '-') { negative = true; ++pos; }
		else if (txt[pos] == '+') { ++pos; }

		// Check for special values (case-insensitive)
		std::string rest = txt.substr(pos);
		if (rest.size() >= 3) {
			char c0 = static_cast<char>(std::tolower(static_cast<unsigned char>(rest[0])));
			char c1 = static_cast<char>(std::tolower(static_cast<unsigned char>(rest[1])));
			char c2 = static_cast<char>(std::tolower(static_cast<unsigned char>(rest[2])));
			if (c0 == 'i' && c1 == 'n' && c2 == 'f') { setinf(negative); return *this; }
			if (c0 == 'n' && c1 == 'a' && c2 == 'n') { setnan(NAN_TYPE_QUIET); return *this; }
		}

		// Parse decimal digits, collecting significand and tracking decimal point
		// Input forms: "123", "123.456", ".456", "123.", "123.456e-78", "123e5"
		significand_t sig(0);
		significand_t ten(10);
		unsigned digit_count = 0;
		int decimal_exponent = 0;
		bool seen_dot = false;
		int frac_digits = 0;

		// Parse integer and fractional parts
		while (pos < txt.size()) {
			char ch = txt[pos];
			if (ch == '.') {
				if (seen_dot) break; // second dot ends parsing
				seen_dot = true;
				++pos;
				continue;
			}
			if (ch >= '0' && ch <= '9') {
				if (digit_count < ndigits) {
					sig = sig * ten + significand_t(static_cast<long long>(ch - '0'));
					digit_count++;
				}
				else {
					// Beyond precision: count but don't store
					if (!seen_dot) decimal_exponent++;
				}
				if (seen_dot) frac_digits++;
				++pos;
				continue;
			}
			break; // non-digit, non-dot ends the mantissa
		}

		// The significand represents: sig * 10^(-frac_digits)
		// So the base exponent before any explicit exponent is -frac_digits
		decimal_exponent -= frac_digits;

		// Parse optional exponent: e/E followed by optional sign and digits
		if (pos < txt.size() && (txt[pos] == 'e' || txt[pos] == 'E')) {
			++pos;
			bool exp_neg = false;
			if (pos < txt.size() && txt[pos] == '-') { exp_neg = true; ++pos; }
			else if (pos < txt.size() && txt[pos] == '+') { ++pos; }

			int exp_val = 0;
			while (pos < txt.size() && txt[pos] >= '0' && txt[pos] <= '9') {
				exp_val = exp_val * 10 + (txt[pos] - '0');
				++pos;
			}
			decimal_exponent += exp_neg ? -exp_val : exp_val;
		}

		// Remove trailing zeros from significand (normalize)
		while (!sig.iszero() && digit_count > 1) {
			significand_t remainder = sig % ten;
			if (!remainder.iszero()) break;
			sig /= ten;
			decimal_exponent++;
			digit_count--;
		}

		if (sig.iszero()) {
			setzero();
			if (negative) setsign(true);
			return *this;
		}

		normalize_and_pack(negative, decimal_exponent, sig);
		return *this;
	}

	// selectors
	bool sign() const noexcept {
		return getbit(nbits - 1);
	}

	bool iszero() const noexcept {
		// zero when all bits except sign are 0
		// Check all bits except the sign bit (nbits-1)
		for (unsigned i = 0; i < nbits - 1; ++i) {
			if (_encoding.at(i)) return false;
		}
		return true;
	}

	bool isone() const noexcept {
		bool s; int e; significand_t sig;
		unpack(s, e, sig);
		return !s && (sig == significand_t(1)) && (e == 0);
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
		bool s; int e; significand_t sig;
		unpack(s, e, sig);
		// scale in powers of 10
		return e + static_cast<int>(count_digits_s(sig)) - 1;
	}

	// Format modes for str()
	enum class FmtMode { automatic, fixed, scientific };

	// convert to string
	// precision: number of significant digits (0 = ndigits)
	// mode: automatic (default), fixed, or scientific
	std::string str(size_t precision = 0, FmtMode mode = FmtMode::automatic) const {
		if (isnan()) return std::string("nan");
		if (isinf()) return sign() ? std::string("-inf") : std::string("inf");
		if (iszero()) return sign() ? std::string("-0") : std::string("0");

		bool s; int e; significand_t sig;
		unpack(s, e, sig);

		// value = (-1)^s * sig * 10^e
		std::string digits = sig_to_string(sig);
		int num_digits = static_cast<int>(digits.size());
		int decimal_pos = num_digits + e; // position of decimal point from left

		// Determine effective precision (number of significant digits to show)
		size_t prec = (precision > 0) ? precision : static_cast<size_t>(ndigits);
		// Trim digits to requested precision
		if (digits.size() > prec) {
			digits.resize(prec);
		}
		num_digits = static_cast<int>(digits.size());

		// Determine format mode
		// automatic: use scientific when the exponent would produce more than
		//            ndigits leading/trailing zeros, otherwise use fixed
		if (mode == FmtMode::automatic) {
			if (decimal_pos > static_cast<int>(ndigits) || decimal_pos < -static_cast<int>(ndigits / 2)) {
				mode = FmtMode::scientific;
			}
			else {
				mode = FmtMode::fixed;
			}
		}

		std::string result;
		if (s) result = "-";

		if (mode == FmtMode::scientific) {
			// Scientific notation: d.ddd...e+/-NNN
			result += digits[0];
			if (num_digits > 1) {
				result += '.';
				result += digits.substr(1);
			}
			// exponent = decimal_pos - 1 (since we placed decimal after first digit)
			int sci_exp = decimal_pos - 1;
			result += 'e';
			if (sci_exp >= 0) {
				result += '+';
			}
			result += std::to_string(sci_exp);
		}
		else {
			// Fixed notation
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
		}

		return result;
	}

	///////////////////////////////////////////////////////////////////
	// Bit access (public for free functions like to_binary, color_print)
	bool getbit(unsigned pos) const noexcept {
		if (pos >= nbits) return false;
		return _encoding.at(pos);
	}

	///////////////////////////////////////////////////////////////////
	// Unpacking / Packing helpers (public for testing)

	// Unpack the dfloat into sign, unbiased exponent, and significand integer
	void unpack(bool& s, int& exponent, significand_t& significand) const noexcept {
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

		// Extract trailing significand (t bits) using blockbinary
		if constexpr (encoding == DecimalEncoding::BID) {
			// Read trailing bits directly from encoding into a significand_t
			significand_t trailing(0);
			for (unsigned i = 0; i < t; ++i) {
				if (getbit(i)) trailing.setbit(i, true);
			}
			significand = significand_t(static_cast<long long>(msd)) * pow10_s(ndigits - 1) + trailing;
		}
		else {
			// DPD: decode declets from trailing bits
			significand = dpd_decode_trailing_wide(msd);
		}
	}

protected:
	encoding_t _encoding;

	///////////////////////////////////////////////////////////////////
	// Bit manipulation helpers
	void setbit(unsigned pos, bool value) noexcept {
		if (pos >= nbits) return;
		_encoding.setbit(pos, value);
	}

	///////////////////////////////////////////////////////////////////
	// Pack sign, unbiased exponent, and significand into the dfloat encoding
	void pack(bool s, int exponent, const significand_t& significand) noexcept {
		clear();
		if (significand.iszero()) return; // zero

		// Determine MSD and trailing
		significand_t msd_val = significand / pow10_s(ndigits - 1);
		unsigned msd = static_cast<unsigned>(static_cast<long long>(msd_val));

		unsigned biased_exp = static_cast<unsigned>(exponent + bias);

		// Encode sign
		setbit(nbits - 1, s);

		// Encode combination field
		unsigned exp_msbs = (biased_exp >> es) & 0x3u;
		unsigned combStart = nbits - 2;

		if (msd < 8) {
			setbit(combStart,     (exp_msbs >> 1) & 1);
			setbit(combStart - 1, exp_msbs & 1);
			setbit(combStart - 2, (msd >> 2) & 1);
			setbit(combStart - 3, (msd >> 1) & 1);
			setbit(combStart - 4, msd & 1);
		}
		else {
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
		if constexpr (encoding == DecimalEncoding::BID) {
			significand_t trailing = significand % pow10_s(ndigits - 1);
			// Extract bits from blockbinary significand_t and write into encoding
			for (unsigned i = 0; i < t; ++i) {
				setbit(i, trailing.at(i));
			}
		}
		else {
			// DPD encoding: encode and write declets directly into bits
			dpd_encode_trailing_wide(significand);
		}
	}

	///////////////////////////////////////////////////////////////////
	// Normalize significand to ndigits and pack
	void normalize_and_pack(bool s, int exponent, significand_t significand) noexcept {
		if (significand.iszero()) { setzero(); if (s) setsign(true); return; }

		// Normalize: ensure significand has exactly ndigits digits
		significand_t ten(10);
		unsigned digits = count_digits_s(significand);
		while (digits > ndigits) {
			significand /= ten;
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
	// DPD encode/decode helpers (unified for all widths)

	// DPD decode: read declets directly from encoding bits
	significand_t dpd_decode_trailing_wide(unsigned msd) const noexcept {
		significand_t result(0);
		significand_t multiplier(1);
		significand_t thousand(1000);
		unsigned remaining = ndigits - 1;
		unsigned bit_offset = 0;

		while (remaining >= 3) {
			// Read 10-bit declet from bit_offset
			uint16_t declet = 0;
			for (unsigned b = 0; b < 10; ++b) {
				if (getbit(bit_offset + b)) declet |= static_cast<uint16_t>(1u << b);
			}
			unsigned value = dpd_decode(declet);
			result += significand_t(static_cast<long long>(value)) * multiplier;
			multiplier *= thousand;
			bit_offset += 10;
			remaining -= 3;
		}

		return significand_t(static_cast<long long>(msd)) * pow10_s(ndigits - 1) + result;
	}

	// DPD encode: write declets directly into encoding bits
	void dpd_encode_trailing_wide(const significand_t& significand) noexcept {
		significand_t msd_factor = pow10_s(ndigits - 1);
		significand_t trailing_val = significand % msd_factor;
		significand_t thousand(1000);
		unsigned remaining = ndigits - 1;
		unsigned bit_offset = 0;

		while (remaining >= 3) {
			significand_t group_bb = trailing_val % thousand;
			unsigned group = static_cast<unsigned>(static_cast<long long>(group_bb));
			trailing_val /= thousand;
			uint16_t declet = dpd_encode(group);
			for (unsigned b = 0; b < 10; ++b) {
				setbit(bit_offset + b, (declet >> b) & 1);
			}
			bit_offset += 10;
			remaining -= 3;
		}
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
		// Double has ~15-17 significant digits, so the significand from double
		// always fits in uint64_t regardless of ndigits.
		int dec_exp = 0;
		if (abs_val != 0.0) {
			dec_exp = static_cast<int>(std::floor(std::log10(abs_val)));
		}

		// Scale to get min(ndigits, 17) significant digits (double precision limit)
		unsigned effective_digits = (ndigits < 17) ? ndigits : 17;
		int target_exp = dec_exp - static_cast<int>(effective_digits) + 1;
		double scaled = abs_val / std::pow(10.0, static_cast<double>(target_exp));
		uint64_t sig_narrow = static_cast<uint64_t>(std::round(scaled));

		// Adjust if rounding pushed us over
		uint64_t limit = pow10_64(effective_digits);
		if (sig_narrow >= limit) {
			sig_narrow /= 10;
			target_exp++;
		}
		// Remove trailing zeros
		while (sig_narrow > 0 && (sig_narrow % 10) == 0) {
			sig_narrow /= 10;
			target_exp++;
		}

		normalize_and_pack(negative, target_exp, significand_t(static_cast<long long>(sig_narrow)));
		return *this;
	}

	// Convert dfloat to native IEEE-754 double
	double convert_to_double() const noexcept {
		if (isnan()) return std::numeric_limits<double>::quiet_NaN();
		if (isinf()) return sign() ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
		if (iszero()) return sign() ? -0.0 : 0.0;

		bool s; int e; significand_t sig;
		unpack(s, e, sig);

		// value = (-1)^s * sig * 10^e
		// For ndigits <= 19, sig fits in uint64_t; for wider, use string conversion
		double sig_d;
		if constexpr (sig_bits <= 64) {
			sig_d = static_cast<double>(static_cast<unsigned long long>(sig));
		}
		else {
			// Use the string representation for wide significands
			std::string sig_str = to_decimal(sig);
			sig_d = std::strtod(sig_str.c_str(), nullptr);
		}
		double result = sig_d * std::pow(10.0, static_cast<double>(e));
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

		normalize_and_pack(negative, exponent, significand_t(static_cast<long long>(abs_v)));
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

		normalize_and_pack(false, exponent, significand_t(static_cast<long long>(v)));
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
	using Dfloat = dfloat<ndigits, es, Encoding, BlockType>;
	using FmtMode = typename Dfloat::FmtMode;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff = ostr.flags();

	// Map iostream format flags to dfloat FmtMode
	FmtMode mode = FmtMode::automatic;
	bool scientific = (ff & std::ios_base::scientific) == std::ios_base::scientific;
	bool fixed      = (ff & std::ios_base::fixed) == std::ios_base::fixed;
	if (scientific && !fixed) mode = FmtMode::scientific;
	else if (fixed && !scientific) mode = FmtMode::fixed;

	// Default to ndigits precision so all stored digits are shown.
	// The iostream default precision is 6, which would silently truncate
	// exact decimal digits. Only use the stream precision when the user
	// has explicitly set scientific or fixed mode.
	size_t effective_prec = (scientific || fixed)
		? static_cast<size_t>(prec)
		: 0;  // 0 tells str() to use ndigits

	std::string representation = i.str(effective_prec, mode);

	// Handle setw and alignment
	std::streamsize repWidth = static_cast<std::streamsize>(representation.size());
	if (width > repWidth) {
		std::streamsize diff = width - repWidth;
		char fill = ostr.fill();
		if ((ff & std::ios_base::left) == std::ios_base::left) {
			representation.append(static_cast<size_t>(diff), fill);
		}
		else {
			representation.insert(0, static_cast<size_t>(diff), fill);
		}
	}

	return ostr << representation;
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
	if (number.empty()) return false;
	value.assign(number);
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// dfloat - dfloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline bool operator==(const dfloat<ndigits, es, Encoding, BlockType>& lhs, const dfloat<ndigits, es, Encoding, BlockType>& rhs) {
	using Dfloat = dfloat<ndigits, es, Encoding, BlockType>;
	// NaN != anything (including itself)
	if (lhs.isnan() || rhs.isnan()) return false;
	// both zero (ignoring sign)
	if (lhs.iszero() && rhs.iszero()) return true;
	// compare unpacked values
	bool ls, rs; int le, re;
	typename Dfloat::significand_t lsig, rsig;
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
	using Dfloat = dfloat<ndigits, es, Encoding, BlockType>;
	// NaN is unordered
	if (lhs.isnan() || rhs.isnan()) return false;
	// handle infinities
	if (lhs.isinf() && rhs.isinf()) {
		return lhs.sign() && !rhs.sign(); // -inf < +inf
	}
	if (lhs.isinf()) return lhs.sign();  // -inf < anything
	if (rhs.isinf()) return !rhs.sign(); // anything < +inf

	// handle zeros
	if (lhs.iszero() && rhs.iszero()) return false;
	if (lhs.iszero()) return !rhs.sign(); // 0 < positive
	if (rhs.iszero()) return lhs.sign();  // negative < 0

	// both nonzero, non-special
	bool ls = lhs.sign(), rs = rhs.sign();
	if (ls != rs) return ls; // negative < positive

	// same sign: compare magnitudes
	bool ls_ign; int le; typename Dfloat::significand_t lsig;
	bool rs_ign; int re; typename Dfloat::significand_t rsig;
	lhs.unpack(ls_ign, le, lsig);
	rhs.unpack(rs_ign, re, rsig);

	// normalize to same scale for comparison
	int l_scale = le + static_cast<int>(Dfloat::count_digits_s(lsig)) - 1;
	int r_scale = re + static_cast<int>(Dfloat::count_digits_s(rsig)) - 1;

	if (l_scale != r_scale) {
		// higher scale means larger magnitude
		return ls ? (l_scale > r_scale) : (l_scale < r_scale);
	}

	// same overall scale: compare significands at same exponent
	// Align to same exponent by adjusting significands
	typename Dfloat::significand_t ten(10);
	if (le < re) {
		int diff = re - le;
		if (diff < static_cast<int>(ndigits)) {
			for (int i = 0; i < diff; ++i) rsig *= ten;
		}
	}
	else if (re < le) {
		int diff = le - re;
		if (diff < static_cast<int>(ndigits)) {
			for (int i = 0; i < diff; ++i) lsig *= ten;
		}
	}

	return ls ? (lsig > rsig) : (lsig < rsig);
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
