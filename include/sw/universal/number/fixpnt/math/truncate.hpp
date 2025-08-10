#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> trunc(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::trunc(double(x)));
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> round(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::round(double(x)));
}

// Round x downward, returning the largest integral value that is not greater than x
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> floor(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::floor(double(x)));
}

// Round x upward, returning the smallest integral value that is greater than x
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> ceil(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::ceil(double(x)));
}

}} // namespace sw::universal
