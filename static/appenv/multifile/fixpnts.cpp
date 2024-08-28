// fixpnts.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/fixpnt/fixpnt.hpp>
#include <iostream>
#include <vector>

using fp8_4s = sw::universal::fixpnt<8, 4, sw::universal::Saturate, uint8_t>;

fp8_4s fixpntPolynomial(const std::vector<float>& coef, const fp8_4s& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return fp8_4s(0);
	}

	fp8_4s v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += fp8_4s(coef[i]) * pow(x, fp8_4s(i));
	}
	return v;
}
