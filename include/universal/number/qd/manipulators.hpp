// manipulators.hpp: definitions of helper functions for quad-double type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
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

}} // namespace sw::universal
