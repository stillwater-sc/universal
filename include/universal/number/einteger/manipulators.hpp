#pragma once
// manipulators.hpp: definition of manipulation functions for adaptiveint
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>

namespace sw { namespace universal {

// Generate a type tag for adaptiveint
template<typename BlockType>
std::string type_tag(const einteger<BlockType>& = {}) {
	std::stringstream s;
	s << "einteger<" << typeid(BlockType).name() << '>';
	return s.str();
}

}} // namespace sw::universal
