#pragma once
// posit_helpers.hpp: definitions of helper functions for posit manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
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
	return (uint64_t(0x000FFFFFFFFFFFFFull) & *(uint64_t*)&fraction);
}

// integral type to bitset transformations

// we are using a full nbits sized bitset even though nbits-3 is the maximum fraction
// a posit would contain. However, we need an extra bit after the cut-off to make the
// round up/down decision. The <nbits-something> size created a lot of sw complexity
// that isn't worth the trouble, so we are simplifying and simply manage a full nbits
// of fraction bits.

template<size_t nbits>
std::bitset<nbits> copy_float_fraction(uint32_t _23b_fraction_without_hidden_bit) {
	std::bitset<nbits> _fraction;
	uint32_t mask = uint32_t(0x00400000ul);
	unsigned int ub = (nbits < 23 ? nbits : 23);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _23b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<size_t nbits>
std::bitset<nbits> copy_double_fraction(uint64_t _52b_fraction_without_hidden_bit) {
	std::bitset<nbits> _fraction;
	if (nbits == 3) return _fraction;
	uint64_t mask = uint64_t(0x0008000000000000ull);
	unsigned int ub = (nbits < 52 ? nbits : 52);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _52b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<size_t nbits>
std::bitset<nbits> copy_integer_fraction(uint64_t _fraction_without_hidden_bit) {
	std::bitset<nbits> _fraction;
	uint64_t mask = uint64_t(0x8000000000000000ull);
	unsigned int ub = (nbits < 64 ? nbits : 64);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

// representation helpers

// nbits binary representation of a signed 64-bit number
template<size_t nbits>
std::string to_binary_(int64_t number) {
	std::stringstream ss;
	uint64_t mask = (uint64_t(1) << nbits - 1);
	for (int i = nbits - 1; i >= 0; --i) {
		ss << (mask & number ? "1" : "0");
		mask >>= 1;
	}
	return ss.str();
}

// full binary representation of a signed 64-bit number
std::string to_binary(int64_t number) {
	std::stringstream ss;
	unsigned int msb = findMostSignificantBit(number) - 1;
	uint64_t mask = (uint64_t(1) << msb);
	for (int i = msb; i >= 0; --i) {
		ss << (mask & number ? "1" : "0");
		mask >>= 1;
	}
	return ss.str();
}

// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t es>
void GeneratePositTable(std::ostream& ostr) 
{
	ostr << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << ">" << std::endl;

	const size_t size = (1 << nbits);
	posit<nbits, es>	myPosit;

	const size_t index_column = 5;
	const size_t bin_column = 16;
	const size_t k_column = 16;
	const size_t sign_column = 16;
	const size_t regime_column = 30;
	const size_t exponent_column = 16;
	const size_t fraction_column = 16;
	const size_t value_column = 30;

	ostr << setw(index_column) << " # "
		<< setw(bin_column) << " Binary"
		<< setw(bin_column) << " Decoded"
		<< setw(k_column) << " k-value"
		<< setw(sign_column) << "sign"
		<< setw(regime_column) << " regime"
		<< setw(exponent_column) << " exponent"
		<< setw(fraction_column) << " fraction"
		<< setw(value_column) << " value" << endl;
	for (int i = 0; i < size; i++) {
		myPosit.set_raw_bits(i);
		regime<nbits,es>   r = myPosit.get_regime();
		exponent<nbits,es> e = myPosit.get_exponent();
		fraction<nbits,es> f = myPosit.get_fraction();
		ostr << setw(4) << i << ": "
			<< setw(bin_column) << myPosit.get()
			<< setw(bin_column) << myPosit.get_decoded()
			<< setw(k_column) << myPosit.regime_k()
			<< setw(sign_column) << myPosit.sign_value()
			<< setw(regime_column) << setprecision(22) << r.value() << std::setprecision(0)
			<< setw(exponent_column) << right << e 
			<< setw(fraction_column) << right << f
			<< setw(value_column) << std::setprecision(22) << myPosit.to_double() << std::setprecision(0)
			<< std::endl;
	}
}