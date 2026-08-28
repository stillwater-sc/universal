// edecimals.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This translation unit exists to be LINKED against main.cpp: a header that defines a
// non-template free function without inline compiles fine in one TU and fails at link
// time with two. edecimal had 12 such symbols (#1424) and no multifile coverage, which
// is exactly why they went unnoticed.
#include <universal/number/edecimal/edecimal.hpp>
#include <vector>

using Decimal = sw::universal::edecimal;

Decimal edecimalPolynomial(const std::vector<int>& coef, const Decimal& x) {
	if (coef.size() < 2) return Decimal(0);

	Decimal v = coef[0];
	Decimal xn = 1;
	for (size_t i = 1; i < coef.size(); ++i) {
		xn *= x;
		v += Decimal(coef[i]) * xn;
	}
	return v;
}
