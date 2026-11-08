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
#include <type_traits>

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
// rounds to 3.0, and a double cannot see the 0.5 in 2^2000 + 0.5, which elreal holds exactly on
// any host. odd reports the parity of an integer y.
//
// While the leading nonzero block is itself an integer (its lowest set bit at or above 2^0), it is
// subtracted exactly, and its bit 0 decides the parity it contributes. That needs no assumption
// about how the remaining blocks overlap. Once the leading block has fractional bits, the
// remainder is below about 2^(host digits), so its nearest integer comes from a double without
// overflow, and the remainder is an integer exactly when subtracting that leaves zero within the
// window -- the window in which elreal decides equality. (A long double host's blocks reach 2^64,
// beyond what a double rounds exactly, so there the subtraction repeats until it is below 2^52.)
template <typename FpType>
inline bool is_integer(ZBCL<FpType> y, std::size_t window, bool& odd) {
	odd = false;
	using W = std::conditional_t<std::is_same_v<FpType, long double>, long double, double>;
	constexpr int D = std::numeric_limits<W>::digits;
	for (std::size_t step = 0; step < window + 8; ++step) {
		const auto lead = first_nonzero_block(y, window);
		if (!lead.found) return true;
		// the lowest set bit of the leading block: |v| = mant * 2^(e - D), mant an integer
		int e = 0;
		W mant = std::ldexp(std::frexp(std::fabs(static_cast<W>(lead.blk.v)), &e), D);
		long long lsb = static_cast<long long>(e) - D + static_cast<long long>(lead.blk.exp);
		while (std::fmod(mant, W(2)) == W(0)) { mant /= W(2); ++lsb; }
		if (lsb >= 0) {
			if (lsb == 0) odd = !odd;                                   // the block's bit 0
			y = add(lead.rest, negate(ZBCL<FpType>::singleton(lead.blk)));
			continue;
		}
		const double c = to_double_approx(lead.rest, window);
		const double piece = static_cast<double>(static_cast<FpType>(std::nearbyint(c)));   // an integer, host exact
		if (std::fmod(piece, 2.0) != 0.0) odd = !odd;
		y = add(lead.rest, from_native<FpType>(-piece));
		// below 2^52, c is within 1/4 of the remainder, so piece is its nearest integer and what is
		// left is zero exactly when the remainder was an integer. Above (a long double host, whose
		// blocks reach 2^64), the double rounded too: what is left is smaller, so go round again.
		if (std::fabs(c) < 4503599627370496.0) return !first_nonzero_block(y, window).found;   // 2^52
	}
	return true;   // more integer blocks than the window holds: an integer to precision
}

}}} // namespace sw::universal::detail
