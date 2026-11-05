// erationals.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Linked against main.cpp to catch ODR violations that a single translation unit cannot
// see. erational had 68 duplicate symbols before #1424 -- its whole mathlib plus the
// comparison operators -- and no multifile coverage.
#include <universal/number/erational/erational.hpp>
#include <vector>

using Rational = sw::universal::erational;

Rational erationalSum(const std::vector<int>& numerators, int denominator) {
	Rational sum(0);
	for (size_t i = 0; i < numerators.size(); ++i) {
		Rational term;
		term = numerators[i];
		term /= Rational(denominator);
		sum += term;
	}
	return sum;
}
