#pragma once
// logarithm.hpp: templated logarithm function stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw { namespace universal {

	// Natural logarithm of x
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar log(Scalar v) {
		return std::log(v);
	}

	// Binary logarithm of x
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar log2(Scalar v) {
		return std::log2(v);
	}

	// Decimal logarithm of x
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar log10(Scalar v) {
		return std::log10(v);
	}
		
	// Natural logarithm of 1+x
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar log1p(Scalar v) {
		return std::log1p(v);
	}

}} // namespace sw::universal
