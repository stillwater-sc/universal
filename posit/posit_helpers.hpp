#pragma once

#include <cmath>  // for frexp/frexpf

inline uint64_t two_to_the_power(uint64_t n) {
	return (uint64_t(1) << n);
}

// find the most significant bit set: first bit is at position 1, so that no bits set returns 0
unsigned int findMostSignificantBit(uint64_t x) {
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

unsigned int findMostSignificantBit(int64_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint64_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xFFFFFFFF00000000) { base += 32; tmp >>= 32; }
	if (tmp & 0x00000000FFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x000000000000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00000000000000F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

unsigned int findMostSignificantBit(int32_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint32_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xFFFF0000) { base += 16; tmp >>= 16; }
	if (tmp & 0x0000FF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x000000F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

unsigned int findMostSignificantBit(int16_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint16_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xFF00) { base += 8;  tmp >>= 8; }
	if (tmp & 0x00F0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

unsigned int findMostSignificantBit(int8_t x) {
	// find the first non-zero bit
	static const unsigned int bval[] =
	{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

	uint8_t tmp = x;
	unsigned int base = 0;
	if (tmp & 0xF0) { base += 4;  tmp >>= 4; }
	return base + bval[tmp];
}

template<size_t nbits>
std::string to_binary_(int64_t number) {
	std::stringstream ss;
	uint64_t mask = (uint64_t(1) << nbits-1);
	for (int i = nbits-1; i >= 0; --i) {
		ss << (mask & number ? "1" : "0");
		mask >>= 1;
	}
	return ss.str();
}

std::string to_binary(int64_t number) {
	std::stringstream ss;
	unsigned int msb = findMostSignificantBit(number)-1;
	uint64_t mask = (uint64_t(1) << msb);
	for (int i = msb; i >= 0; --i) {
		ss << (mask & number ? "1" : "0");
		mask >>= 1;
	}
	return ss.str();
}

// FLOAT component extractions
bool extract_sign(float f) {
	return (uint32_t(0x80000000ul) & *(uint32_t*)&f);
}

int extract_exponent(float f) {
	int exponent;
	frexpf(f, &exponent);
	return exponent;
}

uint32_t extract_fraction(float f) {
	int exponent;
	float fraction = frexpf(f, &exponent);
	return (uint32_t(0x007FFFFFul) & *(uint32_t*)&fraction);
}

// DOUBLE component extractions
bool extract_sign(double f) {
	return (uint64_t(0x8000000000000000ull) & *(uint64_t*)&f);
}

int extract_exponent(double f) {
	int exponent;
	frexp(f, &exponent);
	return exponent;
}

uint64_t extract_fraction(double f) {
	int exponent;
	double fraction = frexp(f, &exponent);
	return (uint64_t(0x001FFFFFFFFFFFFFull) & *(uint64_t*)&fraction);
}

// integral type to bitset transformations
template<size_t nbits>
std::bitset<nbits - 3> copy_float_fraction(uint32_t _23b_fraction_without_hidden_bit) {
	std::bitset<nbits - 3> _fraction;
	uint32_t mask = uint32_t(0x00400000ul);
	for (int i = nbits - 4; i >= 0; --i) {
		_fraction[i] = _23b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<size_t nbits>
std::bitset<nbits - 3> copy_double_fraction(uint64_t _52b_fraction_without_hidden_bit) {
	std::bitset<nbits - 3> _fraction;
	uint32_t mask = uint32_t(0x00400000ul);
	for (int i = nbits - 4; i >= 0; --i) {
		_fraction[i] = _52b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<size_t nbits>
std::bitset<nbits - 3> copy_integer_fraction(uint64_t _fraction_without_hidden_bit) {
	std::bitset<nbits - 3> _fraction;
	uint64_t mask = uint64_t(0x8000000000000000ull);
	for (int i = nbits - 4; i >= 0; --i) {
		_fraction[i] = _fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}