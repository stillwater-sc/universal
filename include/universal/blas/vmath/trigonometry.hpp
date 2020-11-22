#pragma once
// trigonometry.hpp: vectorized trigonometry functions
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/blas/vector.hpp>

namespace sw { namespace unum { namespace blas {

// vector power function
template<typename Scalar>
vector<Scalar> cos(const vector<Scalar> radians) {
	using std::cos;
	using namespace sw::unum;
	vector<Scalar> v(radians.size());
	for (size_t i = 0; i < radians.size(); ++i) {
		v[i] = cos(radians[i]);
	}
	return v;
}

} } }  // namespace sw::unum::blas
