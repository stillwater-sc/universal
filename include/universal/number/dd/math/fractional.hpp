#pragma once
// fractional.hpp: fractional support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fmod retuns x - n*y where n = x/y with the fractional part truncated
	dd fmod(dd x, dd y) {
		return dd(std::fmod(double(x), double(y)));
	}

	// shim to stdlib
	dd remainder(dd x, dd y) {
		return dd(std::remainder(double(x), double(y)));
	}

}} // namespace sw::universal
