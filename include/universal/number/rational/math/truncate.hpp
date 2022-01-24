#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
rational trunc(rational x) {
	return rational(std::trunc(double(x)));
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
rational round(rational x) {
	return rational(std::round(double(x)));
}

// Round x downward, returning the largest integral value that is not greater than x
rational floor(rational x) {
	return rational(std::floor(double(x)));
}

// Round x upward, returning the smallest integral value that is greater than x
rational ceil(rational x) {
	return rational(std::ceil(double(x)));
}

}} // namespace sw::universal
