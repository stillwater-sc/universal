#pragma once
// pow.hpp: templated pow function stubs for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw { namespace universal {

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar pow(Scalar base, Scalar e) {
		return std::pow(base, e);
	}


	// calculate an integer power function base^int
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
	Scalar integer_power(Scalar base, int exponent) {
		if (exponent < 0) {
			base = Scalar(1) / base;
			exponent = -exponent;
		}
		if (exponent == 0) return Scalar(1);
		Scalar power = Scalar(1);
		while (exponent > 1) {
			if (exponent & 0x1) {
				power = base * power;
				base *= base;
				exponent = (exponent - 1) / 2;
			}
			else {
				base *= base;
				exponent /= 2;
			}
		}
		return base * power;
	}

}} // namespace sw::universal
