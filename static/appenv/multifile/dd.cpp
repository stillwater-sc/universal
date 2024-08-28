// dds.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dd/dd.hpp>
#include <iostream>
#include <vector>

using DoubleDouble = sw::universal::dd;

DoubleDouble ddPolynomial(const std::vector<double>& coef, const DoubleDouble& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return DoubleDouble(0);
	}

	DoubleDouble v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += coef[i] * pow(x, DoubleDouble(i)); 
	}
	return v;
}
