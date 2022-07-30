#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> trunc(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::trunc(double(x)));
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> round(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::round(double(x)));
}

// Round x downward, returning the largest integral value that is not greater than x
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> floor(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::floor(double(x)));
}

// Round x upward, returning the smallest integral value that is greater than x
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> ceil(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::ceil(double(x)));
}

}} // namespace sw::universal
