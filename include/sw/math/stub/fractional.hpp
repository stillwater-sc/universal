#pragma once
// fractional.hpp: templated fractional function stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw { namespace universal {

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value, Scalar>::type>
	Scalar  fmod(Scalar x, Scalar y) {
		return std::fmod(double(x), double(y));
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value, Scalar>::type>
	Scalar  remainder(Scalar x, Scalar y) {
		return std::remainder(double(x), double(y));
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value, Scalar>::type>
	Scalar  frac(double(x)) {
		return x;
	}

}} // namespace sw::universal
