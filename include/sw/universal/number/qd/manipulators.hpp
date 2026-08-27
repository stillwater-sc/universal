#pragma once
// manipulators.hpp: definitions of helper functions for quad-double type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the qd headers (#1334): the string-producing half. These build a
// std::string, so they need <sstream> but not <iostream>; the stream operators are in
// iostream.hpp.
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cmath>
#include <type_traits>

#include <universal/number/qd/core.hpp>
#include <universal/number/qd/iostream.hpp>   // to_triple() formats a qd fraction THROUGH operator<<,
                                             // which is declared in the core but DEFINED there (#1334).
                                             // Without this a caller of to_triple() that includes only
                                             // this header gets an undefined reference at link time.
#include <universal/traits/qd_traits.hpp>
#include <string>
#include <iomanip>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/qd/qd_fwd.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for a quad-double
	template<typename QuadDoubleType,
		std::enable_if_t< is_qd<QuadDoubleType>, bool> = true>
	inline std::string type_tag(QuadDoubleType = {}) {
		return std::string("quad-double");
	}

	// generate a binary, color-coded representation of the quad-double
	inline std::string color_print(const qd& r, bool nibbleMarker = false) {
		std::stringstream s;
		for (int i = 0; i < 4; ++i) {
			std::string label = "x[" + std::to_string(i) + "]";
			s << std::setw(20) << label << " : ";
			s << color_print(r[i], nibbleMarker);
			if (i < 3) s << '\n';
		}
		return s.str();
	}

inline std::string to_quad(const qd& v, int precision = 17) {
	std::stringstream s;
	s << std::setprecision(precision) << "( " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ')';
	return s.str();
}

inline std::string to_triple(const qd& v, int precision = 17) {
	std::stringstream s;
	bool isneg = v.isneg();
	int scale = v.scale();
	int exponent;
	qd fraction = frexp(v, &exponent);
	s << '(' << (isneg ? '1' : '0') << ", " << scale << ", " << std::setprecision(precision) << fraction << ')';
	return s.str();
}

inline std::string to_binary(const qd& number, bool nibbleMarker = false) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number[0];	

	s << "0b";
	// print sign bit
	s << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x400;
		for (int bit = 10; bit >= 0; --bit) {
			s << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print first limb's fraction bits
	{
		uint64_t mask = (uint64_t(1) << 51);
		for (int bit = 51; bit >= 0; --bit) {
			s << ((decoder.parts.fraction & mask) ? '1' : '0');
			if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

// remove debugging statements when validated
//	auto defaultPrec = std::cout.precision();
//	std::cout << std::setprecision(7);
	// print the extension fraction bits
	// this is bit of a trick as there can be many different ways in which the limbs represent
	// more precise fraction bits

	// For quad-double we need to enumerate in the qd bit space, 
	// since we know the scale of the bits in this space, set by the scale of the first limb
	int limb{ 0 };
	int scaleOfBit = scale(number[limb++]) - 53;  // this is the scale of the first extension bit
	double bitValue = std::ldexp(1.0, scaleOfBit-1);
	constexpr int firstExtensionBit = 212 - 53;
	double segment = number[limb];
	// when do you know to switch to a new limb?
	for (int bit = firstExtensionBit; bit > 0; --bit) {
		if (bit == firstExtensionBit || bit == 106 || bit == 53) s << '|';
		double diff = segment - bitValue;
//		std::cout << "segment    : " << to_binary(segment) << " : " << segment << '\n';
//		std::cout << "bitValue   : " << to_binary(bitValue) << " : " << bitValue << '\n';
//		std::cout << "difference : " << diff << '\n';
		if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
		if (diff >= 0.0) {
			// segment > bitValue
			segment -= bitValue;
			s << '1';
		}
		else {
			s << '0';
		}
		bitValue /= 2;
		if (segment == 0.0) {
			// configurations where there are segments that are 0.0 have these segments
			// after non-zero segments. This logic is consistent, as the conditional
			// will avoid stepping out the segment array.
			if (limb < 3) segment = number[++limb];
		}
	}
//	std::cout << std::setprecision(defaultPrec);

	return s.str();
}

// native semantic representation: radix-2, delegates to to_binary
inline std::string to_native(const qd& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}

inline std::string to_components(const qd& number, bool nibbleMarker = false) {
	std::stringstream s;
	constexpr int nrLimbs = 4;
	for (int i = 0; i < nrLimbs; ++i) {
		double_decoder decoder;
		decoder.d = number[i];

		std::string label = "x[" + std::to_string(i) + "]";
		s << label << " : ";
		s << "0b";
		// print sign bit
		s << (decoder.parts.sign ? '1' : '0') << '.';

		// print exponent bits
		{
			uint64_t mask = 0x400;
			for (int bit = 10; bit >= 0; --bit) {
				s << ((decoder.parts.exponent & mask) ? '1' : '0');
				if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
				mask >>= 1;
			}
		}

		s << '.';

		// print hi fraction bits
		uint64_t mask = (uint64_t(1) << 51);
		for (int bit = 51; bit >= 0; --bit) {
			s << ((decoder.parts.fraction & mask) ? '1' : '0');
			if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
			mask >>= 1;
		}

		s << std::scientific << std::showpos << std::setprecision(15); // we are printing a double
		s << " : " << number[i] << " : binary scale " << scale(number[i]) << '\n';
	}

	return s.str();
}

}} // namespace sw::universal
