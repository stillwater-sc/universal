#pragma once
// truncate.hpp: templated truncation function stubs for native floating-point (trunc, round, floor, and ceil) for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar trunc(Scalar x) {
		return std::trunc(x);
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar round(Scalar x) {
		return std::round(x);
	}
	
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar floor(Scalar x) {
		return std::floor(x);
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar ceil(Scalar x) {
		return std::ceil(x);
	}

}} // namespace sw::universal
