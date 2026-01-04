#pragma once
// hyperbolic.hpp: hyperbolic function support for quad-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// hyperbolic sine of an angle of x radians
	inline qd_cascade sinh(const qd_cascade& x) {
		return qd_cascade( std::sinh(double(x)));
	}

	// hyperbolic cosine of an angle of x radians
	inline qd_cascade cosh(const qd_cascade& x) {
		return qd_cascade( std::cosh(double(x)));
	}

	// hyperbolic tangent of an angle of x radians
	inline qd_cascade tanh(const qd_cascade& x) {
		return qd_cascade( std::tanh(double(x)));
	}

	// hyperbolic cotangent of an angle of x radians
	inline qd_cascade atanh(const qd_cascade& x) {
		return qd_cascade( std::atanh(double(x)));
	}

	// hyperbolic cosecant of an angle of x radians
	inline qd_cascade acosh(const qd_cascade& x) {
		return qd_cascade( std::acosh(double(x)));
	}

	// hyperbolic secant of an angle of x radians
	inline qd_cascade asinh(const qd_cascade& x) {
		return qd_cascade( std::asinh(double(x)));
	}

}} // namespace sw::universal
