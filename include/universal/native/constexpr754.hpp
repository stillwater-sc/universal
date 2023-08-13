#pragma once
// constexpr754.hpp: constexpr manipulation functions for IEEE-754 native types using C++20 <bit>
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>
#include <bit>    // bit_cast

#include <universal/native/integers.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////////
// numerical helpers

template<typename Real>
inline constexpr void extractFields(Real value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
	if (value == 0) {
		s = false;
		rawExponentBits = 0ull;
		rawFractionBits = 0ull;
	}
	if (value < 0) s = true;
}
// specialization to extract fields from a float
template<>
inline constexpr void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
	uint32_t bc = std::bit_cast<uint32_t, float>(value);
	s = (ieee754_parameter<float>::smask & bc);
	rawExponentBits = (ieee754_parameter<float>::emask & bc) >> ieee754_parameter<float>::fbits;
	rawFractionBits = (ieee754_parameter<float>::fmask & bc);
	bits = bc;
}
// specialization to extract fields from a double
template<>
inline constexpr void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
	uint64_t bc = std::bit_cast<uint64_t, double>(value);
	s = (ieee754_parameter<double>::smask & bc);
	rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
	rawFractionBits = (ieee754_parameter<double>::fmask & bc);
	bits = bc;
}

#if LONG_DOUBLE_SUPPORT
// specialization to extract fields from a long double
template<>
inline constexpr void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
	uint64_t bc = std::bit_cast<uint64_t, long double>(value);
	s = (ieee754_parameter<long double>::smask & bc);
	rawExponentBits = (ieee754_parameter<long double>::emask & bc) >> ieee754_parameter<long double>::fbits;
	rawFractionBits = (ieee754_parameter<long double>::fmask & bc);
}
#else
#define LONG_DOUBLE_DOWNCAST
#ifdef LONG_DOUBLE_DOWNCAST
template<>
inline constexpr void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
	double d = static_cast<double>(value);
	uint64_t bc = std::bit_cast<uint64_t, double>(d);
	s = (ieee754_parameter<double>::smask & bc);
	rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
	rawFractionBits = (ieee754_parameter<double>::fmask & bc);
	bits = bc;
}
#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT

inline std::string to_hex(float number, bool nibbleMarker = false, bool hexPrefix = true) {
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	uint32_t bits = std::bit_cast<uint32_t, float>(number);
	//	std::cout << "\nconvert  : " << to_binary(bits, 32) << " : " << bits << '\n';
	std::stringstream s;
	if (hexPrefix) s << "0x";
	int nrNibbles = 8;
	int nibbleIndex = (nrNibbles - 1);
	uint32_t mask = (0xFull << (nibbleIndex * 4));
	//	std::cout << "mask       : " << to_binary(mask, nbits) << '\n';
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint32_t raw = (bits & mask);
		uint8_t nibble = static_cast<uint8_t>(raw >> (nibbleIndex * 4));
		s << hexChar[nibble];
		if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
		mask >>= 4;
		--nibbleIndex;
	}
	return s.str();
}

inline std::string to_hex(double number, bool nibbleMarker = false, bool hexPrefix = true) {
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	uint64_t bits = std::bit_cast<uint64_t, double>(number);
	//	std::cout << "\nconvert  : " << to_binary(bits, 32) << " : " << bits << '\n';
	std::stringstream s;
	if (hexPrefix) s << "0x";
	int nrNibbles = 16;
	int nibbleIndex = (nrNibbles - 1);
	uint64_t mask = (0xFull << (nibbleIndex * 4));
	//	std::cout << "mask       : " << to_binary(mask, nbits) << '\n';
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint64_t raw = (bits & mask);
		uint8_t nibble = static_cast<uint8_t>(raw >> (nibbleIndex * 4));
		s << hexChar[nibble];
		if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
		mask >>= 4;
		--nibbleIndex;
	}
	return s.str();
}

#if LONG_DOUBLE_SUPPORT
// specialization to convert a long double to hex
template<>
inline std::string to_hex(long double number, bool nibbleMarker = false, bool hexPrefix = true) {
	//uint128_t bits = std::bit_cast<uint128_t, long double>(value);
	std::cerr << "bit_cast for long double is TBD\n";
}
#endif // LONG_DOUBLE_SUPPORT

}} // namespace sw::universal

