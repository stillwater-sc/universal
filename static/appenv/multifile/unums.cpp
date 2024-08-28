// unums.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/unum/unum.hpp>
#include <iostream>
#include <vector>

using Unum = sw::universal::unum<8, 2>;

Unum unumPolynomial(const std::vector<float>& coef, const Unum& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return Unum(0);
	}

	Unum v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += Unum(coef[i]) * Unum(1.0); // until we implement a pow(x, Unum(i));
	}
	return v;
}
