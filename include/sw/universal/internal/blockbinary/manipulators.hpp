#pragma once
// manipulators.hpp: text produced from a blockbinary
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iomanip> half of blockbinary's text layer (#1334): type_tag, to_binary and
// to_hex -- the string producers that format through a std::stringstream.
// blockbinary.hpp holds the arithmetic and needs none of it, which is what keeps
// <sstream> and <iomanip> out of every number system that builds on blockbinary.
// to_decimal() concatenates rather than streams and now lives in to_decimal.hpp,
// included below so this header's surface is unchanged.
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blockbinary/to_decimal.hpp>   // to_decimal moved out (#1334): it
                                                           // concatenates a string and needs no
                                                           // stream, so a core can include it alone

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

}} // namespace sw::universal
