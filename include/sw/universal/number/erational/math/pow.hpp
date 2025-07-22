#pragma once
// pow.hpp: pow functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	erational pow(erational x, erational y) {
		return erational(std::pow(double(x), double(y)));
	}
		
	erational pow(erational x, int y) {
		return erational(std::pow(double(x), double(y)));
	}
		
	erational pow(erational x, double y) {
		return erational(std::pow(double(x), y));
	}

}} // namespace sw::universal
