#pragma once
// attributes.hpp: functions to query number system attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>

namespace sw { namespace universal {

	inline int scale(e8m0 v) {
		return v.scale();
	}

	// generate the range of an e8m0 configuration
	inline std::string e8m0_range() {
		e8m0 v{};
		std::stringstream s;
		s << std::setw(40) << type_tag(v) << " : [ "
			<< v.minpos() << " ... "
			<< v.maxpos() << " ]"
			<< " (2^-127 ... 2^127)";
		return s.str();
	}

}}  // namespace sw::universal
