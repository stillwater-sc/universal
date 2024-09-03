// posits.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>
#include <iostream>
#include <vector>

using Posit = sw::universal::posit<8, 2>;

Posit positPolynomial(const std::vector<float>& coef, const Posit& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return Posit(0);
	}

	Posit v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += Posit(coef[i]) * pow(x, Posit(i));
	}
	return v;
}
