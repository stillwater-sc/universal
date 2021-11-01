#pragma once
// hyperbolic.hpp: hyperbolic functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
rational sinh(rational x) {
	return rational(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
rational cosh(rational x) {
	return rational(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
rational tanh(rational x) {
	return rational(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
rational atanh(rational x) {
	return rational(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
rational acosh(rational x) {
	return rational(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
rational asinh(rational x) {
	return rational(std::asinh(double(x)));
}


}  // namespace sw::universal
