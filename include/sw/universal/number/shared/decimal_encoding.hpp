#pragma once
// decimal_encoding.hpp: shared enum for decimal encoding formats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Decimal encoding formats for decimal number systems
enum class DecimalEncoding {
	BCD,   // Binary-Coded Decimal: 4 bits per digit
	BID,   // Binary Integer Decimal: significand stored as binary integer
	DPD    // Densely Packed Decimal: significand stored as 10-bit declets
};

}} // namespace sw::universal
