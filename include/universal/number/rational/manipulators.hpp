#pragma once
// manipulators.hpp: definition of manipulation functions for rational types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Generate a type tag for rational type
std::string type_tag(const rational& = {}) {
	return "rational";
}

}} // namespace sw::universal
