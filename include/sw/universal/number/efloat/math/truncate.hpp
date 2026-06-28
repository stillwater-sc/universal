// truncate.hpp: truncation and rounding functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

namespace sw { namespace universal {

// Helper to clear fractional bits of a limb vector and return if any non-zero bits were cleared
static constexpr bool clear_fractional_limbs(std::vector<uint32_t>& limbs, int64_t exp) noexcept {
	if (exp < 0) {
		bool non_zero_frac = false;
		for (uint32_t v : limbs) {
			if (v != 0) {
				non_zero_frac = true;
				break;
			}
		}
		limbs.assign(1, 0u);
		return non_zero_frac;
	}

	size_t total_bits = limbs.size() * 32;
	if (exp >= static_cast<int64_t>(total_bits) - 1) {
		return false; // no fractional bits exist!
	}

	bool non_zero_frac = false;
	size_t full_limbs = static_cast<size_t>(exp / 32);
	unsigned bit_idx = static_cast<unsigned>(exp % 32);
	
	// Clear fractional bits of the boundary limb
	uint32_t mask = (1u << (31u - bit_idx)) - 1u;
	if ((limbs[full_limbs] & mask) != 0) {
		non_zero_frac = true;
	}
	limbs[full_limbs] &= ~mask;

	// Clear and check subsequent limbs
	for (size_t i = full_limbs + 1; i < limbs.size(); ++i) {
		if (limbs[i] != 0) {
			non_zero_frac = true;
		}
		limbs[i] = 0;
	}
	return non_zero_frac;
}

// Helper to add 1 to the magnitude of the integer part of limbs at the LSB position
// Returns true if a carry-out occurred that grew the vector
static constexpr bool add_one_integer_magnitude(std::vector<uint32_t>& limbs, int64_t exp) noexcept {
	if (exp < 0) {
		// exp < 0 means the integer part is 0, so adding 1 makes it 1.0!
		limbs.assign(1, 0x80000000u);
		return false;
	}
	size_t full_limbs = static_cast<size_t>(exp / 32);
	unsigned bit_idx = static_cast<unsigned>(exp % 32);

	uint64_t carry = 1ULL << (31u - bit_idx);
	for (int i = static_cast<int>(full_limbs); i >= 0; --i) {
		uint64_t sum = uint64_t(limbs[i]) + carry;
		limbs[i] = static_cast<uint32_t>(sum);
		carry = sum >> 32;
		if (!carry) break;
	}
	if (carry) {
		// Carry-out from the MSB! Insert 1 at index 0 (which normalize will handle!)
		limbs.insert(limbs.begin(), 1u);
		return true;
	}
	return false;
}

// trunc: round toward zero
template<unsigned nlimbs>
constexpr efloat<nlimbs> trunc(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	efloat<nlimbs> res(x);
	bool inexact = clear_fractional_limbs(res._limb, res.scale());
	if (inexact && !std::is_constant_evaluated()) {
		efloat_exception_flags.set(ExceptionFlag::Inexact);
	}
	res.normalize();
	return res;
}

// floor: round downward (toward -infinity)
template<unsigned nlimbs>
constexpr efloat<nlimbs> floor(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	efloat<nlimbs> res(x);
	int64_t exp = res.scale();
	bool inexact = clear_fractional_limbs(res._limb, exp);
	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		if (x.sign() == -1) {
			// Negative downward rounding adds 1 to magnitude
			if (add_one_integer_magnitude(res._limb, exp)) {
				res.setexponent(res.scale() + 32);
			}
		}
	}
	res.normalize();
	return res;
}

// ceil: round upward (toward +infinity)
template<unsigned nlimbs>
constexpr efloat<nlimbs> ceil(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	efloat<nlimbs> res(x);
	int64_t exp = res.scale();
	bool inexact = clear_fractional_limbs(res._limb, exp);
	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		if (x.sign() == 1) {
			// Positive upward rounding adds 1 to magnitude
			if (add_one_integer_magnitude(res._limb, exp)) {
				res.setexponent(res.scale() + 32);
			}
		}
	}
	res.normalize();
	return res;
}

