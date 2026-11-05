// areals.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/areal/areal.hpp>
#include <iostream>
#include <vector>

using Areal = sw::universal::areal<8, 2, uint8_t>;

Areal arealPolynomial(const std::vector<float>& coef, const Areal& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return Areal(0);
	}

	Areal v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += Areal(coef[i]) * Areal(1.0); // until we implement a pow(x, Areal(i));
	}
	return v;
}
