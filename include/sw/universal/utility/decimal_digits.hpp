#pragma once
// decimal_digits.hpp: exact decimal-digit counts for numeric_limits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

// numeric_limits<T>::digits10 and max_digits10 are defined in terms of log10(2), and
// approximating that as digits/3.3 is off by one at widths that matter: it reports 7
// decimal digits for a binary32 where the standard says 6, and 16 for a binary64 where
// the standard says 15 (issue #1597).
//
// These compute the standard quantities exactly, in integer arithmetic so they are
// usable in a constexpr initialiser without a floating-point operation:
//
//   digits10     = floor((digits - 1) * log10(2))
//   max_digits10 = ceil(digits * log10(2)) + 1
//
// log10(2) is carried as the rational 301029995663981195 / 10^18. A single
// `digits * numerator` would overflow int64_t almost immediately at that precision, so
// the product is formed in two halves -- see split_floor below. That keeps every
// intermediate inside int64_t for the whole range of `int`, and the result is the
// exactly-rounded one for every width from 2 to INT_MAX. Verified exhaustively over
// 2..400000 and by random sampling across the full range, against log10(2) computed to
// 140 significant digits.
//
// A narrower ratio was tried first and rejected: 30102999566/10^11 with a single
// multiply is exact only to 325146, and `cfloat<325162, 15>` instantiates (40KB per
// value, but it compiles), so that bound was reachable and returned a max_digits10 one
// too small -- understating the round-trip guarantee is the failure this header exists
// to prevent.

namespace detail {

	// log10(2) = 0.301029995663981195...  as num/10^18, split at 10^9 so that no
	// intermediate exceeds int64_t.
	constexpr std::int64_t log10_2_hi  = 301029995LL;        // high 9 digits of the numerator
	constexpr std::int64_t log10_2_lo  = 663981195LL;        // low 9 digits
	constexpr std::int64_t split_scale = 1000000000LL;       // 10^9
	constexpr std::int64_t full_scale  = 1000000000000000000LL;  // 10^18

	// floor(k * log10(2)) together with the remainder, so a caller can round up without
	// recomputing. Writing k*num directly would overflow; instead
	//     k*num/10^18 = (k*hi)/10^9 + (k*lo)/10^18
	// and carrying the remainder of the first division into the second keeps it exact.
	// Bounds at k = INT_MAX: k*hi <= 6.5e17 and r*10^9 + k*lo <= 2.5e18, both inside
	// int64_t's 9.2e18.
	struct scaled_log10 {
		std::int64_t quotient;
		std::int64_t remainder;   // over full_scale
	};

	constexpr scaled_log10 split_floor(std::int64_t k) noexcept {
		const std::int64_t khi = k * log10_2_hi;
		const std::int64_t q1  = khi / split_scale;
		const std::int64_t r1  = khi % split_scale;
		const std::int64_t rest = r1 * split_scale + k * log10_2_lo;
		return { q1 + rest / full_scale, rest % full_scale };
	}

} // namespace detail

// decimal digits that survive a round trip INTO a binary format of `digits` significand
// bits: floor((digits - 1) * log10(2)).
constexpr int decimal_digits10(int digits) noexcept {
	if (digits < 2) return 0;
	return static_cast<int>(detail::split_floor(static_cast<std::int64_t>(digits) - 1).quotient);
}

// decimal digits an INTEGER format of `digits` value bits can represent without change:
// floor(digits * log10(2)). This is the floating-point formula without the -1, because an
// integer format has no rounding step for a decimal literal to survive. Matches native
// integers: int32 has 31 value bits and reports 9, int64 has 63 and reports 18.
//
// numeric_limits<T>::max_digits10 is 0 for integer types, as the native integers report --
// there is no round trip through a decimal string to size. Use 0 directly rather than
// deriving it from this.
constexpr int decimal_digits10_integer(int digits) noexcept {
	if (digits < 1) return 0;
	return static_cast<int>(detail::split_floor(static_cast<std::int64_t>(digits)).quotient);
}

// decimal digits needed to round trip OUT of and back into that format without loss:
// ceil(digits * log10(2)) + 1.
constexpr int decimal_max_digits10(int digits) noexcept {
	if (digits < 1) return 0;
	const detail::scaled_log10 s = detail::split_floor(static_cast<std::int64_t>(digits));
	return static_cast<int>(s.quotient + (s.remainder != 0 ? 1 : 0)) + 1;
}

