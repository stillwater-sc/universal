// dfloats.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Linked against main.cpp to catch ODR violations. dfloat's type_tag(DecimalEncoding) was
// a non-template free function defined in a header without inline (#1424).
#include <universal/number/dfloat/dfloat.hpp>
#include <vector>

using Decimal32 = sw::universal::decimal32;

Decimal32 dfloatSum(const std::vector<double>& values) {
	Decimal32 sum(0);
	for (size_t i = 0; i < values.size(); ++i) {
		sum += Decimal32(values[i]);
	}
	return sum;
}
