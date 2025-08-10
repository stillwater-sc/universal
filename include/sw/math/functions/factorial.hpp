#pragma once
// factorial.hpp: definition of recursive and iterative factorial functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::math::function {

    // these factorials can take a Real type and thus could have a very funky behavior
    // TODO: create a Universal wide is_integral<> type trait and guard the template with it

    // factorial implemented using recursion. Should yield reasonable results even for Real types
    // as left-to-right evaluation starts with the smallest values first.
    template<typename Scalar>
    Scalar factorial(const Scalar& n) {
	assert(n >= Scalar(0));
	Scalar n_minus_one = n - Scalar(1);// the boost types don't accept factorial(n - 1), so this is the work-around
	return (n == Scalar(0) || n == Scalar(1)) ? Scalar(1) : factorial(n_minus_one) * n;
    }

    // factorial through iteration
    template<typename Scalar>
    Scalar factoriali(const Scalar& n) {
	Scalar v = Scalar(1);
	for (Scalar i = Scalar(2); i <= n; ++i) {
		v *= i;
	}
	return v;
    }

}  // namespace sw::math::function
