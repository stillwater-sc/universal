#pragma once
// scaling.hpp: scaling functions for data preprocessing
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/math/math>  // injection of native IEEE-754 math library functions into sw::universal namespace

namespace sw { namespace universal { namespace blas {

// range returns the minimum and maximum value of the vector
template<typename Vector>
std::pair<typename Vector::value_type, typename Vector::value_type> range(const Vector& v, unsigned incx = 1) {
	using Scalar = typename Vector::value_type;
	if (v.size() == 0) return std::pair(Scalar(0), Scalar(0));
	Scalar e = v[0];
	Scalar running_min{ e }, running_max{ e };
	for (unsigned i = 1; i < v.size(); ++i) {
		e = v[i];
		if (e < running_min) running_min = e;
		if (e > running_max) running_max = e;
	}
	return std::pair(running_min, running_max);
}

// arange returns the absolute minimum and maximum value of the vector
template<typename Vector>
std::pair<typename Vector::value_type, typename Vector::value_type> arange(const Vector& v, unsigned incx = 1) {
	using Scalar = typename Vector::value_type;
	if (v.size() == 0) return std::pair(Scalar(0), Scalar(0));
	Scalar e = abs(v[0]);
	Scalar running_min{ e }, running_max{ e };
	for (unsigned i = 1; i < v.size(); ++i) {
		e = abs(v[i]);
		if (e < running_min) running_min = e;
		if (e > running_max) running_max = e;
	}
	return std::pair(running_min, running_max);
}


}}} // namespace sw::universal::blas
