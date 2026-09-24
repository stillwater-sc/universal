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
// log10(2) is carried as the rational 30102999566 / 100000000000. That ratio reproduces
// the exactly-rounded result for every significand width from 2 to 325146, and the
// largest product it forms stays inside int64_t for widths up to 306 million -- both far
// beyond any type this library can express. The widest today is elreal, whose default 32
// blocks on a double host carry 1696 bits.

namespace detail {
	constexpr std::int64_t log10_2_num = 30102999566LL;
	constexpr std::int64_t log10_2_den = 100000000000LL;
}

// decimal digits that survive a round trip INTO a binary format of `digits` significand
// bits: floor((digits - 1) * log10(2)).
constexpr int decimal_digits10(int digits) noexcept {
	if (digits < 2) return 0;
	return static_cast<int>((static_cast<std::int64_t>(digits) - 1) * detail::log10_2_num / detail::log10_2_den);
}

// decimal digits needed to round trip OUT of and back into that format without loss:
// ceil(digits * log10(2)) + 1.
constexpr int decimal_max_digits10(int digits) noexcept {
	if (digits < 1) return 0;
	const std::int64_t n = static_cast<std::int64_t>(digits) * detail::log10_2_num;
	return static_cast<int>((n + detail::log10_2_den - 1) / detail::log10_2_den) + 1;
}

}} // namespace sw::universal
