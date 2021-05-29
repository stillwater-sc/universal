#pragma once
// constexpr754.hpp: constexpr manipulation functions for IEEE-754 native types using C++20 <bit>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if BIT_CAST_SUPPORT
#include <bit>    // bit_cast

namespace sw::universal {

////////////////////////////////////////////////////////////////////////
// numerical helpers

inline constexpr void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits) {
	// normal number
	uint32_t bc = std::bit_cast<uint32_t>(value);
	s = (0x8000'0000u & bc);
	rawExponentBits = (0x7F80'0000u & bc) >> 23;
	rawFractionBits = (0x007F'FFFFu & bc);
}

inline constexpr void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits) {
	uint64_t bc = std::bit_cast<uint64_t>(value);
	s = (0x8000'0000'0000'0000ull & bc);
	rawExponentBits = (0x7FF0'0000'0000'0000ull & bc) >> 52;
	rawFractionBits = (0x000F'FFFF'FFFF'FFFFull & bc);
}

} // namespace sw::universal

#endif // BIT_CAST_SUPPORT
