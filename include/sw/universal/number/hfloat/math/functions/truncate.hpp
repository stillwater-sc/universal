#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> trunc(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::trunc(double(x)));
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> round(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::round(double(x)));
}

// Round x downward, returning the largest integral value that is not greater than x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> floor(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::floor(double(x)));
}

// Round x upward, returning the smallest integral value that is greater than x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> ceil(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::ceil(double(x)));
}

}} // namespace sw::universal
