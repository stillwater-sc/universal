#pragma once
// manipulators.hpp: definition of manipulation functions for qd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <universal/native/integers.hpp>  // for hex_print

namespace sw { namespace universal {

	// Generate a type tag for this qd_cascade
	inline std::string type_tag(const qd_cascade& = {}) {
		return "qd_cascade";
	}

	// Generate a string representing the qd_cascade components
	inline std::string to_components(const qd_cascade& v, std::streamsize width = 17) {
		std::stringstream s;
		s << "( "
		  << std::setw(width) << v[0] << ", " 
		  << std::setw(width) << v[1] << ", " 
		  << std::setw(width) << v[2] << ", " 
		  << std::setw(width) << v[3] << " )";
		return s.str();
	}

	inline std::string to_quad(const qd_cascade& v, int precision = 17) {
	    std::stringstream s;
	    s << std::setprecision(precision) << "( " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ')';
	    return s.str();
    }

    inline std::string to_triple(const qd_cascade& v, int precision = 17) {
	    std::stringstream s;
	    bool              isneg = v.isneg();
	    int               scale = v.scale();
	    int               exponent;
	    qd_cascade        fraction = frexp(v, &exponent);
	    s << '(' << (isneg ? '1' : '0') << ", " << scale << ", " << std::setprecision(precision) << fraction << ')';
	    return s.str();
    }

    inline std::string to_binary(const qd_cascade& number, bool nibbleMarker = false) {
	    std::stringstream s;
	    double_decoder    decoder;
	    decoder.d = number[0];

	    s << "0b";
	    // print sign bit
	    s << (decoder.parts.sign ? '1' : '0') << '.';

	    // print exponent bits
	    {
		    uint64_t mask = 0x400;
		    for (int bit = 10; bit >= 0; --bit) {
			    s << ((decoder.parts.exponent & mask) ? '1' : '0');
			    if (nibbleMarker && bit != 0 && (bit % 4) == 0)
				    s << '\'';
			    mask >>= 1;
		    }
	    }

	    s << '.';

	    // print first limb's fraction bits
	    {
		    uint64_t mask = (uint64_t(1) << 51);
		    for (int bit = 51; bit >= 0; --bit) {
			    s << ((decoder.parts.fraction & mask) ? '1' : '0');
			    if (nibbleMarker && bit != 0 && (bit % 4) == 0)
				    s << '\'';
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
	    int           limb{0};
	    int           scaleOfBit        = scale(number[limb++]) - 53;  // this is the scale of the first extension bit
	    double        bitValue          = std::ldexp(1.0, scaleOfBit - 1);
	    constexpr int firstExtensionBit = 212 - 53;
	    double        segment           = number[limb];
	    // when do you know to switch to a new limb?
	    for (int bit = firstExtensionBit; bit > 0; --bit) {
		    if (bit == firstExtensionBit || bit == 106 || bit == 53)
			    s << '|';
		    double diff = segment - bitValue;
		    //		std::cout << "segment    : " << to_binary(segment) << " : " << segment << '\n';
		    //		std::cout << "bitValue   : " << to_binary(bitValue) << " : " << bitValue << '\n';
		    //		std::cout << "difference : " << diff << '\n';
		    if (nibbleMarker && bit != 0 && (bit % 4) == 0)
			    s << '\'';
		    if (diff >= 0.0) {
			    // segment > bitValue
			    segment -= bitValue;
			    s << '1';
		    } else {
			    s << '0';
		    }
		    bitValue /= 2;
		    if (segment == 0.0) {
			    // configurations where there are segments that are 0.0 have these segments
			    // after non-zero segments. This logic is consistent, as the conditional
			    // will avoid stepping out the segment array.
			    if (limb < 3)
				    segment = number[++limb];
		    }
	    }
	    //	std::cout << std::setprecision(defaultPrec);

	    return s.str();
    }

	// Generate a hexadecimal string for the qd_cascade
	inline std::string to_hex(const qd_cascade& number, bool bNibbleMarker = false, bool bUpperCase = true) {
		std::stringstream s;
		s << "qd_cascade["
		  << to_hex(number[0], bNibbleMarker, bUpperCase) << ", "
		  << to_hex(number[1], bNibbleMarker, bUpperCase) << ", "
		  << to_hex(number[2], bNibbleMarker, bUpperCase) << ", "
		  << to_hex(number[3], bNibbleMarker, bUpperCase) << "]";
		return s.str();
	}

	// Generate a color-coded string showing all four components
	inline std::string color_print(const qd_cascade& number) {
		std::stringstream s;
		s << "qd_cascade[ "
		  << "c0: " << number[0] << ", "
		  << "c1: " << number[1] << ", "
		  << "c2: " << number[2] << ", "
		  << "c3: " << number[3] << " ]";
		return s.str();
	}

	// Generate a pretty-printed representation
	inline std::string pretty_print(const qd_cascade& number, int precision = 17) {
		std::stringstream s;
		s << std::setprecision(precision);
		s << "qd_cascade value: " << number;
		return s.str();
	}

	// Report the type and value of a qd_cascade
	inline std::string info_print(const qd_cascade& v, int precision = 17) {
		return pretty_print(v, precision);
	}

}} // namespace sw::universal
