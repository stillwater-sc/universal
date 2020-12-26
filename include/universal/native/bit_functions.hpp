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

// find the most significant bit set
// first bit is defined to be at position 1, so that no bits set returns 0
inline unsigned int findMostSignificantBit(unsigned long long x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	unsigned int base = 0;
	if (x & 0xFFFFFFFF00000000) { base += 32; x >>= 32; }
	if (x & 0x00000000FFFF0000) { base += 16; x >>= 16; }
	if (x & 0x000000000000FF00) { base += 8;  x >>= 8; }
	if (x & 0x00000000000000F0) { base += 4;  x >>= 4; }
	return base + bval[x];
}

inline unsigned int findMostSignificantBit(long long x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint64_t tmp = uint64_t(x);
	unsigned int base = 0;
	if (tmp & 0xFFFFFFFF00000000) { base += 32; tmp >>= 32; }
	if (tmp & 0x00000000FFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x000000000000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00000000000000F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

inline unsigned int findMostSignificantBit(int x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint32_t tmp = uint32_t(x);
	unsigned int base = 0;
	if (tmp & 0xFFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x0000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x000000F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

inline unsigned int findMostSignificantBit(short x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint16_t tmp = uint16_t(x);
	unsigned int base = 0;
	if (tmp & 0xFF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

inline unsigned int findMostSignificantBit(signed char x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint8_t tmp = uint8_t(x);
	unsigned int base = 0;
	if (tmp & 0xF0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

}}  // namespace sw::universal
