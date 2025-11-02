#pragma once
// hyperbolic.hpp: templated hyperbolic function stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

    // hyperbolic sine of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar sinh(Scalar v) {
		return std::sinh(double(v));
	}

	// hyperbolic cosine of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar cosh(Scalar v) {
		return std::cosh(double(v));
	}

	// hyperbolic tangent of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar tanh(Scalar v) {
		return std::tanh(double(v));
	}

	// hyperbolic cotangent of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar atanh(Scalar v) {
		return std::atanh(double(v));
	}

	// hyperbolic cosecant of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar acosh(Scalar v) {
		return std::acosh(double(v));
	}

	// hyperbolic secant of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar asinh(Scalar v) {
		return std::asinh(double(v));
	}

}} // namespace sw::universal
