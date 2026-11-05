// dds.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/qd/qd.hpp>
#include <iostream>
#include <vector>

using QuadDouble = sw::universal::qd;

QuadDouble qdPolynomial(const std::vector<double>& coef, const QuadDouble& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return QuadDouble(0);
	}

	QuadDouble v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += coef[i] * pow(x, QuadDouble(i)); 
	}
	return v;
}
