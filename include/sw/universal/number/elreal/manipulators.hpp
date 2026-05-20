#pragma once
// manipulators.hpp: helper functions for elreal type manipulation (Phase A stubs)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/elreal_impl.hpp>
#include <universal/traits/elreal_traits.hpp>

namespace sw { namespace universal {

// Generate a type tag for elreal values. elreal is not templated; the tag is
// fixed, but the function signature mirrors the rest of the library so generic
// reporting code can take any number-system value and ask for its tag.
inline std::string type_tag(const elreal& = {}) {
	return std::string("elreal");
}

// Inspect the materialised components of an elreal value. The full lazy
// stream is unbounded; this prints only what has actually been computed.
template<typename ElrealType,
	std::enable_if_t< is_elreal<ElrealType>, bool > = true
>
inline std::string to_components(const ElrealType& v) {
	std::stringstream s;
	const auto& c = v.components();
	s << "( ";
	for (std::size_t i = 0; i < c.size(); ++i) {
		s << std::setprecision(17) << c[i];
		if (i + 1 < c.size()) s << ", ";
	}
	s << " )";
	return s.str();
}

// Binary representation of the leading component. Full multi-component
// binary inspection lands in Phase B when conversion-out from rationals
// and strings is in place.
template<typename ElrealType,
	std::enable_if_t< is_elreal<ElrealType>, bool > = true
>
inline std::string to_binary(const ElrealType& v) {
	std::stringstream s;
	s << "elreal{ depth = " << v.computed_depth()
	  << ", leading = " << std::setprecision(17) << double(v) << " }";
	return s.str();
}

}} // namespace sw::universal
