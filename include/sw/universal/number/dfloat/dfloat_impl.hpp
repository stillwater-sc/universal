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

#ifdef __SIZEOF_INT128__
// __uint128_t power-of-10 helper for decimal128 (34 digits; 10^38 < 2^128)
static constexpr __uint128_t pow10_128(unsigned n) {
	// build iteratively since we can't have a constexpr __uint128_t array easily
	__uint128_t result = 1;
	for (unsigned i = 0; i < n; ++i) result *= 10;
	return result;
}

// count decimal digits of a __uint128_t
static constexpr unsigned count_decimal_digits_wide(__uint128_t v) {
	if (v == 0) return 1;
	unsigned d = 0;
	while (v > 0) { v /= 10; ++d; }
	return d;
}

// convert __uint128_t to decimal string
static inline std::string uint128_to_string(__uint128_t v) {
	if (v == 0) return "0";
	std::string result;
	while (v > 0) {
		result += static_cast<char>('0' + static_cast<unsigned>(v % 10));
		v /= 10;
	}
	std::reverse(result.begin(), result.end());
	return result;
}
#endif

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

	// Significand type: uint64_t for ndigits <= 19, __uint128_t for ndigits <= 38
#ifdef __SIZEOF_INT128__
	static constexpr unsigned max_ndigits = 38;
	using significand_t = std::conditional_t<(ndigits <= 19), uint64_t, __uint128_t>;
#else
	static constexpr unsigned max_ndigits = 19;
	using significand_t = uint64_t;
