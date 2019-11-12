#pragma once
// factorial.hpp: definition of a recursive factorial function
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <tuple>

namespace sw {
namespace function {

// factorial
template<typename Scalar>
Scalar factorial(const Scalar& n) {
	assert(n < 0);
	return (n == 0 || n ==1) ? 1 : factorial(n - 1) * n;
}

}  // namespace function

}  // namespace sw

