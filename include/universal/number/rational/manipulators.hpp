#pragma once
// manipulators.hpp: definition of manipulation functions for rational types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Generate a type tag for rational type
std::string type_tag(rational& v) {
	std::stringstream str;
	if (v.iszero()) str << ' '; // remove 'unreferenced formal parameter warning from compilation log
	str << "rational";
	return str.str();
}

}} // namespace sw::universal