#endif
	static_assert(ndigits <= max_ndigits,
		"dfloat: ndigits exceeds significand capacity");

	// Helper: power of 10 returning significand_t
	static constexpr significand_t pow10_s(unsigned n) {
		if constexpr (ndigits <= 19) {
			return pow10_64(n);
		}
		else {
#ifdef __SIZEOF_INT128__
			return pow10_128(n);
#else
			return pow10_64(n); // unreachable due to static_assert
#endif
		}
	}

	// Helper: count decimal digits of a significand_t
	static constexpr unsigned count_digits_s(significand_t v) {
		if constexpr (ndigits <= 19) {
			return count_decimal_digits(static_cast<uint64_t>(v));
		}
		else {
#ifdef __SIZEOF_INT128__
			return count_decimal_digits_wide(v);
#else
			return count_decimal_digits(static_cast<uint64_t>(v));
#endif
		}
	}

	// Helper: significand_t to string
	static std::string sig_to_string(significand_t v) {
		if constexpr (ndigits <= 19) {
			return std::to_string(static_cast<uint64_t>(v));
		}
		else {
#ifdef __SIZEOF_INT128__
			return uint128_to_string(v);
#else
			return std::to_string(static_cast<uint64_t>(v));
#endif
		}
	}

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

		if constexpr (ndigits <= 19) {
			// For ndigits <= 19: scale up into __uint128_t (or fallback uint64_t)
#ifdef __SIZEOF_INT128__
			__uint128_t aligned_lhs = static_cast<__uint128_t>(lhs_sig);
			__uint128_t aligned_rhs = static_cast<__uint128_t>(rhs_sig);
			if (shift >= 0) {
				result_exp = rhs_exp;
				for (int i = 0; i < shift; ++i) aligned_lhs *= 10;
			}
			else {
				result_exp = lhs_exp;
				for (int i = 0; i < -shift; ++i) aligned_rhs *= 10;
			}

			__uint128_t abs_wide;
			if (lhs_sign == rhs_sign) {
				abs_wide = aligned_lhs + aligned_rhs;
				result_sign = lhs_sign;
			}
			else {
				if (aligned_lhs >= aligned_rhs) {
					abs_wide = aligned_lhs - aligned_rhs;
					result_sign = lhs_sign;
				}
				else {
					abs_wide = aligned_rhs - aligned_lhs;
					result_sign = rhs_sign;
				}
			}

			while (abs_wide > UINT64_MAX) {
				abs_wide /= 10;
				result_exp++;
			}
			abs_sig = static_cast<uint64_t>(abs_wide);
#else
			// Fallback without __uint128_t: only safe for ndigits <= 9
			uint64_t aligned_lhs_u = lhs_sig;
			uint64_t aligned_rhs_u = rhs_sig;
			if (shift >= 0) {
				result_exp = rhs_exp;
				for (int i = 0; i < shift; ++i) aligned_lhs_u *= 10;
			}
			else {
				result_exp = lhs_exp;
				for (int i = 0; i < -shift; ++i) aligned_rhs_u *= 10;
			}

			int64_t a_val = lhs_sign ? -static_cast<int64_t>(aligned_lhs_u) : static_cast<int64_t>(aligned_lhs_u);
			int64_t b_val = rhs_sign ? -static_cast<int64_t>(aligned_rhs_u) : static_cast<int64_t>(aligned_rhs_u);
			int64_t result_sig_i = a_val + b_val;

			result_sign = (result_sig_i < 0);
			abs_sig = static_cast<uint64_t>(result_sign ? -result_sig_i : result_sig_i);
#endif
		}
		else {
#ifdef __SIZEOF_INT128__
			// For ndigits > 19 (decimal128): scale UP the higher-exponent
			// operand's significand. Max sum ~ 2*10^34 < 10^38, fits __uint128_t.
			// The scale-up factor is at most 10^(ndigits-1) since we short-circuit
			// when abs_shift >= ndigits, so max aligned value is < 10^(2*ndigits-1)
			// which for ndigits=34 is 10^67 — too large! Instead, scale UP but check:
			// significand max is ~10^34, scaled up by 10^(ndigits-1) = 10^33 gives 10^67.
			// This overflows __uint128_t (max ~3.4*10^38).
			//
			// Solution: scale up the higher-exponent operand only up to what fits,
			// and scale down the lower-exponent operand for the remainder.
			significand_t aligned_lhs = lhs_sig;
			significand_t aligned_rhs = rhs_sig;

			// Max safe scaling: __uint128_t holds up to ~3.4*10^38
			// A significand has at most ndigits digits, so max value ~ 10^ndigits
			// Safe scale-up amount: 38 - ndigits = 38 - 34 = 4 digits
			// But we need the SUM to fit too, so be conservative: 38 - ndigits - 1 = 3
			constexpr int safe_scale_up = 38 - static_cast<int>(ndigits) - 1;

			if (shift >= 0) {
				result_exp = rhs_exp;
				if (shift <= safe_scale_up) {
					// Safe to scale up entirely
					for (int i = 0; i < shift; ++i) aligned_lhs *= 10;
				}
				else {
					// Scale up as much as safely possible, scale down the rest
					for (int i = 0; i < safe_scale_up; ++i) aligned_lhs *= 10;
					int remaining = shift - safe_scale_up;
					for (int i = 0; i < remaining; ++i) aligned_rhs /= 10;
					result_exp += remaining;
				}
			}
			else {
				result_exp = lhs_exp;
				int neg_shift = -shift;
				if (neg_shift <= safe_scale_up) {
					for (int i = 0; i < neg_shift; ++i) aligned_rhs *= 10;
				}
				else {
					for (int i = 0; i < safe_scale_up; ++i) aligned_rhs *= 10;
					int remaining = neg_shift - safe_scale_up;
					for (int i = 0; i < remaining; ++i) aligned_lhs /= 10;
					result_exp += remaining;
				}
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
#else
			// unreachable: static_assert blocks ndigits > 19 without __uint128_t
			result_sign = false;
			abs_sig = 0;
			result_exp = 0;
#endif
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
		significand_t result_sig;

		if constexpr (ndigits <= 19) {
#ifdef __SIZEOF_INT128__
			__uint128_t wide = static_cast<__uint128_t>(lhs_sig) * static_cast<__uint128_t>(rhs_sig);
			while (wide > UINT64_MAX) {
				wide /= 10;
				result_exp++;
			}
			result_sig = static_cast<uint64_t>(wide);
#else
			double d = double(*this) * double(rhs);
			*this = d;
			return *this;
#endif
		}
		else {
#ifdef __SIZEOF_INT128__
			// For ndigits > 19: use long multiplication.
			// Multiply lhs_sig by each decimal digit of rhs_sig, accumulating
			// with appropriate shifts. Since lhs_sig * 9 < 10^35 < 10^38,
			// each intermediate fits __uint128_t.
			//
			// Alternative simpler approach: extract digits of rhs, multiply and accumulate.
			// But even simpler: use the iterative approach analogous to division.
			//
			// We split rhs into individual digits and do schoolbook multiplication.
			// Or: split into two halves, each <= 17 digits.
			constexpr unsigned half = (ndigits + 1) / 2; // 17 for ndigits=34
			significand_t divisor = pow10_s(half);
			significand_t a_hi = lhs_sig / divisor;
			significand_t a_lo = lhs_sig % divisor;
			significand_t b_hi = rhs_sig / divisor;
			significand_t b_lo = rhs_sig % divisor;

			// Each partial product: max ~ 10^17 * 10^17 = 10^34, fits __uint128_t
			significand_t p_hh = a_hi * b_hi;  // contributes at position 2*half
			significand_t p_hl = a_hi * b_lo;  // contributes at position half
			significand_t p_lh = a_lo * b_hi;  // contributes at position half
			significand_t p_ll = a_lo * b_lo;  // contributes at position 0

			// We need the top ndigits significant digits of:
			// result = p_hh * 10^(2*half) + (p_hl + p_lh) * 10^half + p_ll
			//
			// Strategy: accumulate from MSB. Start with p_hh, which represents
			// the most significant part. Its exponent contribution is 2*half.
			// Then add contributions from mid and low terms.

			// First, combine p_ll with carry into mid
			significand_t p_mid = p_hl + p_lh;  // max ~ 2*10^34, fits
			// carry from p_ll into p_mid: p_ll / 10^half
			significand_t p_ll_carry = p_ll / divisor;
			p_mid += p_ll_carry;

			// carry from p_mid into p_hh
			significand_t p_mid_carry = p_mid / divisor;
			p_hh += p_mid_carry;
			p_mid = p_mid % divisor;

			// Now result = p_hh * 10^(2*half) + p_mid * 10^half + (p_ll % divisor)
			// p_hh has the most significant digits. We want top ndigits digits.

			if (p_hh > 0) {
				unsigned hh_digits = count_digits_s(p_hh);
				if (hh_digits >= ndigits) {
					result_sig = p_hh;
					result_exp += static_cast<int>(2 * half);
				}
				else {
					unsigned need = ndigits - hh_digits;
					result_sig = p_hh;
					if (need <= half) {
						for (unsigned i = 0; i < need; ++i) result_sig *= 10;
						unsigned mid_digits = count_digits_s(p_mid);
						significand_t mid_top = p_mid;
						if (mid_digits > need) {
							for (unsigned i = 0; i < mid_digits - need; ++i) mid_top /= 10;
						}
						else {
							for (unsigned i = mid_digits; i < need; ++i) mid_top *= 10;
						}
						result_sig += mid_top;
						result_exp += static_cast<int>(2 * half - need);
					}
					else {
						for (unsigned i = 0; i < half; ++i) result_sig *= 10;
						result_sig += p_mid;
						unsigned still_need = need - half;
						significand_t p_ll_rem = p_ll % divisor;
						unsigned ll_digits = count_digits_s(p_ll_rem);
						unsigned cur_digits = count_digits_s(result_sig);
						if (cur_digits + still_need <= 38) {
							for (unsigned i = 0; i < still_need; ++i) result_sig *= 10;
							significand_t ll_top = p_ll_rem;
							if (ll_digits > still_need) {
								for (unsigned i = 0; i < ll_digits - still_need; ++i) ll_top /= 10;
							}
							else {
								for (unsigned i = ll_digits; i < still_need; ++i) ll_top *= 10;
							}
							result_sig += ll_top;
						}
						result_exp += static_cast<int>(half - still_need);
					}
				}
			}
			else if (p_mid > 0) {
				result_sig = p_mid;
				result_exp += static_cast<int>(half);
			}
			else {
				result_sig = p_ll;
			}
#else
			result_sig = 0;
			result_exp = 0;
#endif
		}

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
		significand_t result_sig;

		if constexpr (ndigits <= 19) {
#ifdef __SIZEOF_INT128__
			__uint128_t scaled_num = static_cast<__uint128_t>(lhs_sig) * static_cast<__uint128_t>(pow10_64(ndigits));
			__uint128_t q = scaled_num / static_cast<__uint128_t>(rhs_sig);
			result_exp -= static_cast<int>(ndigits);
			while (q > UINT64_MAX) {
				q /= 10;
				result_exp++;
			}
			result_sig = static_cast<uint64_t>(q);
#else
			double d = double(*this) / double(rhs);
			*this = d;
			return *this;
#endif
		}
		else {
#ifdef __SIZEOF_INT128__
			// For ndigits > 19: iterative long division to avoid overflow.
			// Compute quotient digit-by-digit.
			significand_t remainder = lhs_sig;
			significand_t quotient = 0;
			for (unsigned i = 0; i < ndigits; ++i) {
				remainder *= 10;            // max ~ 10^35, fits __uint128_t
				quotient = quotient * 10 + remainder / rhs_sig;
				remainder = remainder % rhs_sig;
			}
			result_exp -= static_cast<int>(ndigits);
			result_sig = quotient;
#else
			result_sig = 0;
			result_exp = 0;
#endif
		}

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
		unsigned blocks_to_set = (nrBlocks < (64 / bitsInBlock + 1)) ? nrBlocks : static_cast<unsigned>(64 / bitsInBlock + 1);
		for (unsigned i = 0; i < blocks_to_set; ++i) {
			_block[i] = bt(value & BLOCK_MASK);
			value >>= bitsInBlock;
		}
		_block[MSU] &= MSU_MASK;
	}

#ifdef __SIZEOF_INT128__
	// wider overload for decimal128 and other wide formats
	inline void setbits(__uint128_t value) noexcept {
		clear();
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(static_cast<uint64_t>(value) & BLOCK_MASK);
			value >>= bitsInBlock;
		}
		_block[MSU] &= MSU_MASK;
	}
