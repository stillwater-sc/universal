#pragma once
// manipulators.hpp: definition of manipulation functions for triple-double cascade (td_cascade)
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

	// Generate a type tag for this td_cascade
	inline std::string type_tag(const td_cascade& = {}) {
		return "td_cascade";
	}

	// Generate a string representing the td_cascade components
	inline std::string components(const td_cascade& v) {
		std::stringstream s;
		s << "[ "
		  << std::setw(15) << v[0] << ", "
		  << std::setw(15) << v[1] << ", "
		  << std::setw(15) << v[2] << " ]";
		return s.str();
	}

	// Note: to_binary is defined in td_cascade_impl.hpp to avoid duplication

	// Generate a hexadecimal string for the td_cascade
	inline std::string to_hex(const td_cascade& number, bool bNibbleMarker = false, bool bUpperCase = true) {
		std::stringstream s;
		s << "td_cascade["
		  << to_hex(number[0], bNibbleMarker, bUpperCase) << ", "
		  << to_hex(number[1], bNibbleMarker, bUpperCase) << ", "
		  << to_hex(number[2], bNibbleMarker, bUpperCase) << "]";
		return s.str();
	}

	// Generate a color-coded string showing all three components
	inline std::string color_print(const td_cascade& number) {
		std::stringstream s;
		s << "td_cascade[ "
		  << "c0: " << number[0] << ", "
		  << "c1: " << number[1] << ", "
		  << "c2: " << number[2] << " ]";
		return s.str();
	}

	// Generate a pretty-printed representation
	inline std::string pretty_print(const td_cascade& number, int precision = 17) {
		std::stringstream s;
		s << std::setprecision(precision);
		s << "td_cascade value: " << number;
		return s.str();
	}

	// Report the type and value of a td_cascade
	inline std::string info_print(const td_cascade& v, int precision = 17) {
		return pretty_print(v, precision);
	}

}} // namespace sw::universal
