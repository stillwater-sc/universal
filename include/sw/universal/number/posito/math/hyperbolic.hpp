#pragma once
// hyperbolic.hpp: hyperbolic functions for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posito standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> sinh(posito<nbits,es> x) {
	return posito<nbits,es>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> cosh(posito<nbits,es> x) {
	return posito<nbits,es>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> tanh(posito<nbits,es> x) {
	return posito<nbits,es>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> atanh(posito<nbits,es> x) {
	return posito<nbits,es>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> acosh(posito<nbits,es> x) {
	return posito<nbits,es>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<unsigned nbits, unsigned es>
posito<nbits,es> asinh(posito<nbits,es> x) {
	return posito<nbits,es>(std::asinh(double(x)));
}


}} // namespace sw::universal
