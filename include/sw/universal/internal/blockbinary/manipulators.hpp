#pragma once
// manipulators.hpp: text produced from a blockbinary
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iomanip> half of blockbinary's text layer (#1334): type_tag, to_binary,
// to_hex, to_decimal -- everything that returns a std::string. blockbinary.hpp
// holds the arithmetic and needs none of it, which is what keeps <sstream> and
// <iomanip> out of every number system that builds on blockbinary.
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {
// Generate a type tag for blockbinary
template<unsigned N, typename B, BinaryNumberType T>
std::string type_tag(const blockbinary<N, B, T>& = {}) {
	std::stringstream str;
	str << "blockbinary<"
		<< std::setw(4) << N << ", "
		<< typeid(B).name() << ", "
		<< typeid(T).name() << '>';
	return str.str();
}
//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the storage
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_binary(const blockbinary<nbits, BlockType, NumberType>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (unsigned i = 0; i < nbits; ++i) {
		unsigned bitIndex = nbits - 1ull - i;
		s << (number.at(bitIndex) ? '1' : '0');
		if (bitIndex > 0 && (bitIndex % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}

// local helper to display the contents of a byte array
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::string to_hex(const blockbinary<nbits, BlockType, NumberType>& number, bool nibbleMarker = true) {
	static constexpr unsigned bitsInByte = 8;
	static constexpr unsigned bitsInBlock = sizeof(BlockType) * bitsInByte;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	int nrNibbles = int(1 + ((nbits - 1) >> 2));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = number.nibble(static_cast<unsigned>(n));
		ss << hexChar[nibble];
		if (nibbleMarker && n > 0 && ((n * 4ll) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

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
