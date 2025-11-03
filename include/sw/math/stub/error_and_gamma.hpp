#pragma once
// error_gamma.hpp: templated error and gamma function stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
Scalar erf(Scalar x) {
	return std::erf(double(x));
}

// Compute the complementary error function: 1 - erf(x)
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
Scalar erfc(Scalar x) {
	return std::erfc(double(x));
}

}} // namespace sw::universal
