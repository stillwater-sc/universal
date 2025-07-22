#pragma once
// sqrt.hpp: vectorized square root function, takes a base and a vector of exponents, and returns vector of square roots
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <numeric/containers.hpp>

namespace sw { namespace blas {
	using namespace sw::numeric::containers;

	// vector power function
	template<typename Scalar>
	vector<Scalar> sqrt(const vector<Scalar>& y) {
		using namespace sw::universal;
		vector<Scalar> x(y.size());
		for (size_t i = 0; i < y.size(); ++i) {
			x[i] = sqrt(y[i]);
		}
		return x;
	}

} }  // namespace sw::blas
