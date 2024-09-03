#pragma once
// minmax.hpp: minmax support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline dd min(dd x, dd y) {
		return dd(std::min(double(x), double(y)));
	}

	inline dd max(dd x, dd y) {
		return dd(std::max(double(x), double(y)));
	}

}} // namespace sw::universal
