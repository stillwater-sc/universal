#pragma once
// hyperbolic.hpp: hyperbolic function support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// hyperbolic sine of an angle of x radians
	dd sinh(dd x) {
		return dd(std::sinh(double(x)));
	}

	// hyperbolic cosine of an angle of x radians
	dd cosh(dd x) {
		return dd(std::cosh(double(x)));
	}

	// hyperbolic tangent of an angle of x radians
	dd tanh(dd x) {
		return dd(std::tanh(double(x)));
	}

	// hyperbolic cotangent of an angle of x radians
	dd atanh(dd x) {
		return dd(std::atanh(double(x)));
	}

	// hyperbolic cosecant of an angle of x radians
	dd acosh(dd x) {
		return dd(std::acosh(double(x)));
	}

	// hyperbolic secant of an angle of x radians
	dd asinh(dd x) {
		return dd(std::asinh(double(x)));
	}

}} // namespace sw::universal
