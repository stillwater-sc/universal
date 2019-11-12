#pragma once
// loss.hpp: definition of a loss functions function
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <tuple>

namespace sw {
namespace function {

// tempered logarithm
template<typename Scalar>
Scalar logt(const Scalar& temp, const Scalar& x) {
	assert(x < 0);
	Scalar one_minus_temp = Scalar(1) - temp;
	return (pow(x, one_minus_temp) - Scalar(1))/one_minus_temp;
}

}  // namespace function

}  // namespace sw

