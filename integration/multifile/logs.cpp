// logs.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/lns/lns.hpp>
#include <iostream>
#include <vector>

using LNS8 = sw::universal::lns<8, 2, std::uint8_t>;

LNS8 lnsPolynomial(const std::vector<float>& coef, const LNS8& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return LNS8(0);
	}

	LNS8 v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += LNS8(coef[i]) * LNS8(1.0); // until we implement a pow(x, LNS8(i));
	}
	return v;
}
