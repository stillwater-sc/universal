#pragma once
// logarithm.hpp: logarithm functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Natural logarithm of x
	erational log(erational x) {
		return erational(std::log(double(x)));
	}

	// Binary logarithm of x
	erational log2(erational x) {
		return erational(std::log2(double(x)));
	}

	// Decimal logarithm of x
	erational log10(erational x) {
		return erational(std::log10(double(x)));
	}
		
	// Natural logarithm of 1+x
	erational log1p(erational x) {
		return erational(std::log1p(double(x)));
	}

}} // namespace sw::universal
