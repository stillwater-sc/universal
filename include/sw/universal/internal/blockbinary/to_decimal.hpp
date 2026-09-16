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
#include <cstdint>   // uint64_t in the digit-extraction loop
#include <string>
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

// decimal string conversion
//
// The digits are accumulated from the bits rather than by dividing the blockbinary, which had two
// problems (#1395). Dividing used longdivision above 64 bits, and that is signed-only, so a wide
// UNSIGNED blockbinary would not compile the moment to_decimal was instantiated. And the negative
// path complemented in place, which leaves -2^(nbits-1) negative -- it has no positive twin in
// nbits -- so the divide-and-collect loop went on to emit characters below '0': a blockbinary<128>
// of its most negative value printed "-/)0/,//(-,*0,*'.-/...".
//
// Reading the magnitude's bits instead needs neither. The complement is taken bit by bit here, into
// a magnitude that is at most 2^(nbits-1) and so always fits, and the accumulation is the schoolbook
// "double the digits, add the bit" from the most significant bit down.
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_decimal(const blockbinary<nbits, BlockType, NumberType>& number) {
	if (number.iszero()) return "0";

	bool isNegative = false;
	if constexpr (NumberType == BinaryNumberType::Signed) isNegative = number.isneg();

	std::string magnitude(nbits, '0');   // bit i of |number| at index i
	if (isNegative) {
		unsigned carry = 1;              // invert and add one, low bit first
		for (unsigned i = 0; i < nbits; ++i) {
			const unsigned bit = (number.test(i) ? 0u : 1u) + carry;
			magnitude[i] = static_cast<char>('0' + (bit & 1u));
			carry = bit >> 1;
		}
	}
	else {
		for (unsigned i = 0; i < nbits; ++i) magnitude[i] = number.test(i) ? '1' : '0';
	}

	std::string result("0");             // decimal digits, most significant first
	for (unsigned i = nbits; i > 0; --i) {
		unsigned carry = (magnitude[i - 1] == '1') ? 1u : 0u;
		for (unsigned d = static_cast<unsigned>(result.size()); d > 0; --d) {
			const unsigned digit = static_cast<unsigned>(result[d - 1] - '0') * 2u + carry;
			result[d - 1] = static_cast<char>('0' + digit % 10u);
			carry = digit / 10u;
		}
		while (carry > 0) {
			result.insert(result.begin(), static_cast<char>('0' + carry % 10u));
			carry /= 10u;
		}
	}

	if (isNegative) result = "-" + result;

	return result;
}

}} // namespace sw::universal