// round: round to nearest, halfway cases rounded away from zero
template<unsigned nlimbs>
constexpr efloat<nlimbs> round(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	int64_t exp = x.scale();
	size_t total_bits = x.bits().size() * 32;
	if (exp >= static_cast<int64_t>(total_bits) - 1) {
		return x; // already exactly an integer!
	}

	bool guard = false;
	bool sticky = false;

	if (exp < 0) {
		guard = (exp == -1);
		for (size_t i = 0; i < x.bits().size(); ++i) {
			uint32_t val = x.bits()[i];
			if (i == 0) {
				if ((val & 0x7FFFFFFF) != 0) sticky = true;
			} else {
				if (val != 0) sticky = true;
			}
		}
	} else {
		size_t full_limbs = static_cast<size_t>(exp / 32);
		unsigned bit_idx = static_cast<unsigned>(exp % 32);

		if (bit_idx < 31) {
			guard = (x.bits()[full_limbs] & (1u << (30u - bit_idx))) != 0;
			if ((x.bits()[full_limbs] & ((1u << (30u - bit_idx)) - 1u)) != 0) {
				sticky = true;
			}
		} else {
			// bit_idx == 31, guard is MSB of next limb!
			if (full_limbs + 1 < x.bits().size()) {
				guard = (x.bits()[full_limbs + 1] & 0x80000000u) != 0;
				if ((x.bits()[full_limbs + 1] & 0x7FFFFFFFu) != 0) {
					sticky = true;
				}
			}
		}
		// check any subsequent limbs
		for (size_t i = full_limbs + 2; i < x.bits().size(); ++i) {
			if (x.bits()[i] != 0) sticky = true;
		}
	}

	efloat<nlimbs> res(x);
	bool inexact = clear_fractional_limbs(res._limb, exp);
	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		// Round away from zero if guard is set (fraction is >= 0.5)
		if (guard) {
			if (add_one_integer_magnitude(res._limb, exp)) {
				res.setexponent(res.scale() + 32);
			}
		}
	}
	res.normalize();
	return res;
}

// rint / nearbyint: round to nearest, halfway cases rounded to even
template<unsigned nlimbs>
constexpr efloat<nlimbs> rint(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;

	int64_t exp = x.scale();
	size_t total_bits = x.bits().size() * 32;
	if (exp >= static_cast<int64_t>(total_bits) - 1) {
		return x; // already exactly an integer!
	}

	bool lsb = false;
	bool guard = false;
	bool sticky = false;

	if (exp < 0) {
		guard = (exp == -1);
		lsb = false; // integer part is 0 (even!)
		for (size_t i = 0; i < x.bits().size(); ++i) {
			uint32_t val = x.bits()[i];
			if (i == 0) {
				if ((val & 0x7FFFFFFF) != 0) sticky = true;
			} else {
				if (val != 0) sticky = true;
			}
		}
	} else {
		size_t full_limbs = static_cast<size_t>(exp / 32);
		unsigned bit_idx = static_cast<unsigned>(exp % 32);

		lsb = (x.bits()[full_limbs] & (1u << (31u - bit_idx))) != 0;

		if (bit_idx < 31) {
			guard = (x.bits()[full_limbs] & (1u << (30u - bit_idx))) != 0;
			if ((x.bits()[full_limbs] & ((1u << (30u - bit_idx)) - 1u)) != 0) {
				sticky = true;
			}
		} else {
			if (full_limbs + 1 < x.bits().size()) {
				guard = (x.bits()[full_limbs + 1] & 0x80000000u) != 0;
				if ((x.bits()[full_limbs + 1] & 0x7FFFFFFFu) != 0) {
					sticky = true;
				}
			}
		}
		for (size_t i = full_limbs + 2; i < x.bits().size(); ++i) {
			if (x.bits()[i] != 0) sticky = true;
		}
	}

	bool round_up = false;
	if (guard) {
		if (sticky || lsb) {
			round_up = true;
		}
	}

	efloat<nlimbs> res(x);
	bool inexact = clear_fractional_limbs(res._limb, exp);
	if (inexact) {
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::Inexact);
		}
		if (round_up) {
			if (add_one_integer_magnitude(res._limb, exp)) {
				res.setexponent(res.scale() + 32);
			}
		}
	}
	res.normalize();
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

// lrint / llrint: round to nearest-even and return long / long long
template<unsigned nlimbs>
constexpr long lrint(const efloat<nlimbs>& x) {
	return static_cast<long>(double(rint(x)));
}

template<unsigned nlimbs>
constexpr long long llrint(const efloat<nlimbs>& x) {
	return static_cast<long long>(double(rint(x)));
}

}} // namespace sw::universal
