#pragma once
// truncate.hpp: truncate support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
	inline dd trunc(dd x) {
		return dd(std::trunc(double(x)));
	}

	// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
	inline dd round(dd x) {
		return dd(std::round(double(x)));
	}


	// floor and ceil are being used in the class definition and are defined in that file

}} // namespace sw::universal
