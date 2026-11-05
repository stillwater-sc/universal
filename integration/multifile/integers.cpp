
// integers.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/integer/integer.hpp>
#include <iostream>
#include <vector>

using Integer = sw::universal::integer<8, uint8_t, sw::universal::IntegerNumberType::IntegerNumber>;

Integer integerPolynomial(const std::vector<int>& coef, const Integer& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return Integer(0);
	}

	Integer v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += Integer(coef[i]) * pow(x, Integer(i));
	}
	return v;
}
