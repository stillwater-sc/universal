#pragma once
// manipulators.hpp: definition of manipulation functions for e8m0
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <iomanip>
#include <universal/number/e8m0/e8m0_fwd.hpp>
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for e8m0
	inline std::string type_tag(const e8m0& = {}) {
		return std::string("e8m0");
	}

	// generate a hex string for e8m0
	inline std::string to_hex(const e8m0& v, bool = false, bool hexPrefix = true) {
		char hexChar[16] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		};
		std::stringstream s;
		if (hexPrefix) s << "0x";
		uint8_t bits = v.bits();
		s << hexChar[(bits >> 4) & 0x0F] << hexChar[bits & 0x0F];
		return s.str();
	}

	// generate a color-coded representation
	inline std::string color_print(const e8m0& r, bool = false) {
		std::stringstream s;

		Color cyan(ColorCode::FG_CYAN);
		Color def(ColorCode::FG_DEFAULT);

		// all 8 bits are exponent
		uint8_t bits = r.bits();
		for (int j = 7; j >= 0; --j) {
			s << cyan << ((bits & (1u << j)) ? '1' : '0');
		}
		s << def;
		return s.str();
	}

}} // namespace sw::universal
