#pragma once
// fractional.hpp: templated fractional function stubs for native floating-point
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw::universal {

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar  fmod(Scalar x, Scalar y) {
		return std::fmod(x, y);
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar  remainder(Scalar x, Scalar y) {
		return std::remainder(x, y);
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar  frac(Scalar x) {
		std::cout << "frac TBD\n";
		return Scalar(0); // std::frac(x);
	}

}  // namespace sw::universal
