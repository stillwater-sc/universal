// truncate.hpp: truncation and rounding functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <limits>
#include <type_traits>

namespace sw { namespace universal {

// The helpers below take a significand's 64-bit limbs, least significant first, its top bit
// weighing 2^exp (#1820): its bit i counted from the top weighs 2^(exp - i).

// the significand's bit i counted from the top, 0 past its end
template<typename Limbs>
constexpr bool significand_bit(const Limbs& limbs, int64_t i) noexcept {
	const int64_t total = 64 * static_cast<int64_t>(limbs.size());
	if (i < 0 || i >= total) return false;
	const int64_t p = total - 1 - i;
	return ((limbs[static_cast<size_t>(p / 64)] >> (p % 64)) & 1u) != 0u;
}

// whether any bit counted from the top at index first or past it is set
template<typename Limbs>
constexpr bool significand_bits_from(const Limbs& limbs, int64_t first) noexcept {
	const int64_t total = 64 * static_cast<int64_t>(limbs.size());
	if (first <= 0) first = 0;
	if (first >= total) return false;
	const int64_t below = total - first;   // the bits at bottom positions [0, below)
	const size_t  full  = static_cast<size_t>(below / 64);
	for (size_t i = 0; i < full; ++i) {
		if (limbs[i] != 0u) return true;
	}
	const unsigned rest = static_cast<unsigned>(below % 64);
	return rest != 0u && (limbs[full] & ((uint64_t(1) << rest) - 1u)) != 0u;
}

// clears the fractional bits of a significand whose top bit weighs 2^exp, and returns whether
// any were nonzero
template<typename Limbs>
constexpr bool clear_fractional_limbs(Limbs& limbs, int64_t exp) noexcept {
	if (exp < 0) {
		bool non_zero_frac = false;
		for (size_t i = 0; i < limbs.size(); ++i) {
			if (limbs[i] != 0u) {
				non_zero_frac = true;
				break;
			}
		}
		limbs.assign(1, 0u);
		return non_zero_frac;
	}

	const int64_t total_bits = 64 * static_cast<int64_t>(limbs.size());
	if (exp >= total_bits - 1) {
		return false; // no fractional bits exist!
	}

	const int64_t  cut  = total_bits - 1 - exp;   // the fractional bits, at the bottom
	const size_t   full = static_cast<size_t>(cut / 64);
	const unsigned rest = static_cast<unsigned>(cut % 64);
	bool non_zero_frac = false;
	for (size_t i = 0; i < full; ++i) {
		if (limbs[i] != 0u) non_zero_frac = true;
		limbs[i] = 0u;
	}
	if (rest != 0u) {
		const uint64_t mask = (uint64_t(1) << rest) - 1u;
		if ((limbs[full] & mask) != 0u) non_zero_frac = true;
		limbs[full] &= ~mask;
	}
	return non_zero_frac;
}

// adds 1 to the integer part of a significand whose top bit weighs 2^exp, exp >= 0. Returns
// true if a carry out of the top added a limb, whose bit 0 then weighs 2^(exp + 1): the caller
// adds 64 to the exponent, and normalize() moves the leading bit back to the top.
template<typename Limbs>
constexpr bool add_one_integer_magnitude(Limbs& limbs, int64_t exp) noexcept {
	const int64_t p = 64 * static_cast<int64_t>(limbs.size()) - 1 - exp;   // the units bit
	size_t   i     = static_cast<size_t>(p / 64);
	uint64_t carry = uint64_t(1) << (p % 64);
	for (; carry != 0u && i < limbs.size(); ++i) {
		limbs[i] += carry;
		carry = (limbs[i] < carry) ? 1u : 0u;
	}
	if (carry != 0u) {
		limbs.resize(limbs.size() + 1);
		limbs.back() = 1u;
		return true;
	}
	return false;
}

// trunc: round toward zero
template<unsigned nlimbs>
constexpr efloat<nlimbs> trunc(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	efloat<nlimbs> res(x);
	bool original_sign = res._sign;
	bool inexact = clear_fractional_limbs(res._limb, res._exponent);
	if (inexact && !std::is_constant_evaluated()) {
		efloat_exception_flags.set(ExceptionFlag::Inexact);
	}
	res.normalize();
	if (res.iszero()) {
		res.setsign(original_sign); // preserve negative zero
	}
	return res;
}

// floor: round downward (toward -infinity)
template<unsigned nlimbs>
constexpr efloat<nlimbs> floor(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	efloat<nlimbs> res(x);
	bool original_sign = res._sign;
	int64_t exp = res._exponent;
	bool inexact = clear_fractional_limbs(res._limb, exp);
	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		if (x._sign) {
			// Negative downward rounding adds 1 to magnitude
			if (exp < 0) {
				res._limb.assign(1, uint64_t(1) << 63);
				res._exponent = 0;
			} else {
				if (add_one_integer_magnitude(res._limb, exp)) {
					res.setexponent(res.scale() + 64);
				}
			}
		}
	}
	res.normalize();
	if (res.iszero()) {
		res.setsign(original_sign); // preserve negative zero
	}
	return res;
}

// ceil: round upward (toward +infinity)
template<unsigned nlimbs>
constexpr efloat<nlimbs> ceil(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	efloat<nlimbs> res(x);
	bool original_sign = res._sign;
	int64_t exp = res._exponent;
	bool inexact = clear_fractional_limbs(res._limb, exp);
	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		if (!x._sign) {
			// Positive upward rounding adds 1 to magnitude
			if (exp < 0) {
				res._limb.assign(1, uint64_t(1) << 63);
				res._exponent = 0;
			} else {
				if (add_one_integer_magnitude(res._limb, exp)) {
					res.setexponent(res.scale() + 64);
				}
			}
		}
	}
	res.normalize();
	if (res.iszero()) {
		res.setsign(original_sign); // preserve negative zero
	}
	return res;
}

