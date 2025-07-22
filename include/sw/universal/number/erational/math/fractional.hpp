#pragma once
// fractional.hpp: fractional functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	erational fmod(erational x, erational y) {
		return erational(std::fmod(double(x), double(y)));
	}

	erational remainder(erational x, erational y) {
		return erational(std::remainder(double(x), double(y)));
	}

	erational frac(erational x) {
		return erational(double(x)-long(x));
	}

}} // namespace sw::universal
