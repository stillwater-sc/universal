#pragma once
// trigonometry.hpp: vectorized trigonometry functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <numeric/containers.hpp>

namespace sw { namespace universal { namespace blas {

// vector sine function
template<typename Scalar>
vector<Scalar> sin(const vector<Scalar>& radians) {
	using std::sin;
	using namespace sw::universal;
	vector<Scalar> v(radians.size());
	for (size_t i = 0; i < radians.size(); ++i) {
		v[i] = sin(radians[i]);
	}
	return v;
}

// vector cosine function
template<typename Scalar>
vector<Scalar> cos(const vector<Scalar>& radians) {
	using std::cos;
	using namespace sw::universal;
	vector<Scalar> v(radians.size());
	for (size_t i = 0; i < radians.size(); ++i) {
		v[i] = cos(radians[i]);
	}
	return v;
}
// vector tangent function
template<typename Scalar>
vector<Scalar> tan(const vector<Scalar>& radians) {
	using std::tan;
	using namespace sw::universal;
	vector<Scalar> v(radians.size());
	for (size_t i = 0; i < radians.size(); ++i) {
		v[i] = tan(radians[i]);
	}
	return v;
}

} } }  // namespace sw::universal::blas
