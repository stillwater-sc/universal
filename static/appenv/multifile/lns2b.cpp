// lns2b.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/lns2b/lns2b.hpp>
#include <iostream>
#include <vector>

using LNS2B8 = sw::universal::lns2b<8, 6>;

LNS2B8 lns2bPolynomial(const std::vector<float>& coef, const LNS2B8& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return LNS2B8(0);
	}

	LNS2B8 v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += LNS2B8(coef[i]) * LNS2B8(1.0); // until we implement a pow(x, LNS2B8(i));
	}
	return v;
}
