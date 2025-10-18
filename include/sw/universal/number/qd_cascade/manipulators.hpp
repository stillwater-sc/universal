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
	inline std::string components(const qd_cascade& v) {
		std::stringstream s;
		s << "[ "
		  << std::setw(15) << v[0] << ", "
		  << std::setw(15) << v[1] << ", "
		  << std::setw(15) << v[2] << ", "
		  << std::setw(15) << v[3] << " ]";
		return s.str();
	}

	// Generate a binary string for the qd_cascade
	inline std::string to_binary(const qd_cascade& number, bool bNibbleMarker = false) {
		std::stringstream s;
		s << "qd_cascade["
		  << to_binary(number[0], bNibbleMarker) << ", "
		  << to_binary(number[1], bNibbleMarker) << ", "
		  << to_binary(number[2], bNibbleMarker) << ", "
		  << to_binary(number[3], bNibbleMarker) << "]";
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
