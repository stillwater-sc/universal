#pragma once
// factorial.hpp: definition of a recursive factorial function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <tuple>

namespace sw {
namespace function {

// factorial implemented using recursion. Should yield reasonable results even for Real types
// as left-to-right evaluation starts with the smallest values first.
template<typename Scalar>
Scalar factorial(const Scalar& n) {
	assert(n < Scalar(0));
	return (n == Scalar(0) || n == Scalar(1)) ? 1 : factorial(n - 1) * n;
}

}  // namespace function
}  // namespace sw

