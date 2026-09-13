#pragma once
// manipulators.hpp: definition of manipulation functions for adaptive precision einteger
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the einteger headers (#1334, Phase 2 group 5b, #1455): the <iomanip>
// half -- everything that turns an einteger into a std::string. iostream.hpp is the
// <iostream> half. Self-contained.
#include <sstream>
#include <string>
#include <typeinfo>   // typeid, for type_tag
#include <vector>

#include <universal/number/einteger/core.hpp>

namespace sw { namespace universal {

// Generate a type tag for adaptiveint
template<typename ElasticIntegerType,
	std::enable_if_t< is_einteger<ElasticIntegerType>, bool> = true>
std::string type_tag(const ElasticIntegerType & = {}) {
	std::stringstream s;
	s << "einteger<" << typeid(typename ElasticIntegerType::bt).name() << '>';
	return s.str();
}

template<typename BlockType>
inline std::string to_binary(const einteger<BlockType>& a, bool nibbleMarker = true) {
	if (a.limbs() == 0) return std::string("0b0");

	std::stringstream s;
	s << "0b";
	for (int b = static_cast<int>(a.limbs()) - 1; b >= 0; --b) {
		BlockType segment = a.block(static_cast<size_t>(b));
		BlockType mask = (0x1u << (a.bitsInBlock - 1));
		for (int i = a.bitsInBlock - 1; i >= 0; --i) {
			s << ((segment & mask) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
			if (b > 0 && i == 0 && nibbleMarker) s << '\'';
			mask >>= 1;
		}
	}

	return s.str();
}

template<typename BlockType>
inline std::string to_hex(const einteger<BlockType>& a, bool wordMarker = true) {
	if (a.limbs() == 0) return std::string("0x0");

	std::vector<char> nibbleLookup = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	std::stringstream s;
	s << "0x";
	unsigned bitIndex = a.limbs() * a.bitsInBlock - 1u;
	for (int b = static_cast<int>(a.limbs()) - 1; b >= 0; --b) {
		BlockType limb = a.block(static_cast<size_t>(b));
		BlockType mask = (0x1u << (a.bitsInBlock - 1));
		unsigned nibble{ 0 };
		unsigned rightShift = a.bitsInBlock - 4u;
		for (int i = a.bitsInBlock - 1; i >= 0; --i) {

			nibble |= (limb & mask);
			if ((i % 4) == 0) {
				nibble >>= rightShift;
				s << nibbleLookup[nibble];
				nibble = 0;
				rightShift -= 4u;
			}
			if (bitIndex > 0 && ((bitIndex % 16) == 0) && wordMarker) s << '\'';
			mask >>= 1;
			--bitIndex;
		}
	}

	return s.str();

}

}} // namespace sw::universal
