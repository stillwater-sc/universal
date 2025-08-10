#pragma once
// minmax.hpp: min/max functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	erational min(erational x, erational y) {
		return erational(std::min(double(x), double(y)));
	}

	erational max(erational x, erational y) {
		return erational(std::max(double(x), double(y)));
	}

}} // namespace sw::universal
