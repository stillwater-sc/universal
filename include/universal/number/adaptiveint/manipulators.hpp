#pragma once
// manipulators.hpp: definition of manipulation functions for adaptiveint
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>

namespace sw { namespace universal {

// Generate a type tag for adaptiveint
std::string type_tag(const adaptiveint& v) {
	std::stringstream s;
	if (v.iszero()) s << ' '; // remove 'unreferenced formal parameter warning from compilation log
	s << "adaptiveint";
	return s.str();
}


#ifdef ADAPTIVEINT_TYPE_GUARD
// Generate a type tag for adaptiveint
template<typename AdaptiveInt>
std::string type_tag() {
	return type_tag(adaptiveint());
}
#endif

}} // namespace sw::universal
