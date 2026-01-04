#pragma once
// hyperbolic.hpp: hyperbolic function support for triple-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// hyperbolic sine of an angle of x radians
	inline td_cascade sinh(const td_cascade& x) {
		return td_cascade( std::sinh(double(x)));
	}

	// hyperbolic cosine of an angle of x radians
	inline td_cascade cosh(const td_cascade& x) {
		return td_cascade( std::cosh(double(x)));
	}

	// hyperbolic tangent of an angle of x radians
	inline td_cascade tanh(const td_cascade& x) {
		return td_cascade( std::tanh(double(x)));
	}

	// hyperbolic cotangent of an angle of x radians
	inline td_cascade atanh(const td_cascade& x) {
		return td_cascade( std::atanh(double(x)));
	}

	// hyperbolic cosecant of an angle of x radians
	inline td_cascade acosh(const td_cascade& x) {
		return td_cascade( std::acosh(double(x)));
	}

	// hyperbolic secant of an angle of x radians
	inline td_cascade asinh(const td_cascade& x) {
		return td_cascade( std::asinh(double(x)));
	}

}} // namespace sw::universal
