#pragma once
// manipulators.hpp: definition of manipulation functions for dd_cascade
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

	// Generate a type tag for this dd_cascade
	inline std::string type_tag(const dd_cascade& = {}) {
		return "dd_cascade";
	}

	// Generate a string representing the dd_cascade components
	inline std::string components(const dd_cascade& v) {
		std::stringstream s;
		s << "[ "
		  << std::setw(15) << v.high() << ", "
		  << std::setw(15) << v.low() << " ]";
		return s.str();
	}

	// Generate a binary string for the dd_cascade
	inline std::string to_binary(const dd_cascade& number, bool bNibbleMarker = false) {
		std::stringstream s;
		s << "dd_cascade["
		  << to_binary(number.high(), bNibbleMarker) << ", "
		  << to_binary(number.low(), bNibbleMarker) << "]";
		return s.str();
	}

	// Generate a hexadecimal string for the dd_cascade
	inline std::string to_hex(const dd_cascade& number, bool bNibbleMarker = false, bool bUpperCase = true) {
		std::stringstream s;
		s << "dd_cascade["
		  << to_hex(number.high(), bNibbleMarker, bUpperCase) << ", "
		  << to_hex(number.low(), bNibbleMarker, bUpperCase) << "]";
		return s.str();
	}

	// Generate a color-coded string showing the high and low components
	inline std::string color_print(const dd_cascade& number) {
		std::stringstream s;
		s << "dd_cascade[ high: " << number.high() << ", low: " << number.low() << " ]";
		return s.str();
	}

	// Generate a pretty-printed representation
	inline std::string pretty_print(const dd_cascade& number, int precision = 17) {
		std::stringstream s;
		s << std::setprecision(precision);
		s << "dd_cascade value: " << number;
		return s.str();
	}

	// Report the type and value of a dd_cascade
	inline std::string info_print(const dd_cascade& v, int precision = 17) {
		return pretty_print(v, precision);
	}

}} // namespace sw::universal
