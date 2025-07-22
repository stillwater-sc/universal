#pragma once
// square.hpp: vectorized square function, takes a base and a vector of exponents, and returns vector of squares
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas {

// vector power function
template<typename Scalar>
vector<Scalar> square(const vector<Scalar>& y) {
	using namespace sw::universal;
	vector<Scalar> x(y);
	x *= y; // element-wise multiplication
	return x;
}

} } }  // namespace sw::universal::blas