// numeric_limits types min_exponent and max_exponent as int, but a logarithmic format's
// exponent range is 2^(nbits-1-rbits) and outgrows that quickly: lns<48,10> needs 2^37.
// A silent narrowing wraps -- lns<48,10> and lns<64,11> were reporting min_exponent 0 and
// max_exponent -1, which claim a smallest normal of 2^-1 (#1602).
//
// Saturating is the most the standard traits can express. It is honest about the
// direction and the fact that the range is enormous, where a wrapped value is not, but it
// is still a bound rather than the true exponent. A caller that needs the real figure
// should read the type's own int64_t member (LNS::min_exponent) instead of the trait.
constexpr int saturate_exponent_to_int(std::int64_t e) noexcept {
	constexpr std::int64_t hi = static_cast<std::int64_t>((~0u) >> 1);           //  INT_MAX
	constexpr std::int64_t lo = -hi - 1;                                          //  INT_MIN
	if (e > hi) return static_cast<int>(hi);
	if (e < lo) return static_cast<int>(lo);
	return static_cast<int>(e);
}

// decimal exponent range, the other pair numeric_limits derives from log10(2) (#1604).
// These were also computed as exponent/3.3, which additionally relies on
// static_cast<int> truncating a negative quotient toward zero to stand in for ceil.
//
//   min_exponent10 = ceil((min_exponent - 1) * log10(2))   -- most negative n with 10^n normal
//   max_exponent10 = floor(max_exponent * log10(2))        -- largest n with 10^n representable
//
// min_exponent is negative, and split_floor() takes a non-negative argument, so the sign
// is folded out with ceil(-x * c) == -floor(x * c) rather than left to a cast.
// Checked against every native format: float -37/38, double -307/308, binary16 -4/4,
// binary128 -4931/4932.
// Both fold the sign out with split_floor(), which takes a non-negative argument, and both
// have to round the OPPOSITE way once folded. k * log10(2) is irrational for every k != 0,
// so it is never an integer and the +1 below is unconditional:
//     ceil(x)  == floor(x) + 1      for positive non-integral x
//     floor(x) == -(floor(-x) + 1)  for negative non-integral x
// Getting this backwards is invisible to any test built from real types, because every
// number system here has min_exponent <= 1 and max_exponent >= 0, so only one branch of
// each helper is ever reached by a numeric_limits specialization.
// Both take the exponent WIDE. A logarithmic format's binary exponent can exceed int (see
// saturate_exponent_to_int below) while its decimal exponent still fits: lns<35,1> has a
// binary exponent of -2^32 and a decimal one of -1292913986, which is inside int. Deriving
// the decimal from an already-saturated binary value throws that away and reports
// -646456993, off by a factor of two (#1609 review). So the conversion happens first and
// only the decimal RESULT is clamped.
//
// Beyond `decimal_reach` the decimal result cannot fit an int whatever it is, and
// split_floor's own product would overflow, so those saturate without computing.
// INT_MAX / log10(2) is 7133786261; the threshold sits just above it and its product with
// log10_2_hi stays inside int64_t.
namespace detail {
	constexpr std::int64_t decimal_reach = 7200000000LL;
}

constexpr int decimal_min_exponent10(std::int64_t min_exponent) noexcept {
	// ceil((min_exponent - 1) * log10(2)), clamped to int
	if (min_exponent < -detail::decimal_reach) return -2147483647 - 1;
	if (min_exponent >  detail::decimal_reach) return 2147483647;
	const std::int64_t r = (min_exponent > 1)
	    ? detail::split_floor(min_exponent - 1).quotient + 1
	    : -detail::split_floor(1 - min_exponent).quotient;
	return saturate_exponent_to_int(r);
}

constexpr int decimal_max_exponent10(std::int64_t max_exponent) noexcept {
	// floor(max_exponent * log10(2)), clamped to int
	if (max_exponent < -detail::decimal_reach) return -2147483647 - 1;
	if (max_exponent >  detail::decimal_reach) return 2147483647;
	const std::int64_t r = (max_exponent < 0)
	    ? -(detail::split_floor(-max_exponent).quotient + 1)
	    : detail::split_floor(max_exponent).quotient;
	return saturate_exponent_to_int(r);
}

}} // namespace sw::universal
