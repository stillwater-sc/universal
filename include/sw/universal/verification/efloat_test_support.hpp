#pragma once
// efloat_test_support.hpp: functions to support methodical testing of the efloat type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <random>
#include <universal/number/efloat/efloat.hpp>

namespace sw { namespace universal {

// Generate a multi-component efloat expansion of varying length, signs, and
// exponent ranges.
template <unsigned maxlimbs>
efloat<maxlimbs> random_efloat(std::mt19937_64& rng, double leading_magnitude = 1e6) {
	std::uniform_int_distribution<unsigned> n_dist(1, maxlimbs);
	std::uniform_int_distribution<int> sign_dist(0, 1);
	std::uniform_real_distribution<double> unit(0.5, 2.0);

	efloat<maxlimbs> result(0.0);
	unsigned n_limbs = n_dist(rng);
	double mag = leading_magnitude;
	for (unsigned i = 0; i < n_limbs; ++i) {
		double v = unit(rng) * mag * (sign_dist(rng) ? 1.0 : -1.0);
		result += v;
		mag = std::ldexp(mag, -50);
		if (mag < std::ldexp(1.0, -950)) break;
	}
	return result;
}

}} // namespace sw::universal
