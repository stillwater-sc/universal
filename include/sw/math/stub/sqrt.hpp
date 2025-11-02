#pragma once
// sqrt.hpp: templated sqrt function stub for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar sqrt(Scalar v) {
		return std::sqrt(double(v));
	}

}} // namespace sw::universal
