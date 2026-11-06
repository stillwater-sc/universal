// dbns.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dbns/dbns.hpp>
#include <iostream>
#include <vector>

using DBNS8 = sw::universal::dbns<8, 6>;

DBNS8 dbnsPolynomial(const std::vector<float>& coef, const DBNS8& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return DBNS8(0);
	}

	DBNS8 v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += DBNS8(coef[i]) * DBNS8(1.0); // until we implement a pow(x, DBNS8(i));
	}
	return v;
}
