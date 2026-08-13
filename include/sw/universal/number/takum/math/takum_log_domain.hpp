// takum_log_domain.hpp: exact logarithmic-domain arithmetic for takum_log
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A logarithmic takum stores |value| = sqrt(e)^l, with l == c + m recovered
// exactly from the encoding as an integer characteristic c and a p-bit fraction.
// Functions that act on l by a rational factor -- sqrt halves it, squaring
// doubles it, an integer power scales it -- are therefore exact in this domain,
// and the only error left is the single rounding the target layout forces.
//
// The alternative, and what the linear takum's math library does, is to round
// trip through a double: decode, call the libm function, re-encode.  That path
// rounds three times and is capped at a double's 53 significand bits, while
// takum_log<64,3> reaches p = 59.  Staying in the logarithmic domain avoids
// both problems.
//
// The fraction is carried as an integer pair (N, q) meaning N / 2^q rather than
// as a double, for the same reason: halving l produces a fraction one bit wider
// than the source layout holds, so at p = 59 a double would discard the low bits
// before any rounding decision was made.
#pragma once
#include <cstdint>

namespace sw { namespace universal {

// An exact logarithmic value l == c + N / 2^q, with N < 2^q.
struct takum_log_value {
	int64_t  c;   // characteristic, in powers of the value base sqrt(e)
	uint64_t N;   // fraction numerator
	unsigned q;   // fraction denominator exponent
};

// Recover the exact logarithmic value of a magnitude.
// Pre: the caller has handled zero and NaR, which have no logarithmic value.
template<typename TakumLog>
constexpr takum_log_value to_log_value(const TakumLog& x) noexcept {
	auto d = TakumLog::Codec::decode(x.magnitude_bits());
	return takum_log_value{ d.c, d.M_bits, d.p };
}

// l / 2, exactly.  An odd characteristic contributes a half to the fraction,
// which is why the result needs one more fraction bit than the source.
constexpr takum_log_value halve(const takum_log_value& l) noexcept {
	// floor division, so that a negative odd c borrows rather than truncating
	int64_t  c2 = (l.c >= 0) ? (l.c / 2) : -(((-l.c) + 1) / 2);
	uint64_t r  = static_cast<uint64_t>(l.c - 2 * c2);          // 0 or 1
	return takum_log_value{ c2, (r << l.q) + l.N, l.q + 1 };
}

// l * 2, exactly.  Doubling the fraction can carry into the characteristic.
constexpr takum_log_value twice(const takum_log_value& l) noexcept {
	uint64_t N2 = l.N << 1;
	int64_t  c2 = 2 * l.c;
	if (l.q > 0 && N2 >= (1ull << l.q)) { N2 -= (1ull << l.q); ++c2; }
	return takum_log_value{ c2, N2, l.q };
}

// Negate l, i.e. take the reciprocal of the value.  Exact.
constexpr takum_log_value negate(const takum_log_value& l) noexcept {
	if (l.N == 0) return takum_log_value{ -l.c, 0ull, l.q };
	// -(c + N/2^q) == -(c+1) + (2^q - N)/2^q
	return takum_log_value{ -l.c - 1, (1ull << l.q) - l.N, l.q };
}

// l as a real, for the paths that must leave the exact domain.
constexpr double to_double(const takum_log_value& l) noexcept {
	if (l.q == 0) return static_cast<double>(l.c);
	return static_cast<double>(l.c) + static_cast<double>(l.N) / static_cast<double>(1ull << l.q);
}

// Build a takum_log whose magnitude is sqrt(e)^l, with the requested sign.
// Saturation follows the type's own conventions: overflow to maxpos/maxneg,
// underflow to zero, exactly as conversion from a native float does.
template<typename TakumLog>
inline TakumLog from_log_value(const takum_log_value& l, bool negative) noexcept {
	using Codec = typename TakumLog::Codec;
	TakumLog result;
	auto enc = Codec::encode_fraction(l.c, l.N, l.q);
	if (enc.overflowed()) {
		if (negative) result.maxneg(); else result.maxpos();
		return result;
	}
	if (enc.underflowed()) { result.setzero(); return result; }
	// the codec never sets the sign bit (I4); negate in two's complement here
	uint64_t raw = negative ? (((~enc.magnitude) + 1ull) & Codec::nbits_mask()) : enc.magnitude;
	result.setbits(raw);
	return result;
}

}} // namespace sw::universal
