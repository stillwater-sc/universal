#pragma once
// stream_utilities.hpp: queries and constructors on a ZBCL<FpType> that decide a property
// from the stream itself, within a window of blocks (#2003).
//
// A lazy stream can start with zero blocks: a cancelled value such as a - a is a stream of
// zero blocks, and 1 - (1 + 2^-100) starts with one. Anything that reads head() for the
// value's sign or scale must skip them first. "Within the window" is the convention the
// elreal comparison operators use: a value with no nonzero block among its first `window`
// blocks is zero to that precision.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstddef>
#include <limits>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>   // from_native, to_double_approx
#include <universal/number/elreal/threeAdd.hpp>       // add
#include <universal/number/elreal/negate.hpp>         // negate

namespace sw { namespace universal { namespace detail {

// first_nonzero_block(s, window): the first nonzero block among the first `window` blocks of
// s, and the stream from that block on. It walks the memoised stream and stops there, so a
// value whose first block is nonzero forces only that block. found == false when the stream
// ends, or every block in the window is zero.
template <typename FpType>
struct nonzero_lead {
	bool          found;
	block<FpType> blk;
	ZBCL<FpType>  rest;   // the stream starting at blk; empty when !found
};
template <typename FpType>
inline nonzero_lead<FpType> first_nonzero_block(ZBCL<FpType> s, std::size_t window) {
	for (std::size_t i = 0; i < window && !s.is_empty(); ++i) {
		if (!s.head().is_zero_block()) return { true, s.head(), s };
		if (i + 1 < window) s = s.tail();   // do not force a block beyond the window
	}
	return { false, block<FpType>{}, ZBCL<FpType>{} };
}

// from_integer(magnitude, negative): an integer of up to 64 bits, exactly. The magnitude is
// split into chunks of at most the host's digits (and at most 32 bits), each exact in FpType,
// and the chunks are summed. Casting through a double rounded every integer above 2^53, and
// above 2^24 on a float host.
template <typename FpType>
inline ZBCL<FpType> from_integer(unsigned long long magnitude, bool negative) {
	constexpr int hostDigits = std::numeric_limits<FpType>::digits;
	constexpr int chunkBits  = hostDigits < 32 ? hostDigits : 32;
	const unsigned long long chunkMask = (1ull << chunkBits) - 1ull;
	ZBCL<FpType> value{};
	for (int shift = 0; magnitude != 0; shift += chunkBits, magnitude >>= chunkBits) {
		const unsigned long long chunk = magnitude & chunkMask;
		if (chunk == 0) continue;
		const block<FpType> b{ static_cast<FpType>(static_cast<double>(chunk)), typename block<FpType>::exp_t(shift) };
		const ZBCL<FpType> part = ZBCL<FpType>::singleton(b);
		value = value.is_empty() ? part : add(value, part);
	}
	return (negative && !value.is_empty()) ? negate(value) : value;
}

// is_integer(y, window, odd): whether y is an integer to within the window, decided from the
// stream rather than from a double approximation. pow(2, 3 + 2^-200) was 8 because 3 + 2^-200
// rounds to 3.0. Each step subtracts the nearest integer the host holds exactly, so the
// remainder of an integer stays an integer and shrinks by the host's precision each time; the
// test ends when the remainder is zero (integer) or below 3/4 and nonzero (not). odd reports the
// parity of an integer y. A y whose approximation exceeds the host's range is treated as an even
// integer, as every value of that size is on the host.
template <typename FpType>
inline bool is_integer(ZBCL<FpType> y, std::size_t window, bool& odd) {
	odd = false;
	for (int step = 0; step < 64; ++step) {
		if (!first_nonzero_block(y, window).found) return true;
		const double c = to_double_approx(y, window);
		// beyond the host's range, or beyond double's: an even integer, as every value that size
		// is. Tested before the cast below, which is undefined for an out-of-range double on a
		// narrow host. isfinite comes first because a long double host's max() converts to inf.
		if (!std::isfinite(c) || !(std::fabs(c) <= static_cast<double>(std::numeric_limits<FpType>::max()))) return true;
		if (std::fabs(c) < 0.75) return false;
		const double piece = static_cast<double>(static_cast<FpType>(std::nearbyint(c)));   // an integer
		if (std::fmod(piece, 2.0) != 0.0) odd = !odd;
		y = add(y, from_native<FpType>(-piece));
	}
	return true;   // more than 64 host-precision steps: far beyond any parity a host can show
}

}}} // namespace sw::universal::detail
