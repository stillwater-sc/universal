#pragma once
// to_decimal.hpp: decimal string conversion for blockbinary
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Split out of internal/blockbinary/manipulators.hpp (#1334). to_decimal() builds its
// result by prepending characters to a std::string -- it never opens a stream -- but it
// shared a header with type_tag/to_binary/to_hex, which do, and which therefore pull
// <iomanip> and <sstream>: 50,808 lines and four of the five I/O-family headers.
//
// That is the same contract that keeps to_string() in a number system's core. A core
// that needs to render its significand as decimal digits (dfloat's str(), and its
// wide-path double conversion) can now include this header alone and stay at zero
// I/O-family headers.
//
// manipulators.hpp includes this file, so every existing caller is unaffected.
#include <string>
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

// decimal string conversion
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_decimal(const blockbinary<nbits, BlockType, NumberType>& number) {
	if (number.iszero()) return "0";

	std::string result;
	blockbinary<nbits, BlockType, NumberType> dividend(number);
	bool isNegative = false;

	// Handle negative numbers for signed types
	if constexpr (NumberType == BinaryNumberType::Signed) {
		if (dividend.isneg()) {
			isNegative = true;
			dividend.twosComplement(); // Convert to positive
		}
	}

	// Repeatedly divide by 10 and collect remainders
	blockbinary<nbits, BlockType, NumberType> ten(10);
	while (!dividend.iszero()) {
		if constexpr (nbits <= 64) {
			// For smaller sizes, use native division to avoid complexity
			uint64_t temp = dividend.to_ull();
			uint64_t remainder = temp % 10;
			result = char('0' + remainder) + result;
			dividend = temp / 10;
		} else {
			// For larger sizes, use blockbinary division operators
			blockbinary<nbits, BlockType, NumberType> remainder = dividend % ten;
			uint64_t digit = remainder.to_ull();
			result = char('0' + digit) + result;
			dividend /= ten;
		}
	}

	if (isNegative) {
		result = "-" + result;
	}

	return result;
}

}} // namespace sw::universal
