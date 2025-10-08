// bfloat16.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/bfloat16/bfloat16.hpp>
#include <iostream>
#include <vector>

using Bfloat16 = sw::universal::bfloat16;

Bfloat16 bfloat16Polynomial(const std::vector<float>& coef, const Bfloat16& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return Bfloat16(0);
	}

	Bfloat16 v = Bfloat16(coef[0]);  // need explicit conversion to avoid implicit double conversion
	for (size_t i = 1; i < coef.size(); ++i) {
		v += Bfloat16(coef[i]) * Bfloat16(1.0); // until we implement a pow(x, Areal(i));
	}
	return v;
}
