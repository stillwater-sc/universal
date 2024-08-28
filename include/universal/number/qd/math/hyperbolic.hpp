#pragma once
// hyperbolic.hpp: hyperbolic function support for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// hyperbolic sine of an angle of x radians
	inline qd sinh(qd x) {
		return qd(std::sinh(double(x)));
	}

	// hyperbolic cosine of an angle of x radians
	inline qd cosh(qd x) {
		return qd(std::cosh(double(x)));
	}

	// hyperbolic tangent of an angle of x radians
	inline qd tanh(qd x) {
		return qd(std::tanh(double(x)));
	}

	// hyperbolic cotangent of an angle of x radians
	inline qd atanh(qd x) {
		return qd(std::atanh(double(x)));
	}

	// hyperbolic cosecant of an angle of x radians
	inline qd acosh(qd x) {
		return qd(std::acosh(double(x)));
	}

	// hyperbolic secant of an angle of x radians
	inline qd asinh(qd x) {
		return qd(std::asinh(double(x)));
	}

}} // namespace sw::universal
