#pragma once
// manipulators.hpp: definition of manipulation functions for adaptive precision einteger
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>

namespace sw { namespace universal {

// Generate a type tag for adaptiveint
template<typename ElasticIntegerType,
	std::enable_if_t< is_einteger<ElasticIntegerType>, bool> = true>
std::string type_tag(const ElasticIntegerType & = {}) {
	std::stringstream s;
	s << "einteger<" << typeid(typename ElasticIntegerType::bt).name() << '>';
	return s.str();
}

}} // namespace sw::universal
