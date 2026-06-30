// minmax.hpp: minimum, maximum, and positive difference functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/efloat/math/classify.hpp>

namespace sw { namespace universal {

// min: returns the smaller of two efloat values
template<unsigned nlimbs>
constexpr efloat<nlimbs> min(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	return (x < y) ? x : y;
}

// max: returns the larger of two efloat values
template<unsigned nlimbs>
constexpr efloat<nlimbs> max(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	return (x < y) ? y : x;
}

// fmin: returns the smaller of two efloat values, suppressing NaN if only one is NaN
template<unsigned nlimbs>
constexpr efloat<nlimbs> fmin(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan()) return y;
	if (y.isnan()) return x;
	if (x.iszero() && y.iszero()) {
		// IEEE-754: fmin(-0.0, +0.0) must return -0.0
		return (x.sign() == -1) ? x : y;
	}
	return (x < y) ? x : y;
}

// fmax: returns the larger of two efloat values, suppressing NaN if only one is NaN
template<unsigned nlimbs>
constexpr efloat<nlimbs> fmax(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan()) return y;
	if (y.isnan()) return x;
	if (x.iszero() && y.iszero()) {
		// IEEE-754: fmax(-0.0, +0.0) must return +0.0
		return (x.sign() == 1) ? x : y;
	}
	return (x < y) ? y : x;
}

// fdim: returns the positive difference between x and y (x - y if x > y, +0.0 otherwise)
template<unsigned nlimbs>
constexpr efloat<nlimbs> fdim(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	if (x > y) return x - y;
	return efloat<nlimbs>(0.0);
}

}} // namespace sw::universal
