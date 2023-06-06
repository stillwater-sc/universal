#pragma once
// babylonian.hpp: babylonian sqrt algorithms
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

namespace sw { namespace universal {

// straight Babylonian
inline double babylonian(double v) {
	double x_n = 0.5 * v; // initial guess
	const double eps = 1.0e-7;   // 
	do {
		x_n = (x_n + v / x_n) / 2.0;
	} while (std::abs(x_n * x_n - v) > eps);

	return x_n;
}

}} // namespace sw::universal
