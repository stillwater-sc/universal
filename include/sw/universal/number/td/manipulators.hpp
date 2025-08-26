// manipulators.hpp: definitions of helper functions for triple-double (td) type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <iomanip>
#include <universal/number/td/td_fwd.hpp>
#include <universal/traits/td_traits.hpp>
#include <universal/native/manipulators.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for a triple-double
	template<typename TripleDoubleType,
		std::enable_if_t< is_td<TripleDoubleType>, bool> = true >
	std::string type_tag(TripleDoubleType = {}) {
		return std::string("triple-double");
	}

	// generate a binary, color-coded representation of the triple-double
	inline std::string color_print(const td& r, bool nibbleMarker = false) {
		std::stringstream s;
		const double high = r[0];
		const double mid  = r[1];
		const double low  = r[2];
		s << color_print<double>(high, nibbleMarker) << ", " 
		  << color_print<double>(mid, nibbleMarker) <<  ", " 
		  << color_print<double>(low, nibbleMarker);
		return s.str();
	}

}} // namespace sw::universal
