#pragma once
// power.hpp: vectorized power function, takes a base and a vector of exponents, and returns vector of exponentiations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas {

// vector power function
template<typename Scalar1, typename Scalar2>
vector<Scalar1> power(const Scalar1& x, const vector<Scalar2>& y) {
	using std::pow;
	using namespace sw::universal;
	vector<Scalar1> v(y.size());
	for (size_t i = 0; i < y.size(); ++i) {
		v[i] = pow(x, Scalar1(y[i]));
	}
	return v;
}

} } }  // namespace sw::universal::blas
