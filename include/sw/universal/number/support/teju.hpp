#pragma once
// teju.hpp: C++ implementation of the Teju Jagua algorithm for shortest decimal representation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The algorithm implemented here is based on the Teju Jagua project
// by Cassio Neri (https://github.com/cassioneri/teju_jagua),
// licensed under the Apache License, Version 2.0.
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2025 Cassio Neri <cassio.neri@gmail.com>
//
// Teju Jagua converts a binary floating-point value (mantissa, exponent) into
// the shortest decimal representation (decimal_mantissa, decimal_exponent) that
// round-trips back to the original binary value.
//
// This implementation supports two computation widths:
//   width=32: for types with mantissa <= 24 bits (e.g., IEEE float)
//   width=64: for types with mantissa <= 53 bits (e.g., IEEE double)
#include <cstdint>
#include <cstring>
#include <universal/number/support/teju_tables.hpp>

namespace sw { namespace universal { namespace teju {

// Result of Teju Jagua conversion
struct decimal_fp {
	uint64_t mantissa;  // decimal mantissa with no trailing zeros
	int      exponent;  // decimal exponent: value = mantissa * 10^exponent
};

namespace detail {

// floor(log10(2^e)) via fixed-point approximation
// Uses the constant 1292913987 / 2^32 ~ 0.30103 ~ log10(2)
inline constexpr int log10_pow2(int e) {
	return static_cast<int>((static_cast<int64_t>(1292913987) * e) >> 32);
}

// Residual for sub-interval positioning within the exponent range.
// Computes the fractional part of e * log10(2) mapped to a shift value (0..3).
// Reference: Teju Jagua common.h — uses low-32-bits-of-product / constant.
inline constexpr unsigned log10_pow2_residual(int e) {
	return static_cast<uint32_t>(static_cast<int64_t>(1292913987) * e) / 1292913987u;
}

// ============================================================================
// Width-32 specialization
// ============================================================================

// Multiply-and-shift: compute floor(m * M / 2^64)
// M is a 64-bit value stored as {lower, upper} limbs.
inline constexpr uint32_t mshift32(uint32_t m, const multiplier_t<uint32_t>& M) {
	uint64_t upper = static_cast<uint64_t>(M.limbs[1]);
	uint64_t lower = static_cast<uint64_t>(M.limbs[0]);
	// Reconstruct 64-bit multiplier and compute full product
	uint64_t hi = static_cast<uint64_t>(m) * upper;
	uint64_t lo = static_cast<uint64_t>(m) * lower;
	uint64_t carry = lo >> 32;
	return static_cast<uint32_t>((hi + carry) >> 32);
}

// Compute M * 2^k >> 64 (used in uncentred case where m_c is a power of 2)
inline constexpr uint32_t mshift_pow2_32(unsigned k, const multiplier_t<uint32_t>& M) {
	uint32_t u = M.limbs[1];
	uint32_t l = M.limbs[0];
	int s = static_cast<int>(k) - 32;
	if (s <= 0)
		return u >> (-s);
	return (u << s) | (l >> (32 - static_cast<unsigned>(s)));
}

// Fast division by 10 for 32-bit values
inline constexpr uint32_t div10_32(uint32_t n) {
	return static_cast<uint32_t>((static_cast<uint64_t>(n) * 0xCCCCCCCDULL) >> 35);
}

// Check if n is a multiple of 5^f using modular inverse
inline constexpr bool is_multiple_of_pow5_32(uint32_t n, int f) {
	if (f < 0 || static_cast<unsigned>(f) >= ieee32_config::minverse_count) return false;
	uint32_t rotated = n * ieee32_minverse[static_cast<unsigned>(f)].multiplier;
	return rotated <= ieee32_minverse[static_cast<unsigned>(f)].bound;
}

// Can we test divisibility by 5^f?
inline constexpr bool allows_ties_32(int f) {
	return f >= 0 && static_cast<unsigned>(f) < ieee32_config::minverse_count;
}

// Is this a tie case? (value falls exactly on interval boundary)
inline constexpr bool is_tie_32(int f, uint32_t m) {
	return allows_ties_32(f) && is_multiple_of_pow5_32(m, f);
}

// Is this an uncentred tie? (additional m%5==0 check)
inline constexpr bool is_tie_uncentred_32(int f, uint32_t m) {
	return m % 5u == 0 && is_tie_32(f, m);
}

// Ties-to-even: even mantissa wins the tiebreak
inline constexpr bool wins_tiebreak(uint64_t m) {
	return m % 2u == 0;
}

// Is c_2 closer to the left candidate?
inline constexpr bool is_closer_to_left(uint64_t c_2) {
	return c_2 % 2u == 0;
}

// Remove trailing zeros from decimal mantissa
inline constexpr decimal_fp remove_trailing_zeros_32(int f, uint32_t m) {
	for (;;) {
		uint32_t q = div10_32(m);
		if (q * 10u != m) break;
		m = q;
		++f;
	}
	return { m, f };
}

// Core Teju Jagua conversion for width=32
// Input: binary mantissa m and binary exponent e, where value = m * 2^e
//        mantissa_width p (number of significand bits including hidden bit)
// Output: decimal_fp {mantissa, exponent} where value = mantissa * 10^exponent
inline decimal_fp convert32(uint32_t m, int e, unsigned p) {
	// Small integer fast path
	if (e <= 0 && static_cast<unsigned>(-e) < p) {
		auto shift = static_cast<unsigned>(-e);
		if ((m & ((1u << shift) - 1)) == 0) {
			return remove_trailing_zeros_32(0, m >> shift);
		}
	}

	int f = log10_pow2(e);
	unsigned r = log10_pow2_residual(e);
	int idx = f - ieee32_config::index_offset;
	const auto& M = ieee32_multipliers[static_cast<unsigned>(idx)];

	uint32_t const mantissa_uncentred = 1u << (p - 1);
	bool is_uncentred_val = (m == mantissa_uncentred) && (e != ieee32_config::exponent_min);

	if (!is_uncentred_val) {
		// Centred case
		uint32_t m_b = (2u * m + 1u) << r;
		uint32_t m_a = (2u * m - 1u) << r;

		uint32_t b = mshift32(m_b, M);
		uint32_t a = mshift32(m_a, M);
		uint32_t q = div10_32(b);
		uint32_t s = 10u * q;

		if (allows_ties_32(f)) {
			bool shortest =
				s == b ? !is_tie_32(f, m_b) || wins_tiebreak(m) :
				s == a ?  is_tie_32(f, m_a) && wins_tiebreak(m) :
				/*else*/ s > a;
			if (shortest)
				return remove_trailing_zeros_32(f + 1, q);
		}
		else if (s > a) {
			return remove_trailing_zeros_32(f + 1, q);
		}

		// Full precision path
		uint32_t m_c = 4u * m << r;
		uint32_t c_2 = mshift32(m_c, M);
		uint32_t c = c_2 / 2u;
		bool pick_left = (is_tie_32(-f, c_2) && wins_tiebreak(c)) ||
			is_closer_to_left(c_2);

		return { static_cast<uint64_t>(c + (pick_left ? 0u : 1u)), f };
	}
	else {
		// Uncentred case: m == 2^(p-1), predecessor is in different binade
		uint32_t m_a = (4u * m - 1u) << r;
		uint32_t m_b = (2u * m + 1u) << r;

		uint32_t b = mshift32(m_b, M);
		uint32_t a = mshift32(m_a, M) / 2u;
		uint32_t q = div10_32(b);
		uint32_t s = 10u * q;

		if (a < b) {
			// Sorted case
			if (allows_ties_32(f)) {
				bool shortest =
					s == b ? !is_tie_uncentred_32(f, m_b) || wins_tiebreak(m) :
					s == a ?  is_tie_uncentred_32(f, m_a) && wins_tiebreak(m) :
					/*else*/ s > a;
				if (shortest)
					return remove_trailing_zeros_32(f + 1, q);
			}
			else if (s > a) {
				return remove_trailing_zeros_32(f + 1, q);
			}

			// Full precision using mshift_pow2 (since m_c = 4 * 2^(p-1) = 2^(p+1))
			unsigned log2_m_c = p + r + 1u;
			uint32_t c_2 = mshift_pow2_32(log2_m_c, M);
			uint32_t c = c_2 / 2u;

			if (c == a && !is_tie_uncentred_32(f, m_a))
				return { static_cast<uint64_t>(c + 1u), f };

			bool pick_left = (is_tie_32(-f, c_2) && wins_tiebreak(c)) ||
				is_closer_to_left(c_2);
			return { static_cast<uint64_t>(c + (pick_left ? 0u : 1u)), f };
		}
		else {
			// Unsorted case (a >= b)
			if (is_tie_uncentred_32(f, m_a) && wins_tiebreak(m))
				return remove_trailing_zeros_32(f, a);

			uint32_t m_c2 = 40u * m << r;
			uint32_t c_2 = mshift32(m_c2, M);
			uint32_t c = c_2 / 2u;
			bool pick_left = (is_tie_32(-(f - 1), c_2) && wins_tiebreak(c)) ||
				is_closer_to_left(c_2);
			return { static_cast<uint64_t>(c + (pick_left ? 0u : 1u)), f - 1 };
		}
	}
}

// ============================================================================
// Width-64 specialization
// ============================================================================

#ifdef __SIZEOF_INT128__
using uint128_t = __uint128_t;
#else
// Fallback: synthetic 128-bit multiply for platforms without __uint128_t
struct uint128_t {
	uint64_t lo, hi;
};
inline uint128_t mul64(uint64_t a, uint64_t b) {
	uint64_t a_lo = a & 0xFFFFFFFFULL;
	uint64_t a_hi = a >> 32;
	uint64_t b_lo = b & 0xFFFFFFFFULL;
	uint64_t b_hi = b >> 32;
	uint64_t p0 = a_lo * b_lo;
	uint64_t p1 = a_lo * b_hi;
	uint64_t p2 = a_hi * b_lo;
	uint64_t p3 = a_hi * b_hi;
	uint64_t mid = (p0 >> 32) + (p1 & 0xFFFFFFFFULL) + (p2 & 0xFFFFFFFFULL);
	uint64_t hi = p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
	return { (p0 & 0xFFFFFFFFULL) | (mid << 32), hi };
}
#endif

// Multiply-and-shift for width=64
// Computes floor(m * M / 2^128) where M = {lower, upper} is 128 bits
inline uint64_t mshift64(uint64_t m, const multiplier_t<uint64_t>& M) {
#ifdef __SIZEOF_INT128__
	uint128_t full_upper = static_cast<uint128_t>(m) * M.limbs[1];
	uint128_t full_lower = static_cast<uint128_t>(m) * M.limbs[0];
	uint64_t carry = static_cast<uint64_t>(full_lower >> 64);
	uint128_t result = full_upper + carry;
	return static_cast<uint64_t>(result >> 64);
#else
	auto hi_prod = mul64(m, M.limbs[1]);
	auto lo_prod = mul64(m, M.limbs[0]);
	uint64_t carry = lo_prod.hi;
	uint64_t sum_lo = hi_prod.lo + carry;
	uint64_t sum_hi = hi_prod.hi + (sum_lo < hi_prod.lo ? 1ULL : 0ULL);
	return sum_hi;
#endif
}

// Compute M * 2^k >> 128 (used in uncentred case)
inline uint64_t mshift_pow2_64(unsigned k, const multiplier_t<uint64_t>& M) {
	uint64_t u = M.limbs[1];
	uint64_t l = M.limbs[0];
	int s = static_cast<int>(k) - 64;
	if (s <= 0)
		return u >> (-s);
	return (u << s) | (l >> (64 - static_cast<unsigned>(s)));
}

// Fast division by 10 for 64-bit values
inline constexpr uint64_t div10_64(uint64_t n) {
#ifdef __SIZEOF_INT128__
	return static_cast<uint64_t>((static_cast<uint128_t>(n) * 0xCCCCCCCCCCCCCCCDULL) >> 67);
#else
	return n / 10;
#endif
}

// Check if n is a multiple of 5^f using modular inverse
inline constexpr bool is_multiple_of_pow5_64(uint64_t n, int f) {
	if (f < 0 || static_cast<unsigned>(f) >= ieee64_config::minverse_count) return false;
	uint64_t rotated = n * ieee64_minverse[static_cast<unsigned>(f)].multiplier;
	return rotated <= ieee64_minverse[static_cast<unsigned>(f)].bound;
}

inline constexpr bool allows_ties_64(int f) {
	return f >= 0 && static_cast<unsigned>(f) < ieee64_config::minverse_count;
}

inline constexpr bool is_tie_64(int f, uint64_t m) {
	return allows_ties_64(f) && is_multiple_of_pow5_64(m, f);
}

inline constexpr bool is_tie_uncentred_64(int f, uint64_t m) {
	return m % 5u == 0 && is_tie_64(f, m);
}

// Remove trailing zeros from decimal mantissa
inline constexpr decimal_fp remove_trailing_zeros_64(int f, uint64_t m) {
	for (;;) {
		uint64_t q = div10_64(m);
		if (q * 10u != m) break;
		m = q;
		++f;
	}
	return { m, f };
}

// Core Teju Jagua conversion for width=64
inline decimal_fp convert64(uint64_t m, int e, unsigned p) {
	// Small integer fast path
	if (e <= 0 && static_cast<unsigned>(-e) < p) {
		auto shift = static_cast<unsigned>(-e);
		if ((m & ((1ULL << shift) - 1)) == 0) {
			return remove_trailing_zeros_64(0, m >> shift);
		}
	}

	int f = log10_pow2(e);
	unsigned r = log10_pow2_residual(e);
	int idx = f - ieee64_config::index_offset;
	const auto& M = ieee64_multipliers[static_cast<unsigned>(idx)];

	uint64_t const mantissa_uncentred = 1ULL << (p - 1);
	bool is_uncentred_val = (m == mantissa_uncentred) && (e != ieee64_config::exponent_min);

	if (!is_uncentred_val) {
		// Centred case
		uint64_t m_b = (2ULL * m + 1ULL) << r;
		uint64_t m_a = (2ULL * m - 1ULL) << r;

		uint64_t b = mshift64(m_b, M);
		uint64_t a = mshift64(m_a, M);
		uint64_t q = div10_64(b);
		uint64_t s = 10ULL * q;

		if (allows_ties_64(f)) {
			bool shortest =
				s == b ? !is_tie_64(f, m_b) || wins_tiebreak(m) :
				s == a ?  is_tie_64(f, m_a) && wins_tiebreak(m) :
				/*else*/ s > a;
			if (shortest)
				return remove_trailing_zeros_64(f + 1, q);
		}
		else if (s > a) {
			return remove_trailing_zeros_64(f + 1, q);
		}

		// Full precision path
		uint64_t m_c = 4ULL * m << r;
		uint64_t c_2 = mshift64(m_c, M);
		uint64_t c = c_2 / 2u;
		bool pick_left = (is_tie_64(-f, c_2) && wins_tiebreak(c)) ||
			is_closer_to_left(c_2);

		return { c + (pick_left ? 0ULL : 1ULL), f };
	}
	else {
		// Uncentred case
		uint64_t m_a = (4ULL * m - 1ULL) << r;
		uint64_t m_b = (2ULL * m + 1ULL) << r;

		uint64_t b = mshift64(m_b, M);
		uint64_t a = mshift64(m_a, M) / 2u;
		uint64_t q = div10_64(b);
		uint64_t s = 10ULL * q;

		if (a < b) {
			// Sorted case
			if (allows_ties_64(f)) {
				bool shortest =
					s == b ? !is_tie_uncentred_64(f, m_b) || wins_tiebreak(m) :
					s == a ?  is_tie_uncentred_64(f, m_a) && wins_tiebreak(m) :
					/*else*/ s > a;
				if (shortest)
					return remove_trailing_zeros_64(f + 1, q);
			}
			else if (s > a) {
				return remove_trailing_zeros_64(f + 1, q);
			}

			unsigned log2_m_c = p + r + 1u;
			uint64_t c_2 = mshift_pow2_64(log2_m_c, M);
			uint64_t c = c_2 / 2u;

			if (c == a && !is_tie_uncentred_64(f, m_a))
				return { c + 1ULL, f };

			bool pick_left = (is_tie_64(-f, c_2) && wins_tiebreak(c)) ||
				is_closer_to_left(c_2);
			return { c + (pick_left ? 0ULL : 1ULL), f };
		}
		else {
			// Unsorted case (a >= b)
			if (is_tie_uncentred_64(f, m_a) && wins_tiebreak(m))
				return remove_trailing_zeros_64(f, a);

			uint64_t m_c2 = 40ULL * m << r;
			uint64_t c_2 = mshift64(m_c2, M);
			uint64_t c = c_2 / 2u;
			bool pick_left = (is_tie_64(-(f - 1), c_2) && wins_tiebreak(c)) ||
				is_closer_to_left(c_2);
			return { c + (pick_left ? 0ULL : 1ULL), f - 1 };
		}
	}
}

} // namespace detail

// ============================================================================
// Public API
// ============================================================================

/// Convert a binary floating-point value to its shortest decimal representation.
/// @param mantissa  Full binary significand (with implicit 1 bit set for normals)
/// @param exponent  Binary exponent such that value = mantissa * 2^exponent
/// @param mantissa_width  Total number of significand bits (including hidden bit)
/// @return decimal_fp with the shortest decimal mantissa and exponent
///
/// Preconditions: mantissa > 0 (value must be finite and positive)
/// The caller handles sign, zero, NaN, and infinity.
inline decimal_fp to_decimal(uint64_t mantissa, int exponent, unsigned mantissa_width) {
	if (mantissa_width <= 24) {
		return detail::convert32(static_cast<uint32_t>(mantissa), exponent, mantissa_width);
	}
	else {
		return detail::convert64(mantissa, exponent, mantissa_width);
	}
}

/// Convenience: convert a native float to shortest decimal
inline decimal_fp float_to_decimal(float value) {
	static_assert(sizeof(float) == 4, "Expected 32-bit float");
	uint32_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	uint32_t mantissa = bits & 0x7FFFFFu;
	int biased_exp = static_cast<int>((bits >> 23) & 0xFFu);
	if (biased_exp != 0) {
		// Normal: set implicit bit
		mantissa |= (1u << 23);
		biased_exp -= 1;
	}
	int exponent = biased_exp + (-149); // exponent_min for IEEE32
	return detail::convert32(mantissa, exponent, 24);
}

/// Convenience: convert a native double to shortest decimal
inline decimal_fp double_to_decimal(double value) {
	static_assert(sizeof(double) == 8, "Expected 64-bit double");
	uint64_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	uint64_t mantissa = bits & 0xFFFFFFFFFFFFFULL;
	int biased_exp = static_cast<int>((bits >> 52) & 0x7FFu);
	if (biased_exp != 0) {
		// Normal: set implicit bit
		mantissa |= (1ULL << 52);
		biased_exp -= 1;
	}
	int exponent = biased_exp + (-1074); // exponent_min for IEEE64
	return detail::convert64(mantissa, exponent, 53);
}

}}} // namespace sw::universal::teju
