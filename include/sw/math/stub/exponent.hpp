#pragma once
// exponent.hpp: templated exponent function stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Base-e exponential function
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value, Scalar>::type>
Scalar exp(Scalar x) {
	return std::exp(double(x));
}

// Base-2 exponential function
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value, Scalar>::type>
Scalar exp2(Scalar x) {
	return std::exp2(double(x));
}

// Base-10 exponential function
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value, Scalar>::type>
Scalar exp10(Scalar x) {
	return std::pow(10.0, double(x));
}
		
// Base-e exponential function exp(x)-1
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value, Scalar>::type>
Scalar expm1(Scalar x) {
	return std::expm1(double(x));
}

}} // namespace sw::universal
