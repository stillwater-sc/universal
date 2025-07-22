#pragma once
// manipulators.hpp: definition of manipulation functions for integer types
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>
namespace sw { namespace universal {

// Generate a type tag for general integer
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
std::string type_tag(const integer<nbits, BlockType, NumberType>& = {}) {
	std::stringstream str;
	str << "integer<"
	    << std::setw(4) << nbits << ", "
	    << typeid(BlockType).name() << ", "
		<< typeid(NumberType).name() << '>';
	return str.str();
}

// Generate a type field descriptor for this integer
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string type_field(const integer<nbits, BlockType, NumberType> & = {}) {
	std::stringstream s;
	s << "fields(i:" << nbits << ')';
	if constexpr (NumberType == IntegerNumberType::NaturalNumber) {
		s << " unsigned without 0";
	}
	else if constexpr (NumberType == IntegerNumberType::WholeNumber) {
			s << " unsigned including 0";
	}
	else if constexpr (NumberType == IntegerNumberType::IntegerNumber) {
		s << " signed 2's complement";
	}
	else {
		s << " unknown integer Number Type";
	}
	return s.str();
}

// return hex format
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string to_hex(const integer<nbits, BlockType, NumberType>& v, bool nibbleMarker = false, bool hexPrefix = true) {
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream s;
	if (hexPrefix) s << "0x" << std::hex;
	int nrNibbles = int(1ull + ((nbits - 1ull) >> 2ull));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(unsigned(n));
		s << hexChar[nibble];
		if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
	}
	return s.str();
}

// return in triple form (sign, scale, fraction)
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string to_triple(const integer<nbits, BlockType, NumberType>& number) {
	using Integer = integer<nbits+1u, BlockType, NumberType>; // to capture maxneg in magnitude conversion

	std::stringstream str;

	// print sign bit
	str << '(' << (number < 0 ? '-' : '+') << ',';

	Integer magnitude = (number < 0 ? -number : number);

	// scale
	str << scale(magnitude) << ',';

	// print fraction bits
	long msb = findMsb(magnitude);
	if (msb < 0) {
		str << '-';
	}
	else {
		// the msb becomes the hidden bit
		magnitude <<= (nbits - msb);
		for (int i = nbits - 1; i >= 0; --i) {
			str << (magnitude.at(static_cast<unsigned>(i)) ? '1' : '0');
		}
	}

	str << ')';
	return str.str();
}

template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string info_print(const integer<nbits, BlockType, NumberType>& number) {
	return std::string("TBD");
}

template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string pretty_print(const integer<nbits, BlockType, NumberType>& number, bool nibbleMarker = false) {
	std::stringstream s;

	// integer bits
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << '\'';
	}

	return s.str();
}

template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string color_print(const integer<nbits, BlockType, NumberType>& number, bool nibbleMarker = false) {

	std::stringstream s;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// integer bits
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		s << cyan << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << yellow << '\'';
	}

	s << def;
	return s.str();
}

}} // namespace sw::universal
