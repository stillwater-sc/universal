#pragma once
// decimal_bits.hpp: helper functions to compute storage bits for decimal encodings
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

// BCD encoding: 4 bits per decimal digit
static constexpr unsigned bcd_bits(unsigned ndigits) {
	return 4u * ndigits;
}

// BID encoding: ceil(ndigits * log2(10)) bits to hold 10^ndigits - 1
static constexpr unsigned bid_bits(unsigned ndigits) {
	if (ndigits == 0) return 0;
	// log2(10) ~= 3.321928... approximate with 3322/1000
	return static_cast<unsigned>((static_cast<uint64_t>(ndigits) * 3322u + 999u) / 1000u);
}

// DPD encoding: 10 bits per 3-digit group, plus 4 or 7 bits for remainder
static constexpr unsigned dpd_bits(unsigned ndigits) {
	return (ndigits / 3) * 10 + (ndigits % 3 == 1 ? 4 : ndigits % 3 == 2 ? 7 : 0);
}

}} // namespace sw::universal
