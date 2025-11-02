#pragma once
// trigonometric.hpp: templated trigonometric function stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

	// sine of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar sin(Scalar v) {
		return std::sin(double(v));
	}

	// cosine of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar cos(Scalar v) {
		return std::cos(double(v));
	}

	// tangent of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar tan(Scalar v) {
		return std::tan(double(v));
	}

	// cotangent of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar atan(Scalar v) {
		return std::atan(double(v));
	}

	// Arc tangent with two parameters
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar atan2(Scalar y, Scalar x) {
		return std::atan2(double(y), double(x));
	}

	// cosecant of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar acos(Scalar v) {
		return std::acos(double(v));
	}

	// secant of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar asin(Scalar v) {
		return std::asin(double(v));
	}

	// cotangent an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar cot(Scalar v) {
		std::cout << "cot TBD\n";
		return Scalar(0); // std::cot(double(v));
	}

	// secant of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar sec(Scalar v) {
		std::cout << "sec TBD\n";
		return Scalar(0); // std::sec(double(v));
	}

	// cosecant of an angle of x radians
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar csc(Scalar v) {
		std::cout << "csc TBD\n";
		return Scalar(0); // std::csc(double(v));
	}

}} // namespace sw::universal
