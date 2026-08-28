#pragma once
// pow.hpp: pow functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>        // std::pow

namespace sw { namespace universal {

	inline erational pow(erational x, erational y) {
		return erational(std::pow(double(x), double(y)));
	}
		
	inline erational pow(erational x, int y) {
		return erational(std::pow(double(x), double(y)));
	}
		
	inline erational pow(erational x, double y) {
		return erational(std::pow(double(x), y));
	}

}} // namespace sw::universal
