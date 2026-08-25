#pragma once
// manipulators.hpp: text produced from a blockfraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iomanip> half of blockfraction's text layer (#1334): to_binary and to_hex.
// blockfraction.hpp holds the arithmetic and needs none of it, which is what keeps
// <sstream> out of every graph that builds on it.
#include <cstdint>
#include <sstream>
#include <string>
#include <universal/internal/blockfraction/blockfraction.hpp>

namespace sw { namespace universal {

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the blockfraction: 00h.ffff
// by design, the radix point is at nbits-3
template<unsigned nbits, typename bt>
std::string to_binary(const blockfraction<nbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = nbits - 1; i >= 0; --i) {
		s << (number.at(unsigned(i)) ? '1' : '0');
		if (i == number.radix()) {
			s << '.';
		}
		else {
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	return s.str();
}

// local helper to display the contents of a byte array
template<unsigned nbits, typename bt>
std::string to_hex(const blockfraction<nbits, bt>& number, bool wordMarker = true) {
	static constexpr unsigned bitsInByte = 8;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
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
		if (wordMarker && n > 0 && ((n * 4ll) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

///////////////////////////////////////////////////////////////////////////////
// specialty binary operators


}} // namespace sw::universal