// round: round to nearest, halfway cases rounded away from zero
template<unsigned nlimbs>
constexpr efloat<nlimbs> round(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	int64_t exp = x._exponent;
	const auto& limbs = x._limb;
	const int64_t total_bits = 64 * static_cast<int64_t>(limbs.size());
	if (exp >= total_bits - 1) {
		return x; // already exactly an integer!
	}

	// the half bit: below the units bit, or the leading bit itself for |x| in [1/2, 1)
	const bool guard = (exp < 0) ? (exp == -1) : significand_bit(limbs, exp + 1);

	efloat<nlimbs> res(x);
	bool original_sign = res._sign;
	bool inexact = clear_fractional_limbs(res._limb, exp);
	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		// Round away from zero if guard is set (fraction is >= 0.5)
		if (guard) {
			if (exp < 0) {
				res._limb.assign(1, uint64_t(1) << 63);
				res._exponent = 0;
			} else {
				if (add_one_integer_magnitude(res._limb, exp)) {
					res.setexponent(res.scale() + 64);
				}
			}
		}
	}
	res.normalize();
	if (res.iszero()) {
		res.setsign(original_sign); // preserve negative zero
	}
	return res;
}

// rint / nearbyint: round to nearest, according to the active efloat_rounding_mode
template<unsigned nlimbs>
constexpr efloat<nlimbs> rint(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	int64_t exp = x._exponent;
	const auto& limbs = x._limb;
	const int64_t total_bits = 64 * static_cast<int64_t>(limbs.size());
	if (exp >= total_bits - 1) {
		return x; // already exactly an integer!
	}

	bool lsb = false;
	bool guard = false;
	bool sticky = false;

	if (exp < 0) {
		guard = (exp == -1);
		lsb = false; // integer part is 0 (even!)
		sticky = significand_bits_from(limbs, 1);   // the bits below the leading one
	} else {
		lsb    = significand_bit(limbs, exp);
		guard  = significand_bit(limbs, exp + 1);
		sticky = significand_bits_from(limbs, exp + 2);
	}

	efloat<nlimbs> res(x);
	bool original_sign = res._sign;
	bool inexact = clear_fractional_limbs(res._limb, exp);

	bool round_up = false;
	switch (efloat<nlimbs>::rounding_mode()) {   // to nearest in a constant evaluation
	case RoundingMode::RoundToNearest:
		round_up = guard && (sticky || lsb);
		break;
	case RoundingMode::RoundToZero:
		break;
	case RoundingMode::RoundTowardPositive:
		round_up = (!x._sign) && inexact;
		break;
	case RoundingMode::RoundTowardNegative:
		round_up = (x._sign) && inexact;
		break;
	}

	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		if (round_up) {
			if (exp < 0) {
				res._limb.assign(1, uint64_t(1) << 63);
				res._exponent = 0;
			} else {
				if (add_one_integer_magnitude(res._limb, exp)) {
					res.setexponent(res.scale() + 64);
				}
			}
		}
	}
	res.normalize();
	if (res.iszero()) {
		res.setsign(original_sign); // preserve negative zero
	}
	return res;
}

template<unsigned nlimbs>
constexpr efloat<nlimbs> nearbyint(const efloat<nlimbs>& x) {
	// nearbyint is same as rint but does not raise the Inexact exception
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	uint32_t saved_exceptions = 0;
	if (!std::is_constant_evaluated()) {
		saved_exceptions = efloat_exception_flags.get();
	}
	efloat<nlimbs> res = rint(x);
	if (!std::is_constant_evaluated()) {
		efloat_exception_flags.set_state(saved_exceptions); // restore exceptions (suppress inexact!)
	}
	return res;
}

// lrint / llrint: round to nearest-even and return long / long long (lossless, no double cast!)
template<unsigned nlimbs>
constexpr long long llrint_impl(const efloat<nlimbs>& x) noexcept {
	efloat<nlimbs> r = rint(x);
	if (r.isnan() || r.iszero() || r.scale() < 0) return 0LL;
	
	int64_t exp = r.scale();
	if (exp >= 63) {
		return (r.sign() == -1 ? std::numeric_limits<long long>::min() : std::numeric_limits<long long>::max());
	}

	const uint64_t raw_sig = r.limb_data()[r.limb_count() - 1];   // the top 64 bits

	uint64_t val = raw_sig >> (63 - exp);
	if (r.sign() == -1) {
		return -static_cast<long long>(val);
	} else {
		return static_cast<long long>(val);
	}
}

template<unsigned nlimbs>
constexpr long lrint(const efloat<nlimbs>& x) {
	long long val = llrint_impl(x);
	if (val < std::numeric_limits<long>::min()) return std::numeric_limits<long>::min();
	if (val > std::numeric_limits<long>::max()) return std::numeric_limits<long>::max();
	return static_cast<long>(val);
}

template<unsigned nlimbs>
constexpr long long llrint(const efloat<nlimbs>& x) {
	return llrint_impl(x);
}

}} // namespace sw::universal
