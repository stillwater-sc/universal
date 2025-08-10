#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
template<unsigned nbits, unsigned rbits, typename bt,  auto... xtra>
lns<nbits, rbits, bt, xtra...> trunc(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::trunc(double(x)));
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
template<unsigned nbits, unsigned rbits, typename bt,  auto... xtra>
lns<nbits, rbits, bt, xtra...> round(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::round(double(x)));
}

// Round x downward, returning the largest integral value that is not greater than x
template<unsigned nbits, unsigned rbits, typename bt,  auto... xtra>
lns<nbits, rbits, bt, xtra...> floor(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::floor(double(x)));
}

// Round x upward, returning the smallest integral value that is greater than x
template<unsigned nbits, unsigned rbits, typename bt,  auto... xtra>
lns<nbits, rbits, bt, xtra...> ceil(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::ceil(double(x)));
}

}} // namespace sw::universal
