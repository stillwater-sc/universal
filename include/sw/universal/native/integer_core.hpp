#pragma once
// integer_core.hpp: the bit-manipulation half of native/integers.hpp
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// native/integers.hpp is MIXED (a point native/ieee754_core.hpp already records): most
// of it produces strings and pulls <sstream>, but a few functions are pure bit
// manipulation that an arithmetic core legitimately needs. Same split as #1400 gave
// ieee754: the computational half lives here and carries no I/O, and integers.hpp
// includes it so every existing caller is unaffected.
//
// integer<>'s Knuth division needs nlz() to normalise the divisor. Before this split
// it reached it through integers.hpp, which put <sstream>, <ostream> and <istream>
// into the arithmetic core of every type that uses integer<> (#1334 Phase N).
#include <cstddef>   // std::size_t
#include <cstdint>
#include <type_traits>

namespace sw { namespace universal {

// find shift left value to move leading non-zero in a limb to most significant bit
// BlockType must be one of [uint8_t, uint16_t, uint32_t, uint64_t]
template<typename BlockType>
inline int nlz(BlockType x) {
	// std::size_t, qualified: this header includes only <cstddef>/<cstdint>/<type_traits>,
	// none of which is required to declare an unqualified ::size_t. It compiled in its
	// old home only because integers.hpp pulls <string>/<sstream> in ahead of it.
	constexpr std::size_t bitsInBlock = sizeof(BlockType) * 8;
	if (x == 0) return static_cast<int>(bitsInBlock);

	int n = 0;
	if constexpr (bitsInBlock == 64) {
		if (x <= 0x00000000FFFFFFFFull) { n = n + 32; x = static_cast<BlockType>(x << 32); }
		if (x <= 0x0000FFFFFFFFFFFFull) { n = n + 16; x = static_cast<BlockType>(x << 16); }
		if (x <= 0x00FFFFFFFFFFFFFFull) { n = n + 8; x = static_cast<BlockType>(x << 8); }
		if (x <= 0x0FFFFFFFFFFFFFFFull) { n = n + 4; x = static_cast<BlockType>(x << 4); }
		if (x <= 0x3FFFFFFFFFFFFFFFull) { n = n + 2; x = static_cast<BlockType>(x << 2); }
		if (x <= 0x7FFFFFFFFFFFFFFFull) { n = n + 1; }
	}
	else if constexpr (bitsInBlock == 32) {
		if (x <= 0x0000FFFFu) { n = n + 16; x = static_cast<BlockType>(x << 16); }
		if (x <= 0x00FFFFFFu) { n = n + 8; x = static_cast<BlockType>(x << 8); }
		if (x <= 0x0FFFFFFFu) { n = n + 4; x = static_cast<BlockType>(x << 4); }
		if (x <= 0x3FFFFFFFu) { n = n + 2; x = static_cast<BlockType>(x << 2); }
		if (x <= 0x7FFFFFFFu) { n = n + 1; }
	}
	else if constexpr (bitsInBlock == 16) {
		if (x <= 0x00FFu) { n = n + 8; x = static_cast<BlockType>(x << 8); }
		if (x <= 0x0FFFu) { n = n + 4; x = static_cast<BlockType>(x << 4); }
		if (x <= 0x3FFFu) { n = n + 2; x = static_cast<BlockType>(x << 2); }
		if (x <= 0x7FFFu) { n = n + 1; }
	}
	else if constexpr (bitsInBlock == 8) {
		if (x <= 0x0Fu) { n = n + 4; x = static_cast<BlockType>(x << 4); }
		if (x <= 0x3Fu) { n = n + 2; x = static_cast<BlockType>(x << 2); }
		if (x <= 0x7Fu) { n = n + 1; }
	}

	return n;
}

}} // namespace sw::universal
