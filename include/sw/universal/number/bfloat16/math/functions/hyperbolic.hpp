#pragma once
// hyperbolic.hpp: hyperbolic functions for Google brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
inline bfloat16 sinh(bfloat16 x) {
	return bfloat16(std::sinh(float(x)));
}

// hyperbolic cosine of an angle of x radians
inline bfloat16 cosh(bfloat16 x) {
	return bfloat16(std::cosh(float(x)));
}

// hyperbolic tangent of an angle of x radians
inline bfloat16 tanh(bfloat16 x) {
	return bfloat16(std::tanh(float(x)));
}

// hyperbolic cotangent of an angle of x radians
inline bfloat16 atanh(bfloat16 x) {
	return bfloat16(std::atanh(float(x)));
}

// hyperbolic cosecant of an angle of x radians
inline bfloat16 acosh(bfloat16 x) {
	return bfloat16(std::acosh(float(x)));
}

// hyperbolic secant of an angle of x radians
inline bfloat16 asinh(bfloat16 x) {
	return bfloat16(std::asinh(float(x)));
}

}} // namespace sw::universal
