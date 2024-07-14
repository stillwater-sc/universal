#pragma once
// ieee754_float.hpp: manipulation functions for IEEE-754 single precision floating-point type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

////////////////// string operators

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// native single precision IEEE floating point

#ifdef DEPRECATED
// DEPRECATED: we have standardized on raw bit hex, not field hex format
// generate a binary string for a native single precision IEEE floating point
inline std::string to_hex(float number) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;
	s << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return s.str();
}
#endif // DEPRECATED

inline std::string to_hex(float number, bool nibbleMarker = false, bool hexPrefix = true) {
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	float_decoder decoder;
	decoder.f = number;
	uint32_t bits = decoder.bits;
	//	std::cout << "\nconvert  : " << to_binary(bits, 32) << " : " << bits << '\n';
	std::stringstream s;
	if (hexPrefix) s << "0x";
	int nrNibbles = 8;
	int nibbleIndex = (nrNibbles - 1);
	uint32_t mask = static_cast<uint32_t>(0xFull << (nibbleIndex * 4));
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

// generate a binary string for a native single precision IEEE floating point
inline std::string to_binary(float number, bool bNibbleMarker = false) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;

	s << "0b";
	// print sign bit
	s << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint8_t mask = 0x80;
		for (int i = 7; i >= 0; --i) {
			s << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	return s.str();
}

// return in triple form (sign, scale, fraction)
inline std::string to_triple(float number, bool nibbleMarker = false) {
	std::stringstream s;

	float_decoder decoder;
	decoder.f = number;

	// print sign bit
	s << '(' << (decoder.parts.sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e - 127) to be one, e must be 127). 
	// Exponents range from ¿126 to +127 because exponents of ¿127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		s << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		s << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 127;
	s << scale << ",0b";

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (nibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	s << ')';
	return s.str();
}

// specialization for IEEE single precision floats
inline std::string to_base2_scientific(float number) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;
	s << (decoder.parts.sign == 1 ? "-" : "+") << "1.";
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}
	s << "e" << std::showpos << (static_cast<int>(decoder.parts.exponent) - 127);
	return s.str();
}

// ieee_components returns a tuple of sign, exponent, and fraction
inline std::tuple<bool, int, uint32_t> ieee_components(float fp)
{
	static_assert(std::numeric_limits<float>::is_iec559,
		"This function only works when float complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");

	float_decoder fd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int, uint32_t>(
		static_cast<bool>(fd.parts.sign), 
		static_cast<int>(fd.parts.exponent),
		static_cast<uint32_t>(fd.parts.fraction) 
	);
}

// generate a color coded binary string for a native single precision IEEE floating point
inline std::string color_print(float number) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// print prefix
	s << yellow << "0b";

	// print sign bit
	s << red << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint8_t mask = 0x80;
		for (int i = 7; i >= 0; --i) {
			s << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) s << cyan << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) s << magenta << '\'';
		mask >>= 1;
	}
	
	s << def;
	return s.str();
}

}} // namespace sw::universal