#endif

	// create specific number system values of interest
	constexpr dfloat& maxpos() noexcept {
		clear();
		pack(false, emax, pow10_s(ndigits) - 1);
		return *this;
	}
	constexpr dfloat& minpos() noexcept {
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
		pack(true, emax, pow10_s(ndigits) - 1);
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
		bool s; int e; significand_t sig;
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
		bool s; int e; significand_t sig;
		unpack(s, e, sig);
		// scale in powers of 10
		return e + static_cast<int>(count_digits_s(sig)) - 1;
	}

	// convert to string containing digits number of digits
	std::string str(size_t nrDigits = 0) const {
		if (isnan()) return std::string("nan");
		if (isinf()) return sign() ? std::string("-inf") : std::string("inf");
		if (iszero()) return sign() ? std::string("-0") : std::string("0");

		bool s; int e; significand_t sig;
		unpack(s, e, sig);

		// value = (-1)^s * sig * 10^e
		std::string digits = sig_to_string(sig);
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

		// Extract trailing significand (t bits)
		// For t <= 64, a single uint64_t suffices.
		// For t > 64 (e.g., decimal128 BID has t=110), read in two passes.
		if constexpr (encoding == DecimalEncoding::BID) {
			significand_t trailing = 0;
			if constexpr (t <= 64) {
				uint64_t trail_lo = 0;
				for (unsigned i = 0; i < t; ++i) {
					if (getbit(i)) trail_lo |= (1ull << i);
				}
				trailing = static_cast<significand_t>(trail_lo);
			}
			else {
				// Read low 64 bits, then remaining high bits
				uint64_t trail_lo = 0;
				for (unsigned i = 0; i < 64; ++i) {
					if (getbit(i)) trail_lo |= (1ull << i);
				}
				uint64_t trail_hi = 0;
				for (unsigned i = 64; i < t; ++i) {
					if (getbit(i)) trail_hi |= (1ull << (i - 64));
				}
#ifdef __SIZEOF_INT128__
				trailing = (static_cast<significand_t>(trail_hi) << 64) | static_cast<significand_t>(trail_lo);
#else
				trailing = static_cast<significand_t>(trail_lo);
#endif
			}
			significand = static_cast<significand_t>(msd) * pow10_s(ndigits - 1) + trailing;
		}
		else {
			// DPD: decode declets from trailing bits
			if constexpr (t <= 64) {
				uint64_t trailing = 0;
				for (unsigned i = 0; i < t; ++i) {
					if (getbit(i)) trailing |= (1ull << i);
				}
				significand = dpd_decode_trailing(msd, trailing);
			}
			else {
				// For wide DPD: read trailing bits into significand_t
				// DPD declets are 10 bits each, decoded from LSB
				significand = dpd_decode_trailing_wide(msd);
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
	// Pack sign, unbiased exponent, and significand into the dfloat encoding
	constexpr void pack(bool s, int exponent, significand_t significand) noexcept {
		clear();
		if (significand == 0) return; // zero

		// Determine MSD and trailing
		unsigned msd = static_cast<unsigned>(significand / pow10_s(ndigits - 1));

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
			if constexpr (t <= 64) {
				uint64_t trail_val = static_cast<uint64_t>(trailing);
				for (unsigned i = 0; i < t; ++i) {
					setbit(i, (trail_val >> i) & 1);
				}
			}
			else {
				// Wide trailing: write low 64 bits, then high bits
#ifdef __SIZEOF_INT128__
				uint64_t trail_lo = static_cast<uint64_t>(trailing);
				uint64_t trail_hi = static_cast<uint64_t>(trailing >> 64);
				for (unsigned i = 0; i < 64; ++i) {
					setbit(i, (trail_lo >> i) & 1);
				}
				for (unsigned i = 64; i < t; ++i) {
					setbit(i, (trail_hi >> (i - 64)) & 1);
				}
#else
				uint64_t trail_val = static_cast<uint64_t>(trailing);
				for (unsigned i = 0; i < t; ++i) {
					setbit(i, (trail_val >> i) & 1);
				}
#endif
			}
		}
		else {
			// DPD encoding
			if constexpr (t <= 64) {
				uint64_t trailing = dpd_encode_trailing(significand);
				for (unsigned i = 0; i < t; ++i) {
					setbit(i, (trailing >> i) & 1);
				}
			}
			else {
				// Wide DPD: encode and write declets directly into bits
				dpd_encode_trailing_wide(significand);
			}
		}
	}

	///////////////////////////////////////////////////////////////////
	// Normalize significand to ndigits and pack
	void normalize_and_pack(bool s, int exponent, significand_t significand) noexcept {
		if (significand == 0) { setzero(); if (s) setsign(true); return; }

		// Normalize: ensure significand has exactly ndigits digits
		unsigned digits = count_digits_s(significand);
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
	static significand_t dpd_decode_trailing(unsigned msd, uint64_t trailing) noexcept {
		// Decode DPD trailing bits to get the lower (ndigits-1) decimal digits
		uint64_t lower = dpd_decode_significand(trailing, ndigits);
		return static_cast<significand_t>(msd) * pow10_s(ndigits - 1) + static_cast<significand_t>(lower);
	}
	static uint64_t dpd_encode_trailing(significand_t significand) noexcept {
		// Encode the trailing (ndigits-1) decimal digits into DPD format
		// For narrow significands, delegate directly
		if constexpr (ndigits <= 19) {
			return dpd_encode_significand(static_cast<uint64_t>(significand), ndigits);
		}
		else {
			// For wide significands, extract trailing digits into a narrower form
			// The trailing (ndigits-1) digits fit in the DPD bit field
			significand_t msd_factor = pow10_s(ndigits - 1);
			significand_t trailing_val = significand % msd_factor;
			// Encode groups of 3 digits into 10-bit declets
			uint64_t result = 0;
			unsigned shift_bits = 0;
			unsigned remaining = ndigits - 1;
			while (remaining >= 3) {
				unsigned group = static_cast<unsigned>(trailing_val % 1000);
				trailing_val /= 1000;
				uint16_t declet = dpd_encode(group);
				result |= (static_cast<uint64_t>(declet) << shift_bits);
				shift_bits += 10;
				remaining -= 3;
			}
			return result;
		}
	}

	// Wide DPD decode: read declets directly from bits for t > 64
	significand_t dpd_decode_trailing_wide(unsigned msd) const noexcept {
		significand_t result = 0;
		significand_t multiplier = 1;
		unsigned remaining = ndigits - 1;
		unsigned bit_offset = 0;

		while (remaining >= 3) {
			// Read 10-bit declet from bit_offset
			uint16_t declet = 0;
			for (unsigned b = 0; b < 10; ++b) {
				if (getbit(bit_offset + b)) declet |= static_cast<uint16_t>(1u << b);
			}
			unsigned value = dpd_decode(declet);
			result += static_cast<significand_t>(value) * multiplier;
			multiplier *= 1000;
			bit_offset += 10;
			remaining -= 3;
		}

		return static_cast<significand_t>(msd) * pow10_s(ndigits - 1) + result;
	}

	// Wide DPD encode: write declets directly into bits for t > 64
	void dpd_encode_trailing_wide(significand_t significand) noexcept {
		significand_t msd_factor = pow10_s(ndigits - 1);
		significand_t trailing_val = significand % msd_factor;
		unsigned remaining = ndigits - 1;
		unsigned bit_offset = 0;

		while (remaining >= 3) {
			unsigned group = static_cast<unsigned>(trailing_val % 1000);
			trailing_val /= 1000;
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

		normalize_and_pack(negative, target_exp, static_cast<significand_t>(sig_narrow));
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

		normalize_and_pack(negative, exponent, static_cast<significand_t>(abs_v));
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

		normalize_and_pack(false, exponent, static_cast<significand_t>(v));
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
	if (le < re) {
		int diff = re - le;
		if (diff < static_cast<int>(ndigits)) {
			for (int i = 0; i < diff; ++i) rsig *= 10;
		}
	}
	else if (re < le) {
		int diff = le - re;
		if (diff < static_cast<int>(ndigits)) {
			for (int i = 0; i < diff; ++i) lsig *= 10;
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
