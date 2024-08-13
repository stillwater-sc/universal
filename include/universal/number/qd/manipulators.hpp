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
	inline std::string type_tag(const qd& = {}) {
		return std::string("quad-double");
	}

	// generate a binary, color-coded representation of the quad-double
	inline std::string color_print(qd const& r, bool nibbleMarker = false) {
		//constexpr unsigned es = 11;
		//constexpr unsigned fbits = 106;
		std::stringstream s;

		/*
		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		*/
		for (int i = 0; i < 4; ++i) {
			s << color_print(r[i], nibbleMarker);
			if (i < 3) s << ", ";
		}
		return s.str();
	}

}} // namespace sw::universal
