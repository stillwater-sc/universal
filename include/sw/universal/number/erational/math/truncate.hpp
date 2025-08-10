#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
	erational trunc(erational x) {
		return erational(std::trunc(double(x)));
	}

	// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
	erational round(erational x) {
		return erational(std::round(double(x)));
	}

	// Round x downward, returning the largest integral value that is not greater than x
	erational floor(erational x) {
		return erational(std::floor(double(x)));
	}

	// Round x upward, returning the smallest integral value that is greater than x
	erational ceil(erational x) {
		return erational(std::ceil(double(x)));
	}

}} // namespace sw::universal
