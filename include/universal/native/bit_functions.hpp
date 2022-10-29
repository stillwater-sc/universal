#pragma once
// bit_functions.hpp: definitions of helper functions for bit operations on integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

// This file contains bit manipulation functions for native integer types.
namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////
// decoders

//////////////////////////// UNSIGNED integer types ////////////////////////

//static const unsigned int bval[] = { 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

/// <summary>
/// find most significant bit that is set
/// </summary>
/// <param name="x">value to</param>
/// <returns> position of MSB that is set. LSB is defined to be at position 1, so no bits set returns 0</returns>
inline constexpr unsigned int findMostSignificantBit(unsigned int x) {
	// find the first non-zero bit
	unsigned int base = 0;
	if (x & 0xFFFF0000) { base += 16; x >>= 16; }
	if (x & 0x0000FF00) { base += 8;  x >>= 8; }
	if (x & 0x000000F0) { base += 4;  x >>= 4; }
	unsigned int bval = 0;
	while (x > 0) {
		++bval;
		x >>= 1;
	}
	return base + bval;
}

/// <summary>
/// find most significant bit that is set
/// </summary>
/// <param name="x">value to</param>
/// <returns> position of MSB that is set. LSB is defined to be at position 1, so no bits set returns 0</returns>
inline constexpr unsigned int findMostSignificantBit(unsigned long x) {
	// find the first non-zero bit
	unsigned int base = 0;
	if (x & 0xFFFF0000) { base += 16; x >>= 16; }
	if (x & 0x0000FF00) { base += 8;  x >>= 8; }
	if (x & 0x000000F0) { base += 4;  x >>= 4; }
	unsigned int bval = 0;
	while (x > 0) {
		++bval;
		x >>= 1;
	}
	return base + bval;
}

inline constexpr unsigned int findMostSignificantBit(unsigned long long x) {
	// find the first non-zero bit
	unsigned int base = 0;
	if (x & 0xFFFFFFFF00000000) { base += 32; x >>= 32; }
	if (x & 0x00000000FFFF0000) { base += 16; x >>= 16; }
	if (x & 0x000000000000FF00) { base += 8;  x >>= 8; }
	if (x & 0x00000000000000F0) { base += 4;  x >>= 4; }
	unsigned int bval = 0;
	while (x > 0) {
		++bval;
		x >>= 1;
	}
	return base + bval;
}

//////////////////////////// SIGNED integer types ////////////////////////


inline constexpr unsigned int findMostSignificantBit(signed char x) {
	// find the first non-zero bit
	uint8_t tmp = uint8_t(x);
	unsigned int base = 0;
	if (tmp & 0xF0) { base += 4;  tmp >>= 4; }
	unsigned int bval = 0;
	while (tmp > 0) {
		++bval;
		tmp >>= 1;
	}
	return base + bval;
}

inline constexpr unsigned int findMostSignificantBit(short x) {
	// find the first non-zero bit
	uint16_t tmp = uint16_t(x);
	unsigned int base = 0;
	if (tmp & 0xFF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00F0) { base += 4;  tmp >>= 4; }
	unsigned int bval = 0;
	while (tmp > 0) {
		++bval;
		tmp >>= 1;
	}
	return base + bval;
}

inline constexpr unsigned int findMostSignificantBit(int x) {
	// find the first non-zero bit
	uint32_t tmp = uint32_t(x);
	unsigned int base = 0;
	if (tmp & 0xFFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x0000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x000000F0) { base += 4;  tmp >>= 4; }
	unsigned int bval = 0;
	while (tmp > 0) {
		++bval;
		tmp >>= 1;
	}
	return base + bval;
}

inline constexpr unsigned int findMostSignificantBit(long x) {
	// find the first non-zero bit
	uint32_t tmp = uint32_t(x);
	unsigned int base = 0;

	if (tmp & 0xFFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x0000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x000000F0) { base += 4;  tmp >>= 4; }
	unsigned int bval = 0;
	while (tmp > 0) {
		++bval;
		tmp >>= 1;
	}
	return base + bval;
}

inline constexpr unsigned int findMostSignificantBit(long long x) {
	// find the first non-zero bit
	uint64_t tmp = uint64_t(x);
	unsigned int base = 0;
	if (tmp & 0xFFFFFFFF00000000) { base += 32; tmp >>= 32; }
	if (tmp & 0x00000000FFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x000000000000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00000000000000F0) { base += 4;  tmp >>= 4; }
	unsigned int bval = 0;
	while (tmp > 0) {
		++bval;
		tmp >>= 1;
	}
	return base + bval;
}

}} // namespace sw::universal
