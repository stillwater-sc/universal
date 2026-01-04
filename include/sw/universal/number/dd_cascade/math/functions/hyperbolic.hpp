#pragma once
// hyperbolic.hpp: hyperbolic function support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// hyperbolic sine of an angle of x radians
	inline dd_cascade sinh(const dd_cascade& x) {
		return dd_cascade( std::sinh(double(x)));
	}

	// hyperbolic cosine of an angle of x radians
	inline dd_cascade cosh(const dd_cascade& x) {
		return dd_cascade( std::cosh(double(x)));
	}

	// hyperbolic tangent of an angle of x radians
	inline dd_cascade tanh(const dd_cascade& x) {
		return dd_cascade( std::tanh(double(x)));
	}

	// hyperbolic cotangent of an angle of x radians
	inline dd_cascade atanh(const dd_cascade& x) {
		return dd_cascade( std::atanh(double(x)));
	}

	// hyperbolic cosecant of an angle of x radians
	inline dd_cascade acosh(const dd_cascade& x) {
		return dd_cascade( std::acosh(double(x)));
	}

	// hyperbolic secant of an angle of x radians
	inline dd_cascade asinh(const dd_cascade& x) {
		return dd_cascade( std::asinh(double(x)));
	}

}} // namespace sw::universal
