#pragma once
// manipulators.hpp: definition of manipulation functions for bfloat
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>

namespace sw { namespace universal {

// Generate a type tag for bfloat16
std::string type_tag(const bfloat16& = {}) {
	return std::string("bfloat16");
}

}} // namespace sw::universal
