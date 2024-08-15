// manipulators.hpp: definitions of helper functions for double-double (dd) type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <iomanip>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/dd/dd_fwd.hpp>
#include <universal/native/manipulators.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for a doubledouble
	std::string type_tag(const dd& = {}) {
		return std::string("double-double");
	}

	// generate a binary, color-coded representation of the doubledouble
	std::string color_print(const dd& r, bool nibbleMarker = false) {
		std::stringstream s;
		double high = r.high();
		double low = r.low();
		s << color_print<double>(high, nibbleMarker) << ", " << color_print<double>(low, nibbleMarker);
		return s.str();
	}

}} // namespace sw::universal
