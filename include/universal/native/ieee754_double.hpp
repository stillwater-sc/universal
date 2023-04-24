#pragma once
// ieee754_double.hpp: manipulation functions for IEEE-754 double precision floating-point type
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>

#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////////
// union structure helper

union double_decoder {
  double_decoder() : d{0.0} {}
  double_decoder(double _d) : d{_d} {}
  double d;
  struct {
    uint64_t fraction : 52;
    uint64_t exponent : 11;
    uint64_t sign     :  1;
  } parts;
};

inline void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits) noexcept {
	double_decoder decoder;
	decoder.d = value;
	s = decoder.parts.sign ? true : false;
	rawExponentBits = decoder.parts.exponent;
	rawFractionBits = decoder.parts.fraction;
}

inline void setFields(double& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
	double_decoder decoder;
	decoder.parts.sign = s;
	decoder.parts.exponent = rawExponentBits & 0x7FF;
	decoder.parts.fraction = rawFractionBits & 0xF'FFFF'FFFF'FFFF;
	value = decoder.d;
}

////////////////// string operators

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// native double precision IEEE floating point

// generate a binary string for a native double precision IEEE floating point
inline std::string to_hex(double number) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number;
	s << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return s.str();
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(double number, bool bNibbleMarker = false) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number;

	s << "0b";
	// print sign bit
	s << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x400;
		for (int i = 10; i >= 0; --i) {
			s << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	return s.str();
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(double number) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number;

	// print sign bit
	s << '(' << (decoder.parts.sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e - 127) to be one, e must be 127). 
	// Exponents range from -126 to +127 because exponents of -127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		s << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		s << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 1023;
	s << scale << ',';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	s << ')';
	return s.str();
}

// specialization for IEEE double precision floats
inline std::string to_base2_scientific(double number) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number;
	s << (decoder.parts.sign == 1 ? "-" : "+") << "1.";
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	} 
	s << "e" << std::showpos << (static_cast<int>(decoder.parts.exponent) - 1023);
	return s.str();
}


// ieee_components returns a tuple of sign, exponent, and fraction
inline std::tuple<bool, int, std::uint64_t> ieee_components(double fp)
{
	static_assert(std::numeric_limits<double>::is_iec559,
		"This function only works when double complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");

	double_decoder dd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int, std::uint64_t>(
		static_cast<bool>(dd.parts.sign), 
		static_cast<int>(dd.parts.exponent),
		static_cast<std::uint64_t>(dd.parts.fraction) 
	);
}

// generate a color coded binary string for a native double precision IEEE floating point
inline std::string color_print(double number) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number;

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
		uint64_t mask = 0x400;
		for (int i = 10; i >= 0; --i) {
			s << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) s << cyan << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		s << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) s << magenta << '\'';
		mask >>= 1;
	}

	s << def;
	return s.str();
}

}} // namespace sw::universal
