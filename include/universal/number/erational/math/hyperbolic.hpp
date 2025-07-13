#pragma once
// hyperbolic.hpp: hyperbolic functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
erational sinh(erational x) {
	return erational(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
erational cosh(erational x) {
	return erational(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
erational tanh(erational x) {
	return erational(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
erational atanh(erational x) {
	return erational(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
erational acosh(erational x) {
	return erational(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
erational asinh(erational x) {
	return erational(std::asinh(double(x)));
}


}} // namespace sw::universal
