#pragma once
// hypot.hpp: hypot support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	dd hypot(dd x, dd y) {
		return dd(std::hypot(double(x), double(y)));
	}

}} // namespace sw::universal
