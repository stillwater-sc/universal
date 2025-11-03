#pragma once
// abs.hpp: templated abs function stub for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// return absolute value of a native IEEE-754 type
template<typename Scalar,
         typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
Scalar abs(Scalar v) {
	return std::abs(double(v));
}

}} // namespace sw::universal
