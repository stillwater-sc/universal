// mxfloats.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// main.cpp includes mxfloat.hpp too, so a max_elem_exponent specialization without `inline`
// is defined in two translation units and the link fails (#2083).
#include <universal/number/mxfloat/mxfloat.hpp>
#include <cstddef>
#include <vector>

float mxint8Roundtrip(const std::vector<float>& v) {
	using namespace sw::universal;
	mxint8 b;
	b.quantize(v.data(), v.size());
	float sum = 0.0f;
	for (std::size_t i = 0; i < v.size() && i < mxint8::blockSize; ++i) sum += b[i];
	return sum + static_cast<float>(mxblock<e2m1>::elemMaxExp);
}
